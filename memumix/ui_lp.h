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
 *  ui_lp.h - Element que representa la impressora utilitzada en el
 *            simulador MIX.
 *
 */

#ifndef __UI_LP_H__
#define __UI_LP_H__

#include "ui.h"
#include "ui_element.h"
#include "ui_scrollbar.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  int             _x,_y;
  int             _w,_h; // No inclou scrollbar
  ui_sim_state_t *_sim_state;
  int             _cline; // Primera línia visible
  int             _ccol; // Primera columna visible
  ui_scrollbar_t *_vsb,*_hsb;
  
} ui_lp_t;

#define UI_LP(PTR) ((ui_lp_t *) (PTR))

ui_lp_t *
ui_lp_new (
           ui_sim_state_t *sim_state,
           const int       x,
           const int       y
           );

void
ui_lp_show_last_line (
                      ui_lp_t *self
                      );

#endif // __UI_LP_H__
