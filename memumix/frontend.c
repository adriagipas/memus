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
 *  frontend.c - Implementació de 'frontend.h'
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cardpunch.h"
#include "cardreader.h"
#include "cdir.h"
#include "error.h"
#include "frontend.h"
#include "lineprinter.h"
#include "magnetictapes.h"
#include "screen.h"
#include "ui.h"
#include "vgafont.h"

#include "MIX.h"




/**********/
/* MACROS */
/**********/

#define WARNING(MSG) _warning ( NULL, (MSG) )

#define NUM_DEVICES 21




/*********/
/* TIPUS */
/*********/

typedef struct
{
  
  int    cc;
  gint64 last;
  int    timeout_id;
  
} loop_state_t;




/*********/
/* ESTAT */
/*********/

// Verbositat.
static bool _verbose;

// Configuració
static conf_t *_conf;

// Estat simulador per a UI
static ui_sim_state_t _ui_state;

// Per a forçar una aturada de la màquina.
static MIX_Bool _stop;

// Estat necessari en el loop
static loop_state_t _loop_state;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
_warning (
          void       *udata,
          const char *format,
          ...
          )
{
  
  va_list ap;
  
  
  va_start ( ap, format );
  vwarning ( format, ap );
  va_end ( ap );
  
} // end _warning


static void
check_signals (
               void     *udata,
               MIX_Bool *stop
               )
{
  *stop= _stop;
} // end check_signals


static void
init_ioopchar (
               void         *udata,
               MIX_Device    dev,
               MIX_IOOPChar *op,
               MIX_OPType    type
               )
{
  
  switch ( dev )
    {

    case MIX_CARDREADER:
      if ( type == MIX_OUT )
        WARNING ( "s'ha intentat escriure en el lector de targetes" );
      else cr_read ( op );
      break;
      
    case MIX_CARDPUNCH:
      if ( type == MIX_IN )
        WARNING ( "s'ha intentat llegir del perforador de targetes" );
      else cp_write ( op );
      break;

    case MIX_LINEPRINTER:
      if ( type == MIX_IN )
        WARNING ( "s'ha intentat llegir de la impressora de línies" );
      else lp_write ( op );
      break;
      
    default:
      _warning ( NULL,
                "el dispositiu %d no suporta"
                " operacions de caràcters", dev );
    }
  
} // end init_ioopchar


static void
init_ioopword (
               void         *udata,
               MIX_Device    dev,
               MIX_IOOPWord *op,
               MIX_OPType    type
               )
{
  
  switch ( dev )
    {
      
    case MIX_TAPEUNIT1:
    case MIX_TAPEUNIT2:
    case MIX_TAPEUNIT3:
    case MIX_TAPEUNIT4:
    case MIX_TAPEUNIT5:
    case MIX_TAPEUNIT6:
    case MIX_TAPEUNIT7:
    case MIX_TAPEUNIT8:
      if ( type == MIX_OUT ) mt_write ( dev-MIX_TAPEUNIT1, op );
      else                   mt_read ( dev-MIX_TAPEUNIT1, op );
      break;
      
    default:
      _warning ( NULL,
                 "el dispositiu %d no suporta"
                 " operacions de paraules", dev );
    }
  
} // end init_ioopword


static MIX_Bool
device_busy (
             void       *udata,
             MIX_Device  dev
             )
{
  
  MIX_Bool ret;

  
  switch ( dev )
    {
    case MIX_CARDREADER:  ret= cr_busy (); break;
    case MIX_CARDPUNCH:   ret= cp_busy (); break;
    case MIX_LINEPRINTER: ret= lp_busy (); break;
    case MIX_TAPEUNIT1:
    case MIX_TAPEUNIT2:
    case MIX_TAPEUNIT3:
    case MIX_TAPEUNIT4:
    case MIX_TAPEUNIT5:
    case MIX_TAPEUNIT6:
    case MIX_TAPEUNIT7:
    case MIX_TAPEUNIT8:   ret= mt_busy ( dev-MIX_TAPEUNIT1 ); break;
    default: ret= MIX_TRUE;
    }
  
  return ret;
  
} // end device_busy


static void
notify_waiting_device (
                       void             *udata,
                       const MIX_Device  dev,
                       const bool        waiting
                       )
{

  // NOTA!!! Crec que no cal implementar açò en la meua UI
  /*
  if ( waiting ) mainwindow_statusbar_push ( _wait_messages[dev] );
  else           mainwindow_statusbar_pop ();
  */
  
} // end notify_waiting_device


static void
io_control (
            void            *udata,
            MIX_IOControlOp  op,
            ...
            )
{
  
  int tape,nwords;
  va_list ap;
  
  
  va_start ( ap, op );
  switch ( op )
    {
    case MIX_LP_SKIPTOFOLLOWINGPAGE:
      lp_next_page ();
      break;
    case MIX_MT_REWOUND:
      tape= va_arg ( ap, int );
      mt_rewind ( tape );
      break;
    case MIX_MT_SKIPBACKWARD:
      tape= va_arg ( ap, int );
      nwords= va_arg ( ap, int );
      mt_skip_backward ( tape, nwords );
      break;
    case MIX_MT_SKIPFORWARD:
      tape= va_arg ( ap, int );
      nwords= va_arg ( ap, int );
      mt_skip_forward ( tape, nwords );
      break;
    default: _warning ( NULL, "operació de control I/O %d no suportada", op );
    }
  va_end ( ap );
  
} // end io_control


static bool
loop (
      void *user_data
      )
{

  // MIX no té una freqüència, però tenint en compte la seua
  // antiguitat crec que 1MHz ja és molt.
  const int CYCLES_PER_SEC= 1000000;
  
  // Un valor un poc arreu. Si cada T és correspon amb un cicle de
  // rellotge que va a 1MHz, tenim que és comprova cada
  // 1/100 segons.
  static const int CCTOCHECK= 10000;

  
  gint64 t0;
  MIX_Bool halt;
  

  // Temps i casos especials.
  if ( _loop_state.last == 0 )
    {
      _loop_state.last= g_get_monotonic_time ();
      return true;
    }
  t0= g_get_monotonic_time ();
  if ( t0 <= _loop_state.last ) { _loop_state.last= t0; return true; }

  // Executa
  halt= MIX_FALSE;
  _loop_state.cc+=
    (int) ((CYCLES_PER_SEC/1000000.0)*(t0-_loop_state.last) + 0.5);
  while ( _loop_state.cc > 0 )
    {
      _loop_state.cc-= MIX_iter ( CCTOCHECK, &halt );
      if ( halt )
        {
          _loop_state.timeout_id= -1;
          _ui_state.running= false;
          return false;
        }
    }
  
  // Actualitza.
  _loop_state.last= t0;

  return true;
  
} // end loop        




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_frontend (void)
{

  const gchar *cdir;

  
  // Força aturada
  frontend_action_stop ( NULL, NULL );
  
  // Elimina timeout.
  if ( _loop_state.timeout_id != -1 )
    ui_remove_callback ( _loop_state.timeout_id );

  // Tanca els dispositius.
  lp_close ();
  cp_close ();
  cr_close ();
  mt_close ();
  
  // Desa current dirs
  cdir= ui_get_cdir_cr ();
  if ( cdir != NULL ) cdir_write ( "cr", cdir, _verbose );
  cdir= ui_get_cdir_mt ();
  if ( cdir != NULL ) cdir_write ( "mt", cdir, _verbose );
  cdir= ui_get_cdir_lp ();
  if ( cdir != NULL ) cdir_write ( "lp", cdir, _verbose );
  cdir= ui_get_cdir_cp ();
  if ( cdir != NULL ) cdir_write ( "cp", cdir, _verbose );
  
  // Tanca moduls
  close_ui ();
  close_vgafont ();
  close_screen ();
  SDL_Quit ();
  
} // end close_frontend


void
frontend_run (void)
{
  ui_run ();
} // end frontend_run


void
init_frontend (
               conf_t     *conf,
               const bool  verbose
               )
{

  static const MIX_Frontend frontend=
    {
      _warning,
      check_signals,
      init_ioopchar,
      init_ioopword,
      device_busy,
      io_control,
      notify_waiting_device
    };
  
  int ret;
  gchar *cdir_cr,*cdir_lp,*cdir_mt,*cdir_cp;
  
  
  _verbose= verbose;
  _conf= conf;
  
  // Inicialitza.
  ret= SDL_Init ( SDL_INIT_VIDEO|SDL_INIT_EVENTS );
  if ( ret != 0 )
    error ( "no s'ha pogut inicialitzar SDL: %s", SDL_GetError () );
  SDL_DisableScreenSaver ();
  init_screen ( conf );
  init_vgafont ();

  // Inicialitza frontend.
  _stop= MIX_FALSE;
  _loop_state.timeout_id= -1;
  _loop_state.last= 0;
  _ui_state.running= false;
  
  // Inicialitza els dispositius.
  mt_init ( &_ui_state );
  cp_init ( &_ui_state );
  cr_init ( &_ui_state );
  lp_init ( &_ui_state );
  
  // Inicialitza el simulador.
  MIX_init ( &frontend, NULL );

  // Inicialitza ui
  cdir_cr= cdir_read ( "cr", verbose );
  cdir_lp= cdir_read ( "lp", verbose );
  cdir_mt= cdir_read ( "mt", verbose );
  cdir_cp= cdir_read ( "cp", verbose );
  init_ui ( conf, &_ui_state, cdir_cr, cdir_lp, cdir_mt, cdir_cp );
  g_free ( cdir_cr );
  g_free ( cdir_lp );
  g_free ( cdir_mt );
  g_free ( cdir_cp );
  
} // end init_frontend


void
frontend_action_change_screen_size (
                                    ui_element_t *e,
                                    void         *udata
                                    )
{
  screen_change_size ( _conf->screen_size );
} // end frontend_action_change_screen_size


void
frontend_action_stop (
                      ui_element_t *e,
                      void         *udata
                      )
{

  // Indique al loop que vull parar generant la senyal stop. NOTA!! La
  // parada no es produeix immediatament.
  _stop= MIX_TRUE;
  
  // Força l'aturada immediata dels diferents dispositius.
  lp_stop ();
  cr_stop ();
  cp_stop ();
  mt_stop ();
  
  // Torna el focus
  ui_set_focus_line_printer ();
  
} // end frontend_action_stop


void
frontend_action_run (
                     ui_element_t *e,
                     void         *udata
                     )
{

  if ( _loop_state.timeout_id != -1 )
    {
      warning ( "el simulador ja es troba en execució" );
      return;
    }
  
  // Prepara gui.
  _ui_state.running= true;
  
  // Prepara estat.
  _stop= MIX_FALSE;
  MIX_go ();

  // Força l'aturada de dispositius que puguen estar encara fent
  // coses.
  lp_stop ();
  cr_stop ();
  cp_stop ();
  mt_stop ();
  
  // Llança
  _loop_state.cc= 0;
  _loop_state.last= 0;
  _loop_state.timeout_id=
    ui_register_callback ( loop, NULL, 1000 );

  // Torna el focus
  ui_set_focus_line_printer ();
  
} // end frontend_action_run
