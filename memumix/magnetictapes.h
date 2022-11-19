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
 *  magnetictapes.h - Cintes magnètiques.
 *
 */


#ifndef __MAGNETICTAPES_H__
#define __MAGNETICTAPES_H__

#include <glib.h>

#include "MIX.h"
#include "ui.h"
#include "ui_element.h"

// Inicialitza l'estat de les huit cintes, que inicialment estaran
// sense cinta. Es crida abans de mostrar la finestra dels
// dispositius.
void
mt_init (
         ui_sim_state_t *ui_state
         );

// LLivera els recursos de les cintes. Es crida després de tancar la
// finestra dels dispositius.
void
mt_close (void);

// Llig de la cinta magnètica indicada. Esta funció la crida el
// simulador.
void
mt_read (
	 const int     tape,
	 MIX_IOOPWord *op
	 );

// Rebobina una cinta magnètica. A esta funció la crida el simulador.
void
mt_rewind (
	   const int tape
	   );

// En la cinta indicada retrocedeix el número de paraules indicat. A
// esta funció la crida el simulador.
void
mt_skip_backward (
		  const int tape,
		  const int nwords
		  );

// En la cinta indicada avança el número de paraules indicat. A esta
// funció la crida el simulador.
void
mt_skip_forward (
		 const int tape,
		 const int nwords
		 );

// Força l'aturada de totes les operacions.
void
mt_stop (void);

// Escriu en la cinta magnètica indicada. Esta funció la crida el
// simulador.
void
mt_write (
	  const int     tape,
	  MIX_IOOPWord *op
	  );

MIX_Bool
mt_busy (
         const int tape
         );

// Per a ser cridada des de la UI.
void
mt_action_load_file (
                     ui_element_t *e,
                     void         *udata
                     );

// Per a ser cridada des de la UI.
void
mt_action_clear (
                 ui_element_t *e,
                 void         *udata
                 );

void
mt_action_rewind (
                  ui_element_t *e,
                  void         *udata
                  );

#endif // __MAGNETICTAPES_H__
