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
 *  mpad.h - Wrapper sobre pad emprat en els menús. He incorporat
 *           també el ratolí.
 *
 */


#ifndef __MPAD_H__
#define __MPAD_H__

#include <SDL.h>

// Botons.
#define K_UP 0x1
#define K_DOWN 0x2
#define K_TL 0x4
#define K_TR 0x8
#define K_START 0x10
#define K_ESCAPE 0x20
#define K_QUIT 0x40
#define K_BUTTON (K_TL|K_TR|K_START)

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
