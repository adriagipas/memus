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
 *  load_hdd.h - Carrega el HDD.
 *
 */

#ifndef __LOAD_HDD_H__
#define __LOAD_HDD_H__

#include <stdbool.h>
#include <stdint.h>

#include "conf.h"

#include "PC.h"

// NOTA!! S'assumeix que la pantalla està inicialitzada.

// Aquesta funció no torna fins que es carrega el HDD especificat en
// el fixer de configuració, o en cas de no estar o donar algun error,
// fins que l'usuari no selecciona un nou HDD. Cap la possibilitat de
// matar el programa, en eixe cas torna NULL. Si tot va bé torna el
// HDD obert.
PC_File *
load_hdd (
          conf_t    *conf,
          const int  verbose
          );

// Aquesta funció sols es pot cridar una vegada inicialitzar el
// frontend. La funció reinicia el simulador després de canviar el
// disc dur. Torna cert si ha modificat el disc dur, fals en cas
// contrari. quit indica que l'usuari ha demanat eixir.
bool
change_hdd (
            conf_t    *conf,
            bool      *quit,
            const int  verbose
            );

// Allibera tota la memòria associada a la BIOS. No cal cridar-ho
// abans de 'change_bios'.
void
free_hdd (void);

void
remove_hdd (
            conf_t *conf
            );

#endif // __LOAD_HDD_H__
