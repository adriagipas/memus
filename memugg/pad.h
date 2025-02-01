/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
 *  pad.h - Controlador.
 *
 */

#ifndef __PAD_H__
#define __PAD_H__

#include <SDL.h>

#include "conf.h"

enum {
  PAD_JOY_XBOX360,
  PAD_JOY_UNK
};

void
close_pad (void);

/* S'assumeix que el joystiq està activat i els events també. */
void
init_pad (
          const conf_t *conf
          );

void
pad_change_conf (
        	 conf_t *conf
        	 );

int
pad_check_buttons (
        	   void *udata
        	   );

void
pad_clear (void);

/* Torna cert si s'ha apretat el botó Escape o equivalent. */
gboolean
pad_event (
           const SDL_Event *event
           );

#endif /* __PAD_H__ */
