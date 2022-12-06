/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
 *  cd.h - Gestiona el CD.
 *
 */

#ifndef __MEMU_CD_H__
#define __MEMU_CD_H__

#include <stdbool.h>

void
close_cd (void);

// Init CD no crida a PSX!!!!
void
init_cd (
         const int verbose
         );

// Aquesta funció fixa un nou CD en el simulador (NULL és
// llevar-lo). Torna cert si tot ha anat bé, false en cas d'error. En
// cas d'error l'anterior CD continua ficat. Quit indica que l'usuari
// volia eixir.
bool
cd_set_disc (
             bool *quit
             );

bool
cd_is_empty (void);

#endif // __MEMU_CD_H__
