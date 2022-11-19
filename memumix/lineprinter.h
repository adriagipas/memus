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
 *  lineprinter.h - Impressora de línies. Funciona sobre l'àrea de
 *                  text visible de la finestra principal. Encara que
 *                  l'àrea es pot vore tot el gran que es vulga, la
 *                  pàgina és de 120 caràcters d'ample i 60 línies. A
 *                  més afegeix la possibilitat de netejar tot el text
 *                  imprès.
 *
 */

#ifndef __LINEPRINTER_H__
#define __LINEPRINTER_H__

#include "ui.h"
#include "ui_element.h"

void
lp_close (void);

// Inicialitza l'impressora.
void
lp_init (
         ui_sim_state_t *ui_state
         );

// Bota a la següent pàgina.
void
lp_next_page (void);

// Fixa la variable interna STOP. Si està a cert el dispositiu no farà
// ninguna operació i pararà les que està fent-se.
void
lp_stop (void);

// Imprimeix una línia.
void
lp_write (
	  MIX_IOOPChar *op
	  );

MIX_Bool
lp_busy (void);

// Per a ser cridada des de la UI.
void
lp_action_clear (
                 ui_element_t *e,
                 void         *udata
                 );

// Per a ser cridada des de la UI.
void
lp_action_save_content (
                        ui_element_t *e,
                        void         *udata
                        );

#endif // __LINEPRINTER_H__
