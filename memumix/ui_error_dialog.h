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
 *  ui_error_dialog.h - Implementa un dialeg per mostrar missatges
 *                      d'error.
 *
 */

#ifndef __UI_ERROR_DIALOG_H__
#define __UI_ERROR_DIALOG_H__

#include "ui_element.h"

typedef struct
{

  UI_ELEMENT_CLASS;
  
  int          _x,_y;
  int          _w,_h;
  char        *_msg;
  int          _msg_length;
  ui_button_t *_but;
  
} ui_error_dialog_t;

#define UI_ERROR_DIALOG(PTR) ((ui_error_dialog_t *) (PTR))

ui_error_dialog_t *
ui_error_dialog_new (void);

void
ui_error_dialog_set_msg (
                         ui_error_dialog_t *self,
                         const char        *msg
                         );

#endif // __UI_ERROR_DIALOG_H__
