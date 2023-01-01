/*
 * Copyright 2016-2023 Adrià Giménez Pastor.
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
 *  eeprom.h - EEPROM.
 *
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include "MD.h"

void
close_eeprom (void);

void
init_eeprom (
             const char *rom_id,
             const char *eeprom_fn,
             const int   verbose
             );

MDu8 *
eeprom_get_eeprom (
        	   const size_t  nbytes,
        	   const MDu8    init_val,
        	   void         *udata
        	   );

#endif /* __EEPROM_H__ */
