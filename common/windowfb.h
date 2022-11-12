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
 *  windowfb.h - Finestra amb un FrameBuffer.
 *
 */

#ifndef __WINDOWFB_H__
#define __WINDOWFB_H__

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

void
close_windowfb (void);

/* wwidth i wheight són les dimensions de la finestra, fbwidth i
 * fbheight són les dimensions del framebuffer, title és el títol, i
 * icon és una icona on icon[0] i icon[1] són l'amplada i altura
 * respectivament. wwidth<=0 || wheight<=0 indica pantalla completa.
 */
void
init_windowfb (
               const int       wwidth,
               const int       wheight,
               const int       fbwidth,
               const int       fbheight,
               const char     *title,
               const int      *icon,
               const gboolean  vsync
               );

gboolean
windowfb_next_event (
        	     SDL_Event *event
        	     );

void
windowfb_set_wsize (
        	    const int width,
        	    const int height
        	    );

void
windowfb_set_fbsize (
        	     const int width,
        	     const int height
        	     );

void
windowfb_set_title (
        	    const char *title
        	    );

void
windowfb_set_vsync (
                    const bool vsync
                    );

/* fb ha de ser de dimensión fbwidth*fbheight. */
void
windowfb_update (
        	 const int      *fb,
        	 const uint32_t *palette
        	 );

void
windowfb_update_no_pal (
        		const uint32_t *fb
        		);

void
windowfb_redraw (void);

void
windowfb_show_error (
        	     const char *title,
        	     const char *message
        	     );

void
windowfb_hide (void);

void
windowfb_show (void);

void
windowfb_raise (void);

// Necessita que windowfb estiga inicialitzada.
uint32_t
windowfb_get_color (
                    const uint8_t r,
                    const uint8_t g,
                    const uint8_t b
                    );


// val == false: S'amaga el cursor i es filtren els events.
// val == true:
//    * Les posicions dels events se corregeixen.
//    * El cursor es mostra/amaga d'acord amb show.
void
windowfb_enable_cursor (
                        const bool val,
                        const bool show
                        );

#endif /* __WINDOWFB_H__ */
