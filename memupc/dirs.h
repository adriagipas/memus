/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
 *  dirs.h - Directoris base de l'aplicació.
 *
 */

#ifndef __DIRS_H__
#define __DIRS_H__

#include <glib.h>

void
close_dirs (void);

void
init_dirs (void);

const gchar *
get_confdir (void);

const gchar *
get_sharedir (void);

#endif // __DIRS_H__
