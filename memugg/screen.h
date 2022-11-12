/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
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
 *  screen.h - Pantalla.
 *
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <glib.h>
#include <SDL.h>
#include <stdbool.h>

#include "conf.h"

#define WIDTH 160
#define HEIGHT 144

void
close_screen (void);

/* Es supossa que s'ha inicialitzat SDL amb VIDEO i EVENTS. */
void
init_screen (
             const conf_t   *conf,
             const char     *title,
             const gboolean  big_screen
             );

/* Llig el següent event. Si no n'hi han torna FALSE. */
gboolean
screen_next_event (
        	   SDL_Event *event
        	   );

void
screen_change_scaler (
        	      const int scaler
        	      );

void
screen_change_size (
        	    const int screen_size
        	    );

void
screen_change_title (
        	     const char *title
        	     );

void
screen_change_vsync (
                     const bool vsync
                     );

void
screen_update (
               const int  fb[WIDTH*HEIGHT],
               void      *udata
               );

void
screen_show_error (
        	   const char *title,
        	   const char *message
        	   );

const int *
screen_get_last_fb (void);

void
screen_enable_cursor (
                      const bool enable
                      );

#endif // __SCREEN_H__
