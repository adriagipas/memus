/*
 * Copyright 2015-2022 Adrià Giménez Pastor.
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
 *  state.c - Implementació de 'state.h'.
 *
 */


#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"
#include "state.h"




/*********/
/* ESTAT */
/*********/

/* Identificador de ROM i verbositat. */
static const char *_rom_id;
static int _verbose;

/* El prefix on escriure l'estat, si és NULL es gasta el per
   defecte. */
static const char *_prefix_fn;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_state_file_name (
        	     const int num
        	     )
{
  
  GString *buffer;
  gchar *aux;
  
  
  if ( _prefix_fn == NULL )
    {
      aux= g_build_filename ( get_sharedir (), _rom_id, NULL );
      buffer= g_string_new ( aux );
      g_free ( aux );
    }
  else buffer= g_string_new ( _prefix_fn );
  g_string_append_printf ( buffer, ".st%d", num );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_state_file_name */


static FILE *
open_file (
           const int      num,
           const gboolean read
           )
{
  
  FILE *f;
  gchar *fname;
  
  
  if ( num < 1 || num > NUM_STATE_MAX ) return NULL;
  
  fname= get_state_file_name ( num );
  f= fopen ( fname, read ? "rb" : "wb" );
  if ( f != NULL && _verbose )
    fprintf ( stderr, read ?
              "Llegint l'estat de '%s'\n" :
              "Escrivint l'estat en '%s'\n",
              fname );
  g_free ( fname );
  
  return f;
  
} /* end open_file */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
init_state (
            const char *rom_id,
            const char *prefix_fn,
            const int   verbose
            )
{

  _rom_id= rom_id;
  _prefix_fn= prefix_fn;
  _verbose= verbose;
  
} /* end init_state */


FILE *
state_open_read (
        	 const int num
        	 )
{
  return open_file ( num, TRUE );
} /* end state_open_read */


FILE *
state_open_write (
        	  const int num
        	  )
{
  return open_file ( num, FALSE );
} /* end state_open_write */
