/*
 * Copyright 2020-2025 Adrià Giménez Pastor.
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
 *  fchooser.h - Menú per a seleccionar fitxers.
 *
 */

#ifndef __FCHOOSER_H__
#define __FCHOOSER_H__

#include <glib.h>
#include <stdbool.h>

// NOTA!! S'assumeix que la pantalla està inicialitzada.

// Torna el nom del fitxer seleccionat o NULL si no s'ha seleccionat
// ningun. Es pot cridar altra vegada sense problemes.
// Si quit és cert cal tancar el programa.
const char *
fchooser_run (
              bool *quit
              );

// -1 indica que s'ha forçat l'eixida.
int
fchooser_error_dialog (void);

void
close_fchooser (
                const gchar *id
                );

void
init_fchooser (
               const gchar *id,
               const gchar *title,
               const gchar *selector, // Exemple: "[.](gen|bin)$"
               const bool   empty_entry,
               const bool   verbose
               );

void
fchooser_hide_cursor (void);

#endif // __FCHOOSER_H__
