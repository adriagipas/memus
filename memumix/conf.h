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

#include "MIX.h"

// Grandària de la pantalla.
enum
  {
   SCREEN_SIZE_WIN_X1,
   SCREEN_SIZE_WIN_X1_5,
   SCREEN_SIZE_WIN_X2,
   SCREEN_SIZE_FULLSCREEN,
   SCREEN_SIZE_SENTINEL
  };

// Configuració.
typedef struct
{
  
  int             screen_size;
  gboolean        vsync;
  
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

#endif // __CONF_H__
