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
 *  cardreader.c - Implementació de 'cardreader.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cardreader.h"
#include "error.h"
#include "MIX.h"
#include "ui.h"
#include "ui_open_dialog.h"




/**********/
/* MACROS */
/**********/

// Número de microsegons que tarda en llegir una targeta. He ficat un
// temps de 300 targetes per minut, es a dir 5 targetes per segón.
#define SLEEP_TIME 200000




/*********/
/* TIPUS */
/*********/

typedef struct
{

  int           timeout_id;
  MIX_IOOPChar *op;
  
} read_state_t;

#define READ_STATE(PTR) ((read_state_t *) (PTR))




/*************/
/* CONSTANTS */
/*************/

static const int _c2cmix[256]=
  {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, MIX_SPACE,
                -1, -1, -1, -1, -1, MIX_DELTA,
                                         -1,
    MIX_OPARENTHESE,
        MIX_CPARENTHESE,
            MIX_ASTERISK,
                MIX_PLUS,
                    MIX_COMMA,
                        MIX_MINUS,
                            MIX_DOT,
                                MIX_SLASH,
                                    MIX_0,
                                        MIX_1,
    MIX_2,
        MIX_3,
            MIX_4,
                MIX_5,
                    MIX_6,
                        MIX_7,
                            MIX_8,
                                MIX_9,
                                    -1, -1,
    -1, -1, -1, -1, -1, MIX_A,
                            MIX_B,
                                MIX_C,
                                    MIX_D,
                                        MIX_E,
    MIX_F,
        MIX_G,
            MIX_H,
                MIX_I,
                    MIX_J,
                        MIX_K,
                            MIX_L,
                                MIX_M,
                                    MIX_N,
                                        MIX_O,
    MIX_P,
        MIX_Q,
            MIX_R,
                MIX_S,
                    MIX_T,
                        MIX_U,
                            MIX_V,
                                MIX_W,
                                    MIX_X,
                                        MIX_Y,
    MIX_Z,
        -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1
  };




/*********/
/* ESTAT */
/*********/

static MIX_Char _cards[99][80];

static read_state_t _read_state;

static ui_sim_state_t *_ui_state;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
read_op (
         void *user_data
         )
{
  
  // Intenta llegir.
  if ( _ui_state->cr.N_read >= _ui_state->cr.N ) return true;
  MIX_write_chars ( _cards[_ui_state->cr.N_read], 80, _read_state.op );
  ++(_ui_state->cr.N_read);
  
  // Acaba.
  _read_state.timeout_id= -1;
  _ui_state->cr.busy= false;
  
  return false;
  
} // end read_op




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
cr_close (void)
{
  
  if ( _read_state.timeout_id != -1 )
    ui_remove_callback ( _read_state.timeout_id );
  
} // end cr_close


void
cr_init (
         ui_sim_state_t *ui_state
         )
{

  _ui_state= ui_state;
  _ui_state->cr.N= 0;
  _ui_state->cr.N_read= 0;
  _ui_state->cr.busy= false;
  _read_state.timeout_id= -1;
  
} // end cr_init



void
cr_read (
	 MIX_IOOPChar *op
	 )
{
  
  assert ( _read_state.timeout_id == -1 );
  _read_state.op= op;
  _read_state.timeout_id= ui_register_callback ( read_op, NULL, SLEEP_TIME );
  _ui_state->cr.busy= true;
  
} // end cr_read


void
cr_stop (void)
{
  
  if ( _read_state.timeout_id != -1 )
    ui_remove_callback ( _read_state.timeout_id );
  _read_state.timeout_id= -1;
  _ui_state->cr.busy= false;
  
} // end cr_stop


MIX_Bool
cr_busy (void)
{
  return _read_state.timeout_id!=-1 ? MIX_TRUE : MIX_FALSE;
} // end cr_busy


void
cr_action_load_file (
                     ui_element_t *e,
                     void         *udata
                     )
{

  ui_open_dialog_t *d;
  const gchar *fn;
  FILE *f;
  char buffer[82],c;
  int i;
  MIX_Char cmix;
  unsigned n;
  
  
  d= UI_OPEN_DIALOG(e);
  fn= ui_open_dialog_get_current_file ( d );
  f= fopen ( fn, "r" );
  if ( f == NULL )
    {
      warning ( "No s'ha pogut obrir el fitxer %s", fn );
      goto error_noopen;
    }
  n= 0;
  while ( fgets ( buffer, 82, f ) != NULL )
    {
      for ( i= 0, cmix= 0;
            cmix != -1 && (c= buffer[i]) != '\0' && c != '\n' && i < 80;
            cmix= _c2cmix[(unsigned char)c], ++i );
      if ( cmix == -1 )
        {
          warning ( "Línia %d, posició %d: 'x%02x' no està"
                    " suportat pel lector de targetes",
                    n+1, i, (unsigned char)c );
          break;
        }
      else if ( c != '\n' )
        {
          warning ( "La línia %d té més de 80 caràcters o no té '\\n'", n+1 );
          break;
        }
      for ( ; i < 80; ++i ) buffer[i]= ' ';
      if ( _ui_state->cr.N < 99 )
        {
          for ( i= 0; i < 80; ++i )
            _cards[_ui_state->cr.N][i]= _c2cmix[(unsigned char) buffer[i]];
          ++(_ui_state->cr.N);
          ++n;
        }
      else
        {
          warning ( "No es poden llegir més targetes", n+1 );
          break;
        }
    }
  fclose ( f );
  if ( n == 0 )
    {
      warning ( "No s'ha aconseguit carregar ninguna targeta" );
      goto error_nocard;
    }
  ui_open_cr_set_visible ( false );

  return;

 error_noopen:
  ui_error ( "No s'ha pogut obrir el fitxer" );
  return;
 error_nocard:
  ui_error ( "No s'ha pogut llegir cap targeta" );
  return;
  
} // end cr_action_load_file


void
cr_action_clear (
                 ui_element_t *e,
                 void         *udata
                 )
{
  
  _ui_state->cr.N_read= _ui_state->cr.N= 0;
  ui_set_focus_line_printer ();
  
} // end cr_action_clear


void
cr_action_rewind (
                  ui_element_t *e,
                  void         *udata
                  )
{
  
  _ui_state->cr.N_read= 0;
  ui_set_focus_line_printer ();
  
} // end cr_action_rewind
