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
 *  cardpunch.c - Implementació de 'cardpunch.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "cardpunch.h"
#include "cmix2c.h"
#include "error.h"
#include "MIX.h"
#include "ui.h"
#include "ui_save_dialog.h"




/**********/
/* MACROS */
/**********/

// Número de milisegons que tarda en perforar una targeta. He ficat un
// temps de 1 targeta per segon.
#define SLEEP_TIME 1000000




/*********/
/* TIPUS */
/*********/

typedef struct
{
  
  int           timeout_id;
  MIX_IOOPChar *op;
  
} write_state_t;

#define WRITE_STATE(PTR) ((write_state_t *) (PTR))




/*********/
/* ESTAT */
/*********/

static char _cards[99][81];

static write_state_t _write_state;

static ui_sim_state_t *_ui_state;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static bool
write_op (
          void *data
          )
{

  char *buffer;
  MIX_Char line[80], cmix;
  int i,j;
  
  
  
  // Escriu.
  MIX_read_chars ( line, 80, _write_state.op );
  i= j= 0;
  if ( _ui_state->cp.N >= 99 )
    warning ( "cardpunch: no es poden escriure més targetes en memòria" );
  else
    {
      buffer= &(_cards[_ui_state->cp.N++][0]);
      for ( i= 0; i < 80; ++i )
        {
          cmix= line[i];
          if ( cmix == MIX_DELTA )
            buffer[i]= '&';
          else if ( cmix == MIX_SIGMA || cmix == MIX_PI || cmix >= MIX_EQUAL )
            {
              buffer[i]= ' ';
              warning ( "el perforador de targetes no pot"
                        " escriure el caràcter %d", cmix );
              cmix= MIX_SPACE;
            }
          else buffer[i]= cmix2c[cmix];
          if ( cmix != MIX_SPACE ) j= i;
        }
      buffer[j+1]= '\0';
    }
  
  // Acaba.
  _write_state.timeout_id= -1;
  _ui_state->cp.busy= false;
  
  return false;
  
} // end write_op




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
cp_close (void)
{
  
  if ( _write_state.timeout_id != -1 )
    ui_remove_callback ( _write_state.timeout_id );
  
} // end cp_close


void
cp_init (
         ui_sim_state_t *ui_state
         )
{

  _ui_state= ui_state;
  
  _write_state.timeout_id= -1;
  _ui_state->cp.N= 0;
  _ui_state->cp.busy= false;
  
} // end cp_init


void
cp_stop (void)
{
  
  if ( _write_state.timeout_id != -1 )
    ui_remove_callback ( _write_state.timeout_id );
  _write_state.timeout_id= -1;
  
} // end cp_stop


void
cp_write (
	  MIX_IOOPChar *op
	  )
{
  
  assert ( _write_state.timeout_id == -1 );
  
  _write_state.op= op;
  _write_state.timeout_id=
    ui_register_callback ( write_op, NULL, SLEEP_TIME );
  _ui_state->cp.busy= true;
  
} // end cp_write


MIX_Bool
cp_busy (void)
{
  return _write_state.timeout_id!=-1 ? MIX_TRUE : MIX_FALSE;
} // end cp_busy


void
cp_action_save (
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
  for ( n= 0; n < _ui_state->cp.N; ++n )
    fprintf ( f, "%s\n", _cards[n] );
  fclose ( f );
  ui_save_cp_set_visible ( false );

  return;

 error_noname:
  ui_error ( "Cal introduir un nom de fitxer" );
  return;
 error_noopen:
  ui_error ( "No s'ha pogut crear el fitxer" );
  return;
  
} // end cp_action_save


void
cp_action_clear (
                 ui_element_t *e,
                 void         *udata
                 )
{

  _ui_state->cp.N= 0;
  ui_set_focus_line_printer ();
  
} // end cp_action_clear
