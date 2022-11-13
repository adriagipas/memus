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
 *  effects.h - Per a manipular el framebuffer.
 *
 */

#ifndef __EFFECTS_H__
#define __EFFECTS_H__

/* Fa oscur o claret un rectangle de fb. */
void
effect_fade (
             int       *fb,
             const int  x,
             const int  y,
             const int  width,
             const int  height,
             const int  level /* oscur -> -4,-3,-2,-1,0,1,2,3,4 -> clar */
             );

#endif /* __EFFECTS_H__ */
