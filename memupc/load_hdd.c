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
 *  load_hdd.c - Implementació de 'load_hdd.h'.
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
#include "load_hdd.h"

#include "PC.h"




/*********/
/* ESTAT */
/*********/

static PC_File *_hdd= NULL;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static PC_File *
load_hdd_fn (
             const char *fn,
             const int   verbose
             )
{

  PC_File *ret;

  
  if ( verbose )
    fprintf ( stderr, "S'està montant el disc dur en '%s'\n", fn );

  free_hdd ();
  ret= _hdd= PC_file_new_from_file ( fn, false );
  
  return ret;
  
} // end load_hdd_fn




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

PC_File *
load_hdd (
          conf_t    *conf,
          const int  verbose
          )
{

  PC_File *ret;
  const char *fn;
  bool quit,first;
  
  
  init_fchooser ( "hdd", "SELECCIONA DISC DUR",
                  "[.](bin|dat|hdd|img)$", false, verbose );
  quit= false;
  first= true;
  while ( !quit &&
          (conf->hdd_fn == NULL ||
           (ret= load_hdd_fn ( conf->hdd_fn, verbose )) == NULL) )
    {
      
      // Si es perque ha fallat la carrega.
      if ( conf->hdd_fn != NULL )
        {
          if ( ret == NULL )
            warning ( "no s'ha pogut montar correctament '%s'",
                      conf->hdd_fn );
          else
            printf("load_hdd - WTF!!!\n");
          if ( !first && fchooser_error_dialog () == -1 ) quit= true;
        }
      
      // Selecciona nou fitxer. Ignorem eixir del menú, per això el bucle.
      while ( !quit && (fn= fchooser_run ( &quit ))==NULL );
      if ( !quit ) conf_set_hdd_fn ( conf, fn );
      first= false;
      
    }
  close_fchooser ("hdd");
  
  // Ix sense carregar (quan l'usuari demana tancar aplicació)
  if ( quit )
    {
      if ( conf->hdd_fn != NULL )
        {
          conf_set_hdd_fn ( conf, NULL );
          warning ( "s'ha esborrat el disc dur del fitxer de configuració" );
        }
      return NULL;
    }
  else return ret;
  
} // end load_hdd


bool
change_hdd (
            conf_t    *conf,
            bool      *quit,
            const int  verbose
            )
{
  
  PC_File *hdd;
  bool ret,stop;
  const char *fn;
  
  
  assert ( conf->hdd_fn != NULL );
  
  init_fchooser ( "hdd", "SELECCIONA DISC DUR",
                  "[.](bin|dat|hdd|img)$", false, verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else
        {
          hdd= load_hdd_fn ( fn, verbose );
          if ( hdd == NULL )
            warning ( "no s'ha pogut montar correctament '%s'", fn );
          else
            {
              stop= true;
              conf_set_hdd_fn ( conf, fn );
              ret= true;
              frontend_change_hdd ( hdd );
            }
          if ( hdd == NULL )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
        }
    }
  close_fchooser ("hdd");
  
  return ret;

} // end change_hdd


void
free_hdd (void)
{
  
  if ( _hdd != NULL )
    {
      PC_file_free ( _hdd );
      _hdd= NULL;
    }
  
} // end free_hdd


void
remove_hdd (
            conf_t *conf
            )
{

  free_hdd ();
  conf_set_hdd_fn ( conf, NULL );
  
} // end remove_hdd
