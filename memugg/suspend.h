/*
 * Copyright 2022 Adrià Giménez Pastor.
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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  suspend.h - Supsen el simulador.
 *
 */

#ifndef __SUSPEND_H__
#define __SUSPEND_H__

#include <glib.h>
#include <stdbool.h>
#include <stdio.h>

void
close_suspend (void);

void
init_suspend (void);


// Torna NULL si no es pot obrir. S'ha de tancar.
FILE *
get_suspend_state_file (
                        const bool     write,
                        const gboolean verbose
                        );

// Si rom_fn == "" es considera que s'està netejant.
bool
suspend_write_rom_file_name (
                             const gchar    *rom_fn,
                             const gboolean  verbose
                             );

// Torne NULL en cas d'error, però també si està buit.
gchar *
suspend_read_rom_file_name (
                            const gboolean verbose
                            );

#endif // __SUSPEND_H__
