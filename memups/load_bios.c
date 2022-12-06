/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fchooser.h"
#include "load_bios.h"

#include "PSX.h"




/*********/
/* TIPUS */
/*********/

typedef enum
  {
   STAT_OK= 0,
   STAT_ERR_READ, // Algun error durant la lectura
   STAT_ERR_SIZE
  } status_t;



/*********/
/* ESTAT */
/*********/

static uint8_t _bios[PSX_BIOS_SIZE];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static status_t
load_bios_fn (
              const char *fn,
              const int   verbose
              )
{

  FILE *f;
  long size;
  

  if ( verbose )
    fprintf ( stderr, "Carregant la BIOS de '%s'\n", fn );
  
  f= fopen ( fn, "rb" );
  if ( f == NULL ) goto error_read;
  if ( fseek ( f, 0, SEEK_END ) == -1 ) goto error_read;
  size= ftell ( f );
  if ( size == -1 ) goto error_read;
  if ( size != PSX_BIOS_SIZE ) goto error_size;
  rewind ( f );
  if ( fread ( _bios, PSX_BIOS_SIZE, 1, f ) != 1 ) goto error_read;
  fclose ( f );
  
  return STAT_OK;

 error_read:
  if ( f != NULL ) fclose ( f );
  return STAT_ERR_READ;
 error_size:
  if ( f != NULL ) fclose ( f );
  return STAT_ERR_SIZE;
  
} // end load_bios_fn




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

const uint8_t *
load_bios (
           conf_t    *conf,
           const int  verbose
           )
{
  
  status_t ret;
  const char *fn;
  bool quit,first;
  
  
  init_fchooser ( "bios", "SELECCIONA BIOS", "[.](bin)$", false, verbose );
  quit= false;
  first= true;
  while ( !quit &&
          (conf->bios_fn == NULL ||
           (ret= load_bios_fn ( conf->bios_fn, verbose )) != STAT_OK) )
    {

      // Si es perque ha fallat la carrega.
      if ( conf->bios_fn != NULL )
        {
          switch ( ret )
            {
            case STAT_ERR_READ:
              warning ( "no s'ha pogut llegir correctament '%s'",
                        conf->bios_fn );
              break;
            case STAT_ERR_SIZE:
              warning ( "'%s' no té la grandària adecuada per ser"
                        " un fitxer de BIOS", conf->bios_fn );
              break;
            default:
              printf("load_bios - WTF!!!\n");
            }
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
      return NULL;
    }
  else return &(_bios[0]);
  
} // end load_bios


bool
change_bios (
             conf_t    *conf,
             bool      *quit,
             const int  verbose
             )
{

  status_t st;
  bool ret,stop;
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
          st= load_bios_fn ( fn, verbose );
          switch ( st )
            {
            case STAT_ERR_READ:
              warning ( "no s'ha pogut llegir correctament '%s'", fn );
              break;
            case STAT_ERR_SIZE:
              warning ( "'%s' no té la grandària adecuada per ser"
                        " un fitxer de BIOS", fn );
              break;
            case STAT_OK:
              stop= true;
              conf_set_bios_fn ( conf, fn );
              ret= true;
              PSX_change_bios ( _bios );
              break;
            default:
              printf("load_bios - WTF!!!\n");
            }
          if ( st!=STAT_OK )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
        }
    }
  close_fchooser ("bios");
  
  return ret;

} // end change_bios
