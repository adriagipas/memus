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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  conf.h - Configuració.
 *
 */

#ifndef __CONF_H__
#define __CONF_H__

#include <glib.h>
#include <SDL.h>

#include "PSX.h"

// Grandària de la pantalla.
enum
  {
   SCREEN_SIZE_WIN_X1,
   SCREEN_SIZE_WIN_X1_5,
   SCREEN_SIZE_WIN_X2,
   SCREEN_SIZE_FULLSCREEN,
   SCREEN_SIZE_SENTINEL
  };

// Tecles.
typedef struct
{
  
  SDL_Keycode up;
  SDL_Keycode down;
  SDL_Keycode left;
  SDL_Keycode right;
  SDL_Keycode button_triangle;
  SDL_Keycode button_circle;
  SDL_Keycode button_cross;
  SDL_Keycode button_square;
  SDL_Keycode button_L2;
  SDL_Keycode button_R2;
  SDL_Keycode button_L1;
  SDL_Keycode button_R1;
  SDL_Keycode start;
  SDL_Keycode select;
  
} keys_t;

// Configuració.
typedef struct
{
  
  keys_t          keys[2];
  int             screen_size;
  gboolean        vsync;
  PSX_Controller  controllers[2];
  gchar          *bios_fn;
  gboolean        tvres_is_pal;
  gchar          *memc_fn[2];
  
} conf_t;

// Sols allibera interna!!!
void
free_conf (
           conf_t *conf
           );

// Escriu en conf els valors per defecte del sistema.
void
get_conf (
          conf_t     *conf,
          const char *conf_fn, // Pot ser NULL.
          const int   verbose
          );

void
write_conf (
            const conf_t *conf,
            const char   *conf_fn, // Pot ser NULL
            const int     verbose
            );

void
conf_set_bios_fn (
                  conf_t      *conf,
                  const gchar *bios_fn
                  );

void
conf_set_memc_fn (
                  conf_t      *conf,
                  const int    ind,
                  const gchar *memc_fn
                  );

#endif // __CONF_H__
