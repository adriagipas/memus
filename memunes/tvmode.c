/*
 * Copyright 2016-2022 Adrià Giménez Pastor.
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
 *  tvmode.c - Implementació de 'tvmode.h'.
 *
 */


#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"
#include "error.h"
#include "tvmode.h"




/*********/
/* ESTAT */
/*********/

static gchar *_tvmode_fn;
static NES_TVMode _val;
static int _verbose;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_conf_file_name (
                    const char *rom_id
                    )
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_confdir (), rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, "-tvmode.cfg" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_conf_file_name */


static void
try_load_tvmode (
        	 const char  *name,
        	 NES_TVMode  *val,
        	 const int    verbose
        	 )
{

  FILE *f;
  
  
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) )
    {
      if ( verbose )
        fprintf ( stderr,
        	  "Llegint el tipus de consola (NTSC o PAL) de '%s'\n", name );
      f= fopen ( name, "rb" );
      if ( f == NULL )
        error ( "no s'ha pogut obrir '%s': %s", name, strerror ( errno )  );
      if ( fread ( val, sizeof(*val), 1, f ) != 1 )
        error ( "no s'ha pogut llegir el tipus de consola de '%s': %s",
                name, strerror ( errno ) );
      fclose ( f );
    }
  
} /* end try_load_tvmode */


static void
save_tvmode (
             const char       *name,
             const NES_TVMode  val,
             const int         verbose
             )
{

  FILE *f;

  
  if ( verbose )
    fprintf ( stderr,
              "Escrivint el tipus de consola (NTSC o PAL) en '%s'\n", name );
  
  f= fopen ( name, "wb" );
  if ( f == NULL )
    error ( "no s'ha pogut crear el fitxer '%s': %s",
            name, strerror ( errno ) );
  if ( fwrite ( &val, sizeof(val), 1, f ) != 1 )
    error ( "no s'ha pogut escriure el tipus de consola: %s",
            strerror ( errno ) );
  fclose ( f );
  
} /* end save_tvmode */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
init_tvmode (
             const char    *rom_id,
             const NES_Rom *rom,
             const int      verbose
             )
{

  _verbose= verbose;
  _tvmode_fn= get_conf_file_name ( rom_id );
  _val= rom->tvmode;
  try_load_tvmode ( _tvmode_fn, &_val, verbose );
  
  if ( verbose )
    fprintf ( stderr, "Tipus de consola: %s\n",
              _val==NES_PAL ? "PAL" : "NTSC" );
  
} /* end init_tvmode */


void
close_tvmode (void)
{

  save_tvmode ( _tvmode_fn, _val, _verbose );
  free ( _tvmode_fn );
  
} /* end close_tvmode */


NES_TVMode
tvmode_get_val (void)
{
  return _val;
} /* end tvmode_get_val */


void
tvmode_set_val (
        	const NES_TVMode val
        	)
{
  _val= val;
} /* end tvmode_set_val */
