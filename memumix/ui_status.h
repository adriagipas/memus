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
 *  ui_status.h - Barra que mostra diversos aspected de l'estat del
 *                simulador.
 *
 */

#ifndef __UI_STATUS_H__
#define __UI_STATUS_H__

#include "ui.h"
#include "ui_element.h"

typedef struct
{

  UI_ELEMENT_CLASS;

  const ui_sim_state_t *_state;
  int                   _running_bar;
  bool                  _running_bar_inc;
  int                   _cb_run_bar;
  
} ui_status_t;

#define UI_STATUS(PTR) ((ui_status_t *) (PTR))

ui_status_t *
ui_status_new (
               const ui_sim_state_t *state
               );

#endif // __UI_STATUS_H__
