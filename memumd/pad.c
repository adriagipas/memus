/*
 * Copyright 2014-2023 Adrià Giménez Pastor.
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
#include "MD.h"
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

/* Dispositius. */
static MD_IOPluggedDevices *_devs;




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
        	_jbuttons[ind].stick|= MD_LEFT;
        	_jbuttons[ind].stick&= ~MD_RIGHT;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~MD_LEFT;
        	_jbuttons[ind].stick|= MD_RIGHT;
              }
            else
              {
        	_jbuttons[ind].stick&= ~MD_LEFT;
        	_jbuttons[ind].stick&= ~MD_RIGHT;
              }
            break;
          case 1: /* UP_DOWN */
            if ( event->jaxis.value < -10000 )
              {
        	_jbuttons[ind].stick|= MD_UP;
        	_jbuttons[ind].stick&= ~MD_DOWN;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~MD_UP;
        	_jbuttons[ind].stick|= MD_DOWN;
              }
            else
              {
        	_jbuttons[ind].stick&= ~MD_UP;
        	_jbuttons[ind].stick&= ~MD_DOWN;
              }
            break;
          }
        break;
        
    case SDL_JOYBUTTONDOWN:
      switch ( event->jbutton.button )
        {
        case 5: _jbuttons[ind].buttons|= MD_A; break;
        case 0: _jbuttons[ind].buttons|= MD_B; break;
        case 1: _jbuttons[ind].buttons|= MD_C; break;
        case 4: _jbuttons[ind].buttons|= MD_X; break;
        case 2: _jbuttons[ind].buttons|= MD_Y; break;
        case 3: _jbuttons[ind].buttons|= MD_Z; break;
        case 7: _jbuttons[ind].buttons|= MD_START; break;
        case 8: *esc= TRUE; break;
        default: break;
        }
      break;
      
    case SDL_JOYBUTTONUP:
      switch ( event->jbutton.button )
        {
        case 5: _jbuttons[ind].buttons&= ~MD_A; break;
        case 0: _jbuttons[ind].buttons&= ~MD_B; break;
        case 1: _jbuttons[ind].buttons&= ~MD_C; break;
        case 4: _jbuttons[ind].buttons&= ~MD_X; break;
        case 2: _jbuttons[ind].buttons&= ~MD_Y; break;
        case 3: _jbuttons[ind].buttons&= ~MD_Z; break;
        case 7: _jbuttons[ind].buttons&= ~MD_START; break;
        default: break;
        }
      break;
      
    case SDL_JOYHATMOTION:
      if ( event->jhat.hat != 0 ) break;
      if ( event->jhat.value&0x1 ) _jbuttons[ind].buttons|= MD_UP;
      else                         _jbuttons[ind].buttons&= ~MD_UP;
      if ( event->jhat.value&0x2 ) _jbuttons[ind].buttons|= MD_RIGHT;
      else                         _jbuttons[ind].buttons&= ~MD_RIGHT;
      if ( event->jhat.value&0x4 ) _jbuttons[ind].buttons|= MD_DOWN;
      else                         _jbuttons[ind].buttons&= ~MD_DOWN;
      if ( event->jhat.value&0x8 ) _jbuttons[ind].buttons|= MD_LEFT;
      else                         _jbuttons[ind].buttons&= ~MD_LEFT;
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
          conf_t *conf,
          void   *udata
          )
{
  
  /* Inicialitza valors. */
  _devs= &(conf->devs);
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
        	   const int  pad,
        	   void      *udata
        	   )
{
  
  int ret;
  
  
  ret= _kbuttons[pad] | _jbuttons[pad].buttons | _jbuttons[pad].stick;
  if ( (ret&(MD_UP|MD_DOWN)) == (MD_UP|MD_DOWN) ) ret&= ~MD_DOWN;
  if ( (ret&(MD_LEFT|MD_RIGHT)) == (MD_LEFT|MD_RIGHT) ) ret&= ~MD_RIGHT;
  
  return ret;
  
} /* end pad_check_buttons */


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
        _kbuttons[0]|= MD_UP;
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]|= MD_DOWN;
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]|= MD_LEFT;
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]|= MD_RIGHT;
      else if ( event->key.keysym.sym == _keys[0].button_A )
        _kbuttons[0]|= MD_A;
      else if ( event->key.keysym.sym == _keys[0].button_B )
        _kbuttons[0]|= MD_B;
      else if ( event->key.keysym.sym == _keys[0].button_C )
        _kbuttons[0]|= MD_C;
      else if ( event->key.keysym.sym == _keys[0].button_X )
        _kbuttons[0]|= MD_X;
      else if ( event->key.keysym.sym == _keys[0].button_Y )
        _kbuttons[0]|= MD_Y;
      else if ( event->key.keysym.sym == _keys[0].button_Z )
        _kbuttons[0]|= MD_Z;
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]|= MD_START;
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]|= MD_UP;
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]|= MD_DOWN;
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]|= MD_LEFT;
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]|= MD_RIGHT;
      else if ( event->key.keysym.sym == _keys[1].button_A )
        _kbuttons[1]|= MD_A;
      else if ( event->key.keysym.sym == _keys[1].button_B )
        _kbuttons[1]|= MD_B;
      else if ( event->key.keysym.sym == _keys[1].button_C )
        _kbuttons[1]|= MD_C;
      else if ( event->key.keysym.sym == _keys[1].button_X )
        _kbuttons[1]|= MD_X;
      else if ( event->key.keysym.sym == _keys[1].button_Y )
        _kbuttons[1]|= MD_Y;
      else if ( event->key.keysym.sym == _keys[1].button_Z )
        _kbuttons[1]|= MD_Z;
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]|= MD_START;
      break;
      
    case SDL_KEYUP:
      if ( event->key.keysym.sym == _keys[0].up )
        _kbuttons[0]&= ~MD_UP;
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]&= ~MD_DOWN;
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]&= ~MD_LEFT;
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]&= ~MD_RIGHT;
      else if ( event->key.keysym.sym == _keys[0].button_A )
        _kbuttons[0]&= ~MD_A;
      else if ( event->key.keysym.sym == _keys[0].button_B )
        _kbuttons[0]&= ~MD_B;
      else if ( event->key.keysym.sym == _keys[0].button_C )
        _kbuttons[0]&= ~MD_C;
      else if ( event->key.keysym.sym == _keys[0].button_X )
        _kbuttons[0]&= ~MD_X;
      else if ( event->key.keysym.sym == _keys[0].button_Y )
        _kbuttons[0]&= ~MD_Y;
      else if ( event->key.keysym.sym == _keys[0].button_Z )
        _kbuttons[0]&= ~MD_Z;
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]&= ~MD_START;
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]&= ~MD_UP;
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]&= ~MD_DOWN;
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]&= ~MD_LEFT;
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]&= ~MD_RIGHT;
      else if ( event->key.keysym.sym == _keys[1].button_A )
        _kbuttons[1]&= ~MD_A;
      else if ( event->key.keysym.sym == _keys[1].button_B )
        _kbuttons[1]&= ~MD_B;
      else if ( event->key.keysym.sym == _keys[1].button_C )
        _kbuttons[1]&= ~MD_C;
      else if ( event->key.keysym.sym == _keys[1].button_X )
        _kbuttons[1]&= ~MD_X;
      else if ( event->key.keysym.sym == _keys[1].button_Y )
        _kbuttons[1]&= ~MD_Y;
      else if ( event->key.keysym.sym == _keys[1].button_Z )
        _kbuttons[1]&= ~MD_Z;
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]&= ~MD_START;
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

MD_IOPluggedDevices
pad_get_devices (void)
{

  MD_get_io_devs ( _devs );
  
  return *_devs;
  
} /* end pad_get_devices */


void
pad_set_devices (
        	 const MD_IOPluggedDevices devs
        	 )
{

  *_devs= devs;
  MD_set_io_devs ( devs );
  
} /* pad_set_devices */
