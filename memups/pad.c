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
 * along with adriagipas/memus. If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  pad.c - Implementació de 'pad.h'.
 *
 */


#include <assert.h>
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "pad.h"
#include "PSX.h"




/*********/
/* ESTAT */
/*********/

// Estat del joystick.
static struct
{
  SDL_Joystick *dev;
  int           index;
  int           type;
} _joy[2];

// Tecles.
static const keys_t *_keys;

// Estat dels botons segons el teclat.
static int _kbuttons[2];

// Estat dels botons del joypad.
static struct
{
  
  int buttons;
  int stick;
  
} _jbuttons[2];

// Estat PSX.
static PSX_ControllerState _state[2];




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
      
      // Comprova el nom.
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
      
      // Intenta obrir.
      _joy[ind].dev= SDL_JoystickOpen ( i );
      if ( _joy[ind].dev == NULL )
        {
          warning ( "no s'ha pogut obrir el joystick %d: %s",
        	    i, SDL_GetError () );
          return;
        }
      
      // Obté l'index.
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
  
} // end try_open_joy


static void
try_open_joys (void)
{
  
  try_open_joy ( 0 );
  try_open_joy ( 1 );
  
} // end try_open_joys


static void
close_joy (
           const int ind
           )
{
  
  if ( _joy[ind].dev != NULL && SDL_JoystickGetAttached ( _joy[ind].dev ) )
    SDL_JoystickClose ( _joy[ind].dev );
  _joy[ind].dev= NULL;
  
} // end close_joy


static void
close_joys (void)
{
  
  close_joy ( 0 );
  close_joy ( 1 );
  
} // end close_joys


static void
process_joy_event_xbox360 (
        		   const int        ind,
        		   const SDL_Event *event,
        		   bool            *esc
        		   )
{
  
  switch ( event->type )
    {
      
      case SDL_JOYAXISMOTION:
        switch ( event->jaxis.axis )
          {
          case 0: // LEFT_RIGHT
            if ( event->jaxis.value < -10000 )
              {
        	_jbuttons[ind].stick|= PSX_BUTTON_LEFT;
        	_jbuttons[ind].stick&= ~PSX_BUTTON_RIGHT;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~PSX_BUTTON_LEFT;
        	_jbuttons[ind].stick|= PSX_BUTTON_RIGHT;
              }
            else
              {
        	_jbuttons[ind].stick&= ~PSX_BUTTON_LEFT;
        	_jbuttons[ind].stick&= ~PSX_BUTTON_RIGHT;
              }
            break;
          case 1: // UP_DOWN
            if ( event->jaxis.value < -10000 )
              {
        	_jbuttons[ind].stick|= PSX_BUTTON_UP;
        	_jbuttons[ind].stick&= ~PSX_BUTTON_DOWN;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons[ind].stick&= ~PSX_BUTTON_UP;
        	_jbuttons[ind].stick|= PSX_BUTTON_DOWN;
              }
            else
              {
        	_jbuttons[ind].stick&= ~PSX_BUTTON_UP;
        	_jbuttons[ind].stick&= ~PSX_BUTTON_DOWN;
              }
            break;
          case 2: // L2
            if ( event->jaxis.value > 10000 )
              _jbuttons[ind].stick|= PSX_BUTTON_L2;
            else
              _jbuttons[ind].stick&= ~PSX_BUTTON_L2;
          case 5: // R2
            if ( event->jaxis.value > 10000 )
              _jbuttons[ind].stick|= PSX_BUTTON_R2;
            else
              _jbuttons[ind].stick&= ~PSX_BUTTON_R2;
            break;
          }
        break;
        
    case SDL_JOYBUTTONDOWN:
      switch ( event->jbutton.button )
        {
        case 3: _jbuttons[ind].buttons|= PSX_BUTTON_TRIANGLE; break;
        case 1: _jbuttons[ind].buttons|= PSX_BUTTON_CIRCLE; break;
        case 0: _jbuttons[ind].buttons|= PSX_BUTTON_CROSS; break;
        case 2: _jbuttons[ind].buttons|= PSX_BUTTON_SQUARE; break;
        case 4: _jbuttons[ind].buttons|= PSX_BUTTON_L1; break;
        case 5: _jbuttons[ind].buttons|= PSX_BUTTON_R1; break;
        case 7: _jbuttons[ind].buttons|= PSX_BUTTON_START; break;
        case 6: _jbuttons[ind].buttons|= PSX_BUTTON_SELECT; break;
        case 8: *esc= true; break;
        default: break;
        }
      break;
      
    case SDL_JOYBUTTONUP:
      switch ( event->jbutton.button )
        {
        case 3: _jbuttons[ind].buttons&= ~PSX_BUTTON_TRIANGLE; break;
        case 1: _jbuttons[ind].buttons&= ~PSX_BUTTON_CIRCLE; break;
        case 0: _jbuttons[ind].buttons&= ~PSX_BUTTON_CROSS; break;
        case 2: _jbuttons[ind].buttons&= ~PSX_BUTTON_SQUARE; break;
        case 4: _jbuttons[ind].buttons&= ~PSX_BUTTON_L1; break;
        case 5: _jbuttons[ind].buttons&= ~PSX_BUTTON_R1; break;
        case 7: _jbuttons[ind].buttons&= ~PSX_BUTTON_START; break;
        case 6: _jbuttons[ind].buttons&= ~PSX_BUTTON_SELECT; break;
        default: break;
        }
      break;
      
    case SDL_JOYHATMOTION:
      if ( event->jhat.hat != 0 ) break;
      if ( event->jhat.value&0x1 ) _jbuttons[ind].buttons|= PSX_BUTTON_UP;
      else                         _jbuttons[ind].buttons&= ~PSX_BUTTON_UP;
      if ( event->jhat.value&0x2 ) _jbuttons[ind].buttons|= PSX_BUTTON_RIGHT;
      else                         _jbuttons[ind].buttons&= ~PSX_BUTTON_RIGHT;
      if ( event->jhat.value&0x4 ) _jbuttons[ind].buttons|= PSX_BUTTON_DOWN;
      else                         _jbuttons[ind].buttons&= ~PSX_BUTTON_DOWN;
      if ( event->jhat.value&0x8 ) _jbuttons[ind].buttons|= PSX_BUTTON_LEFT;
      else                         _jbuttons[ind].buttons&= ~PSX_BUTTON_LEFT;
      break;
      
    default: break;
      
    }
  
} // end process_joy_event_xbox360


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
        	   bool        *esc
        	   )
{
  
  // ATENCIÓ!! Tots els events de joystick tenen els tres primer camps
  // iguals.
  int ind;
  
  
  ind= get_joy_ind ( event->jdevice.which );
  if ( ind == -1 ) return;
  switch ( _joy[ind].type )
    {
    case PAD_JOY_XBOX360:
    default:
      process_joy_event_xbox360 ( ind, event, esc );
    }
  
} // end process_joy_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_pad (void)
{
  close_joys ();
} // end close_pad


void
init_pad (
          conf_t *conf,
          void   *udata
          )
{
  
  // Inicialitza valors.
  _keys= &(conf->keys[0]);
  _kbuttons[0]= _kbuttons[1]= 0;
  _jbuttons[0].buttons= _jbuttons[1].buttons= 0;
  _jbuttons[0].stick= _jbuttons[1].stick= 0;
  _state[0].buttons= 0;
  _state[1].buttons= 1;
  
  // Si hi ha jostick l'obri.
  _joy[0].dev= _joy[1].dev= NULL;
  if ( SDL_JoystickEventState ( SDL_ENABLE ) != 1 )
    error ( "no s'han pogut habilitat els events dels joysticks: %s",
            SDL_GetError () );
  try_open_joys ();

  
} // end init_pad


const PSX_ControllerState *
pad_get_controller_state (
                          const int  joy,
                          void      *udata
                          )
{

  uint16_t ret;

  
  ret= (uint16_t)
    (_kbuttons[joy] | _jbuttons[joy].buttons | _jbuttons[joy].stick);
  if ( (ret&(PSX_BUTTON_UP|PSX_BUTTON_DOWN)) ==
       (PSX_BUTTON_UP|PSX_BUTTON_DOWN) )
    ret&= ~PSX_BUTTON_DOWN;
  if ( (ret&(PSX_BUTTON_LEFT|PSX_BUTTON_RIGHT)) ==
       (PSX_BUTTON_LEFT|PSX_BUTTON_RIGHT) )
    ret&= ~PSX_BUTTON_RIGHT;
  _state[joy].buttons= ret;
  
  return &_state[joy];
  
} // end pad_get_controller_state


void
pad_clear (void)
{
  
  _kbuttons[0]= _kbuttons[1]= 0;
  _jbuttons[0].buttons= _jbuttons[1].buttons= 0;
  _jbuttons[0].stick= _jbuttons[1].stick= 0;
  SDL_FlushEvents ( SDL_FIRSTEVENT, SDL_LASTEVENT );
  
} // end pad_clear


bool
pad_event (
           const SDL_Event *event
           )
{
  
  bool esc;
  
  
  esc= false;
  switch ( event->type )
    {
      
      // Tecles.
    case SDL_KEYDOWN:
      if ( event->key.keysym.sym == SDLK_ESCAPE ) esc= TRUE;
      else if ( event->key.keysym.sym == _keys[0].up )
        _kbuttons[0]|= PSX_BUTTON_UP;
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]|= PSX_BUTTON_DOWN;
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]|= PSX_BUTTON_LEFT;
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]|= PSX_BUTTON_RIGHT;
      else if ( event->key.keysym.sym == _keys[0].button_triangle )
        _kbuttons[0]|= PSX_BUTTON_TRIANGLE;
      else if ( event->key.keysym.sym == _keys[0].button_circle )
        _kbuttons[0]|= PSX_BUTTON_CIRCLE;
      else if ( event->key.keysym.sym == _keys[0].button_cross )
        _kbuttons[0]|= PSX_BUTTON_CROSS;
      else if ( event->key.keysym.sym == _keys[0].button_square )
        _kbuttons[0]|= PSX_BUTTON_SQUARE;
      else if ( event->key.keysym.sym == _keys[0].button_L2 )
        _kbuttons[0]|= PSX_BUTTON_L2;
      else if ( event->key.keysym.sym == _keys[0].button_R2 )
        _kbuttons[0]|= PSX_BUTTON_R2;
      else if ( event->key.keysym.sym == _keys[0].button_L1 )
        _kbuttons[0]|= PSX_BUTTON_L1;
      else if ( event->key.keysym.sym == _keys[0].button_R1 )
        _kbuttons[0]|= PSX_BUTTON_R1;
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]|= PSX_BUTTON_START;
      else if ( event->key.keysym.sym == _keys[0].select )
        _kbuttons[0]|= PSX_BUTTON_SELECT;
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]|= PSX_BUTTON_UP;
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]|= PSX_BUTTON_DOWN;
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]|= PSX_BUTTON_LEFT;
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]|= PSX_BUTTON_RIGHT;
      else if ( event->key.keysym.sym == _keys[1].button_triangle )
        _kbuttons[1]|= PSX_BUTTON_TRIANGLE;
      else if ( event->key.keysym.sym == _keys[1].button_circle )
        _kbuttons[1]|= PSX_BUTTON_CIRCLE;
      else if ( event->key.keysym.sym == _keys[1].button_cross )
        _kbuttons[1]|= PSX_BUTTON_CROSS;
      else if ( event->key.keysym.sym == _keys[1].button_square )
        _kbuttons[1]|= PSX_BUTTON_SQUARE;
      else if ( event->key.keysym.sym == _keys[1].button_L2 )
        _kbuttons[1]|= PSX_BUTTON_L2;
      else if ( event->key.keysym.sym == _keys[1].button_R2 )
        _kbuttons[1]|= PSX_BUTTON_R2;
      else if ( event->key.keysym.sym == _keys[1].button_L1 )
        _kbuttons[1]|= PSX_BUTTON_L1;
      else if ( event->key.keysym.sym == _keys[1].button_R1 )
        _kbuttons[1]|= PSX_BUTTON_R1;
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]|= PSX_BUTTON_START;
      else if ( event->key.keysym.sym == _keys[1].select )
        _kbuttons[1]|= PSX_BUTTON_SELECT;
      break;
      
    case SDL_KEYUP:
      if ( event->key.keysym.sym == _keys[0].up )
        _kbuttons[0]&= ~PSX_BUTTON_UP;
      else if ( event->key.keysym.sym == _keys[0].down )
        _kbuttons[0]&= ~PSX_BUTTON_DOWN;
      else if ( event->key.keysym.sym == _keys[0].left )
        _kbuttons[0]&= ~PSX_BUTTON_LEFT;
      else if ( event->key.keysym.sym == _keys[0].right )
        _kbuttons[0]&= ~PSX_BUTTON_RIGHT;
      else if ( event->key.keysym.sym == _keys[0].button_triangle )
        _kbuttons[0]&= ~PSX_BUTTON_TRIANGLE;
      else if ( event->key.keysym.sym == _keys[0].button_circle )
        _kbuttons[0]&= ~PSX_BUTTON_CIRCLE;
      else if ( event->key.keysym.sym == _keys[0].button_cross )
        _kbuttons[0]&= ~PSX_BUTTON_CROSS;
      else if ( event->key.keysym.sym == _keys[0].button_square )
        _kbuttons[0]&= ~PSX_BUTTON_SQUARE;
      else if ( event->key.keysym.sym == _keys[0].button_L2 )
        _kbuttons[0]&= ~PSX_BUTTON_L2;
      else if ( event->key.keysym.sym == _keys[0].button_R2 )
        _kbuttons[0]&= ~PSX_BUTTON_R2;
      else if ( event->key.keysym.sym == _keys[0].button_L1 )
        _kbuttons[0]&= ~PSX_BUTTON_L1;
      else if ( event->key.keysym.sym == _keys[0].button_R1 )
        _kbuttons[0]&= ~PSX_BUTTON_R1;
      else if ( event->key.keysym.sym == _keys[0].start )
        _kbuttons[0]&= ~PSX_BUTTON_START;
      else if ( event->key.keysym.sym == _keys[0].select )
        _kbuttons[0]&= ~PSX_BUTTON_SELECT;
      else if ( event->key.keysym.sym == _keys[1].up )
        _kbuttons[1]&= ~PSX_BUTTON_UP;
      else if ( event->key.keysym.sym == _keys[1].down )
        _kbuttons[1]&= ~PSX_BUTTON_DOWN;
      else if ( event->key.keysym.sym == _keys[1].left )
        _kbuttons[1]&= ~PSX_BUTTON_LEFT;
      else if ( event->key.keysym.sym == _keys[1].right )
        _kbuttons[1]&= ~PSX_BUTTON_RIGHT;
      else if ( event->key.keysym.sym == _keys[1].button_triangle )
        _kbuttons[1]&= ~PSX_BUTTON_TRIANGLE;
      else if ( event->key.keysym.sym == _keys[1].button_circle )
        _kbuttons[1]&= ~PSX_BUTTON_CIRCLE;
      else if ( event->key.keysym.sym == _keys[1].button_cross )
        _kbuttons[1]&= ~PSX_BUTTON_CROSS;
      else if ( event->key.keysym.sym == _keys[1].button_square )
        _kbuttons[1]&= ~PSX_BUTTON_SQUARE;
      else if ( event->key.keysym.sym == _keys[1].button_L2 )
        _kbuttons[1]&= ~PSX_BUTTON_L2;
      else if ( event->key.keysym.sym == _keys[1].button_R2 )
        _kbuttons[1]&= ~PSX_BUTTON_R2;
      else if ( event->key.keysym.sym == _keys[1].button_L1 )
        _kbuttons[1]&= ~PSX_BUTTON_L1;
      else if ( event->key.keysym.sym == _keys[1].button_R1 )
        _kbuttons[1]&= ~PSX_BUTTON_R1;
      else if ( event->key.keysym.sym == _keys[1].start )
        _kbuttons[1]&= ~PSX_BUTTON_START;
      else if ( event->key.keysym.sym == _keys[1].select )
        _kbuttons[1]&= ~PSX_BUTTON_SELECT;
      break;
      
    case SDL_WINDOWEVENT:
      if ( event->window.event == SDL_WINDOWEVENT_FOCUS_LOST )
        _kbuttons[0]= _kbuttons[1]= 0;
      break;
      
      // Joysticks.
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
  
} // end pad_event
