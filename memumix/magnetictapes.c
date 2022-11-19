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
 *  magnetictapes.c - Implementació de 'magnetictapes.h'.
 *
 */


#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "magnetictapes.h"
#include "MIX.h"
#include "ui.h"
#include "ui_element.h"
#include "ui_open_dialog.h"




/**********/
/* MACROS */
/**********/

// Número de milisegons que tarda en processar una paraula. Ho he ficat
// un poc arreu, concretament un bloc per segon.
#define SLEEP_TIME 10000


#define BLOCKSIZE 100


#define _MIN(A,B) ((A)<(B)?(A):(B))

#define TAPEID(PTR) ((int) ((PTR)-&_tapes[0]))

#define RVAR s->v.r
#define BVAR s->v.b
#define FVAR s->v.f
#define WVAR s->v.w




/*********/
/* TIPUS */
/*********/

typedef struct
{
  
  FILE *f;          // Fitxer de la cinta. NULL indica que no hi ha.
  int   nwords;     // Número de paraules de la cinta.
  int   N;          // Número de paraules processades.
  
} tape_t;

typedef struct
{

  guint   timeout_id;
  tape_t *tape;
  enum
    {
     READ,
     REWIND,
     BACKWARD,
     FORWARD,
     WRITE
    }     type;
  bool          start; // es fica a true al principi.
  
  // Variables.
  union {
    struct
    {
      MIX_Word      buffer[BLOCKSIZE];
      MIX_IOOPWord *op;
      size_t        remain;
      size_t        min;
      int           remain_tape;
      int           i;
    } r;
    struct
    {
      int nwords;
    } b;
    struct
    {
      int nwords;
    } f;
    struct
    {
      MIX_Word      buffer[BLOCKSIZE];
      MIX_IOOPWord *op;
      size_t        remain;
      size_t        min;
      int           remain_tape;
      int           i;
    } w;
  }             v;
  
} op_state_t;

#define OP_STATE(PTR) ((op_state_t *) (PTR))




/*********/
/* ESTAT */
/*********/

static tape_t _tapes[8];

static op_state_t _op_states[8];

static ui_sim_state_t *_ui_state;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
update_ui (
           const tape_t *tape,
           const bool    busy
           )
{

  int id;


  id= TAPEID(tape);
  _ui_state->tapes[id].N= tape->nwords;
  _ui_state->tapes[id].pos= tape->N;
  _ui_state->tapes[id].busy= busy;
  
} // end update_ui


static void
reset_ui_state (
                const tape_t *tape
                )
{
  
  int id;
  
  
  id= TAPEID(tape);
  _ui_state->tapes[id].N= 0;
  _ui_state->tapes[id].pos= 0;
  _ui_state->tapes[id].busy= false;
  
} // end reset_ui_state


static bool
no_tape (
         const tape_t *tape
         )
{
  
  warning ( "no hi ha ninguna cinta magnètica en el lector %d",
            TAPEID(tape)+1 );
  reset_ui_state ( tape );
  
  return false;
  
} // end no_tape


static bool
skip_error (
            tape_t *tape
            )
{
  
  fclose ( tape->f );
  tape->f= NULL;
  reset_ui_state ( tape );
  warning ( "Mentre es modificava l'indicador de posició en el fitxer"
            " associat a la cinta magnètica %d"
            "s'ha produït el següent error: %s",
            TAPEID(tape)+1, strerror ( errno ) );
  
  return false;
  
} // end skip_error


static bool
op_read (
         op_state_t *s
         )
{
  
  MIX_Word aux;
  int j;
  unsigned char byte;
  
  
  // Inicialització.
  if ( s->start )
    {
      if ( s->tape->f == NULL ) return no_tape ( s->tape );
      RVAR.remain= RVAR.op->remain;
      if ( RVAR.remain == 0 ) { update_ui ( s->tape, false ); return false; }
      RVAR.remain_tape= s->tape->nwords-s->tape->N;
      s->start= false;
      RVAR.i= -1; // Abans d'entrar en el bucle
    }

  // Emule un do { ... for(i=0;i<min;++i)... } while(remain )
  // --> Abans d'entrar en el for
  if ( RVAR.i == -1 )
    {
      if ( RVAR.remain_tape == 0 )
        {
          warning ( "ja no es poden llegir"
                    " més paraules de la cinta magnètica %d",
                    TAPEID(s->tape)+1 );
          update_ui ( s->tape, false );
          return false;
        }
      RVAR.min= _MIN ( RVAR.remain_tape, BLOCKSIZE );
      RVAR.min= _MIN ( RVAR.min, RVAR.remain );
      RVAR.remain_tape-= RVAR.min;
      RVAR.i= 0;
    }
  // --> Iteració.
  aux= 0;
  for ( j= 0; j < 4; ++j )
    {
      if ( !fread ( &byte, 1, 1, s->tape->f ) ) goto error;
      aux<<= 8;
      aux|= byte;
    }
  RVAR.buffer[RVAR.i]= aux;
  ++(s->tape->N);
  ++(RVAR.i);
  update_ui ( s->tape, true );
  // --> Acaba for
  if ( RVAR.i < RVAR.min ) return true;
  RVAR.i= -1; // La pròxima vegada entra en el codi previ al for.
  RVAR.remain= MIX_write_words ( RVAR.buffer, RVAR.min, RVAR.op );
  
  // --> Finalitza do {} while
  if ( RVAR.remain ) return true;
  else
    {
      update_ui ( s->tape, false );
      return false;
    }
  
 error:
  fclose ( s->tape->f );
  s->tape->f= NULL;
  reset_ui_state ( s->tape );
  update_ui ( s->tape, false );
  warning ( "Mentre es llegia del fitxer associat a la cinta"
            " magnètica %d s'ha produït el següent error: %s",
            TAPEID(s->tape)+1, strerror ( errno ) );
  
  return false;
  
} // end op_read


static bool
op_rewind (
           op_state_t *s
           )
{

  if ( s->start )
    {
      if ( s->tape->f == NULL ) return no_tape ( s->tape );
      if ( s->tape->N == 0 ) { update_ui ( s->tape, false ); return false; }
      rewind ( s->tape->f );
      s->start= false;
    }

  // Itera.
  --(s->tape->N);
  update_ui ( s->tape, s->tape->N ? true : false );
  
  return s->tape->N ? true : false;
  
} // end op_rewind


static bool
op_backward (
             op_state_t *s
             )
{

  bool ret;

  
  // Inicialització.
  if ( s->start )
    {
      if ( s->tape->f == NULL ) return no_tape ( s->tape );
      if ( s->tape->N == 0 )
        {
          warning ( "ja no es pot retrocedir"
                    " més en la cinta magnètica %d",
                    TAPEID(s->tape)+1 );
          update_ui ( s->tape, false );
          return false;
        }
      if ( BVAR.nwords > s->tape->N )
        {
          BVAR.nwords= s->tape->N;
          warning ( "No s'ha pogut retrocedir més de %d"
                    " paraules en la cinta magnètica %d",
                    s->tape->N, TAPEID(s->tape)+1 );
        }
      if ( fseek ( s->tape->f, -(BVAR.nwords)*4, SEEK_CUR ) )
        return skip_error ( s->tape );
      s->start= false;
    }

  // Itera.
  if ( BVAR.nwords )
    {
      --(BVAR.nwords);
      --(s->tape->N);
    }
  ret= BVAR.nwords ? true : false;
  update_ui ( s->tape, ret );
  
  return ret;
  
} // end op_backward


static bool
op_forward (
            op_state_t *s
            )
{
  
  int remain;
  bool ret;
  
  
  // Inicialització.
  if ( s->start )
    {
      if ( s->tape->f == NULL ) return no_tape ( s->tape );
      if ( s->tape->N == s->tape->nwords )
        {
          warning ( "ja no es pot avançar"
                    " més la cinta magnètica %d",
                    TAPEID(s->tape)+1 );
          update_ui ( s->tape, false );
          return false;
        }
      remain= s->tape->nwords-s->tape->N;
      if ( FVAR.nwords > remain )
        {
          FVAR.nwords= remain;
          warning ( "No s'ha pogut avançar més de %d"
                    " paraules en la cinta magnètica %d",
                    remain, TAPEID(s->tape)+1 );
        }
      if ( fseek ( s->tape->f, (FVAR.nwords)*4, SEEK_CUR ) )
        return skip_error ( s->tape );
      s->start= false;
    }
  
  // Itera.
  if ( FVAR.nwords )
    {
      --(FVAR.nwords);
      ++(s->tape->N);
    }
  ret= FVAR.nwords ? true : false;
  update_ui ( s->tape, ret );
  
  return ret;
  
} // end op_forward


static bool
op_write (
          op_state_t *s
          )
{
  
  MIX_Word aux;
  int j;
  unsigned char byte;
  
  
  // Inicialització.
  if ( s->start )
    {
      if ( s->tape->f == NULL ) return no_tape ( s->tape );
      WVAR.remain= WVAR.op->remain;
      if ( WVAR.remain == 0 ) { update_ui ( s->tape, false ); return false; }
      WVAR.remain_tape= s->tape->nwords-s->tape->N;
      s->start= false;
      WVAR.i= -1; // Abans d'entrar en el bucle
    }

  // Emule un do { ... for(i=0;i<min;++i)... } while(remain )
  // --> Abans d'entrar en el for
  if ( WVAR.i == -1 )
    {
      if ( WVAR.remain_tape == 0 )
        {
          warning ( "ja no es poden escriure"
                    " més paraules en la cinta magnètica %d",
                    TAPEID(s->tape)+1 );
          update_ui ( s->tape, false );
          return false;
        }
      WVAR.min= _MIN ( WVAR.remain_tape, BLOCKSIZE );
      WVAR.min= _MIN ( WVAR.min, WVAR.remain );
      WVAR.remain_tape-= WVAR.min;
      WVAR.remain= MIX_read_words ( WVAR.buffer, WVAR.min, WVAR.op );
      WVAR.i= 0;
    }
  // --> Iteració.
  aux= WVAR.buffer[WVAR.i];
  for ( j= 0; j < 4; ++j )
    {
      byte= (unsigned char) (aux>>24);
      aux<<= 8;
      if ( !fwrite ( &byte, 1, 1, s->tape->f ) ) goto error;
    }
  ++(s->tape->N);
  ++(WVAR.i);
  update_ui ( s->tape, true );
  // --> Acaba for
  if ( WVAR.i < WVAR.min ) return true;
  WVAR.i= -1; // La pròxima vegada entra en el codi previ al for.
  
  // --> Finalitza do {} while
  if ( WVAR.remain ) return true;
  else
    {
      update_ui ( s->tape, false );
      return false;
    }
  
 error:
  fclose ( s->tape->f );
  s->tape->f= NULL;
  reset_ui_state ( s->tape );
  warning ( "Mentre s'escrivia en el fitxer associat a la cinta"
            " magnètica %d s'ha"
            " produït el següent error: %s",
            TAPEID(s->tape)+1, strerror ( errno ) );
  return false;
  
} // end op_write


static bool
op_func (
         void *data
         )
{
  
  op_state_t *s;
  bool ret;
  
  
  s= OP_STATE(data);
  if ( s->start ) update_ui ( s->tape, true );
  switch ( s->type )
    {
    case READ: ret= op_read ( s ); break;
    case REWIND: ret= op_rewind ( s ); break;
    case BACKWARD: ret= op_backward ( s ); break;
    case FORWARD: ret= op_forward ( s ); break;
    case WRITE: ret= op_write ( s ); break;
    default: ret= false;
    }
  if ( ret == false )
    {
      s->timeout_id= -1;
      update_ui ( s->tape, false );
    }
  
  return ret;
  
} // end op_func




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
mt_init (
         ui_sim_state_t *ui_state
         )
{
  
  int i;
  

  _ui_state= ui_state;
  for ( i= 0; i < 8; ++i )
    {
      _tapes[i].f= NULL;
      _op_states[i].timeout_id= 0;
      _op_states[i].tape= &_tapes[i];
      _ui_state->tapes[i].N= 0;
      _ui_state->tapes[i].pos= 0;
      _ui_state->tapes[i].busy= false;
    }
  
} // end mt_init


void
mt_close (void)
{
  
  int i;
  
  
  for ( i= 0; i < 8; ++i )
    {
      if ( _op_states[i].timeout_id != -1 )
        ui_remove_callback ( _op_states[i].timeout_id );
      if ( _tapes[i].f != NULL )
        fclose ( _tapes[i].f );
    }
  
} // end mt_close


void
mt_read (
	 const int     tape,
	 MIX_IOOPWord *op
	 )
{

  op_state_t *state;


  assert ( _op_states[tape].timeout_id == -1 );

  state= &_op_states[tape];
  state->v.r.op= op;
  state->type= READ;
  state->start= true;
  state->timeout_id=
    ui_register_callback ( op_func, state, SLEEP_TIME );
  
} // end mt_read


void
mt_rewind (
	   const int tape
	   )
{
  
  op_state_t *state;
  
  
  assert ( _op_states[tape].timeout_id == -1 );

  state= &_op_states[tape];
  state->type= REWIND;
  state->start= true;
  state->timeout_id=
    ui_register_callback ( op_func, state, SLEEP_TIME );
  
} // end mt_rewind


void
mt_skip_backward (
		  const int tape,
		  const int nwords
		  )
{

  op_state_t *state;


  assert ( _op_states[tape].timeout_id == -1 );

  state= &_op_states[tape];
  state->type= BACKWARD;
  state->start= true;
  state->v.b.nwords= nwords;
  state->timeout_id=
    ui_register_callback ( op_func, state, SLEEP_TIME );
  
} // end mt_skip_backward


void
mt_skip_forward (
		 const int tape,
		 const int nwords
		 )
{

  op_state_t *state;


  assert ( _op_states[tape].timeout_id == -1 );

  state= &_op_states[tape];
  state->type= FORWARD;
  state->start= true;
  state->v.f.nwords= nwords;
  state->timeout_id=
    ui_register_callback ( op_func, state, SLEEP_TIME );
  
} // end mt_skip_forward


void
mt_stop (void)
{
  
  int i;
  
  
  for ( i= 0; i < 8; ++i )
    {
      if ( _op_states[i].timeout_id != -1 )
        {
          ui_remove_callback ( _op_states[i].timeout_id );
          // Casos especials.
          switch ( _op_states[i].type )
            {
            case REWIND:
              _op_states[i].tape->N= 0;
              break;
            case BACKWARD:
              _op_states[i].tape->N-= _op_states[i].v.b.nwords;
              break;
            case FORWARD:
              _op_states[i].tape->N+= _op_states[i].v.f.nwords;
              break;
            default: break;
            }
          update_ui ( _op_states[i].tape, false );
        }
      _op_states[i].timeout_id= -1;
    }
  
} // end mt_stop


void
mt_write (
	  const int     tape,
	  MIX_IOOPWord *op
	  )
{

  op_state_t *state;


  assert ( _op_states[tape].timeout_id == -1 );

  state= &_op_states[tape];
  state->v.w.op= op;
  state->type= WRITE;
  state->start= true;
  state->timeout_id=
    ui_register_callback ( op_func, state, SLEEP_TIME );
  
} // end mt_write


MIX_Bool
mt_busy (
         const int tape
         )
{
  return _op_states[tape].timeout_id!=-1 ? MIX_TRUE : MIX_FALSE;
} // end mt_busy


void
mt_action_load_file (
                     ui_element_t *e,
                     void         *udata
                     )
{

  ui_open_dialog_t *d;
  const gchar *fn;
  int tape;
  struct stat info;
  

  // Prepara
  d= UI_OPEN_DIALOG(e);
  fn= ui_open_dialog_get_current_file ( d );
  tape= (int) (udata-NULL);

  // Tanca cinta anterior
  if ( _tapes[tape].f != NULL )
    {
      fclose ( _tapes[tape].f );
      _tapes[tape].f= NULL;
      _tapes[tape].nwords= 0;
      _tapes[tape].N= 0;
      _ui_state->tapes[tape].pos= 0;
      _ui_state->tapes[tape].N= 0;
      _ui_state->tapes[tape].busy= false;
    }
  
  // Obri  
  g_stat ( fn, &info );
  if ( info.st_size == 0 || info.st_size%(BLOCKSIZE*4) != 0 )
    {
      warning ( "La grandària del fitxer '"
                "%s' no és propia d'una cinta magnètica", fn );
      goto error_format;
    }
  // Inicialitza el canal.
  _tapes[tape].nwords= info.st_size/4;
  _tapes[tape].N= 0;
  _tapes[tape].f= fopen ( fn, "r+b" );
  if ( _tapes[tape].f == NULL )
    {
      warning ( "El següent error s'ha produït al intentar obrir"
                " el fitxer '%s': %s",
                fn, strerror ( errno ) );
      _tapes[tape].nwords= 0;
      _tapes[tape].N= 0;
      goto error_noopen;
    }
  _ui_state->tapes[tape].pos= 0;
  _ui_state->tapes[tape].N= _tapes[tape].nwords;
  _ui_state->tapes[tape].busy= false;
  ui_open_mt_set_visible ( false, tape );
  
  return;

 error_format:
  ui_error ( "Format de cinta incorrecte" );
  return;
 error_noopen:
  ui_error ( "No s'ha pogut obrir el fitxer" );
  return;
  
} // end mt_action_load_file


void
mt_action_clear (
                 ui_element_t *e,
                 void         *udata
                 )
{

  int tape;


  tape= (int) (udata-NULL);
  
  // Tanca cinta anterior si cal
  if ( _tapes[tape].f != NULL )
    {
      fclose ( _tapes[tape].f );
      _tapes[tape].f= NULL;
      _tapes[tape].nwords= 0;
      _tapes[tape].N= 0;
    }
  _ui_state->tapes[tape].pos= 0;
  _ui_state->tapes[tape].N= 0;
  _ui_state->tapes[tape].busy= false;
  ui_set_focus_line_printer ();
  
} // end mt_action_clear


void
mt_action_rewind (
                  ui_element_t *e,
                  void         *udata
                  )
{

  int tape;


  tape= (int) (udata-NULL);

  // Rebobina si hi ha fitxer
  if ( _tapes[tape].f != NULL )
    {
      rewind ( _tapes[tape].f );
      _tapes[tape].N= 0;
      _ui_state->tapes[tape].pos= 0;
    }
  ui_set_focus_line_printer ();
  
} // end mt_action_rewind
