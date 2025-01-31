/*
 * Copyright 2013-2025 Adrià Giménez Pastor.
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
#include <SDL_image.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cd.h"
#include "error.h"
#include "frontend.h"
/*
#include "hud.h"
*/
#include "load_bios.h"
#include "memc.h"
#include "menu.h"
#include "pad.h"
#include "screen.h"
#include "sound.h"
/*
#include "state.h"
*/
#include "tiles8b.h"
#include "tiles16b.h"
#include "t8biso.h"




/*********/
/* ESTAT */
/*********/

// Verbositat.
static bool _verbose;

// Configuració
static conf_t *_conf;

// Per a indicar que es vol eixir.
static bool _quit;

// Per a indicar que des de el menú s'ha fet un reset.
static bool _reset;

// Frontend.
static PSX_Frontend _frontend;

// Renderer.
static PSX_Renderer *_renderer;

// Controllers. Punter a la configuración de conf.
static const PSX_Controller *_controllers;




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
               bool *stop,
               bool *reset,
               void *udata
               )
{
  
  SDL_Event event;
  
  
  *stop= false;
  *reset= _reset; _reset= false;
  while ( screen_next_event ( &event, NULL ) )
    switch ( event.type )
      {

      case SDL_QUIT:
        _quit= TRUE;
        *stop= true;
        return;

      case SDL_MOUSEBUTTONDOWN:
        if ( event.button.button == SDL_BUTTON_RIGHT )
          {
            *stop= true;
            return;
          }
        break;
        
        // Tecles ràpides. No hi ha BREAK APOSTA!!! D'aquesta manera
        // es passa al pad.
      case SDL_KEYDOWN:
        if ( event.key.keysym.mod&KMOD_CTRL )
          {
            switch ( event.key.keysym.sym )
              {
                /*
              case SDLK_1: save_state ( 1 ); break;
              case SDLK_2: save_state ( 2 ); break;
              case SDLK_3: save_state ( 3 ); break;
              case SDLK_4: save_state ( 4 ); break;
              case SDLK_5: save_state ( 5 ); break;
              case SDLK_s: take_screenshot (); break;
                */
              case SDLK_q:
                _quit= true;
                *stop= true;
                return;
              default: break;
              }
            break;
          }
        else if ( event.key.keysym.mod&KMOD_ALT )
          {
            switch ( event.key.keysym.sym )
              {
                /*
              case SDLK_1: load_state ( 1 ); break;
              case SDLK_2: load_state ( 2 ); break;
              case SDLK_3: load_state ( 3 ); break;
              case SDLK_4: load_state ( 4 ); break;
              case SDLK_5: load_state ( 5 ); break;
                */
              default: break;
              }
            break;
          }

      default:
        if ( pad_event ( &event ) )
          {
            *stop= true;
            return;
          }
        
      }
  
} // end check_signals


static void
loop (void)
{

  // Un valor un poc arreu. Si cada T és correspon amb un cicle de
  // rellotge que va a 33.87MHz approx, tenim que és comprova cada
  // 1/100 segons.
  static const int CCTOCHECK= 338700;
  
  const gint64 SLEEP= 1000;
  const gint64 MIN_DELAY= -10000;
  
  int cc,cc_iter;
  gint64 t0,tf,delay;
  bool stop;


  screen_enable_cursor ( true );
  stop= false;
  t0= g_get_monotonic_time ();
  cc_iter= (int) ((PSX_CYCLES_PER_SEC/1000000.0)*SLEEP + 0.5);
  cc= 0;
  delay= 0;
  for (;;)
    {
      
      // Executa.
      cc+= cc_iter;
      while ( cc > 0 )
        {
          cc-= PSX_iter ( CCTOCHECK, &stop );
          if ( stop ) return;
        }
      
      // Delay
      tf= g_get_monotonic_time ();
      delay+= SLEEP-(tf-t0); t0= tf;
      if ( delay >= SLEEP ) g_usleep ( SLEEP );
      else if ( delay < MIN_DELAY ) delay= MIN_DELAY;
      
    }
  
} // end loop




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_frontend (void)
{

  close_memc ();
  close_cd ();
  PSX_renderer_free ( _renderer );
  close_menu ();
  close_t8biso ();
  close_pad ();
  close_sound ();
  close_screen ();
  SDL_Quit ();
  
} // end close_frontend


menu_response_t
frontend_run (void)
{
  
  menu_response_t ret;
  const uint8_t *bios;
  
  
  ret= MENU_QUIT;
  _quit= _reset= false;

  // Carrega BIOS.
  bios= load_bios ( _conf, _verbose );
  if ( bios == NULL ) return MENU_QUIT;
  
  // Inicialitza Simulador.
  PSX_init ( bios, &_frontend, NULL, _renderer );
  PSX_plug_controllers ( _controllers[0], _controllers[1] );
  memc_replug ();
  
  // Executa loop.
  for (;;)
    {
      pad_clear ();
      loop ();
      if ( _quit ) { ret= MENU_QUIT; break; }
      ret= menu_run ();
      if ( ret == MENU_RESET ) _reset= TRUE;
      /*
      else if ( ret == MENU_REINIT )
        {
          close_sram ();
          close_eeprom ();
          model= model_get_val ();
          _ciclespersec= model&MD_MODEL_PAL ?
            MD_CICLES_PER_SEC_PAL : MD_CICLES_PER_SEC_NTSC;
          _frontend.plugged_devs= pad_get_devices (); // Guarda l'última
        					      //  configuració.
          MD_close ();
          MD_init ( rom, model, &_frontend, NULL );
          init_eeprom ( rom_id, eeprom_fn, verbose );
          init_sram ( rom_id, sram_fn, verbose );
        }
      */
      else if ( ret != MENU_RESUME ) break;
    }
  
  return ret;

} // end frontend_run


void
init_frontend (
               conf_t     *conf,
               const char *title,
               const bool  big_screen,
               const bool  verbose
               )
{
  
  int ret;
  

  _verbose= verbose;
  _conf= conf;
  
  // Inicialitza.
  ret= SDL_Init ( SDL_INIT_AUDIO|SDL_INIT_VIDEO|
                  SDL_INIT_JOYSTICK|SDL_INIT_EVENTS );
  if ( ret != 0 )
    error ( "no s'ha pogut inicialitzar SDL: %s", SDL_GetError () );
  SDL_DisableScreenSaver ();
  init_screen ( conf, title, big_screen );
  init_sound ();
  init_pad ( conf, NULL );
  init_tiles8b ();
  init_tiles16b ();
  init_t8biso ();
  init_menu ( conf, big_screen, verbose );
  init_cd ( verbose );
  init_memc ( conf, verbose );
  /*
  init_hud ();
  */
  
  // Inicialitza frontend.
  _frontend.warning= _warning;
  _frontend.check= check_signals;
  _frontend.play_sound= sound_play;
  _frontend.get_ctrl_state= pad_get_controller_state;
  _frontend.trace= NULL;

  // Renderer.
  _renderer= PSX_create_default_renderer ( screen_update, NULL );
  if ( _renderer == NULL )
    error ( "no s'ha pogut crear el PSX_Renderer" );

  // Controllers.
  _controllers= &(conf->controllers[0]);
  
} // end init_frontend
