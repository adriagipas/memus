/*
 * Copyright 2011-2022 Adrià Giménez Pastor.
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
 *  scalers2d.c - Implementació de 'scalers2d.h'.
 *
 */


#include "scalers2d.h"




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static const int *
s2d_scale2x_first_line (
        		const int  *src0,
        		const int  *src1,
        		int       **dst,
        		const int   width,
        		const int   desp
        		)
{
  
  int *dst0, *dst1, D, H, F, E, E2, E3, c;
  
  
  /* Prepara. */
  dst0= *dst;
  dst1= dst0+desp;
  
  /* Primer píxel. */
  E= *(src0++); F= *(src0++); H= *(src1++);
  E3= (H==F) ? F : E;
  *(dst0++)= E; *(dst0++)= E;
  *(dst1++)= E; *(dst1++)= E3;
  
  /* Pixels intermitjos. */
  for ( c= 1; c < width-1; ++c )
    {
      D= E; E= F; F= *(src0++); H= *(src1++);
      if ( E != H && D != F )
        {
          E2= (D==H) ? D : E;
          E3= (H==F) ? F : E;
        }
      else { E2= E; E3= E; }
      *(dst0++)= E; *(dst0++)= E;
      *(dst1++)= E2; *(dst1++)= E3;
    }
  
  /* Últim píxel. */
  D= E; E= F; H= *(src1++);
  E2= (D==H) ? D : E;
  *(dst0++)= E; *(dst0++)= E;
  *(dst1++)= E2; *(dst1++)= E;
  
  /* Torna nou valor de 'dst' i 'src1'. */
  *dst= dst1;
  
  return src1;
  
} /* end s2d_scale2x_first_line */


static void
s2d_scale2x_last_line (
        	       const int *src_1,
        	       const int *src,
        	       int       *dst0,
        	       int       *dst1,
        	       const int  width
        	       )
{
  
  int B, D, F, E, E0, E1, c;
  
  
  /* Primer píxel. */
  B= *(src_1++); E= *(src++); F= *(src++);
  E1= (B==F) ? F : E;
  *(dst0++)= E; *(dst0++)= E1;
  *(dst1++)= E; *(dst1++)= E;
  
  /* Píxels intermitjos. */
  for ( c= 1; c < width-1; ++c )
    {
      B= *(src_1++); D= E; E= F; F= *(src++);
      if ( B != E && D != F )
        {
          E0= (D==B) ? D : E;
          E1= (B==F) ? F : E;
        }
      else { E0= E; E1= E; }
      *(dst0++)= E0; *(dst0++)= E1;
      *(dst1++)= E; *(dst1++)= E;
    }
  
  /* Últim píxel. */
  B= *(src_1++); D= E; E= F;
  E0= (D==B) ? D : E;
  *(dst0++)= E0; *(dst0++)= E;
  *(dst1++)= E; *(dst1++)= E;
  
} /* end s2d_scale2x_last_line */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
s2d_scale2x (
             const int *src,
             int       *dst,
             const int  width,
             const int  height
             )
{
  
  int r, c, desp, *dst1, B, D, H, F, E, E0, E1, E2, E3;
  const int *src_1, *src1;
  
  
  desp= width*2;
  src_1= src; src+= width;
  src1= s2d_scale2x_first_line ( src_1, src, &dst, width, desp );
  dst1= dst+desp;
  for ( r= 1; r < height-1; ++r )
    {
      
      /* Primer píxel. */
      B= *(src_1++); E= *(src++); F= *(src++); H= *(src1++);
      if ( B != H && E != F )
        {
          E1= (B==F) ? F : E;
          E3= (H==F) ? F : E;
        }
      else { E1 = E; E3 = E; }
      *(dst++)= E; *(dst++)= E1;
      *(dst1++)= E; *(dst1++)= E3;
      
      /* Píxels intermitjos. */
      for ( c= 1; c < width-1; ++c )
        {
          B= *(src_1++); D= E; E= F; F= *(src++); H= *(src1++);
          if ( B != H && D != F )
            {
              E0= (D==B) ? D : E;
              E1= (B==F) ? F : E;
              E2= (D==H) ? D : E;
              E3= (H==F) ? F : E;
            }
          else { E0= E; E1= E; E2= E; E3= E; }
          *(dst++)= E0; *(dst++)= E1;
          *(dst1++)= E2; *(dst1++)= E3;
        }
      
      /* Últim píxel. */
      B= *(src_1++); D= E; E= F; H= *(src1++);
      if ( B != H && D != E )
        {
          E0= (D==B) ? D : E;
          E2= (D==H) ? D : E;
        }
      else { E0= E; E2= E; }
      *(dst++)= E0; *(dst++)= E;
      *(dst1++)= E2; *(dst1++)= E;
      
      /* Prepara. */
      dst= dst1; dst1+= desp;
      
    }
  s2d_scale2x_last_line ( src_1, src, dst, dst1, width );
  
} /* end s2d_scale2x */
