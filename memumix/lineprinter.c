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
 *  lineprinter.c - Implementació de 'lineprinter.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmix2c.h"
#include "error.h"
#include "lineprinter.h"
#include "MIX.h"
#include "ui.h"
#include "ui_save_dialog.h"




/**********/
/* MACROS */
/**********/

// Vaig a ficar 2 línies per segon.
#define SLEEP_TIME 500000




/*********/
/* TIPUS */
/*********/

typedef struct
{

  int   timeout_id;
  enum
    {
     NEXT_PAGE,
     PRINT_BUF
    }   mode;
  char  buffer[121];
  int   buf_len;
  
} print_state_t;

#define PRINT_STATE(PTR) ((print_state_t *) (PTR))




/*********/
/* ESTAT */
/*********/

static int _line;

static print_state_t _print_state;

static ui_sim_state_t *_ui_state;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
print_op (
          void *user_data
          )
{

  int i;

  
  if ( _print_state.mode == NEXT_PAGE )
    {
      if ( _ui_state->lp.N < UI_LP_MAX_LINES )
        {
          _ui_state->lp.lines[_ui_state->lp.N][0]= '\0';
          _ui_state->lp.lengths[_ui_state->lp.N]= 0;
          ++(_ui_state->lp.N);
          ui_show_last_line_printer ();
        }
      else
        warning ( "LP: s'ha alcançat el número màxim de línies" );
      if ( ++_line < 60 ) return true;
      else _line= 0;
    }
  else
    {
      assert ( _print_state.mode == PRINT_BUF );
      if ( _ui_state->lp.N < UI_LP_MAX_LINES )
        {
          for ( i= 0; i < _print_state.buf_len; ++i )
            _ui_state->lp.lines[_ui_state->lp.N][i]= _print_state.buffer[i];
          _ui_state->lp.lines[_ui_state->lp.N][i]= '\0';
          _ui_state->lp.lengths[_ui_state->lp.N]= _print_state.buf_len;
          ++(_ui_state->lp.N);
          ui_show_last_line_printer ();
        }
      else
        warning ( "LP: s'ha alcançat el número màxim de línies" );
      if ( ++_line == 60 ) _line= 0;
    }

  _print_state.timeout_id= -1;

  return false;
  
} // end print_op




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
lp_close (void)
{

  if ( _print_state.timeout_id != -1 )
    ui_remove_callback ( _print_state.timeout_id );
  
} // end lp_close


void
lp_init (
         ui_sim_state_t *ui_state
         )
{
  
  _ui_state= ui_state;
  _ui_state->lp.N= 0;
  _print_state.timeout_id= -1;
  _line= 0;
  
} // end lp_init


void
lp_next_page (void)
{

  assert ( _print_state.timeout_id == -1 );
  
  _print_state.mode= NEXT_PAGE;
  _print_state.timeout_id=
    ui_register_callback ( print_op, NULL, SLEEP_TIME );
  
} // end lp_next_page


void
lp_stop (void)
{

  if ( _print_state.timeout_id != -1 )
    ui_remove_callback ( _print_state.timeout_id );
  _print_state.timeout_id= -1;
  
} // end lp_stop


void
lp_write (
	  MIX_IOOPChar *op
	  )
{
  
  MIX_Char line[120], cmix;
  int i, k;
  
  
  assert ( _print_state.timeout_id == -1 );
  
  // Llig de MIX.
  MIX_read_chars ( line, 120, op );
  i= k= 0;
  for ( i= 0; i < 120; ++i )
    {
      cmix= line[i];
      if ( cmix == MIX_DELTA )      _print_state.buffer[i]= '\xeb';
      else if ( cmix == MIX_SIGMA ) _print_state.buffer[i]= '\xe4';
      else if ( cmix == MIX_PI )    _print_state.buffer[i]= '\xe3';
      else _print_state.buffer[i]= cmix2c[cmix];
      if ( cmix != MIX_SPACE ) k= i+1;
    }
  _print_state.buffer[k]= '\0';
  _print_state.buf_len= k;

  // Imprimeix.
  _print_state.mode= PRINT_BUF;
  _print_state.timeout_id=
    ui_register_callback ( print_op, NULL, SLEEP_TIME );
  
} // end lp_write


MIX_Bool
lp_busy (void)
{
  return _print_state.timeout_id!=-1 ? MIX_TRUE : MIX_FALSE;
} // end lp_busy


void
lp_action_clear (
                 ui_element_t *e,
                 void         *udata
                 )
{

  _ui_state->lp.N= 0;
  ui_set_focus_line_printer ();
  
} // end lp_action_clear


void
lp_action_save_content (
                        ui_element_t *e,
                        void         *udata
                        )
{

  ui_save_dialog_t *d;
  gchar *fn;
  FILE *f;
  int n;
  
  
  d= UI_SAVE_DIALOG(e);
  fn= ui_save_dialog_get_file ( d );
  if ( fn == NULL ) goto error_noname;
  f= fopen ( fn, "wt" );
  g_free ( fn );
  if ( f == NULL ) goto error_noopen;
  for ( n= 0; n < _ui_state->lp.N; ++n )
    fprintf ( f, "%s\n", _ui_state->lp.lines[n] );
  fclose ( f );
  ui_save_lp_set_visible ( false );

  return;

 error_noname:
  ui_error ( "Cal introduir un nom de fitxer" );
  return;
 error_noopen:
  ui_error ( "No s'ha pogut crear el fitxer" );
  return;
  
} // end lp_action_save_content
