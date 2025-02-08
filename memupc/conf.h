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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  conf.h - Configuració.
 *
 */
/*
 * NOTA! Es poden configurar moltes coses, però de moment vaig a fixar
 * la majoria de valors.
 */

#ifndef __CONF_H__
#define __CONF_H__

#include <glib.h>
#include <SDL.h>

// Grandària de la pantalla.
enum
  {
   SCREEN_SIZE_WIN_640_480,
   SCREEN_SIZE_WIN_800_600,
   SCREEN_SIZE_WIN_960_720,
   SCREEN_SIZE_FULLSCREEN,
   SCREEN_SIZE_SENTINEL
  };

// Configuració.
typedef struct
{
  
  int             screen_size;
  gboolean        vsync;
  gchar          *bios_fn;
  gchar          *vgabios_fn;
  gchar          *hdd_fn;
  
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
conf_set_vgabios_fn (
                     conf_t      *conf,
                     const gchar *vgabios_fn
                     );

void
conf_set_hdd_fn (
                 conf_t      *conf,
                 const gchar *hdd_fn
                 );

#endif // __CONF_H__
