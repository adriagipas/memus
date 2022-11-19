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
 *  ui_open_dialog.h - Implementa un dialeg per a obrir fitxers.
 *
 */

#ifndef __UI_OPEN_DIALOG_H__
#define __UI_OPEN_DIALOG_H__

#include "ui_element.h"
#include "ui_file_chooser.h"
#include "ui_list.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  int                _x,_y;
  ui_list_t         *_root;
  ui_file_chooser_t *_fchooser;
  char              *_title;
  int                _title_length;
  ui_action_t       *_action;
  void              *_udata;
  
} ui_open_dialog_t;

#define UI_OPEN_DIALOG(PTR) ((ui_open_dialog_t *) (PTR))

// L'acció s'executa cada vegada que es selecciona un fitxer o
// s'apreta el botó acceptar.
ui_open_dialog_t *
ui_open_dialog_new (
                    const char  *title, // cp437
                    const gchar *cdir,
                    const gchar *selector,
                    const bool   empty_entry,
                    ui_action_t *action,
                    void        *user_data
                    );

// Torna NULL si ara mateixa no hi ha cap fitxer regular seleccionat
// ara mateixa. El PATH "" vol dir --EMPTY--
const gchar *
ui_open_dialog_get_current_file (
                                 ui_open_dialog_t *self
                                 );

#define ui_open_dialog_get_current_dir(SELF)            \
  (ui_file_chooser_get_current_dir((SELF)->_fchooser))

void
ui_open_dialog_set_user_data (
                              ui_open_dialog_t *self,
                              void             *user_data
                              );

#endif // __UI_OPEN_DIALOG_H__
