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
 *  sound.h - So.
 *
 */

#ifndef __SOUND_H__
#define __SOUND_H__

#include "NES.h"

void
close_sound (void);

/* S'assumeix que SDL s'ha inicialitzat. El so comença pausat. */
void
init_sound (void);

void
sound_play (
            const double  frame[NES_APU_BUFFER_SIZE],
            void         *udata
            );

void
sound_set_tvmode (
        	  const NES_TVMode tvmode
        	  );

#endif /* __SOUND_H__ */
