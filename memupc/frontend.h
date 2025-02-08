/*
 * Copyright 2025 Adrià Giménez Pastor.
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

#include <glib.h>
#include <stdbool.h>

#include "conf.h"
#include "menu.h"
#include "PC.h"

enum {
  DISC_D_CDROM= 0,
  DISC_A_FLOPPY_1M44,
  DISC_B_FLOPPY_1M2
};

void
close_frontend (void);

menu_response_t
frontend_run (
              const gchar *disc_D,
              const gchar *disc_A,
              const gchar *disc_B
              );

// Si file_name és NULL lleva el disc. Si ja hi havia un disc el
// lleva.
bool
frontend_set_disc (
                   const gchar *file_name,
                   const int    type
                   );

bool
frontend_disc_is_empty (
                        const int type
                        );

// Canvia la BIOS i reinicia.
void
frontend_change_bios (
                      uint8_t      *bios,
                      const size_t  bios_size
                      );

// Canvia la VGA BIOS i reinicia.
void
frontend_change_vgabios (
                         uint8_t      *bios,
                         const size_t  bios_size
                         );

// Canvia el disc dur i reinicia.
void
frontend_change_hdd (
                     PC_File *hdd
                     );

void
init_frontend (
               conf_t     *conf,
               const char *title,
               const bool  verbose
               );

#endif // __FRONTEND_H__
