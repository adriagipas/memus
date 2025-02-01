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
 *  cd.c - Implementació de 'cd.h'.
 *
 */


#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "cd.h"
#include "error.h"
#include "fchooser.h"

#include "CD.h"
#include "PSX.h"





/*********/
/* ESTAT */
/*********/

// Verbositat.
static bool _verbose;

// Disc.
static CD_Disc *_disc;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_cd (void)
{

  if ( _disc != NULL )
    CD_disc_free ( _disc );
  
} // end close_cd

void
init_cd (
         const int verbose
         )
{

  _verbose= verbose;
  _disc= NULL;
  
} // end init_cd


bool
cd_set_disc (
             bool *quit
             )
{
  
  bool ret,stop;
  const char *fn;
  
  
  init_fchooser ( "cd", "INSERTA CDROM", "[.](cue)$", true, _verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else if ( fn[0] == '\0' ) // Lleva el CD
        {
          if ( _verbose ) fprintf ( stderr, "Llevant el CD\n" );
          if ( _disc != NULL ) CD_disc_free ( _disc );
          _disc= NULL;
          PSX_set_disc ( _disc );
          stop= true;
        }
      else
        {
          if ( !cd_set_disc_from_file_name ( fn ) )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
          else stop= true;
        }
    }
  close_fchooser ( "cd" );
  
  return ret;
  
} // end cd_set_disc


bool
cd_set_disc_from_file_name (
                            const gchar *file_name
                            )
{

  char *err;
  CD_Disc *cd_new;
  bool ret;
  
  
  if ( _verbose ) fprintf ( stderr, "Carregant el CD de '%s'\n", file_name );
  cd_new= CD_disc_new ( file_name, &err );
  if ( cd_new == NULL )
    {
      warning ( "no s'ha pogut llegir correctament '%s': %s",
                file_name, err );
      free ( err );
      ret= false;
    }
  else // Inserta nou CD.
    {
      if ( _disc != NULL ) CD_disc_free ( _disc );
      _disc= cd_new;
      PSX_set_disc ( _disc );
      ret= true;
    }
  
  return ret;
  
} // end cd_set_disc_from_file_name


bool
cd_is_empty (void)
{
  return _disc==NULL;
} // end cd_is_empty
