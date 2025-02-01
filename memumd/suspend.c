/*
 * Copyright 2022-2025 Adrià Giménez Pastor.
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
 *  suspend.c - Implementació de 'suspend.h'.
 *
 */


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"
#include "error.h"
#include "suspend.h"




/*********/
/* ESTAT */
/*********/

static gchar *_state_fn;
static gchar *_rom_fn;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_suspend (void)
{

  g_free ( _state_fn );
  g_free ( _rom_fn );
  
} // end close_suspend


void
init_suspend (void)
{

  _state_fn= g_build_filename ( get_sharedir (), "suspend.st", NULL );
  _rom_fn= g_build_filename ( get_sharedir (), "suspend.rom", NULL );
  
} // end init_suspend


FILE *
get_suspend_state_file (
                        const bool     write,
                        const gboolean verbose
                        )
{

  FILE *f;

  
  f= fopen ( _state_fn, write ? "wb" : "rb" );
  if ( f != NULL && verbose )
    fprintf ( stderr, write ?
              "Escrivint l'estat en '%s'\n" :
              "Llegint l'estat de '%s'\n",
              _state_fn );
  
  return f;
  
} // end get_suspend_state_file


bool
suspend_write_rom_file_name (
                             const gchar    *rom_fn,
                             const gboolean  verbose
                             )
{

  bool ret;
  GError *err;
  int len;


  len= strlen ( rom_fn );
  err= NULL;
  if ( !g_file_set_contents ( _rom_fn, rom_fn, len + 1, &err ) )
    {
      if ( len > 0 )
        warning ( "no s'ha pogut desar el nom de la rom actual en '%s': %s",
                  _rom_fn, err->message );
      else
        warning ( "no s'ha pogut netejar el nom de la rom actual en '%s': %s",
                  _rom_fn, err->message );
      g_error_free ( err );
      ret= false;
    }
  else
    {
      if ( verbose )
        {
          if ( len > 0 )
            fprintf ( stderr,
                      "S'ha escrit el nom de la rom actual ('%s') en '%s'\n",
                      rom_fn, _rom_fn );
          else
            fprintf ( stderr,
                      "S'ha netejat el nom de la rom actual en '%s'\n",
                      _rom_fn );
        }
      ret= true;
    }

  return ret;
  
} // end suspend_write_rom_file_name


gchar *
suspend_read_rom_file_name (
                            const gboolean verbose
                            )
{

  gchar *ret;
  gsize length;
  gboolean ok;


  ret= NULL;
  length= 0;
  ok= g_file_get_contents ( _rom_fn, &ret, &length, NULL );
  if ( ok && length > 0 && ret[0]!='\0' )
    {
      if ( verbose )
        fprintf ( stderr, "s'ha llegit el nom de la rom suspesa de '%s'\n",
                  _rom_fn );
      ret[length-1]= '\0';
    }
  else
    {
      if ( ret != NULL ) g_free ( ret );
      ret= NULL;
    }
  
  return ret;
  
} // end suspend_read_rom_file_name
