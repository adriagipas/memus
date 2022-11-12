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
 *  menu.h - Menús.
 *
 */

#ifndef __MENU_H__
#define __MENU_H__

#include "conf.h"

typedef enum {
  MENU_MODE_INGAME_MAINMENU= 0,
  MENU_MODE_INGAME_NOMAINMENU= 1,
  MENU_MODE_MAINMENU= 2,
  MENU_MODE_SENTINEL= 3
} menu_mode_t;

typedef enum {
  MENU_QUIT,
  MENU_QUIT_MAINMENU,
  MENU_RESUME
} menu_response_t;

void
init_menu (
           conf_t         *conf,
           const gboolean  big_screen
           );

void
menu_change_conf (
                  conf_t *conf
                  );

menu_response_t
menu_run (
          const menu_mode_t mode
          );

#endif /* __MENU_H__ */
