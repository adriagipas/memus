/*
 * Copyright 2025 Adrià Giménez Pastor.
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
#include "lock.h"
#include "session.h"

#include "PC.h"




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
  gchar    *session_name;
  gchar    *conf_fn;
  gchar    *title;
  gchar    *disc_D;
  gchar    *disc_A;
  gchar    *disc_B;
  
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
     NULL,     // session_name
     NULL,     // conf_fn
     NULL,     // title
     NULL,     // disc_D
     NULL,     // disc_A
     NULL      // disc_B
    };
  
  static GOptionEntry entries[]=
    {
      { "cd", 'D', 0, G_OPTION_ARG_STRING, &vals.disc_D,
        "Inicialitza el simulador amb DISC introduït en la unitat de CD-Rom D",
        "DISC" },
      { "floppy-a", 'A', 0, G_OPTION_ARG_STRING, &vals.disc_A,
        "Inicialitza el simulador amb DISC introduït en la unitat de disquet A",
        "DISC" },
      { "floppy-b", 'B', 0, G_OPTION_ARG_STRING, &vals.disc_B,
        "Inicialitza el simulador amb DISC introduït en la unitat de disquet B",
        "DISC" },
      { "conf", 'c', 0, G_OPTION_ARG_STRING, &vals.conf_fn,
        "Empra com a fitxer de configuració CONF",
        "CONF" },
      { "title", 't', 0, G_OPTION_ARG_STRING, &vals.title,
        "Fixa el nom de la finestra (sols amb ROM)",
        "TITLE" },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &vals.verbose,
        "Verbose",
        NULL },
      { "session", G_OPTION_ARG_NONE, 0, G_OPTION_ARG_STRING,
        &vals.session_name,
        "Nom de la sessió. Cada sessió manté un conjunt de fitxers"
        " de configuració i desat separats. Per defecte la sessió estàndard",
        "NAME" },
      { NULL }
    };
  
  GError *err;
  GOptionContext *context;
  
  
  // Paresja opcions i obté valors.
  err= NULL;
  context= g_option_context_new
    ( "executa simulador d'un Pentium PC" );
  g_option_context_set_description
    ( context,
      "Dreceres de teclat:\n"
      "\n"
      "  * Eixir: L.Ctrl + L.Gui + Q\n"
      "  * Menú: L.Ctrl + L.Gui + (RETORN|ESCAPE)\n"
      );
  g_option_context_add_main_entries ( context, entries, NULL );
  if ( !g_option_context_parse ( context, argc, argv, &err ) )
    error ( "error al parsejar la línia de comandaments: %s", err->message );
  g_option_context_free ( context );
  *opts= vals;
  
  // Comprova arguments.
  if ( *argc-1 != NUM_ARGS )
    error ( "número d'arguments incorrecte" );
  
} // end usage


static void
free_opts (
           struct opts *opts
           )
{

  if ( opts->disc_B != NULL ) g_free ( opts->disc_B );
  if ( opts->disc_A != NULL ) g_free ( opts->disc_A );
  if ( opts->disc_D != NULL ) g_free ( opts->disc_D );
  if ( opts->session_name != NULL ) g_free ( opts->session_name );
  if ( opts->conf_fn != NULL ) g_free ( opts->conf_fn );
  
} // end free_opts


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
          g_string_printf ( buf, "memuPC (%s)", sname );
          ret= g_string_free_and_steal ( buf );
        }
      else ret= g_strdup ( "memuPC" );
    }
  else ret= g_strdup ( title );
  
  return ret;
  
} // end get_title


static void
run (
     const struct opts *opts
     )
{
  
  conf_t conf;
  gchar *title;
  
  
  // Inicialitza
  init_session ( opts->session_name, opts->verbose );
  if ( init_lock ( opts->verbose ) )
    {
      init_dirs ();
      get_conf ( &conf, opts->conf_fn, opts->verbose );
      title= get_title ( opts->title );
      init_frontend ( &conf, title, opts->verbose );
      g_free ( title );
      
      // Executa.
      frontend_run ( opts->disc_D, opts->disc_A, opts->disc_B );
      
      // Allibera memòria i tanca
      close_frontend ();
      write_conf ( &conf, opts->conf_fn, opts->verbose );
      close_dirs ();
      free_conf ( &conf );
      close_lock ();
    }
  close_session ();
  
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
