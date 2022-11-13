/*
 * Copyright 2016-2022 Adrià Giménez Pastor.
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
 *  sram.h - Memòria estàtica.
 *
 */

#ifndef __SRAM_H__
#define __SRAM_H__

#include "NES.h"

void
close_sram (void);

void
init_sram (
           const char *rom_id,
           const char *sram_fn,
           const int   verbose
           );

/* Demana 0x2000 bytes de memòria estàtica. Si no es crida a questa
   funció no es gasta memòria estàtica. */
NESu8 *
sram_get_static_ram (void);

#endif /* __SRAM_H__ */
