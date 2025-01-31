/*
 * Copyright 2015-2025 Adrià Giménez Pastor.
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

#include <SDL.h>

/* Botons. */
#define K_UP 0x1
#define K_DOWN 0x2
#define K_A 0x4
#define K_B 0x8
#define K_C 0x10
#define K_X 0x20
#define K_Y 0x40
#define K_Z 0x80
#define K_START 0x100
#define K_ESCAPE 0x200
#define K_QUIT 0x400
#define K_BUTTON (K_A|K_B|K_C|K_X|K_Y|K_Z|K_START)

// S'utilitza per a processar els events del ratolí.
typedef void (*mpad_mouse_callback_t) (SDL_Event *event, void *);

void
mpad_clear (void);

int
mpad_check_buttons (
                    mpad_mouse_callback_t  cb, // Pot ser NULL
                    void                  *udata // Pot ser NULL
                    );

#endif // __MPAD_H__
