/*
 * Copyright 2022 Adrià Giménez Pastor.
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
 *  cursor.c - implementació de 'cursor.h'.
 *
 */


#include <assert.h>
#include <stddef.h>
#include <stdlib.h>




/**********/
/* MACROS */
/**********/
#define WIDTH 16
#define HEIGHT 16




/*************/
/* CONSTANTS */
/*************/

// -1 - fons, 0 - negre, 1- blanc

static const int CURSOR[WIDTH][HEIGHT]= {
  { 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},

  { 1, 0, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 0, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 0, 0, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 0, 0, 0, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1},
  
  { 1, 0, 0, 0, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  { 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}  
};




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
cursor_draw (
             int       *fb,
             const int  fb_width,
             const int  fb_height,
             const int  x,
             const int  y,
             const int  color_black,
             const int  color_white
             )
{

  int width,height,tmp,r,c,color;
  int *pos;
  

  if ( x < 0 || x >= fb_width || y < 0 || y >= fb_height )
    return;
  
  // Calcula width i height
  tmp= x+WIDTH;
  width= tmp>fb_width ? (WIDTH-(tmp-fb_width)) : WIDTH;
  tmp= y+HEIGHT;
  height= tmp>fb_height ? (HEIGHT-(tmp-fb_height)) : HEIGHT;

  // Dibuixa
  pos= &fb[y*fb_width + x];
  for ( r= 0; r < height; ++r )
    {
      for ( c= 0; c < width; ++c )
        {
          color= CURSOR[r][c];
          if ( color != -1 )
            pos[c]= color==0 ? color_black : color_white;
        }
      pos+= fb_width;
    }
  
} // end cursor_draw
