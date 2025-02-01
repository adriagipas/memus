/*
 * Copyright 2016-2025 Adrià Giménez Pastor.
 *
 * This file is part of adriagipas/memus.
 *
 * adriagipas/memus is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * adriagipas/memus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  main.c - Programa principal.
 *
 */


#include <assert.h>
#include <ctype.h>
#include <glib.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "dirs.h"
#include "error.h"
#include "frontend.h"
#include "lock.h"
#include "mainmenu.h"
#include "rom.h"
#include "session.h"

#include "NES.h"




/**********/
/* MACROS */
/**********/

#define NUM_ARGS 1




/*********/
/* TIPUS */
/*********/

struct args
{
  
  const gchar *rom_fn;
  
};


struct opts
{
  
  gboolean  verbose;
  gboolean  print_header;
  gboolean  print_id;
  gchar    *session_name;
  gchar    *conf_fn;
  gchar    *title;
  gchar    *sram_fn;
  gchar    *state_prefix;
  gboolean  big_screen;
  
};




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
usage (
       int          *argc,
       char        **argv[],
       struct args  *args,
       struct opts  *opts
       )
{
  
  static struct opts vals=
    {
      FALSE,    /* verbose */
      FALSE,    /* print_header */
      FALSE,    /* print_id */
      NULL,     // session_name
      NULL,     /* conf_fn */
      NULL,     /* title */
      NULL,     /* sram_fn */
      NULL,     /* state_prefix */
      FALSE     /* big_screen */
    };
  
  static GOptionEntry entries[]=
    {
      { "big-screen", 0, 0, G_OPTION_ARG_NONE, &vals.big_screen,
        "Executa en mode pantalla completa i deshabilitat opcions relacionades"
        " en el mode finestra i el teclat", NULL },
      { "conf", 'c', 0, G_OPTION_ARG_STRING, &vals.conf_fn,
        "Empra com a fitxer de configuració CONF Si es passa una ROM"
        " fa referència al fitxer de configuració de la ROM, sinó fa"
        " referència al fitxer de configuració per defecte",
        "CONF" },
      { "print-header", 'H', 0, G_OPTION_ARG_NONE, &vals.print_header,
        "Imprimeix la capçalera de la ROM i surt (sols amb ROM)",
        NULL },
      { "print-id", 'I', 0, G_OPTION_ARG_NONE, &vals.print_id,
        "Imprimeix l'identificador de la ROM i surt (sols amb ROM)",
        NULL },
      { "session", G_OPTION_ARG_NONE, 0, G_OPTION_ARG_STRING,
        &vals.session_name,
        "Nom de la sessió. Cada sessió manté un conjunt de fitxers"
        " de configuració i desat separats. Per defecte la sessió estàndard",
        "NAME" },
      { "sram", 's', 0, G_OPTION_ARG_STRING, &vals.sram_fn,
        "Empra com a memòria estàtica SRAM (sols amb ROM)",
        "SRAM" },
      { "state", 'S', 0, G_OPTION_ARG_STRING, &vals.state_prefix,
        "Empra com a prefixe per als fitxers d'estat (sols amb ROM)",
        "PREFIX" },
      { "title", 't', 0, G_OPTION_ARG_STRING, &vals.title,
        "Fixa el nom de la finestra (sols amb ROM)",
        "TITLE" },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &vals.verbose,
        "Verbose",
        NULL },
      { NULL }
    };
  
  GError *err;
  GOptionContext *context;

  
  /* Paresja opcions i obté valors. */
  err= NULL;
  context= g_option_context_new ( "<rom> - executa ROMS de Nintendo"
        			  " Entertainment System" );
  g_option_context_add_main_entries ( context, entries, NULL );
  if ( !g_option_context_parse ( context, argc, argv, &err ) )
    error ( "error al parsejar la línia de comandaments: %s", err->message );
  g_option_context_free ( context );
  *opts= vals;
  
  /* Comprova arguments. */
  args->rom_fn= NULL;
  if ( *argc-1 > NUM_ARGS )
    error ( "número d'arguments incorrecte" );
  else if ( *argc-1 == 1 )
    args->rom_fn= (*argv)[1];
  
} /* end usage */


static void
free_opts (
           struct opts *opts
           )
{

  if ( opts->session_name != NULL ) g_free ( opts->session_name );
  if ( opts->conf_fn != NULL ) g_free ( opts->conf_fn );
  if ( opts->title != NULL ) g_free ( opts->title );
  if ( opts->sram_fn != NULL ) g_free ( opts->sram_fn );
  
} // end free_opts


static void
print_header (
              const NES_Rom *rom
              )
{

  printf ( "Nº pàgines PRG:    %d\n", rom->nprg );
  printf ( "Nº pàgines CHR:    %d\n", rom->nchr );
  printf ( "Mapper:            %s\n", NES_mapper_name ( rom->mapper ) );
  printf ( "Model TV:          %s\n", rom->tvmode==NES_PAL ? "PAL" : "NTSC" );
  printf ( "Mirroring inicial: ");
  switch ( rom->mirroring )
    {
    case NES_SINGLE: printf ( "Pantalla única" ); break;
    case NES_HORIZONTAL: printf ( "Horitzontal" ); break;
    case NES_VERTICAL: printf ( "Vertical" ); break;
    case NES_FOURSCREEN: printf ( "Quatre pantalles" ); break;
    }
  fputc ( '\n', stdout );
  printf ( "SRAM:              %s\n", rom->sram ? "Sí" : "No" );
  printf ( "Trainer:           %s\n", rom->trainer!=NULL ? "Sí" : "No" );
  
} /* end print_header */


static gchar *
get_title (
           const gchar *title
           )
{
  
  gchar *ret;
  const char *sname;
  GString *buf;

  
  if ( title == NULL )
    {
      sname= session_get_name ();
      if ( sname != NULL )
        {
          buf= g_string_new ( NULL );
          g_string_printf ( buf, "memuNES (%s)", sname );
          ret= g_string_free_and_steal ( buf );
        }
      else ret= g_strdup ( "memuNES" );
    }
  else ret= g_strdup ( title );
  
  return ret;
  
} // end get_title


static void
run_with_rom (
              const struct args *args,
              const struct opts *opts
              )
{
  
  NES_Rom rom;
  const char *rom_id;
  conf_t conf;
  gchar *title;
  
  
  // Carrega la ROM.
  load_rom ( args->rom_fn, &rom, opts->verbose );
  if ( opts->print_header )
    {
      print_header ( &rom );
      goto quit;
    }
  rom_id= get_rom_id ( &rom, opts->verbose );
  if ( opts->print_id )
    {
      printf ( "%s\n", rom_id );
      goto quit;
    }
  
  // Inicialitza.
  init_session ( opts->session_name, opts->verbose );
  if ( init_lock ( rom_id, opts->verbose ) )
    {
      init_dirs ();
      get_conf ( &conf, rom_id, opts->conf_fn, NULL, opts->verbose );
      title= get_title ( opts->title );
      init_frontend ( &conf, title, opts->big_screen );
      g_free ( title );
  
      // Executa.
      frontend_run ( &rom, rom_id, opts->sram_fn, opts->state_prefix,
                     MENU_MODE_INGAME_NOMAINMENU, NULL, opts->verbose );
  
      // Allibera memòria. i tanca
      close_frontend ();
      write_conf ( &conf, rom_id, opts->conf_fn, opts->verbose );
      close_dirs ();
      close_lock ();
    }
  close_session ();
  
 quit:
  NES_rom_free ( rom );
  
} // end run_with_rom


static void
run_without_rom (
                 const struct args *args,
                 const struct opts *opts
                 )
{
  
  conf_t conf;
  gchar *title;
  
  
  // Inicialitza.
  init_session ( opts->session_name, opts->verbose );
  if ( init_lock ( NULL, opts->verbose ) )
    {
      init_dirs ();
      get_default_conf ( &conf, opts->conf_fn, opts->verbose );
      title= get_title ( NULL );
      init_frontend ( &conf, title, opts->big_screen );
      g_free ( title );
      
      // Executa.
      main_menu ( &conf, opts->verbose );
      
      // Allibera memòria i tanca.
      close_frontend ();
      write_default_conf ( &conf, opts->conf_fn, opts->verbose );
      close_dirs ();
      close_lock ();
    }
  close_session ();
  
} // end run_without_rom




/**********************/
/* PROGRAMA PRINCIPAL */
/**********************/

int main ( int argc, char *argv[] )
{
  
  struct args args;
  struct opts opts;
  
  
  setlocale ( LC_ALL, "" );
  
  /* Parseja línea de comandaments. */
  usage ( &argc, &argv, &args, &opts );
  
  /* Executa. */
  if ( args.rom_fn != NULL ) run_with_rom ( &args, &opts );
  else                       run_without_rom ( &args, &opts );
  
  /* Despedida. */
  free_opts ( &opts );
  if ( opts.verbose ) fprintf ( stderr, "Adéu!\n" );
  
  return EXIT_SUCCESS;
  
}
