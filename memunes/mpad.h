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
 *  mpad.h - Wrapper sobre pad emprat en els menús.
 *
 */

#ifndef __MPAD_H__
#define __MPAD_H__

/* Botons. */
#define K_UP 0x1
#define K_DOWN 0x2
#define K_A 0x4
#define K_B 0x8
#define K_SELECT 0x10
#define K_START 0x20
#define K_ESCAPE 0x40
#define K_QUIT 0x80
#define K_BUTTON (K_A|K_B|K_SELECT|K_START)

void
mpad_clear (void);

int
mpad_check_buttons (void);

#endif /* __MPAD_H__ */
