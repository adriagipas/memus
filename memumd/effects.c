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
 *  effects.c - Implementació de 'effects.h'.
 *
 */
/*
 *  NOTA: El format del color MD és:
 *   - r: 0x7 (3 bit)
 *   - g: 0x38 (3 bit)
 *   - b: 0x1C0 (3 bit)
 *   - mode: 0x600 (2 bit)
 *   
 *   La idea és no tocar el mode.
 *
 */


#include <stddef.h>
#include <stdlib.h>

#include "effects.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
effect_fade (
             int          *fb,
             const int     fb_width,
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
  off= y*fb_width + x;
  for ( row= 0; row < height; ++row, off+= fb_width )
    {
      p= &(fb[off]);
      for ( col= 0; col < width; ++col )
        {
          color= p[col];
          r= (int) ((color&0x7)*f + 0.5);
          g= (int) (((color>>3)&0x7)*f + 0.5);
          b= (int) (((color>>6)&0x7)*f + 0.5);
          p[col]= (color&0x600) | (b<<6) | (g<<3) | r;
        }
    }
  
} /* end effect_fade */


void
effect_interpolate_color (
        		  int          *fb,
        		  const int     fb_width,
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
  off= y*fb_width + x;
  for ( row= 0; row < height; ++row, off+= fb_width )
    {
      p= &(fb[off]);
      for ( col= 0; col < width; ++col )
        {
          color= p[col];
          r= (int) ((color&0x7)*f + (color_b&0x7)*factor + 0.5);
          g= (int) (((color>>3)&0x7)*f + ((color_b>>3)&0x7)*factor + 0.5);
          b= (int) (((color>>6)&0x7)*f + ((color_b>>6)&0x7)*factor + 0.5);
          p[col]= (color&0x600) | (b<<6) | (g<<3) | r;
        }
    }
  
} /* end effect_interpolate_color */
