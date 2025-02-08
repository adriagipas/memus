/*
 * Copyright 2020-2025 Adrià Giménez Pastor.
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

#include "PC.h"
#include "windowtex.h"

typedef struct
{
  const draw_area_t *area;
  int w,h;
} mouse_area_t;

void
close_screen (void);

// Es supossa que s'ha inicialitzat SDL amb VIDEO i EVENTS.
void
init_screen (
             const conf_t *conf,
             const char   *title
             );

// Llig el següent event. Si no n'hi han torna false
bool
screen_next_event (
        	   SDL_Event          *event,
                   const mouse_area_t *mouse_area // Pot ser NULL
        	   );

void
screen_change_size (
        	    const int screen_size
        	    );

void
screen_change_title (
        	     const char *title
        	     );

// IMPORTANT!! Cal refer les textures.
void
screen_change_vsync (
                     const bool vsync
                     );

void
screen_update (
               void         *udata,
               const PC_RGB *fb,
               const int     width,
               const int     height,
               const int     line_stride
               );

void
screen_show_error (
                   const char *title,
                   const char *message
                   );

// Executa l'ordre de dibuixar el contingut.
void
screen_draw_body (void);

void
screen_enable_cursor (
                      const bool enable
                      );

void
screen_grab_cursor (
                    const bool grab
                    );

#endif // __SCREEN_H__
