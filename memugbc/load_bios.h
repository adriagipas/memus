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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  load_bios.h - Carrega la bios.
 *
 */

#ifndef __LOAD_BIOS_H__
#define __LOAD_BIOS_H__

#include "conf.h"

#include "GBC.h"

const GBCu8 *
load_bios (
           conf_t    *conf,
           const int  verbose
           );

#endif /* __LOAD_BIOS_H__ */
