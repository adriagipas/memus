/*
 * Copyright 2014-2024 Adrià Giménez Pastor.
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

#include "conf.h"
#include "menu.h"
#include "GBC.h"

void
close_frontend (void);

/* NOTA! El VSync no canvia. Açò és bo. */
void
frontend_reconfigure (
                      conf_t     *conf,
                      const char *title
                      );

/* Torna -1 en cas d'error, 0 si tot ha anat bé. */
int
frontend_run (
              const GBC_Rom     *rom,
              const char        *rom_id,
              const char        *sram_fn,
              const char        *state_prefix,
              const menu_mode_t  menu_mode,
              menu_response_t   *response
              );

void
init_frontend (
               conf_t          *conf,
               const char      *title,
               const GBCu8    **bios,
               const gboolean   big_screen,
               const int        verbose
               );

#endif /* __FRONTEND_H__ */
