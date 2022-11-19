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
 *  ui_menu_bar.h - Barra horizontal superior amb el menú.
 *
 */

#ifndef __UI_MENU_BAR_H__
#define __UI_MENU_BAR_H__

#include <stdbool.h>

#include "ui_element.h"
#include "ui_menu.h"


typedef struct ui_menu_bar ui_menu_bar_t;

typedef struct
{
  char      *label;
  ui_menu_t *menu;
  bool       has_focus;
  int        width_body;
} ui_menu_bar_entry_t;

struct ui_menu_bar
{

  UI_ELEMENT_CLASS;
  
  int                  _w,_h;
  int                  _selected; // Entrada seleccionada (-1 no seleccionada)
  ui_menu_bar_entry_t *_v; // |N| Llista d'entrades
  bool                *_enabled; // |N| Indica les entrades habilitades.
  int                 *_end_x; // |N| Indica posició primera columna següent
                           // |entrada.
  int                  _N;
  int                  _size;
  
};

#define UI_MENU_BAR(PTR) ((ui_menu_bar_t *) (PTR))

// S'indica la posició inicial.a
ui_menu_bar_t *
ui_menu_bar_new (void);

void
ui_menu_bar_enable_entry (
                          ui_menu_bar_t *self,
                          const int  entry,
                          const bool val
                          );

// Afegeix una entrada. Torna l'identificador de l'entrada.
// NOTA!! La barra de menús allibera la memòria del menú
int
ui_menu_bar_add_entry (
                       ui_menu_bar_t *self,
                       const char    *label, // CP437
                       ui_menu_t     *menu
                       );

#endif // __UI_MENU_BAR_H__
