/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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




/**********/
/* MACROS */
/**********/

#define NUM_ARGS 0




/*********/
/* TIPUS */
/*********/


struct opts
{
  
  gboolean  verbose;
  gchar    *conf_fn;
  gboolean  enable_vsync;
  gboolean  disable_vsync;
  
};




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
usage (
       int          *argc,
       char        **argv[],
       struct opts  *opts
       )
{
  
  static struct opts vals=
    {
     FALSE,    // verbose 
     NULL,     // conf_fn
     FALSE,    // enable_vsync
     FALSE,    // disable vsync
    };
  
  static GOptionEntry entries[]=
    {
      { "conf", 'c', 0, G_OPTION_ARG_STRING, &vals.conf_fn,
        "Empra com a fitxer de configuració CONF",
        "CONF" },
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
  
  
  // Paresja opcions i obté valors.
  err= NULL;
  context= g_option_context_new ( "executa simulador de MIX" );
  g_option_context_add_main_entries ( context, entries, NULL );
  if ( !g_option_context_parse ( context, argc, argv, &err ) )
    error ( "error al parsejar la línia de comandaments: %s", err->message );
  g_option_context_free ( context );
  *opts= vals;
  
  // Comprova arguments.
  if ( *argc-1 > NUM_ARGS )
    error ( "número d'arguments incorrecte" );
  
} // end usage


static void
free_opts (
           struct opts *opts
           )
{
  
  if ( opts->conf_fn != NULL ) g_free ( opts->conf_fn );
  
} // end free_opts


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
  
} // end set_vsync


static void
run (
     const struct opts *opts
     )
{
  
  conf_t conf;
  
  
  // Inicialitza
  init_dirs ();
  get_conf ( &conf, opts->conf_fn, opts->verbose );
  set_vsync ( opts, &conf );
  init_frontend ( &conf, opts->verbose );
  
  // Executa.
  frontend_run ();
  
  // Allibera memòria. i tanca
  close_frontend ();
  write_conf ( &conf, opts->conf_fn, opts->verbose );
  close_dirs ();
  free_conf ( &conf );
  
} // end run



/**********************/
/* PROGRAMA PRINCIPAL */
/**********************/

int main ( int argc, char *argv[] )
{
  
  struct opts opts;
  
  
  setlocale ( LC_ALL, "" );
  
  // Parseja línea de comandaments.
  usage ( &argc, &argv, &opts );
  
  // Executa.
  run ( &opts );
  
  // Despedida.
  free_opts ( &opts );
  if ( opts.verbose ) fprintf ( stderr, "Adéu!\n" );
  
  return EXIT_SUCCESS;
  
}
