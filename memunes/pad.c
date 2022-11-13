/*
 * Copyright 2016-2022 Adrià Giménez Pastor.
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
#include "NES.h"
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
} _joy[2];

/* Tecles. */
static const keys_t *_keys;

/* Estat dels botons segons el teclat. */
static int _kbuttons[2];

/* Estat dels botons del joypad. */
static struct
{
  
  int buttons;
  int stick;
  
} _jbuttons[2];




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

static void
try_open_joy (
              const int ind
              )
{
  
  int num, i;
  const char *name;
  
  
  if ( _joy[ind].dev != NULL ) return;
  
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
      if ( !strcmp ( name, "Microsoft X-Box 360 pad" ) ||
           !strcmp ( name, "Xbox 360 Controller" ) )
        _joy[ind].type= PAD_JOY_XBOX360;
      else
        {
          warning ( "model de joystick no suportat: %s", name );
          continue;
        }
      
      /* Intenta obrir. */
      _joy[ind].dev= SDL_JoystickOpen ( i );
      if ( _joy[ind].dev == NULL )
        {
          warning ( "no s'ha pogut obrir el joystick %d: %s",
        	    i, SDL_GetError () );
          return;
        }
      
      /* Obté l'index. */
      _joy[ind].index= (int) SDL_JoystickInstanceID ( _joy[ind].dev );
      if ( _joy[ind].index < 0 )
        {
          warning ( "no s'ha pogut obtenir l'identificador del joystick %d: %s",
        	    i, SDL_GetError () );
          SDL_JoystickClose ( _joy[ind].dev );
          _joy[ind].dev= NULL;
          return;
        }
      
    }
  
} /* end try_open_joy */


static void
try_open_joys (void)
{
  
  try_open_joy ( 0 );
  try_open_joy ( 1 );
  
} /* end try_open_joys */


static void
close_joy (
           const int ind
           )
{
  
  if ( _joy[ind].dev != NULL && SDL_JoystickGetAttached ( _joy[ind].dev ) )
    SDL_JoystickClose ( _joy[ind].dev );
  _joy[ind].dev= NULL;
  
} /* end close_joy */


static void
close_joys (void)
{
  
  close_joy ( 0 );
  close_joy ( 1 );
  
} /* end close_joys */


static void
process_joy_event_xbox360 (
        		   const int        ind,
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
              {
        	_jbuttons[ind].stick|= (1<<NES_LEFT);
        	_jbuttons[ind].stick&= ~(1<<NES_RIGHT);
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~(1<<NES_LEFT);
        	_jbuttons[ind].stick|= (1<<NES_RIGHT);
              }
            else
              {
        	_jbuttons[ind].stick&= ~(1<<NES_LEFT);
        	_jbuttons[ind].stick&= ~(1<<NES_RIGHT);
              }
            break;
          case 1: /* UP_DOWN */
            if ( event->jaxis.value < -10000 )
              {
        	_jbuttons[ind].stick|= (1<<NES_UP);
        	_jbuttons[ind].stick&= ~(1<<NES_DOWN);
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~(1<<NES_UP);
        	_jbuttons[ind].stick|= (1<<NES_DOWN);
              }
            else
              {
        	_jbuttons[ind].stick&= ~(1<<NES_UP);
        	_jbuttons[ind].stick&= ~(1<<NES_DOWN);
              }
            break;
          }
        break;
        
    case SDL_JOYBUTTONDOWN:
      switch ( event->jbutton.button )
        {
        case 0: _jbuttons[ind].buttons|= (1<<NES_B); break;
        case 1: _jbuttons[ind].buttons|= (1<<NES_A); break;
        case 6: _jbuttons[ind].buttons|= (1<<NES_SELECT); break;
        case 7: _jbuttons[ind].buttons|= (1<<NES_START); break;
        case 8: *esc= TRUE; break;
        default: break;
        }
      break;
      
    case SDL_JOYBUTTONUP:
      switch ( event->jbutton.button )
        {
        case 0: _jbuttons[ind].buttons&= ~(1<<NES_B); break;
        case 1: _jbuttons[ind].buttons&= ~(1<<NES_A); break;
        case 6: _jbuttons[ind].buttons&= ~(1<<NES_SELECT); break;
        case 7: _jbuttons[ind].buttons&= ~(1<<NES_START); break;
        default: break;
        }
      break;
      
    case SDL_JOYHATMOTION:
      if ( event->jhat.hat != 0 ) break;
      if ( event->jhat.value&0x1 ) _jbuttons[ind].buttons|= (1<<NES_UP);
      else                         _jbuttons[ind].buttons&= ~(1<<NES_UP);
      if ( event->jhat.value&0x2 ) _jbuttons[ind].buttons|= (1<<NES_RIGHT);
      else                         _jbuttons[ind].buttons&= ~(1<<NES_RIGHT);
      if ( event->jhat.value&0x4 ) _jbuttons[ind].buttons|= (1<<NES_DOWN);
      else                         _jbuttons[ind].buttons&= ~(1<<NES_DOWN);
      if ( event->jhat.value&0x8 ) _jbuttons[ind].buttons|= (1<<NES_LEFT);
      else                         _jbuttons[ind].buttons&= ~(1<<NES_LEFT);
      break;
      
    default: break;
      
    }
  
} /* end process_joy_event_xbox360 */


static int
get_joy_ind (
             const int jdevice_index
             )
{
  
  int i;
  
  
  for ( i= 0; i < 2; ++i )
    if ( _joy[i].dev != NULL && _joy[i].index == jdevice_index )
      return i;
  
  return -1;
  
} /* end get_joy_ind */


static void
process_joy_event (
        	   const SDL_Event *event,
        	   gboolean        *esc
        	   )
{
  
  /* ATENCIÓ!! Tots els events de joystick tenen els tres primer camps
     iguals. */
  int ind;
  
  
  ind= get_joy_ind ( event->jdevice.which );
  if ( ind == -1 ) return;
  switch ( _joy[ind].type )
    {
    case PAD_JOY_XBOX360:
    default:
      process_joy_event_xbox360 ( ind, event, esc );
    }
  
} /* end process_joy_event */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_pad (void)
{
  close_joys ();
} /* end close_pad */


void
init_pad (
          conf_t *conf
          )
{
  
  /* Inicialitza valors. */
  _keys= &(conf->keys[0]);
  _kbuttons[0]= _kbuttons[1]= 0;
  _jbuttons[0].buttons= _jbuttons[1].buttons= 0;
  _jbuttons[0].stick= _jbuttons[1].stick= 0;
  
  /* Si hi ha jostick l'obri. */
  _joy[0].dev= _joy[1].dev= NULL;
  if ( SDL_JoystickEventState ( SDL_ENABLE ) != 1 )
    error ( "no s'han pogut habilitat els events dels joysticks: %s",
            SDL_GetError () );
  try_open_joys ();
  
} /* end init_pad */


int
pad_check_buttons (
        	   const int pad
        	   )
{
  
  int ret;
  
  
  ret= _kbuttons[pad] | _jbuttons[pad].buttons | _jbuttons[pad].stick;
  if ( (ret&((1<<NES_UP)|(1<<NES_DOWN))) ==
       ((1<<NES_UP)|(1<<NES_DOWN)) ) ret&= ~(1<<NES_DOWN);
  if ( (ret&((1<<NES_LEFT)|(1<<NES_RIGHT))) ==
       ((1<<NES_LEFT)|(1<<NES_RIGHT)) ) ret&= ~(1<<NES_RIGHT);
  
  return ret;
  
} /* end pad_check_buttons */


NES_Bool
pad_check_pad1_buttons (
        		NES_PadButton  button,
        		void          *udata
        		)
{
  return (pad_check_buttons ( 0 )&(1<<button))!=0;
} /* end pad_check_pad1_buttons */


NES_Bool
pad_check_pad2_buttons (
        		NES_PadButton  button,
        		void          *udata
        		)
{
  return (pad_check_buttons ( 1 )&(1<<button))!=0;
} /* end pad_check_pad2_buttons */


void
pad_clear (void)
{
  
  _kbuttons[0]= _kbuttons[1]= 0;
  _jbuttons[0].buttons= _jbuttons[1].buttons= 0;
  _jbuttons[0].stick= _jbuttons[1].stick= 0;
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
      else if ( event->key.keysym.sym == _keys[0].up )
        _kbuttons[0]|= (1<<NES_UP);
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]|= (1<<NES_DOWN);
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]|= (1<<NES_LEFT);
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]|= (1<<NES_RIGHT);
      else if ( event->key.keysym.sym == _keys[0].button_A )
        _kbuttons[0]|= (1<<NES_A);
      else if ( event->key.keysym.sym == _keys[0].button_B )
        _kbuttons[0]|= (1<<NES_B);
      else if ( event->key.keysym.sym == _keys[0].select )
        _kbuttons[0]|= (1<<NES_SELECT);
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]|= (1<<NES_START);
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]|= (1<<NES_UP);
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]|= (1<<NES_DOWN);
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]|= (1<<NES_LEFT);
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]|= (1<<NES_RIGHT);
      else if ( event->key.keysym.sym == _keys[1].button_A )
        _kbuttons[1]|= (1<<NES_A);
      else if ( event->key.keysym.sym == _keys[1].button_B )
        _kbuttons[1]|= (1<<NES_B);
      else if ( event->key.keysym.sym == _keys[1].select )
        _kbuttons[1]|= (1<<NES_SELECT);
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]|= (1<<NES_START);
      break;
      
    case SDL_KEYUP:
      if ( event->key.keysym.sym == _keys[0].up )
        _kbuttons[0]&= ~(1<<NES_UP);
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]&= ~(1<<NES_DOWN);
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]&= ~(1<<NES_LEFT);
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]&= ~(1<<NES_RIGHT);
      else if ( event->key.keysym.sym == _keys[0].button_A )
        _kbuttons[0]&= ~(1<<NES_A);
      else if ( event->key.keysym.sym == _keys[0].button_B )
        _kbuttons[0]&= ~(1<<NES_B);
      else if ( event->key.keysym.sym == _keys[0].select )
        _kbuttons[0]&= ~(1<<NES_SELECT);
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]&= ~(1<<NES_START);
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]&= ~(1<<NES_UP);
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]&= ~(1<<NES_DOWN);
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]&= ~(1<<NES_LEFT);
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]&= ~(1<<NES_RIGHT);
      else if ( event->key.keysym.sym == _keys[1].button_A )
        _kbuttons[1]&= ~(1<<NES_A);
      else if ( event->key.keysym.sym == _keys[1].button_B )
        _kbuttons[1]&= ~(1<<NES_B);
      else if ( event->key.keysym.sym == _keys[1].select )
        _kbuttons[1]&= ~(1<<NES_SELECT);
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]&= ~(1<<NES_START);
      break;
      
    case SDL_WINDOWEVENT:
      if ( event->window.event == SDL_WINDOWEVENT_FOCUS_LOST )
        _kbuttons[0]= _kbuttons[1]= 0;
      break;
      
      /* Joysticks. */
    case SDL_JOYDEVICEADDED:
      try_open_joys ();
      break;
      
    case SDL_JOYDEVICEREMOVED:
      if ( event->jdevice.which == _joy[0].index ) close_joy ( 0 );
      if ( event->jdevice.which == _joy[1].index ) close_joy ( 1 );
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
  
} /* end pad_event */
