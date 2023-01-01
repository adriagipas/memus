/*
 * Copyright 2015-2023 Adrià Giménez Pastor.
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
 *  model.h - Per a gestionar el model de consola.
 *
 */

#ifndef __MODEL_H__
#define __MODEL_H__

#include "MD.h"

void
init_model (
            const char         *rom_id,
            const MD_RomHeader *header,
            const int           verbose
            );

void
close_model (void);

MDu8
model_get_val (void);

void
model_set_val (
              const MDu8 MDu8
              );

#endif /* __MODEL_H__ */
