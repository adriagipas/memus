/*
 * Copyright 2015-2022 Adrià Giménez Pastor.
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
 *  rom.h - Utilitats relacionades amb la ROM.
 *
 */

#ifndef __ROM_H__
#define __ROM_H__

#include "GG.h"

/* Torna 0 si tot ha anat bé. */
int
load_rom (
          const char *fn,
          GG_Rom     *rom,
          const int   verbose
          );

const char *
get_rom_id (
            const GG_Rom       *rom,
            const GG_RomHeader *header,
            const int           verbose
            );

#endif /* __ROM_H__ */
