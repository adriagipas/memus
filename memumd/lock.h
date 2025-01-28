/*
 * Copyright 2022-2025 Adrià Giménez Pastor.
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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  lock.h - Serveix per impedir que hi hasquen múltiples processos
 *           executant-se per a la mateixa sessió. Es pot fer a nivell
 *           global (sense rom) o a nivell de rom (quan s'executa amb
 *           rom)
 *
 */

#ifndef __LOCK_H__
#define __LOCK_H__

#include <glib.h>
#include <stdbool.h>

void
close_lock (void);

// Torna TRUE si s'ha pogut bloquejar
gboolean
init_lock (
           const gchar    *romid, // Pot ser NULL
           const gboolean  verbose
           );

// Torna cert si s'ha generat alguna senyal.
bool
lock_check_signals (void);

#endif // __LOCK_H__
