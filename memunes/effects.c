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
 *  effects.c - Implementació de 'effects.h'.
 *
 */
/*
 *  NOTA: El format del color NES és aproximadament:
 *   - Hue: 0x00F
 *   - Value: 0x030
 *   - Emphasis: 0x1C0 (~ més alt més oscur)
 *
 */


#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "effects.h"




/**********/
/* MACROS */
/**********/

#define WIDTH 256




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
effect_fade (
             int       *fb,
             const int  x,
             const int  y,
             const int  width,
             const int  height,
             const int  level
             )
{

  /* NOTA: No toque el emphasis. */
  
  int off, row, col, color, value;
  int *p;
  

  assert ( level >= -4 && level <= 4 );

  if ( level == 0 ) return;
  
  off= y*WIDTH + x;
  for ( row= 0; row < height; ++row, off+= WIDTH )
    {
      p= &(fb[off]);
      for ( col= 0; col < width; ++col )
        {
          color= p[col];
          value= (color>>4)&0x3;
          if ( level > 0 ) /* Fa més claret */
            {
              if ( value+level > 3 ) color= (color&0x1C0)|0x30;
              else                   color+= (level<<4);
            }
          else /* Fa més claret. */
            {
              if ( value+level < 0 ) color= (color&0x1C0)|0x0F;
              else                   color-= ((-level)<<4);
            }
          p[col]= color;
        }
    }
  
} /* end effect_fade */
