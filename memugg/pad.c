/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  pad.c - Implementació de 'pad.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "GG.h"
#include "pad.h"




/*********/
/* ESTAT */
/*********/

/* Estar del joystick. */
static struct
{
  SDL_Joystick *dev;
  int           index;
  int           type;
} _joy;

/* Tecles. */
static const keys_t *_keys;

/* Estat dels botons segons el teclat. */
static int _kbuttons;

/* Estat dels botons del joypad. */
static struct
{
  
  int buttons;
  int stick;
  
} _jbuttons;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

static void
try_open_joy (void)
{
  
  int num, i;
  const char *name;
  
  
  assert ( _joy.dev == NULL );
  num= SDL_NumJoysticks ();
  for ( i= 0; i < num; ++i )
    {
      
      /* Comprova el nom. */
      name= SDL_JoystickNameForIndex ( i );
      if ( name == NULL )
        {
          warning ( "no s'ha pogut obtenir el nom del joystick %d: %s",
        	    i, SDL_GetError () );
          continue;
        }
      if ( !strcmp ( name, "Xbox 360 Controller" ) ||
           !strcmp ( name, "Microsoft X-Box 360 pad" ) )
        _joy.type= PAD_JOY_XBOX360;
      else
        {
          warning ( "model de joystick no suportat: %s", name );
          continue;
        }
      
      /* Intenta obrir. */
      _joy.dev= SDL_JoystickOpen ( i );
      if ( _joy.dev == NULL )
        {
          warning ( "no s'ha pogut obrir el joystick %d: %s",
        	    i, SDL_GetError () );
          return;
        }
      
      /* Obté l'index. */
      _joy.index= (int) SDL_JoystickInstanceID ( _joy.dev );
      if ( _joy.index < 0 )
        {
          warning ( "no s'ha pogut obtenir l'identificador del joystick %d: %s",
        	    i, SDL_GetError () );
          SDL_JoystickClose ( _joy.dev );
          _joy.dev= NULL;
          return;
        }
      
    }
  
} /* end try_open_joy */


static void
close_joy (void)
{
  
  if ( _joy.dev != NULL && SDL_JoystickGetAttached ( _joy.dev ) )
    SDL_JoystickClose ( _joy.dev );
  _joy.dev= NULL;
  
} /* end close_joy */


static void
process_joy_event_xbox360 (
        		   const SDL_Event *event,
        		   gboolean        *esc
        		   )
{
  
  switch ( event->type )
    {
      
      case SDL_JOYAXISMOTION:
        switch ( event->jaxis.axis )
          {
          case 0: /* LEFT_RIGHT */
            if ( event->jaxis.value < -10000 )
              { _jbuttons.stick|= GG_LEFT; _jbuttons.stick&= ~GG_RIGHT; }
            else if ( event->jaxis.value > 10000 )
              { _jbuttons.stick&= ~GG_LEFT; _jbuttons.stick|= GG_RIGHT; }
            else
              { _jbuttons.stick&= ~GG_LEFT; _jbuttons.stick&= ~GG_RIGHT; }
            break;
          case 1: /* UP_DOWN */
            if ( event->jaxis.value < -10000 )
              { _jbuttons.stick|= GG_UP; _jbuttons.stick&= ~GG_DOWN; }
            else if ( event->jaxis.value > 10000 )
              { _jbuttons.stick&= ~GG_UP; _jbuttons.stick|= GG_DOWN; }
            else
              { _jbuttons.stick&= ~GG_UP; _jbuttons.stick&= ~GG_DOWN; }
            break;
          }
        break;
        
    case SDL_JOYBUTTONDOWN:
      switch ( event->jbutton.button )
        {
        case 0: _jbuttons.buttons|= GG_TL; break;
        case 1: _jbuttons.buttons|= GG_TR; break;
        case 7: _jbuttons.buttons|= GG_START; break;
        case 8: *esc= TRUE; break;
        default: break;
        }
      break;
      
    case SDL_JOYBUTTONUP:
      switch ( event->jbutton.button )
        {
        case 0: _jbuttons.buttons&= ~GG_TL; break;
        case 1: _jbuttons.buttons&= ~GG_TR; break;
        case 7: _jbuttons.buttons&= ~GG_START; break;
        default: break;
        }
      break;
      
    case SDL_JOYHATMOTION:
      if ( event->jhat.hat != 0 ) break;
      if ( event->jhat.value&0x1 ) _jbuttons.buttons|= GG_UP;
      else                         _jbuttons.buttons&= ~GG_UP;
      if ( event->jhat.value&0x2 ) _jbuttons.buttons|= GG_RIGHT;
      else                         _jbuttons.buttons&= ~GG_RIGHT;
      if ( event->jhat.value&0x4 ) _jbuttons.buttons|= GG_DOWN;
      else                         _jbuttons.buttons&= ~GG_DOWN;
      if ( event->jhat.value&0x8 ) _jbuttons.buttons|= GG_LEFT;
      else                         _jbuttons.buttons&= ~GG_LEFT;
      break;
      
    default: break;
      
    }
  
} /* end process_joy_event_xbox360 */


static void
process_joy_event (
        	   const SDL_Event *event,
        	   gboolean        *esc
        	   )
{
  
  /* ATENCIÓ!! Tots els events de joystick tenen els tres primer camps
     iguals. */
  if ( _joy.dev == NULL || _joy.index != event->jdevice.which ) return;
  switch ( _joy.type )
    {
    case PAD_JOY_XBOX360:
    default:
      process_joy_event_xbox360 ( event, esc );
    }
  
} /* end process_joy_event */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_pad (void)
{
  close_joy ();
} /* end close_pad */


void
init_pad (
          const conf_t *conf
          )
{
  
  /* Inicialitza valors. */
  _keys= &(conf->keys);
  _kbuttons= 0;
  _jbuttons.buttons= 0;
  _jbuttons.stick= 0;
  
  /* Si hi ha jostick l'obri. */
  _joy.dev= NULL;
  if ( SDL_JoystickEventState ( SDL_ENABLE ) != 1 )
    error ( "no s'han pogut habilitat els events dels joysticks: %s",
            SDL_GetError () );
  try_open_joy ();
  
} /* end init_pad */


void
pad_change_conf (
        	 conf_t *conf
        	 )
{
  _keys= &(conf->keys);
} /* end pad_change_conf */


int
pad_check_buttons (
        	   void *udata
        	   )
{
  
  int ret;
  
  
  ret= _kbuttons | _jbuttons.buttons | _jbuttons.stick;
  if ( (ret&(GG_UP|GG_DOWN)) == (GG_UP|GG_DOWN) ) ret&= ~GG_DOWN;
  if ( (ret&(GG_LEFT|GG_RIGHT)) == (GG_LEFT|GG_RIGHT) ) ret&= ~GG_RIGHT;
  
  return ret;
  
} /* end pad_check_buttons */


void
pad_clear (void)
{
  
  _kbuttons= 0;
  _jbuttons.buttons= 0;
  _jbuttons.stick= 0;
  SDL_FlushEvents ( SDL_FIRSTEVENT, SDL_LASTEVENT );
  
} /* end pad_clear */


gboolean
pad_event (
           const SDL_Event *event
           )
{
  
  gboolean esc;
  
  
  esc= FALSE;
  switch ( event->type )
    {
      
      /* Tecles. */
    case SDL_KEYDOWN:
      if ( event->key.keysym.sym == SDLK_ESCAPE ) esc= TRUE;
      else if ( event->key.keysym.sym == _keys->up )    _kbuttons|= GG_UP;
      else if ( event->key.keysym.sym == _keys->down )  _kbuttons|= GG_DOWN;
      else if ( event->key.keysym.sym == _keys->left )  _kbuttons|= GG_LEFT;
      else if ( event->key.keysym.sym == _keys->right ) _kbuttons|= GG_RIGHT;
      else if ( event->key.keysym.sym == _keys->tl )    _kbuttons|= GG_TL;
      else if ( event->key.keysym.sym == _keys->tr )    _kbuttons|= GG_TR;
      else if ( event->key.keysym.sym == _keys->start ) _kbuttons|= GG_START;
      break;
      
    case SDL_KEYUP:
      if ( event->key.keysym.sym == _keys->up )         _kbuttons&= ~GG_UP;
      else if ( event->key.keysym.sym == _keys->down )  _kbuttons&= ~GG_DOWN;
      else if ( event->key.keysym.sym == _keys->left )  _kbuttons&= ~GG_LEFT;
      else if ( event->key.keysym.sym == _keys->right ) _kbuttons&= ~GG_RIGHT;
      else if ( event->key.keysym.sym == _keys->tl )    _kbuttons&= ~GG_TL;
      else if ( event->key.keysym.sym == _keys->tr )    _kbuttons&= ~GG_TR;
      else if ( event->key.keysym.sym == _keys->start ) _kbuttons&= ~GG_START;
      break;
      
    case SDL_WINDOWEVENT:
      if ( event->window.event == SDL_WINDOWEVENT_FOCUS_LOST )
        _kbuttons= 0;
      break;
      
      /* Joysticks. */
    case SDL_JOYDEVICEADDED:
      if ( _joy.dev == NULL ) try_open_joy ();
      break;
      
    case SDL_JOYDEVICEREMOVED:
      if ( event->jdevice.which == _joy.index ) close_joy ();
      break;
      
    case SDL_JOYAXISMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    case SDL_JOYHATMOTION:
    case SDL_JOYBALLMOTION:
      process_joy_event ( event, &esc );
      break;
      
    default: break;
      
    }
  
  return esc;
  
} /* end pad_key_event */
