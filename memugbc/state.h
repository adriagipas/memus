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
 *  state.h - Estat del simulador.
 *
 */

#ifndef __STATE_H__
#define __STATE_H__

#include <stdio.h>

#define NUM_STATE_MAX 5

/* Els fitxer d'estat s'escriuran en 'prefix_fn'.st%d. Si és NULL
   gasta el per defecte. */
void
init_state (
            const char *rom_id,
            const char *prefix_fn, /* Pot ser NULL. */
            const int   verbose
            );

/* Obri el fitxer d'estat 'num' per a llegir. Pot tornar NULL. */
FILE *
state_open_read (
        	 const int num
        	 );

/* Obri el fitxer d'estat 'num' per a escriure. Pot tornar NULL. */
FILE *
state_open_write (
        	  const int num
        	  );

#endif /* __STATE_H__ */
