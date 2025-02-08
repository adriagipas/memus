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
 *  set_disc - Gestiona la selecció de discs.
 *
 */

#ifndef __SET_DISC_H__
#define __SET_DISC_H__

#include <stdbool.h>

#include "frontend.h"

// Aquesta funció fixa un nou disc en el simulador (NULL és
// llevar-lo). Torna cert si tot ha anat bé, false en cas
// d'error. Quit indica que l'usuari volia eixir.
bool
set_disc (
          bool      *quit,
          const int  type,
          const bool verbose
          );

#endif // __SET_DISC_H__
