/*
 * Copyright 2015-2023 Adrià Giménez Pastor.
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
 *  model.c - Implementació de 'model.h'.
 *
 */


#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"
#include "error.h"
#include "model.h"




/*********/
/* ESTAT */
/*********/

static gchar *_model_fn;
static MDu8 _val;
static int _verbose;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static MDu8
get_init_val (
              const MD_RomHeader *header
              )
{

  int ret;
  gboolean ispal;
  const char *p;
  
  
  ispal= TRUE;
  ret= 0;
  for ( p= header->ccodes; *p; ++p )
    if ( *p == 'J' )
      {
        ispal= FALSE;
        ret= 0;
        break;
      }
    else if ( *p == 'U' )
      {
        ispal= FALSE;
        ret= MD_MODEL_OVERSEAS;
        break;
      }
  if ( ispal ) ret= MD_MODEL_PAL|MD_MODEL_OVERSEAS;
  
  return ret;
  
} /* end get_init_val */


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
  g_string_append ( buffer, "-model.cfg" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_conf_file_name */


static void
try_load_model (
        	const char *name,
        	MDu8       *val,
        	const int   verbose
        	)
{

  FILE *f;


  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) )
    {
      if ( verbose )
        fprintf ( stderr, "Llegint el model de consola de '%s'\n", name );
      f= fopen ( name, "rb" );
      if ( f == NULL )
        error ( "no s'ha pogut obrir '%s': %s", name, strerror ( errno )  );
      if ( fread ( val, sizeof(*val), 1, f ) != 1 )
        error ( "no s'ha pogut llegir el model de consola de '%s': %s",
                name, strerror ( errno ) );
      fclose ( f );
    }
  
} /* end try_load_model */


static void
save_model (
            const char *name,
            const MDu8  val,
            const int   verbose
            )
{

  FILE *f;


  if ( verbose )
    fprintf ( stderr, "Escrivint el model de consola en '%s'\n", name );
  
  f= fopen ( name, "wb" );
  if ( f == NULL )
    error ( "no s'ha pogut crear el fitxer '%s': %s",
            name, strerror ( errno ) );
  if ( fwrite ( &val, sizeof(val), 1, f ) != 1 )
    error ( "no s'ha pogut escriure el model de consola: %s",
            strerror ( errno ) );
  fclose ( f );
  
} /* end save_model */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
init_model (
            const char         *rom_id,
            const MD_RomHeader *header,
            const int           verbose
            )
{

  _verbose= verbose;
  _model_fn= get_conf_file_name ( rom_id );
  _val= get_init_val ( header );
  try_load_model ( _model_fn, &_val, verbose );
  
  if ( verbose )
    fprintf ( stderr, "Sistema: %s %s\n",
              _val&MD_MODEL_PAL ? "PAL" : "NTSC",
              _val&MD_MODEL_OVERSEAS ? "OVERSEAS" : "DOMESTIC" );
  
} /* end init_model */


void
close_model (void)
{

  save_model ( _model_fn, _val, _verbose );
  free ( _model_fn );
  
} /* end close_model */


MDu8
model_get_val (void)
{
  return _val;
} /* end model_get_val */


void
model_set_val (
               const MDu8 val
               )
{
  _val= val;
} /* end model_set_val */
