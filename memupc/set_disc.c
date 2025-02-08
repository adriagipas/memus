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
 *  set_disc.c - Implementació de 'set_disc.h'.
 *
 */


#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "error.h"
#include "fchooser.h"
#include "frontend.h"
#include "set_disc.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

bool
set_disc (
          bool       *quit,
          const int   type,
          const bool  verbose
          )
{

  static const char *TITOL[]= {
    "D - INSERIX CDROM",
    "A - INSERIX DISQUET",
    "B - INSERIX DISQUET"
  };
  static const char *ID[]= {
    "cd",
    "floppy_a",
    "floppy_b"
  };
  
  
  bool ret,stop;
  const char *fn;
  
  
  init_fchooser ( ID[type], TITOL[type],
                  "[.](cue|iso|bin|img)$",
                  true, verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else if ( fn[0] == '\0' ) // Lleva el disc
        {
          if ( verbose ) fprintf ( stderr, "Llevant el disc\n" );
          frontend_set_disc ( NULL, type );
          stop= true;
        }
      else
        {
          if ( !frontend_set_disc ( fn, type ) )
            {
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
          else stop= true;
        }
    }
  close_fchooser ( ID[type] );
  
  return ret;
  
} // end set_disc
