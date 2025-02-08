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
 *  session.h - Identificador de sessió. Ha de ser un valor [A-Za-z_].
 *
 */

#ifndef __SESSION_H__
#define __SESSION_H__

#include <glib.h>

void
close_session (void);

void
init_session (
              const gchar    *name,
              const gboolean  verbose
              );

// Torna NULL si és la per defecte.
const gchar *
session_get_name (void);

#endif // __SESSION_H__
