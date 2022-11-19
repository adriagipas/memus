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
 *  cdir.c - Implementació de 'cdir.h'.
 *
 */

#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dirs.h"
#include "error.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

gchar *
cdir_read (
           const gchar    *id,
           const gboolean  verbose
           )
{

  gchar *fn, *cdir;
  gsize length;
  gboolean ret;
  GString *buffer;
  
  
  buffer= g_string_new ( NULL );
  g_string_printf ( buffer, "cdir_%s", id );
  fn= g_build_filename ( get_sharedir (), buffer->str, NULL );
  g_string_free ( buffer, TRUE );
  cdir= NULL;
  ret= g_file_get_contents ( fn, &cdir, &length, NULL );
  if ( ret )
    {
      if ( verbose )
        fprintf ( stderr, "S'ha llegit el directori actual per"
                  " a '%s' de '%s'\n", id, fn );
      cdir[length-1]= '\0';
    }
  else cdir= NULL;
  g_free ( fn );
  
  return cdir;
  
} // end cdir_read


void
cdir_write (
            const gchar    *id,
            const gchar    *cdir,
            const gboolean  verbose
            )
{

  gchar *fn;
  GError *err;
  GString *buffer;
  
  
  buffer= g_string_new ( NULL );
  g_string_printf ( buffer, "cdir_%s", id );
  fn= g_build_filename ( get_sharedir (), buffer->str, NULL );
  g_string_free ( buffer, TRUE );
  err= NULL;
  if ( !g_file_set_contents ( fn, cdir, strlen ( cdir ) + 1, &err ) )
    {
      warning ( "no s'ha pogut desar el directori actual"
                " per a '%s' en '%s': %s",
                id, fn, err->message );
      g_error_free ( err );
    }
  else if ( verbose )
    fprintf ( stderr, "S'ha escrit el directori actual per a '%s' en '%s'\n",
              id, fn );
  g_free ( fn );
  
} // end cdir_write
