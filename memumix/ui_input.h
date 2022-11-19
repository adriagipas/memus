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
 *  ui_input.h - Caixa per a escriure (molt limitat).
 *
 */

#ifndef __UI_INPUT_H__
#define __UI_INPUT_H__

#include <stdbool.h>

#include "ui_element.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  int          _x,_y;
  int          _w,_h;
  int          _length;
  int          _N; // nombre de caracters
  char        *_buf; // length+1

  ui_action_t *_action;
  void        *_udata;

  int          _cb_show_cursor;
  bool         _show_cursor;
  
} ui_input_t;

#define UI_INPUT(PTR) ((ui_input_t *) (PTR))

ui_input_t *
ui_input_new (
              const int    x,
              const int    y,
              const int    length, // Longitut en caràcters
              ui_action_t *action,
              void        *udata
              );

const char *
ui_input_get_text (
                   ui_input_t *self
                   );

void
ui_input_clear (
                ui_input_t *self
                );

void
ui_input_set_text (
                   ui_input_t *self,
                   const char *text
                   );

#endif // __UI_INPUT_H__
