/*
 * Copyright 2015-2024 Adrià Giménez Pastor.
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

void
close_fchooser (void);

void
init_fchooser (
               const int      *background,
               const gboolean  verbose
               );

// NULL vol dir que s'ha eixit sense elegir res.
const char *
fchooser_run (
              bool *quit
              );

// Un -1 indica que s'ha forçat l'eixida.
int
fchooser_error_dialog (void);

#endif // __FCHOOSER_H__
