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
 *  scalers2d.h - Algorismes d'escalat 2D.
 *
 */

#ifndef __SCALERS2D_H__
#define __SCALERS2D_H__

#include <limits.h>

#if CHAR_BIT != 8
#error Arquitectura no suportada
#endif

/* Implementació pròpia de l'algorisme Scale2x
   (http://scale2x.sourceforge.net). Requereix que l'amplària i altura
   de la imatge origen siguen almeyns 2. */
void
s2d_scale2x (
             const int *src,
             int       *dst,
             const int  width,
             const int  height
             );

#endif /* __SCALERS2D_H__ */
