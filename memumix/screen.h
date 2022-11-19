/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  screen.h - Pantalla.
 *
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdbool.h>
#include <SDL.h>

#include "conf.h"

enum {
  SCREEN_PAL_WHITE= 0,
  SCREEN_PAL_BLACK,
  SCREEN_PAL_RED,
  SCREEN_PAL_RED_2,
  SCREEN_PAL_RED_3,
  SCREEN_PAL_GRAY_1,
  SCREEN_PAL_GRAY_2,
  SCREEN_PAL_GRAY_3,
  SCREEN_PAL_GRAY_4,
  SCREEN_PAL_GRAY_5,
  SCREEN_PAL_BLUE_LP1,
  SCREEN_PAL_BLUE_LP1B,
  SCREEN_PAL_BLUE_LP2,
  SCREEN_PAL_BLUE_LP2B,
  SCREEN_PAL_GREEN,
  SCREEN_PAL_SIZE
};

#define SCREEN_WIDTH 580
#define SCREEN_HEIGHT 460

void
close_screen (void);

// Es supossa que s'ha inicialitzat SDL amb VIDEO i EVENTS.
void
init_screen (
             const conf_t *conf
             );

// Llig el següent event. Si no n'hi han torna false
// Les coordenades se reescalen a 1.
bool
screen_next_event (
        	   SDL_Event *event
        	   );

void
screen_change_size (
        	    const int screen_size
        	    );

void
screen_update (
               const int *fb
               );

void
screen_show_error (
                   const char *title,
                   const char *message
                   );

#endif // __SCREEN_H__
