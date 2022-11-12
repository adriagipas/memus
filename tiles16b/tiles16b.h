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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  tiles16b.h - Tiles de 16 bits, amb 3 colors.
 *
 *  NOTES:
 *    Ç : c; À : a; È : e; É : r; Í : i; Ò : o; Ó : p; Ú : u;
 *
 */

#ifndef __TILES16B_H__
#define __TILES16B_H__

#define T16_FG1_TRANS 0x1
#define T16_FG2_TRANS 0x2
#define T16_BG_TRANS  0x4
#define T16_XY_IN_PIXELS 0x8

void
tiles16b_draw_string (
        	      int        *fb,
        	      const int   fb_width,
        	      const char *string,
        	      int         x,
        	      const int   y,
        	      const int   fgcolor1,
        	      const int   fgcolor2,
        	      const int   bgcolor,
        	      const int   flags
        	      );

void
tiles16b_draw_string_s (
        		short       *fb,
        		const int    fb_width,
        		const char  *string,
        		int          x,
        		const int    y,
        		const short  fgcolor1,
        		const short  fgcolor2,
        		const short  bgcolor,
        		const int    flags
        		);

void
init_tiles16b (void);

#endif /* __TILES16B_H__ */
