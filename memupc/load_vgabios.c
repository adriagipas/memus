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
 *  load_vgabios.c - Implementació de 'load_vgabios.h'.
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
#include "load_vgabios.h"

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
load_vgabios_fn (
                 const char *fn,
                 const int   verbose
                 )
{
  
  FILE *f;
  long size;
  

  if ( verbose )
    fprintf ( stderr, "Carregant la VGABIOS de '%s'\n", fn );
  
  // Obri i calcula grandària.
  f= fopen ( fn, "rb" );
  if ( f == NULL ) goto error_read;
  if ( fseek ( f, 0, SEEK_END ) == -1 ) goto error_read;
  size= ftell ( f );
  if ( size == -1 ) goto error_read;
  rewind ( f );
  
  // Llig.
  free_vgabios ();
  _bios_size= (size_t) size;
  _bios= g_new ( uint8_t, _bios_size );
  if ( fread ( _bios, _bios_size, 1, f ) != 1 ) goto error_read;
  fclose ( f );
  
  return true;
  
 error_read:
  free_vgabios ();
  if ( f != NULL ) fclose ( f );
  return false;
  
} // end load_vgabios_fn




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

bool
load_vgabios (
              conf_t     *conf,
              uint8_t   **bios,
              size_t     *bios_size,
              const int   verbose
              )
{

  const char *fn;
  bool ret,quit,first;
  
  
  init_fchooser ( "vgabios", "SELECCIONA VGA BIOS",
                  "[.](bin)$", false, verbose );
  quit= false;
  first= true;
  while ( !quit &&
          (conf->vgabios_fn == NULL ||
           (ret= load_vgabios_fn ( conf->vgabios_fn, verbose )) == false) )
    {
      
      // Si es perque ha fallat la carrega.
      if ( conf->vgabios_fn != NULL )
        {
          if ( !ret )
            warning ( "no s'ha pogut llegir correctament '%s'",
                      conf->vgabios_fn );
          else
            printf("load_vga_bios - WTF!!!\n");
          if ( !first && fchooser_error_dialog () == -1 ) quit= true;
        }
      
      // Selecciona nou fitxer. Ignorem eixir del menú, per això el bucle.
      while ( !quit && (fn= fchooser_run ( &quit ))==NULL );
      if ( !quit ) conf_set_vgabios_fn ( conf, fn );
      first= false;
      
    }
  close_fchooser ("vgabios");
  
  // Ix sense carregar (quan l'usuari demana tancar aplicació)
  if ( quit )
    {
      if ( conf->vgabios_fn != NULL )
        {
          conf_set_vgabios_fn ( conf, NULL );
          warning ( "s'ha esborrat la VGA BIOS del fitxer de configuració" );
        }
      return false;
    }
  else
    {
      *bios= _bios;
      *bios_size= _bios_size;
      return true;
    }
  
} // end load_vga_bios


bool
change_vgabios (
                conf_t    *conf,
                bool      *quit,
                const int  verbose
                )
{

  bool ok,ret,stop;
  const char *fn;
  
  
  assert ( conf->vgabios_fn != NULL );
  
  init_fchooser ( "vgabios", "SELECCIONA VGA BIOS",
                  "[.](bin)$", false, verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else
        {
          ok= load_vgabios_fn ( fn, verbose );
          if ( !ok )
            warning ( "no s'ha pogut llegir correctament '%s'", fn );
          else
            {
              stop= true;
              conf_set_vgabios_fn ( conf, fn );
              ret= true;
              frontend_change_vgabios ( _bios, _bios_size );
            }
          if ( !ok )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
        }
    }
  close_fchooser ("vgabios");
  
  return ret;

} // end change_vgabios


void
free_vgabios (void)
{
  
  if ( _bios != NULL )
    {
      g_free ( _bios );
      _bios= NULL;
    }
  
} // end free_vgabios


void
remove_vgabios (
                conf_t *conf
                )
{

  free_vgabios ();
  conf_set_vgabios_fn ( conf, NULL );
  
} // end remove_vgabios
