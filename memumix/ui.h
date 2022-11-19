/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  ui.h - User interface.
 *
 */

#ifndef __UI_H__
#define __UI_H__

#include <stdbool.h>

#include "conf.h"


// TIPUS

#define UI_LP_MAX_LINES 500
#define UI_LP_MAX_LENGTH 120

// Aquest tipus emmagatzema la informació que necessita la UI del
// simulador per a poder renderitzar la interfície. La idea es que la
// ui sols consulte les dades d'ací i no modifique res.
typedef struct
{
  // Global
  bool running;
  // Dades sobre el 'cardpunch'.
  struct
  {
    int  N; // targetes actualment escrites que encara estan en
            // memòria. Valor màxim 99.
    bool busy; // Indica que està treballant
  } cp;
  // Dades sobre el 'cardreader'.
  struct
  {
    int  N; // Número total de targetes en memòria. Valor màxim 99.
    int  N_read; // Número total de targetes en memòria ja llegides.
    bool busy; // Indica que està treballant
  } cr;
  // Dades sobre les cintes magnètiques.
  struct
  {
    int  N; // Número de paraules (pot ser 0, sense màxim)
    int  pos; // Següent paraules a processar
    bool busy;
  } tapes[8];
  // Dades sobre la impresora
  struct
  {
    // Cada línia es representa com una cadena de caràcters de
    // longitut màxima UI_LP_MAX_LENGTH. No estab acabades amb '\0',
    // però deixe espai per a fer-ho per simplificar el procés de
    // renderitzat.
    // Per saber la longitut cal vore lengths;
    char lines[UI_LP_MAX_LINES][UI_LP_MAX_LENGTH+1];
    int  lengths[UI_LP_MAX_LINES];
    int  N; // Número de línies
  } lp;
  
} ui_sim_state_t;

// Quan torna false elmina el callback.
typedef bool (ui_callback_t) (void *user_data);


// FUNCIONS

void
close_ui (void);

void
ui_run (void);

// sim_state en realitat no l'ha de modificar la UI, però no el passe
// constant perquè així puc fer un truquet per renderitzar més ràpid.
void
init_ui (
         conf_t         *conf,
         ui_sim_state_t *sim_state,
         const gchar    *cdir_cr,
         const gchar    *cdir_lp,
         const gchar    *cdir_mt,
         const gchar    *cdir_cp
         );

void
ui_remove_callback (
                    const int callback_id
                    );

// Torna l'identificador del callback
int
ui_register_callback (
                      ui_callback_t *callback,
                      void          *user_data,
                      const gulong   microsecs
                      );

// Torna el current dir per a cardreader
const gchar *
ui_get_cdir_cr (void);

const gchar *
ui_get_cdir_mt (void);

const gchar *
ui_get_cdir_lp (void);

const gchar *
ui_get_cdir_cp (void);

void
ui_open_cr_set_visible (
                        const bool visible
                        );

void
ui_open_mt_set_visible (
                        const bool visible,
                        const int  tapeid
                        );

void
ui_save_lp_set_visible (
                        const bool visible
                        );

void
ui_save_cp_set_visible (
                        const bool visible
                        );

void
ui_error (
          const char *msg
          );

void
ui_show_last_line_printer (void);

void
ui_set_focus_line_printer (void);

#endif // __UI_H__
