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
 *  ui_save_dialog.h - Implementa un dialeg per a desar fitxers.
 *
 */

#ifndef __UI_SAVE_DIALOG_H__
#define __UI_SAVE_DIALOG_H__

#include "ui_element.h"
#include "ui_file_chooser.h"
#include "ui_input.h"
#include "ui_list.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  int                _x,_y;
  ui_list_t         *_root;
  ui_file_chooser_t *_fchooser;
  ui_input_t        *_input;
  char              *_title;
  int                _title_length;
  ui_action_t       *_action;
  void              *_udata;
  
} ui_save_dialog_t;

#define UI_SAVE_DIALOG(PTR) ((ui_save_dialog_t *) (PTR))

// L'acció s'executa cada vegada que es selecciona un fitxer o
// s'apreta el botó acceptar.
ui_save_dialog_t *
ui_save_dialog_new (
                    const char  *title, // cp437
                    const gchar *cdir,
                    ui_action_t *action,
                    void        *user_data
                    );

// Torna NULL si no s'ha escrit cap fitxer. Cal alliberar la cadena
// que retorna.
gchar *
ui_save_dialog_get_file (
                         ui_save_dialog_t *self
                         );

#define ui_save_dialog_get_current_dir(SELF)            \
  (ui_file_chooser_get_current_dir((SELF)->_fchooser))

#endif // __UI_SAVE_DIALOG_H__
