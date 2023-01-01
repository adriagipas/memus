/*
 * Copyright 2013-2023 Adrià Giménez Pastor.
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
#include "mainmenu.h"
#include "rom.h"

#include "MD.h"




/**********/
/* MACROS */
/**********/

#define NUM_ARGS 1

#define ICONV_BSIZE 100




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
  gchar    *sram_fn;
  gchar    *eeprom_fn;
  gchar    *state_prefix;
  gboolean  enable_vsync;
  gboolean  disable_vsync;
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
      NULL,     /* conf_fn */
      NULL,     /* title */
      NULL,     /* sram_fn */
      NULL,     /* eeprom_fn */
      NULL,     /* state_prefix */
      FALSE,    /* enable_vsync */
      FALSE,    /* disable vsync */
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
      { "sram", 's', 0, G_OPTION_ARG_STRING, &vals.sram_fn,
        "Empra com a memòria estàtica SRAM (sols amb ROM)",
        "SRAM" },
      { "eeprom", 'e', 0, G_OPTION_ARG_STRING, &vals.eeprom_fn,
        "Empra com a EEPROM (sols amb ROM)",
        "EEPROM" },
      { "state", 'S', 0, G_OPTION_ARG_STRING, &vals.state_prefix,
        "Empra com a prefixe per als fitxers d'estat (sols amb ROM)",
        "PREFIX" },
      { "title", 't', 0, G_OPTION_ARG_STRING, &vals.title,
        "Fixa el nom de la finestra (sols amb ROM)",
        "TITLE" },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &vals.verbose,
        "Verbose",
        NULL },
      { "enable-vsync", 0, 0, G_OPTION_ARG_NONE, &vals.enable_vsync,
        "Habilita la sincronització vertical. Aquest valor és manté"
        " per a futures execucions", NULL },
      { "disable-vsync", 0, 0, G_OPTION_ARG_NONE, &vals.disable_vsync,
        "Deshabilita la sincronització vertical. Aquest valor és manté"
        " per a futures execucions", NULL },
      { NULL }
    };
  
  GError *err;
  GOptionContext *context;
  
  
  /* Paresja opcions i obté valors. */
  err= NULL;
  context= g_option_context_new ( "<rom> - executa ROMS de Mega Drive" );
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
  if ( opts->sram_fn != NULL ) g_free ( opts->sram_fn );
  if ( opts->eeprom_fn != NULL ) g_free ( opts->eeprom_fn );
  
} /* end free_opts */


/* Torna la mateixa cadena. */
static char *
strip (
       char *str
       )
{
  
  int len;
  char *p;
  
  
  len= strlen ( str );
  if ( len == 0 ) goto quit;
  for ( p= &(str[len-1]); p!=str && isspace ( *p ); --p );
  *(p+1)= '\0';
  if ( p == str ) goto quit;
  for ( p= str; *p && isspace ( *p ); ++p );
  if ( !*p ) goto quit;
  
  return p;
  
 quit:
  str[0]= '-';
  str[1]= '\0';
  return str;
  
} /* end strip */


/* Torna output */
static const char *
strip_and_decode (
        	  char *input
        	  )
{
  
  char *aux, *itmp, *otmp;
  gsize ileft, oleft, ret;
  static char output[ICONV_BSIZE];
  GIConv cd;
  
  
  cd= g_iconv_open ( "UTF-8", "SHIFT_JISX0213" );
  if ( cd == (GIConv) -1 ) 
    error ( "no s'ha pogut crear el conversor de caràcters" );
  aux= strip ( input );
  ileft= strlen ( aux ); oleft= ICONV_BSIZE-1;
  itmp= aux; otmp= &(output[0]);
  ret= g_iconv ( cd, &itmp, &ileft, &otmp, &oleft );
  if ( ret == (gsize) -1 ) cerror ();
  output[ICONV_BSIZE-1-oleft]= '\0';
  g_iconv_close ( cd );
  
  return output;
  
} /* end strip_and_decode */


/* Modifica el header. */
static void
print_header (
              MD_RomHeader *header,
              const MD_Rom *rom
              )
{
  
  printf ( "Consola:                 %s\n", strip ( header->console ) );
  printf ( "Firma i data:            %s\n", strip ( header->firm_build ) );
  printf ( "Nom domèstic:            %s\n",
           strip_and_decode ( header->dom_name ) );
  printf ( "Nom internacional:       %s\n",
           strip_and_decode ( header->int_name ) );
  printf ( "Tipus i número de serie: %s\n", strip ( header->type_snumber ) );
  printf ( "Checksum:                %04x (%s)\n", header->checksum,
           MD_rom_check_checksum ( rom ) ? "Sí" : "No" );
  printf ( "Suport I/O:              %s\n", strip ( header->io ) );
  printf ( "Inici ROM:               %08x\n", header->start );
  printf ( "Fi ROM:                  %08x\n", header->end );
  printf ( "Inici RAM:               %08x\n", header->start_ram );
  printf ( "Fi RAM:                  %08x\n", header->end_ram );
  printf ( "SRAM:                    " );
  if ( header->sram_flags&MD_SRAMINFO_AVAILABLE )
    {
      printf ( "Sí" );
      if ( header->sram_flags&MD_SRAMINFO_ISFORBACKUP )
        printf ( ", sols backup" );
      if ( header->sram_flags&MD_SRAMINFO_ODDBYTES )
        printf ( ", bytes impars" );
      if ( header->sram_flags&MD_SRAMINFO_EVENBYTES )
        printf ( ", bytes pars" );
      printf ( "\n" );
      printf ( "Inici SRAM:              %08x\n", header->start_sram );
      printf ( "Fi SRAM:                 %08x\n", header->end_sram );
    }
  else printf ( "No\n" );
  printf ( "Suport módem:            %s\n", strip ( header->modem ) );
  printf ( "Notes:                   %s\n", strip ( header->notes ) );
  printf ( "Codi països:             %s\n", strip ( header->ccodes ) );
  
} /* end print_header */


static void
set_vsync (
           const struct opts *opts,
           conf_t            *conf
           )
{

  if ( opts->enable_vsync )
    {
      if ( opts->verbose ) printf ( "V-Sync habilitat\n" );
      conf->vsync= TRUE;
    }
  if ( opts->disable_vsync )
    {
      if ( opts->verbose ) printf ( "V-Sync deshabilitat\n" );
      conf->vsync= FALSE;
    }
  
} /* end set_vsync */


static void
run_with_rom (
              const struct args *args,
              const struct opts *opts
              )
{
  
  MD_Rom rom;
  MD_RomHeader header;
  const char *rom_id;
  conf_t conf;
  
  
  /* Carrega la ROM. */
  load_rom ( args->rom_fn, &rom, opts->verbose );
  MD_rom_get_header ( &rom, &header );
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
  set_vsync ( opts, &conf );
  init_frontend ( &conf,
        	  opts->title==NULL ? "memuMD" : opts->title,
        	  opts->big_screen );
  
  /* Executa. */
  frontend_run ( &rom, &header, rom_id, opts->sram_fn, opts->eeprom_fn,
        	 opts->state_prefix, MENU_MODE_INGAME_NOMAINMENU,
        	 opts->verbose );
  
  /* Allibera memòria. i tanca */
  close_frontend ();
  write_conf ( &conf, rom_id, opts->conf_fn, opts->verbose );
  close_dirs ();
 quit:
  MD_rom_free ( &rom );
  
} /* end run_with_rom */


static void
run_without_rom (
                 const struct args *args,
                 const struct opts *opts
                 )
{

  conf_t conf;


  /* Inicialitza. */
  init_dirs ();
  get_default_conf ( &conf, opts->conf_fn, opts->verbose );
  set_vsync ( opts, &conf );
  init_frontend ( &conf, "memuMD", opts->big_screen );
  
  /* Executa. */
  main_menu ( &conf, opts->verbose );
  
  /* Allibera memòria i tanca. */
  close_frontend ();
  write_default_conf ( &conf, opts->conf_fn, opts->verbose );
  close_dirs ();
  
} /* end run_without_rom */




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
