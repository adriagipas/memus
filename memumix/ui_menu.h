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
 *  ui_menu.h - Element que representa un menú.
 *
 */

#ifndef __UI_MENU_H__
#define __UI_MENU_H__

#include <stdbool.h>

#include "ui_element.h"

typedef enum {
  UI_MENU_ACTION= 0,
  UI_MENU_SEPARATOR,
  UI_MENU_SUBMENU,
  UI_MENU_CHOICE
} ui_menu_entry_type_t;

typedef struct ui_menu ui_menu_t;

typedef union
{
  ui_menu_entry_type_t type;
  struct
  {
    ui_menu_entry_type_t  type;
    char                 *label;
    ui_action_t          *action;
    void                 *udata;
  }                    action;
  struct
  {
    ui_menu_entry_type_t  type;
    char                 *label;
    ui_menu_t            *menu;
    bool                  has_focus;
  }                    submenu;
  struct
  {
    ui_menu_entry_type_t  type;
    char                 *label;
    ui_action_t          *action;
    void                 *udata;
    int                  *var; // Variable on es desa el valor seleccionat
    int                   val; // Valor d'aquesta opció
  }                    choice;
} ui_menu_entry_t;

struct ui_menu
{

  UI_ELEMENT_CLASS;
  
  int              _x,_y;
  int              _w,_h;
  int              _selected; // Entrada seleccionada (-1 no seleccionada)
  ui_menu_entry_t *_v; // |N| Llista d'entrades
  bool            *_enabled; // |N| Indica les entrades habilitades.
  int             *_end_y; // |N| Indica posició primera línea següent
                           // |entrada.
  int              _N;
  int              _size;
  
};

#define UI_MENU(PTR) ((ui_menu_t *) (PTR))

// S'indica la posició inicial.a
ui_menu_t *
ui_menu_new (
             const int x,
             const int y
             );

void
ui_menu_enable_entry (
                      ui_menu_t *self,
                      const int  entry,
                      const bool val
                      );

// Mou i fa visible un menú.
void
ui_menu_show (
              ui_menu_t *self,
              const int  x,
              const int  y
              );

// Afegeix una entrada de tipus acció. Torna l'identificador de
// l'entrada.
int
ui_menu_add_entry_action (
                          ui_menu_t   *self,
                          const char  *label, // CP437
                          ui_action_t *action,
                          void        *user_data
                          );

// Afegeix una entrada de tipus separador. Torna l'identificador de
// l'entrada.
// NOTA!! Els separadors per defecte estan deshabilitats.
int
ui_menu_add_entry_separator (
                             ui_menu_t *self
                             );

// Afegeix una entrada de tipus submenú. Torna l'identificador de
// l'entrada.
// NOTA!! El menu allibera la memòria del submenú.
int
ui_menu_add_entry_submenu (
                          ui_menu_t  *self,
                          const char *label, // CP437
                          ui_menu_t  *menu
                          );

// Afegeix una entrada de tipus opció. Torna l'identificador de
// l'entrada. És com una acció però quan s'apreta canvia el valor de
// la variable indicada amb el valor associat a esta opció.
int
ui_menu_add_entry_choice (
                          ui_menu_t   *self,
                          const char  *label, // CP437
                          int         *var,  // Variable opció
                          const int    val,  // Valor d'aquesta entrada
                          ui_action_t *action,
                          void        *user_data
                          );

void
ui_menu_set_min_width (
                       ui_menu_t *self,
                       const int  width
                       );

#endif // __UI_MENU_H__
