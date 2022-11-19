/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  ui_button.h - Element que implementa un botó.
 *
 */

#ifndef __UI_BUTTON_H__
#define __UI_BUTTON_H__

#include "ui_element.h"

typedef struct
{
  
  UI_ELEMENT_CLASS;
  
  char        *_label;
  int          _label_w;
  int          _x,_y;
  int          _w,_h;
  enum {
    UI_BUTTON_PRESSED_KEY,
    UI_BUTTON_PRESSED_MOUSE,
    UI_BUTTON_RELEASED
  }     _state;
  ui_action_t *_action;
  void        *_udata;
  int          _cb_release_key;
  
} ui_button_t;

#define UI_BUTTON(PTR) ((ui_button_t *) (PTR))

ui_button_t *
ui_button_new (
               const char  *label, // En CP437
               const int    x,
               const int    y,
               ui_action_t *action, // Pot ser NULL
               void        *udata
               );

#endif // __UI_BUTTON_H__
