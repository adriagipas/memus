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
 *  tiles8b.h - Tiles de 8 bits amb 2 colors.
 *
 *  NOTES:
 *    Ç : c; À : a; È : e; É : r; Í : i; Ò : o; Ó : p; Ú : u;
 *
 */

#ifndef __TILES8B_H__
#define __TILES8B_H__

#define FG_TRANS 0x1
#define BG_TRANS 0x2

void
tiles8b_draw_string (
        	     int        *fb,
        	     const int   fb_width,
        	     const char *string,
        	     int         x,
        	     const int   y,
        	     const int   fgcolor,
        	     const int   bgcolor,
        	     const int   flags
        	     );

void
init_tiles8b (void);

#endif /* __TILES8B_H__ */
