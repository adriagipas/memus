/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
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
 *  sram.h - Memòria estàtica.
 *
 */

#ifndef __SRAM_H__
#define __SRAM_H__

#include "GG.h"

void
close_sram (void);

void
init_sram (
           const char *rom_id,
           const char *sram_fn, /* Pot ser NULL. */
           const int   verbose
           );

Z80u8 *
sram_get_external_ram (
        	       void *udata
        	       );

#endif /* __SRAM_H__ */
