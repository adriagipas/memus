/*
 * Copyright 2016-2025 Adrià Giménez Pastor.
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
 *  frontend.h - Frontend del simulador.
 *
 */

#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <glib.h>

#include "conf.h"
#include "menu.h"
#include "NES.h"

void
close_frontend (void);

menu_response_t
frontend_run (
              const NES_Rom     *rom,
              const char        *rom_id,
              const char        *sram_fn,
              const char        *state_prefix,
              const menu_mode_t  menu_mode,
              const gchar       *rom_fn, // Pot ser NULL. Per a la suspensió.
              const int          verbose
              );

menu_response_t
frontend_resume (
                 const menu_mode_t menu_mode,
                 const int         verbose
                 );

void
init_frontend (
               conf_t         *conf,
               const char     *title,
               const gboolean  big_screen
               );

#endif // __FRONTEND_H__
