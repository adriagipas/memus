/*
 * Copyright 2010-2022 Adrià Giménez Pastor.
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
 *  cardreader.h - Lector de targetes.
 *
 */

#ifndef __CARDREADER_H__
#define __CARDREADER_H__

#include "MIX.h"
#include "ui.h"
#include "ui_element.h"

// Llibera els recursos del lector.
void
cr_close (void);

void
cr_init (
         ui_sim_state_t *ui_state
         );

// Llig de la targeta.
void
cr_read (
	 MIX_IOOPChar *op
	 );

// Fixa la variable interna STOP. Si està a cert el dispositiu no farà
// ninguna operació i pararà les que està fent-se.
void
cr_stop (void);


MIX_Bool
cr_busy (void);

// Per a ser cridada des de la UI.
void
cr_action_load_file (
                     ui_element_t *e,
                     void         *udata
                     );

// Per a ser cridada des de la UI.
void
cr_action_clear (
                 ui_element_t *e,
                 void         *udata
                 );

// Per a ser cridada des de la UI.
void
cr_action_rewind (
                  ui_element_t *e,
                  void         *udata
                  );

#endif // __CARDREADER_H__
