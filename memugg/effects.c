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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  effects.c - Implementació de 'effects.h'.
 *
 */
/*
 *  NOTA: El format del color GG és BBBBGGGGRRRR
 *
 */


#include <stddef.h>
#include <stdlib.h>

#include "effects.h"




/**********/
/* MACROS */
/**********/

#define WIDTH 160




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
effect_fade (
             int          *fb,
             const int     x,
             const int     y,
             const int     width,
             const int     height,
             const double  factor
             )
{
  
  int off, row, col, color, r, g, b;
  int *p;
  double f;
  

  f= 1.0-factor;
  off= y*WIDTH + x;
  for ( row= 0; row < height; ++row, off+= WIDTH )
    {
      p= &(fb[off]);
      for ( col= 0; col < width; ++col )
        {
          color= p[col];
          r= (int) ((color&0xF)*f + 0.5);
          g= (int) (((color>>4)&0xF)*f + 0.5);
          b= (int) (((color>>8)&0xF)*f + 0.5);
          p[col]= (b<<8) | (g<<4) | r;
        }
    }
  
} /* end effect_fade */


void
effect_interpolate_color (
        		  int          *fb,
        		  const int     x,
        		  const int     y,
        		  const int     width,
        		  const int     height,
        		  const int     color_b,
        		  const double  factor
        		  )
{
  
  int off, row, col, color, r, g, b;
  int *p;
  double f;
  

  f= 1.0-factor;
  off= y*WIDTH + x;
  for ( row= 0; row < height; ++row, off+= WIDTH )
    {
      p= &(fb[off]);
      for ( col= 0; col < width; ++col )
        {
          color= p[col];
          r= (int) ((color&0xF)*f + (color_b&0xF)*factor + 0.5);
          g= (int) (((color>>4)&0xF)*f + ((color_b>>4)&0xF)*factor + 0.5);
          b= (int) (((color>>8)&0xF)*f + ((color_b>>8)&0xF)*factor + 0.5);
          p[col]= (b<<8) | (g<<4) | r;
        }
    }
  
} /* end effect_interpolate_color */
