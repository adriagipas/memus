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
 *  ui.c - Implementació de 'ui.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "cardpunch.h"
#include "cardreader.h"
#include "conf.h"
#include "error.h"
#include "frontend.h"
#include "lineprinter.h"
#include "magnetictapes.h"
#include "screen.h"
#include "ui.h"
#include "ui_button.h"
#include "ui_input.h"
#include "ui_list.h"
#include "ui_lp.h"
#include "ui_menu.h"
#include "ui_menu_bar.h"
#include "ui_error_dialog.h"
#include "ui_open_dialog.h"
#include "ui_save_dialog.h"
#include "ui_scrollbar.h"
#include "ui_status.h"




/*********/
/* TIPUS */
/*********/

typedef struct
{

  // Llista
  int next,prev;

  // Estat
  ui_callback_t *f;
  void          *udata;
  gint64         microsecs;
  gint64         waiting; // microsecs esperant
  gint64         last_t;
  bool           started;
  bool           running;
  
} callback_node_t;




/*********/
/* ESTAT */
/*********/

// Framebuffer
static int _fb[SCREEN_WIDTH*SCREEN_HEIGHT];

// Callbacks
static struct
{
  callback_node_t *v;
  size_t           size;
  int              fnodes; // Llista de nodes disponibles.
  int              anodes; // Llista de nodes actius.
  gulong           global_wait;
} _cb;

// Elements UI
static ui_list_t *_root;
static ui_open_dialog_t *_open_cr;
static ui_error_dialog_t *_error;
static ui_lp_t *_lp;
static ui_save_dialog_t *_save_lp;
static ui_open_dialog_t *_open_mt;
static ui_save_dialog_t *_save_cp;

// Estat simulador
static ui_sim_state_t *_sim_state;

// Coses de la ui
static bool _stop;
static struct
{

  // file_open
  struct
  {
    ui_menu_t *menu;
    int        cr;
    int        tape[8];
  } open;
  struct
  {
    ui_menu_t *menu;
    int        lp;
    int        cp;
  } save;
  struct
  {
    ui_menu_t *menu;
    int        run;
    int        stop;
    int        clear;
    int        rewind;
  } sim;
  struct
  {
    ui_menu_t *menu;
    int        lp;
    int        cr;
    int        cp;
    int        tape[8];
  } clear;
  struct
  {
    ui_menu_t *menu;
    int        cr;
    int        tape[8];
  } rewind;

} _actions;

static int _set_focus_lineprinter;



/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
cb_free (void)
{
  
  if ( _cb.v != NULL )
    g_free ( _cb.v );
  _cb.v= NULL;
  _cb.anodes= -1;
  _cb.fnodes= -1;
  
} // end cb_free


static void
cb_init (void)
{
  
  assert ( _cb.v == NULL );
  
  _cb.size= 1;
  _cb.v= g_new ( callback_node_t, 1 );
  _cb.v[0].next= -1;
  _cb.v[0].prev= -1;
  _cb.v[0].running= false;
  _cb.v[0].started= false;
  _cb.fnodes= 0;
  _cb.anodes= -1;
  
} // end cb_init


static void
cb_unlink_node (
                const int  node_id,
                int       *list_head
                )
{

  callback_node_t *node;


  node= &(_cb.v[node_id]);
  if ( node->next != -1 )
    _cb.v[node->next].prev= node->prev;
  if ( node->prev != -1 )
    _cb.v[node->prev].next= node->next;
  else
    *list_head= node->next;
  node->next= -1;
  node->prev= -1;
  
} // end cb_unlink_node


static void
cb_add_node (
             const int  node_id,
             int       *list_head
             )
{

  if ( *list_head != -1 )
    _cb.v[*list_head].prev= node_id;
  _cb.v[node_id].next= *list_head;
  _cb.v[node_id].prev= -1;
  *list_head= node_id;
  
} // end cb_add_node


static void
cb_free_node (
              const int node_id
              )
{

  cb_unlink_node ( node_id, &_cb.anodes );
  cb_add_node ( node_id, &_cb.fnodes );
  _cb.v[node_id].running= false;
  
} // end gui_cb_free_node


static int
cb_get_new_node (void)
{
  
  size_t nsize,p;
  int ret;
  
  
  // Reserva més memòria.
  if ( _cb.fnodes == -1 )
    {
      nsize= _cb.size*2;
      if ( nsize <= _cb.size ||
           nsize > (size_t) INT_MAX )
        goto error_mem;
      _cb.v= g_realloc ( _cb.v, sizeof(callback_node_t)*nsize );
      for ( p= nsize-1; p >= _cb.size; --p )
        {
          _cb.v[p].next= -1;
          _cb.v[p].prev= -1;
          _cb.v[p].running= false;
          cb_add_node ( (int) p, &(_cb.fnodes) );
        }
      _cb.size= nsize;
    }

  // Torna el node nou
  ret= _cb.fnodes;
  cb_unlink_node ( ret, &(_cb.fnodes) );
  
  return ret;
  
 error_mem:
  error ( "cannot allocate memory" );
  return -1;
  
} // end cb_get_new_node


// Torna el node_id
static int
cb_register (
             ui_callback_t *callback,
             void          *user_data,
             const gulong   microsecs
             )
{
  
  int node_id;
  gint64 wait;
  
  
  // Comprova microsecs
  if ( microsecs == 0 )
    error ( "cb_register - s'ha intentat registrar un"
            " callback amb 0 microsecs" );
  
  // Obté node.
  node_id= cb_get_new_node ();
  
  // Fica actiu i ompli.
  wait= (gint64) microsecs;
  if ( wait < _cb.global_wait )
    _cb.global_wait= wait;
  cb_add_node ( node_id, &_cb.anodes );
  _cb.v[node_id].f= callback;
  _cb.v[node_id].udata= user_data;
  _cb.v[node_id].microsecs= wait;
  _cb.v[node_id].running= true;
  _cb.v[node_id].started= false;
  
  return node_id;
  
} // end cb_register


static void
cb_remove (
           const int node_id
           )
{

  if ( node_id < 0 || (size_t) node_id >= _cb.size )
    error ( "%d is not a valid identifier for a callback",
            node_id );
  if ( _cb.v[node_id].running )
    cb_free_node ( node_id );
  
} // end cb_remove


// Torna els microsecs que cal esperar
static gulong
cb_run (
        const gulong microsecs
        )
{
  
  int p,node_id;
  gint64 t1;
  bool ok;
  
  
  _cb.global_wait= (gint64) microsecs;
  p= _cb.anodes;
  while ( p != -1 )
    {
      
      // Itera
      node_id= p;
      p= _cb.v[p].next;
      
      // Processa
      // --> Primera vegada
      if ( !_cb.v[node_id].started )
        {
          _cb.v[node_id].last_t= g_get_monotonic_time ();
          _cb.v[node_id].waiting= _cb.v[node_id].microsecs;
          if ( _cb.v[node_id].waiting < _cb.global_wait )
            _cb.global_wait= _cb.v[node_id].waiting;
          _cb.v[node_id].started= true;
        }
      // --> Ja està en execució
      else
        {
          ok= true;
          t1= g_get_monotonic_time ();
          _cb.v[node_id].waiting-= (t1-_cb.v[node_id].last_t);
          _cb.v[node_id].last_t= t1;
          while ( ok && _cb.v[node_id].waiting <= 0 )
            {
              ok= _cb.v[node_id].f ( _cb.v[node_id].udata );
              _cb.v[node_id].waiting+= _cb.v[node_id].microsecs;
            }
          // remove no deuria fallar ací !!!!!
          if ( !ok ) cb_remove ( node_id );
          else if ( _cb.v[node_id].waiting < _cb.global_wait )
            _cb.global_wait= _cb.v[node_id].waiting;
        }
      
    }

  return _cb.global_wait;
  
} // end cb_run


static bool
global_key_event (
                  const SDL_Event *event,
                  void            *udata
                  )
{

  switch ( event->type )
    {
    case SDL_KEYDOWN:
      if ( event->key.keysym.sym == SDLK_q &&
           (event->key.keysym.mod&KMOD_CTRL)!=0 )
        {
          _stop= true;
          return true;
        }
      break;
    }

  return false;
  
} // end global_key_event


static void
cb_file_quit (
              ui_element_t *e,
              void         *udata
              )
{
  _stop= true;
} // end cb_file_quit


static void
cb_file_open_cr (
                 ui_element_t *e,
                 void         *udata
                 )
{
  ui_open_cr_set_visible ( true );
} // end cb_file_open_cr


static void
cb_file_open_mt (
                 ui_element_t *e,
                 void         *udata
                 )
{
  ui_open_mt_set_visible ( true, (int) (udata-NULL) );
} // end cb_file_open_mt


static void
cb_file_save_lp (
                 ui_element_t *e,
                 void         *udata
                 )
{
  ui_save_lp_set_visible ( true );
} // end cb_file_save_lp


static void
cb_file_save_cp (
                 ui_element_t *e,
                 void         *udata
                 )
{
  ui_save_cp_set_visible ( true );
} // end cb_file_save_cp


static bool
callback_set_focus_lineprinter (
                                void *user_data
                                )
{
  
  ui_list_set_focus_begin ( _root );
  _set_focus_lineprinter= -1;
  
  return false;
  
} // end callback_set_focus_lineprinter


static void
update_actions (void)
{

  int i;


  // Depenent de si està en execució o no
  if ( _sim_state->running )
    {
      ui_menu_enable_entry ( _actions.open.menu, _actions.open.cr, false );
      for ( i= 0; i < 8; ++i )
        ui_menu_enable_entry ( _actions.open.menu,
                               _actions.open.tape[i], false );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.run, false );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.stop, true );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.clear, false );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.rewind, false );
    }
  else
    {
      ui_menu_enable_entry ( _actions.open.menu, _actions.open.cr, true );
      for ( i= 0; i < 8; ++i )
        ui_menu_enable_entry ( _actions.open.menu,
                               _actions.open.tape[i], true );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.run,
                             _sim_state->cr.N_read < _sim_state->cr.N );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.stop, false );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.clear, true );
      ui_menu_enable_entry ( _actions.sim.menu, _actions.sim.rewind, true );
    }

  // Depenent dels valors
  if ( _sim_state->lp.N == 0 )
    {
      ui_menu_enable_entry ( _actions.save.menu, _actions.save.lp, false );
      ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.lp, false );
    }
  else
    {
      ui_menu_enable_entry ( _actions.save.menu, _actions.save.lp, true );
      ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.lp, true );
    }
  if ( _sim_state->cp.N == 0 )
    {
      ui_menu_enable_entry ( _actions.save.menu, _actions.save.cp, false );
      ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.cp, false );
    }
  else
    {
      ui_menu_enable_entry ( _actions.save.menu, _actions.save.cp, true );
      ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.cp, true );
    }
  if ( _sim_state->cr.N == 0 )
    ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.cr, false );
  else
    ui_menu_enable_entry ( _actions.clear.menu, _actions.clear.cr, true );
  if ( _sim_state->cr.N_read == 0 )
    ui_menu_enable_entry ( _actions.rewind.menu, _actions.rewind.cr, false );
  else
    ui_menu_enable_entry ( _actions.rewind.menu, _actions.rewind.cr, true );
  for ( i= 0; i < 8; ++i )
    {
      if ( _sim_state->tapes[i].N == 0 )
        ui_menu_enable_entry ( _actions.clear.menu,
                               _actions.clear.tape[i], false );
      else
        ui_menu_enable_entry ( _actions.clear.menu,
                               _actions.clear.tape[i], true );
      if ( _sim_state->tapes[i].pos == 0 )
        ui_menu_enable_entry ( _actions.rewind.menu,
                               _actions.rewind.tape[i], false );
      else
        ui_menu_enable_entry ( _actions.rewind.menu,
                               _actions.rewind.tape[i], true );
    }
  
} // end update_actions


static void
draw (void)
{

  // Actualitza estat accions
  update_actions ();
  
  // Redibuixa
  // --> Fons a blanc
  memset ( _fb, 0, sizeof(_fb) );
  // --> Elements
  ui_element_draw ( _root, _fb, SCREEN_WIDTH, true );
  
  // Actualitza
  screen_update ( _fb );
  
} // end draw




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_ui (void)
{

  if ( _set_focus_lineprinter != -1 )
      ui_remove_callback ( _set_focus_lineprinter );
  ui_element_free ( _root );
  cb_free ();
  
} // end close_ui


void
ui_run (void)
{

  SDL_Event event;
  gulong sleep;
  
  
  _stop= false;
  while ( !_stop )
    {

      // Gestiona events.
      while ( screen_next_event ( &event ) )
        switch ( event.type )
          {
          case SDL_QUIT:
            _stop= true;
            break;
          case SDL_MOUSEMOTION:
          case SDL_MOUSEBUTTONDOWN:
          case SDL_MOUSEBUTTONUP:
          case SDL_MOUSEWHEEL:
            ui_element_mouse_event ( _root, &event );
            break;
          case SDL_KEYDOWN:
          case SDL_KEYUP:
            ui_element_key_event ( _root, &event );
            break;
          default: break;
          }

      // Redibuixa i dorm.
      draw ();
      sleep= cb_run ( 10000 );
      g_usleep ( sleep );
      
    }
  
} // end ui_run


void
init_ui (
         conf_t         *conf,
         ui_sim_state_t *sim_state,
         const gchar    *cdir_cr,
         const gchar    *cdir_lp,
         const gchar    *cdir_mt,
         const gchar    *cdir_cp
         )
{

  int i;
  char buf[100];
  

  _sim_state= sim_state;
  
  cb_init ();
  
  // Inicialitza els elements de la UI
  _root= ui_list_new ( NULL, global_key_event, NULL );

  // Crea l'element status.
  ui_status_t *status= ui_status_new ( sim_state );
  ui_list_add ( _root, UI_ELEMENT(status) );
  
  // Crea l'element impressora.
  _lp= ui_lp_new ( sim_state, 3, 16+18 );
  ui_list_add ( _root, UI_ELEMENT(_lp) );
  
  // Crea el menú principal
  ui_menu_bar_t *bar= ui_menu_bar_new ();
  ui_list_add ( _root, UI_ELEMENT(bar) );
  // --> Menú fitxer
  ui_menu_t *m_file= ui_menu_new ( 0, 0 );
  ui_menu_bar_add_entry ( bar, "Fitxer", m_file );
  ui_menu_t *m_file_open= ui_menu_new ( 0, 0 );
  _actions.open.menu= m_file_open;
  _actions.open.cr=
    ui_menu_add_entry_action ( m_file_open, "Targetes perforades",
                               cb_file_open_cr, NULL );
  for ( i= 0; i < 8; ++i )
    {
      sprintf ( buf, "Cinta magn\x8atica %d", i+1 );
      _actions.open.tape[i]=
        ui_menu_add_entry_action ( m_file_open, buf,
                                   cb_file_open_mt, NULL+i );
    }
  ui_menu_add_entry_submenu ( m_file, "Obre", m_file_open );
  ui_menu_t *m_file_save= ui_menu_new ( 0, 0 );
  _actions.save.menu= m_file_save;
  _actions.save.lp=
    ui_menu_add_entry_action ( m_file_save, "Impressora",
                               cb_file_save_lp, NULL );
  _actions.save.cp=
    ui_menu_add_entry_action ( m_file_save, "Targetes perforades",
                               cb_file_save_cp, NULL );
  ui_menu_add_entry_submenu ( m_file, "Desa", m_file_save );
  ui_menu_add_entry_separator ( m_file );
  ui_menu_add_entry_action ( m_file, "Surt", cb_file_quit, NULL );
  // --> Menú simulador
  ui_menu_t *m_sim= ui_menu_new ( 0, 0 );
  ui_menu_bar_add_entry ( bar, "Simulador", m_sim );
  _actions.sim.menu= m_sim;
  _actions.sim.run=
    ui_menu_add_entry_action ( m_sim, "Engega", frontend_action_run, NULL );
  _actions.sim.stop=
    ui_menu_add_entry_action ( m_sim, "Atura", frontend_action_stop, NULL );
  ui_menu_add_entry_separator ( m_sim );
  ui_menu_t *m_sim_clear= ui_menu_new ( 0, 0 );
  _actions.clear.menu= m_sim_clear;
  _actions.clear.lp=
    ui_menu_add_entry_action ( m_sim_clear, "Impressora",
                               lp_action_clear, NULL );
  _actions.clear.cr=
    ui_menu_add_entry_action ( m_sim_clear, "Lector de targetes",
                               cr_action_clear, NULL );
  _actions.clear.cp=
    ui_menu_add_entry_action ( m_sim_clear, "Perforadora de targetes",
                               cp_action_clear, NULL );
  for ( i= 0; i < 8; ++i )
    {
      sprintf ( buf, "Cinta magn\x8atica %d", i+1 );
      _actions.clear.tape[i]=
        ui_menu_add_entry_action ( m_sim_clear, buf,
                                   mt_action_clear, NULL+i );
    }
  _actions.sim.clear=
    ui_menu_add_entry_submenu ( m_sim, "Buida", m_sim_clear );
  ui_menu_t *m_sim_rewind= ui_menu_new ( 0, 0 );
  _actions.rewind.menu= m_sim_rewind;
  _actions.rewind.cr=
    ui_menu_add_entry_action ( m_sim_rewind, "Lector de targetes",
                               cr_action_rewind, NULL );
  for ( i= 0; i < 8; ++i )
    {
      sprintf ( buf, "Cinta magn\x8atica %d", i+1 );
      _actions.rewind.tape[i]=
        ui_menu_add_entry_action ( m_sim_rewind, buf,
                                   mt_action_rewind, NULL+i );
    }
  _actions.sim.rewind=
    ui_menu_add_entry_submenu ( m_sim, "Rebobina", m_sim_rewind );
  // --> Menú visualització
  ui_menu_t *m_view= ui_menu_new ( 0, 0 );
  ui_menu_bar_add_entry ( bar, "Visualitza", m_view );
  ui_menu_add_entry_choice ( m_view, "Finestra x1", &(conf->screen_size),
                             SCREEN_SIZE_WIN_X1,
                             frontend_action_change_screen_size, NULL );
  ui_menu_add_entry_choice ( m_view, "Finestra x1.5", &(conf->screen_size),
                             SCREEN_SIZE_WIN_X1_5,
                             frontend_action_change_screen_size, NULL );
  ui_menu_add_entry_choice ( m_view, "Finestra x2", &(conf->screen_size),
                             SCREEN_SIZE_WIN_X2,
                             frontend_action_change_screen_size, NULL );
  ui_menu_add_entry_choice ( m_view, "Pantalla completa", &(conf->screen_size),
                             SCREEN_SIZE_FULLSCREEN,
                             frontend_action_change_screen_size, NULL );

  // Crea el dialeg d'obrir targetes perforades.
  _open_cr= ui_open_dialog_new ( "Obre targetes perforades",
                                 cdir_cr, "[.](deck)$",
                                 false, cr_action_load_file, NULL );
  ui_element_set_visible ( _open_cr, false );
  ui_list_add ( _root, UI_ELEMENT(_open_cr) );
  
  // Crea el dialeg d'obrir cintest magnètiques.
  _open_mt= ui_open_dialog_new ( "Obre cinta magn\x8atica",
                                 cdir_mt, "[.](tape)[0-9]?$",
                                 false, mt_action_load_file, NULL );
  ui_element_set_visible ( _open_mt, false );
  ui_list_add ( _root, UI_ELEMENT(_open_mt) );
  
  // Crea el dialeg de desar la impressora.
  _save_lp= ui_save_dialog_new ( "Desa contingut impressora",
                                 cdir_lp, lp_action_save_content, NULL );
  ui_element_set_visible ( _save_lp, false );
  ui_list_add ( _root, UI_ELEMENT(_save_lp) );

  // Crea el dialeg de desar les targetes.
  _save_cp= ui_save_dialog_new ( "Desa targetes perforades",
                                 cdir_cp, cp_action_save, NULL );
  ui_element_set_visible ( _save_cp, false );
  ui_list_add ( _root, UI_ELEMENT(_save_cp) );
  
  // Dialeg error
  _error= ui_error_dialog_new ();
  ui_element_set_visible ( _error, false );
  ui_list_add ( _root, UI_ELEMENT(_error) );
    
  // Fixa focus
  ui_list_set_focus_begin ( _root );

  _set_focus_lineprinter= -1;
  
} // init_ui


void
ui_remove_callback (
                    const int callback_id
                    )
{
  cb_remove ( callback_id );
} // end ui_remove_callback


int
ui_register_callback (
                      ui_callback_t *callback,
                      void          *user_data,
                      const gulong   microsecs
                      )
{
  return cb_register ( callback, user_data, microsecs );
} // end ui_register_callback


const gchar *
ui_get_cdir_cr (void)
{
  return ui_open_dialog_get_current_dir ( _open_cr );
} // end ui_get_cdir_cr


const gchar *
ui_get_cdir_mt (void)
{
  return ui_open_dialog_get_current_dir ( _open_mt );
} // end ui_get_cdir_mt


const gchar *
ui_get_cdir_lp (void)
{
  return ui_save_dialog_get_current_dir ( _save_lp );
} // end ui_get_cdir_lp


const gchar *
ui_get_cdir_cp (void)
{
  return ui_save_dialog_get_current_dir ( _save_cp );
} // end ui_get_cdir_cp


void
ui_open_cr_set_visible (
                        const bool visible
                        )
{

  if ( visible )
    {
      ui_element_set_visible ( _open_cr, true );
      ui_list_set_focus_end ( _root );
    }
  else ui_element_set_visible ( _open_cr, false );
  
} // end ui_open_cr_set_visbile


void
ui_open_mt_set_visible (
                        const bool visible,
                        const int  tapeid
                        )
{

  if ( visible )
    {
      ui_open_dialog_set_user_data ( _open_mt, NULL+tapeid );
      ui_element_set_visible ( _open_mt, true );
      ui_list_set_focus_end ( _root );
    }
  else ui_element_set_visible ( _open_mt, false );
  
} // end ui_open_mt_set_visible


void
ui_save_lp_set_visible (
                        const bool visible
                        )
{

  if ( visible )
    {
      ui_element_set_visible ( _save_lp, true );
      ui_list_set_focus_end ( _root );
    }
  else ui_element_set_visible ( _save_lp, false );
  
} // end ui_save_lp_set_visible


void
ui_save_cp_set_visible (
                        const bool visible
                        )
{
  
  if ( visible )
    {
      ui_element_set_visible ( _save_cp, true );
      ui_list_set_focus_end ( _root );
    }
  else ui_element_set_visible ( _save_cp, false );
  
} // end ui_save_cp_set_visible


void
ui_error (
          const char *msg
          )
{

  ui_error_dialog_set_msg ( _error, msg );
  ui_element_set_visible ( _error, true );
  ui_list_set_focus_end ( _root );
  
} // end ui_error


void
ui_show_last_line_printer (void)
{
  ui_lp_show_last_line ( _lp );
} // end ui_show_last_list_printer


void
ui_set_focus_line_printer (void)
{

  if ( _set_focus_lineprinter == -1 )
    _set_focus_lineprinter=
      ui_register_callback ( callback_set_focus_lineprinter, NULL, 1000 );
  
} // end ui_set_focus_line_printer
