/*
 * Copyright 2014-2024 Adrià Giménez Pastor.
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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  main.c - Programa principal.
 *
 */


#include <assert.h>
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
#include "load_bios.h"
#include "mainmenu.h"
#include "rom.h"




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
  gchar    *conf_fn;
  gchar    *title;
  gchar    *set_bios_fn;
  gboolean  unset_bios_fn;
  gchar    *sram_fn;
  gchar    *state_prefix;
  gboolean  big_screen;
  
};




/************/
/* FUNCIONS */
/************/

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
      NULL,     /* conf_fn */
      NULL,     /* title */
      NULL,     /* set_bios_fn */
      FALSE,    /* unset_bios_fn */
      NULL,     /* sram_fn */
      NULL,     /* state_prefix */
      FALSE     /* big_screen */
    };
  
  static GOptionEntry entries[]=
    {
      { "big-screen", 0, 0, G_OPTION_ARG_NONE, &vals.big_screen,
        "Executa en mode pantalla completa i deshabilitat opcions relacionades"
        " en el mode finestra i el teclat", NULL },
      { "bios", 'b', 0, G_OPTION_ARG_STRING, &vals.set_bios_fn,
        "Fixa com a bios BIOS. La bios es manté entre execucions",
        "BIOS" },
      { "conf", 'c', 0, G_OPTION_ARG_STRING, &vals.conf_fn,
        "Empra com a fitxer de configuració CONF. Si es passa una ROM"
        " fa referència al fitxer de configuració de la ROM, sinó fa"
        " referència al fitxer de configuració per defecte",
        "CONF" },
      { "unset-bios", 'u', 0, G_OPTION_ARG_NONE, &vals.unset_bios_fn,
        "Deshabilita permanentment la BIOS",
        NULL },
      { "print-header", 'H', 0, G_OPTION_ARG_NONE, &vals.print_header,
        "Imprimeix la capçalera de la ROM i surt (sols amb ROM)",
        NULL },
      { "print-id", 'I', 0, G_OPTION_ARG_NONE, &vals.print_id,
        "Imprimeix l'identificador de la ROM i surt (sols amb ROM)",
        NULL },
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
  context= g_option_context_new ( "<rom> - executa ROMS de Game Boy Color" );
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
  
  if ( opts->conf_fn != NULL ) g_free ( opts->conf_fn );
  if ( opts->title != NULL ) g_free ( opts->title );
  if ( opts->set_bios_fn != NULL ) g_free ( opts->set_bios_fn );
  if ( opts->sram_fn != NULL ) g_free ( opts->sram_fn );
  
} /* end free_opts */


static const char *
get_header_title (
                  const char title[17],
                  const char manufacturer[5]
                  )
{
  
  static char buffer[17];
  int ltitle, lman;
  
  
  ltitle= strlen ( title );
  lman= strlen ( manufacturer );
  if ( lman == 0 || ltitle <= lman ) return title;
  if ( !strcmp ( &(title[ltitle-lman]), manufacturer ) )
    {
      memcpy ( buffer, title, ltitle-lman );
      buffer[ltitle-lman]= '\0';
      return buffer;
    }
  
  return title;
  
} /* end get_header_title */


static void
print_header (
              const GBC_RomHeader *header,
              const GBC_Rom       *rom
              )
{
  
  printf ( "Títol:           %s\n", get_header_title ( header->title,
                                                       header->manufacturer ) );
  printf ( "Fabricant:       %s\n", (header->manufacturer[0]=='\0' ?
                                     "--" : header->manufacturer) );
  printf ( "Compatibilitat:  " );
  switch ( header->cgb_flag )
    {
    case GBC_ROM_ONLY_GBC: printf ( "GBC\n" ); break;
    case GBC_ROM_GBC: printf ( "GBC/GB\n" ); break;
    default: printf ( "GB\n" );
    }
  printf ( "Llicència:       "  );
  if ( header->old_license != 0x33 ) printf ( "%02x\n", header->old_license );
  else                               printf ( "%s\n", header->new_license );
  printf ( "Suport SGB:      %s\n", header->sgb_flag ? "Sí" : "No" );
  printf ( "Mapper:          %s\n", header->mapper );
  printf ( "Grandària ROM:   %d KB\n", header->rom_size*16 );
  printf ( "Grandària SRAM:  " );
  if ( header->ram_size == -1 ) printf ( "--\n" );
  else                          printf ( "%d KB\n", header->ram_size );
  printf ( "ROM japonesa:    %s\n", header->japanese_rom ? "Sí" : "No" );
  printf ( "Versió:          %u\n", header->version );
  printf ( "Checksum:        %02x (%s)\n", header->checksum,
           GBC_rom_check_checksum ( rom ) ? "Sí" : "No");
  printf ( "Checksum global: %04x (%s)\n", header->global_checksum,
           GBC_rom_check_global_checksum ( rom ) ? "Sí" : "No");
  printf ( "Logo Nintendo:   %s\n",
           GBC_rom_check_nintendo_logo ( rom ) ? "Sí" : "No" );
  
} /* end print_header */


static void
run_with_rom (
              const struct args *args,
              const struct opts *opts
              )
{

  GBC_Rom rom;
  GBC_RomHeader header;
  const GBCu8 *bios;
  const char *rom_id;
  conf_t conf;
  menu_response_t response;
  

  /* Carrega la ROM. */
  if ( load_rom ( args->rom_fn, &rom, opts->verbose ) != 0 )
    error ( "no s'ha pogut carregar la ROM de '%s'", args->rom_fn );
  GBC_rom_get_header ( &rom, &header );
  if ( opts->print_header )
    {
      print_header ( &header, &rom );
      goto quit;
    }
  rom_id= get_rom_id ( &rom, &header, opts->verbose );
  if ( opts->print_id )
    {
      printf ( "%s\n", rom_id );
      goto quit;
    }
  
  /* Inicialitza. */
  init_dirs ();
  get_conf ( &conf, rom_id, opts->conf_fn, NULL, opts->verbose );
  if ( opts->set_bios_fn != NULL )
    conf_set_bios_fn ( &conf, opts->set_bios_fn );
  if ( opts->unset_bios_fn ) conf_set_bios_fn ( &conf, NULL );
  bios= load_bios ( &conf, opts->verbose );
  init_frontend ( &conf,
        	  opts->title==NULL ? "memuGBC" : opts->title,
                  &bios,
        	  opts->big_screen,
                  opts->verbose );
  
  /* Executa. */
  do {
    if ( frontend_run ( &rom, rom_id, opts->sram_fn,
                        opts->state_prefix, MENU_MODE_INGAME_NOMAINMENU,
                        &response ) == -1 )
      error ( "no s'ha pogut executar la ROM '%s'", args->rom_fn );
  } while ( response == MENU_QUIT_MAINMENU );
  
  /* Allibera memòria i tanca. */
  close_frontend ();
  write_conf ( &conf, rom_id, opts->conf_fn, opts->verbose );
  free_conf ( &conf );
  close_dirs ();
 quit:
  GBC_rom_free ( rom );
  
} /* end run_with_rom */


static void
run_without_rom (
                 const struct args *args,
                 const struct opts *opts
                 )
{

  conf_t conf;
  const GBCu8 *bios;
  

  /* Inicialitza. */
  init_dirs ();
  get_default_conf ( &conf, opts->conf_fn, opts->verbose );
  if ( opts->set_bios_fn != NULL )
    conf_set_bios_fn ( &conf, opts->set_bios_fn );
  if ( opts->unset_bios_fn ) conf_set_bios_fn ( &conf, NULL );
  bios= load_bios ( &conf, opts->verbose );
  init_frontend ( &conf, "memuGBC", &bios, opts->big_screen, opts->verbose );
  
  /* Executa. */
  main_menu ( &conf, opts->verbose );
  
  /* Allibera memòria i tanca. */
  close_frontend ();
  write_default_conf ( &conf, opts->conf_fn, opts->verbose );
  close_dirs ();
  
} /* end run_without_rom */




/********/
/* MAIN */
/********/

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
