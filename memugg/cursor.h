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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  cursor.h - Dibuixa el cursor del ratolí.
 *
 */

#ifndef __CURSOR_H__
#define __CURSOR_H__

void
cursor_draw (
             int       *fb,
             const int  fb_width,
             const int  fb_height,
             const int  x,
             const int  y,
             const int  color_black,
             const int  color_white
             );

#endif // __CURSOR_H__
