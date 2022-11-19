/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  cdir.h - Per recuperer i desar directoris actuals.
 *
 */

#ifndef __CDIR_H__
#define __CDIR_H__

#include <glib.h>

// Pot tornar NULL
gchar *
cdir_read (
           const gchar    *id,
           const gboolean  verbose
           );

void
cdir_write (
            const gchar    *id,
            const gchar    *cdir,
            const gboolean  verbose
            );
  
#endif // __CDIR_H__
