/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
 *  memc.h - Gestiona els memory cards.
 *
 */

#ifndef __MEMC_H__
#define __MEMC_H__

#include <stdbool.h>

#include "conf.h"

void
close_memc (void);

// Init CD no crida a PSX!!!!
void
init_memc (
           conf_t    *conf,
           const int  verbose
           );

bool
memc_set (
          const int  port, // 0 o 1
          bool      *quit
          );

// Fica els memcard en la PSX. Es pot cridar tantes vegades com es
// vullga.
void
memc_replug (void);

#endif // __MEMC_H__
