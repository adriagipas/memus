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
 *  ui_scrollbar.h - Implementa una barra de 'scroll'. Aquesta barra
 *                   ha sigut dissenyada perquè no tinga mai el
 *                   focus. Ha de ser moguda per altres
 *                   elements. L'única interacció directa que es fa és
 *                   quan es fa un arrastre directe.
 *
 */

#ifndef __UI_SCROLLBAR_H__
#define __UI_SCROLLBAR_H__

#include <stdbool.h>

#include "ui_element.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  int          _x,_y;
  int          _length;
  bool         _vertical;
  int          _N; // Número d'elements
  int          _wsize; // Finestra que mostra wsize<=N
  int          _pos; // Primer element de la finestra [0..N[
  ui_action_t *_action; // Pot ser NULL
  void        *_udata;

  // Controla dimensions barra selecció
  int _sel_pos;
  int _sel_length;

  // Controla moviment ratolí
  bool _pressed;
  int _old_pos; // Valor de _pos quan hem apretat
  int _old_pos_cord; // Valor de la coordenada quan s'ha apretat
  
} ui_scrollbar_t;

#define UI_SCROLLBAR(PTR) ((ui_scrollbar_t *) (PTR))

ui_scrollbar_t *
ui_scrollbar_new (
                  const int    x,
                  const int    y,
                  const int    length,
                  const bool   is_vertical,
                  ui_action_t *action,
                  void        *user_data
                  );

void
ui_scrollbar_set_state (
                        ui_scrollbar_t *self,
                        const int       num_element,
                        const int       window_size,
                        const int       pos
                        );

#define ui_scrollbar_get_pos(SELF) ((SELF)->_pos)

#endif // __UI_SCROLLBAR_H__
