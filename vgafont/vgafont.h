/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  vgafont.h - Font VGA.
 *
 *  La font que s'utilitzat és en realitat VGA-ROM.F16 que es troba en
 *  el domini públic.
 *
 */


#ifndef __VGAFONT_H__
#define __VGAFONT_H__

#define VGAFONT_FG_TRANS  0x1
#define VGAFONT_BG_TRANS  0x2
#define VGAFONT_XY_PIXELS 0x4

void
close_vgafont (void);

void
init_vgafont (void);

// Escriu assumint que la codificació de 'string' és CP437.
// Per defecte x/y són tiles 9x16
void
vgafont_draw_string (
                     int        *fb,
                     const int   fb_width,
                     const char *string,
                     int         x,
                     const int   y,
                     const int   fgcolor,
                     const int   bgcolor,
                     const int   flags
                     );

// Escriu assumint que la codificació de 'string' és UTF-8.
// Per defecte x/y són tiles 9x16
// Torna la longitut en caracters de la cadena dibuixada.
int
vgafont_draw_string_utf8 (
                          int        *fb,
                          const int   fb_width,
                          const char *string,
                          int         x,
                          const int   y,
                          const int   fgcolor,
                          const int   bgcolor,
                          const int   flags
                          );

#endif // __VGAFONT_H__
