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
 *  cardpunch.h - Perforador de targetes.
 *
 */

#ifndef __CARDPUNCH_H__
#define __CARDPUNCH_H__

#include "MIX.h"
#include "ui.h"
#include "ui_element.h"

void
cp_close (void);

void
cp_init (
         ui_sim_state_t *ui_state
         );

// Fixa la variable interna STOP. Si està a cert el dispositiu no farà
// ninguna operació i pararà les que està fent-se.
void
cp_stop (void);

// Escriu la targeta.
void
cp_write (
	  MIX_IOOPChar *op
	  );

MIX_Bool
cp_busy (void);

// Per a ser cridada des de la UI.
void
cp_action_save (
                ui_element_t *e,
                void         *udata
                );

// Per a ser cridada des de la UI.
void
cp_action_clear (
                 ui_element_t *e,
                 void         *udata
                 );

#endif // __CARDPUNCH_H__
