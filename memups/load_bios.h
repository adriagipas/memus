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
 *  load_bios.h - Carrega la bios.
 *
 */

#ifndef __LOAD_BIOS_H__
#define __LOAD_BIOS_H__

#include <stdbool.h>
#include <stdint.h>

#include "conf.h"

// NOTA!! S'assumeix que la pantalla està inicialitzada.

// Aquesta funció no torna fins que es carrega la bios especificada en
// el fixer de configuració, o en cas de no estar o donar algun error,
// fins que l'usuari no selecciona una nova BIOS. Cap la possibilitat
// de matar el programa, en eixe cas torna NULL. De normal torna la
// bios.
const uint8_t *
load_bios (
           conf_t    *conf,
           const int  verbose
           );

// Aquesta funció carrega una altra bios en la mateixa posició de
// memòria que load_bios. Torna cert si ha modificat la bios, fals en
// cas contrari. quit indica que l'usuari ha demanat eixir.
bool
change_bios (
             conf_t    *conf,
             bool      *quit,
             const int  verbose
             );

#endif // __LOAD_BIOS_H__
