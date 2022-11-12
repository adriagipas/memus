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
 *  conf.h - Configuració.
 *
 */

#ifndef __CONF_H__
#define __CONF_H__

#include <glib.h>
#include <SDL.h>

/* Tipus d'escalat. */
enum {
  SCALER_NONE,
  SCALER_SCALE2X,
  SCALER_SENTINEL
};

/* Grandària de la pantalla. */
enum {
  SCREEN_SIZE_WIN_X1,
  SCREEN_SIZE_WIN_X2,
  SCREEN_SIZE_WIN_X3,
  SCREEN_SIZE_WIN_X4,
  SCREEN_SIZE_FULLSCREEN,
  SCREEN_SIZE_SENTINEL
};

/* Tecles. */
typedef struct
{
  
  SDL_Keycode up;
  SDL_Keycode down;
  SDL_Keycode left;
  SDL_Keycode right;
  SDL_Keycode button_A;
  SDL_Keycode button_B;
  SDL_Keycode select;
  SDL_Keycode start;
  
} keys_t;

/* Configuració. */
typedef struct
{
  
  keys_t    keys;
  int       screen_size;
  int       scaler;
  gchar    *bios_fn;
  gboolean  vsync;
  
} conf_t;

void
conf_set_bios_fn (
        	  conf_t      *conf,
        	  const gchar *bios_fn
        	  );

/* Sols allibera interna!!!! */
void
free_conf (
           conf_t *conf
           );

/* Escriu en conf els valors. */
void
get_conf (
          conf_t       *conf,
          const char   *rom_id,
          const char   *conf_fn, /* Pot ser NULL. */
          const conf_t *default_conf,   /* Pot ser NULL. */
          const int     verbose
          );

/* Escriu en conf els valors per defecte del sistema. */
void
get_default_conf (
                  conf_t     *conf,
                  const char *conf_fn, /* Pot ser NULL. */
                  const int   verbose
                  );

void
write_conf (
            const conf_t *conf,
            const char   *rom_id,
            const char   *conf_fn,
            const int     verbose
            );

void
write_default_conf (
                    const conf_t *conf,
                    const char   *conf_fn,
                    const int     verbose
                    );

#endif /* __CONF_H__ */
