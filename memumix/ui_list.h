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
 *  ui_list.h - Crea un element que agrupa a molts elements en una
 *              llista. I pinta en l'ordre en els que s'ha afegit en
 *              la llista.
 *
 */

#ifndef __UI_LIST_H__
#define __UI_LIST_H__

#include <glib.h>

#include "ui_element.h"

typedef struct
{
  UI_ELEMENT_CLASS;
  
  const GList         *_focus; // Pot ser NULL
  GList               *_v; // Inicialment buida
  GList               *_tail;
  ui_event_callback_t *_global_mouse_event;
  ui_event_callback_t *_global_key_event;
  void                *_udata;
  
} ui_list_t;

#define UI_LIST(PTR) ((ui_list_t *) (PTR))

// Crea una llista nova. Se li pot passar callbacks d'events que es
// criden quan ningun element de la llista processa l'event.
ui_list_t *
ui_list_new (
             ui_event_callback_t *mouse_event, // Pot ser NULL
             ui_event_callback_t *key_event,    // Pot ser NULL
             void                *udata
             );

// Afegeix un element a la llista. Quan s'esborre la llista
// s'esborrarà l'element.
void
ui_list_add (
             ui_list_t    *self,
             ui_element_t *e
             );

void
ui_list_set_focus (
                   ui_list_t *self
                   );

void
ui_list_set_focus_begin (
                         ui_list_t *self
                         );

void
ui_list_set_focus_end (
                       ui_list_t *self
                       );

#endif // __UI_LIST_H__
