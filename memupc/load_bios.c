/*
 * Copyright 2020-2025 Adrià Giménez Pastor.
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
 *  load_bios.c - Implementació de 'load_bios.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fchooser.h"
#include "frontend.h"
#include "load_bios.h"

#include "PC.h"




/*********/
/* ESTAT */
/*********/

static uint8_t *_bios= NULL;
static size_t _bios_size= 0;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
load_bios_fn (
              const char *fn,
              const int   verbose
              )
{
  
  FILE *f;
  long size;
  

  if ( verbose )
    fprintf ( stderr, "Carregant la BIOS de '%s'\n", fn );

  // Obri i calcula grandària.
  f= fopen ( fn, "rb" );
  if ( f == NULL ) goto error_read;
  if ( fseek ( f, 0, SEEK_END ) == -1 ) goto error_read;
  size= ftell ( f );
  if ( size == -1 ) goto error_read;
  rewind ( f );
  
  // Llig.
  free_bios ();
  _bios_size= (size_t) size;
  _bios= g_new ( uint8_t, _bios_size );
  if ( fread ( _bios, _bios_size, 1, f ) != 1 ) goto error_read;
  fclose ( f );
  
  return true;
  
 error_read:
  free_bios ();
  if ( f != NULL ) fclose ( f );
  return false;
  
} // end load_bios_fn




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

bool
load_bios (
           conf_t     *conf,
           uint8_t   **bios,
           size_t     *bios_size,
           const int   verbose
           )
{

  const char *fn;
  bool ret,quit,first;
  
  
  init_fchooser ( "bios", "SELECCIONA BIOS", "[.](bin)$", false, verbose );
  quit= false;
  first= true;
  while ( !quit &&
          (conf->bios_fn == NULL ||
           (ret= load_bios_fn ( conf->bios_fn, verbose )) == false) )
    {
      
      // Si es perque ha fallat la carrega.
      if ( conf->bios_fn != NULL )
        {
          if ( !ret )
            warning ( "no s'ha pogut llegir correctament '%s'",
                      conf->bios_fn );
          else
            printf("load_bios - WTF!!!\n");
          if ( !first && fchooser_error_dialog () == -1 ) quit= true;
        }

      // Selecciona nou fitxer. Ignorem eixir del menú, per això el bucle.
      while ( !quit && (fn= fchooser_run ( &quit ))==NULL );
      if ( !quit ) conf_set_bios_fn ( conf, fn );
      first= false;
      
    }
  close_fchooser ("bios");
  
  // Ix sense carregar (quan l'usuari demana tancar aplicació)
  if ( quit )
    {
      if ( conf->bios_fn != NULL )
        {
          conf_set_bios_fn ( conf, NULL );
          warning ( "s'ha esborrat la BIOS del fitxer de configuració" );
        }
      return false;
    }
  else
    {
      *bios= _bios;
      *bios_size= _bios_size;
      return true;
    }
  
} // end load_bios


bool
change_bios (
             conf_t    *conf,
             bool      *quit,
             const int  verbose
             )
{

  bool ok,ret,stop;
  const char *fn;
  
  
  assert ( conf->bios_fn != NULL );
  
  init_fchooser ( "bios", "SELECCIONA BIOS", "[.](bin)$", false, verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else
        {
          ok= load_bios_fn ( fn, verbose );
          if ( !ok )
            warning ( "no s'ha pogut llegir correctament '%s'", fn );
          else
            {
              stop= true;
              conf_set_bios_fn ( conf, fn );
              ret= true;
              frontend_change_bios ( _bios, _bios_size );
            }
          if ( !ok )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
        }
    }
  close_fchooser ("bios");
  
  return ret;
  
} // end change_bios


void
free_bios (void)
{

  if ( _bios != NULL )
    {
      g_free ( _bios );
      _bios= NULL;
    }
  
} // end free_bios


void
remove_bios (
             conf_t *conf
             )
{

  free_bios ();
  conf_set_bios_fn ( conf, NULL );
  
} // end remove_bios
