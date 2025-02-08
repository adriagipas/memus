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
 *  menu.h - Menús.
 *
 */

#ifndef __MENU_H__
#define __MENU_H__

#include <stdbool.h>

#include "conf.h"

typedef enum {
  MENU_QUIT,
  MENU_RESUME,
  MENU_RESET,
  MENU_REINIT
} menu_response_t;

void
close_menu (void);

// Torna 0 si s'ha tancat el menú o !=0 si s'ha seleccionat eixir.
void
init_menu (
           conf_t     *conf,
           const bool  verbose
           );

menu_response_t
menu_run (void);

#endif // __MENU_H__
