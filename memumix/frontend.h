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
 *  frontend.h - Frontend del simulador.
 *
 */

#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <stdbool.h>
#include <stdint.h>

#include "conf.h"
#include "ui_element.h"

void
close_frontend (void);

void
frontend_run (void);

void
init_frontend (
               conf_t     *conf,
               const bool  verbose
               );

// ACTIONS UI
void
frontend_action_change_screen_size (
                                    ui_element_t *e,
                                    void         *udata
                                    );

void
frontend_action_stop (
                      ui_element_t *e,
                      void         *udata
                      );

void
frontend_action_run (
                     ui_element_t *e,
                     void         *udata
                     );

#endif // __FRONTEND_H__
