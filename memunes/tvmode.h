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
 *  tvmode.h - Per a gestionar si és NTSC/PAL
 *
 */

#ifndef __TVMODE_H__
#define __TVMODE_H__

#include "NES.h"

void
init_tvmode (
             const char    *rom_id,
             const NES_Rom *rom,
             const int      verbose
             );

void
close_tvmode (void);

NES_TVMode
tvmode_get_val (void);

void
tvmode_set_val (
        	const NES_TVMode tvmode
        	);

#endif /* __TVMODE_H__ */
