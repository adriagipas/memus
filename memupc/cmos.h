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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  cmos.h - Memòria RAM de la CMOS.
 *
 */

#ifndef __CMOS_H__
#define __CMOS_H__

#include <stdbool.h>
#include <stdint.h>

void
close_cmos (void);

void
init_cmos (
           const bool verbose
           );

uint8_t *
cmos_get_ram (
              void *udata
              );

#endif // __CMOS_H__
