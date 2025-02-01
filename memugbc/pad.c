/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "GBC.h"
#include "pad.h"




/*********/
/* ESTAT */
/*********/

/* Estar del joystick. */
static struct
{
  SDL_Joystick *dev;
  SDL_Haptic   *hap;
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
          warning ( "model de joystick desconegut: %s", name );
          _joy.type= PAD_JOY_UNK;
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
      
      /* Intenta obrir el rumble. */
      if ( !SDL_JoystickIsHaptic ( _joy.dev ) ) return;
      _joy.hap= SDL_HapticOpenFromJoystick ( _joy.dev );
      if ( _joy.hap == NULL )
        {
          warning ( "no s'ha pogut obrir el dispositiu 'haptic': %s",
        	    SDL_GetError () );
          return;
        }
      
      /* Comprova que suporta l'efecte rumble. */
      if ( !SDL_HapticRumbleSupported ( _joy.hap ) )
        {
          warning ( "el joystick no suporta vibració: %s", SDL_GetError () );
          SDL_HapticClose ( _joy.hap );
          _joy.hap= NULL;
          return;
        }
      
      /* Inicialitza el dispositiu haptic. */
      if ( SDL_HapticRumbleInit ( _joy.hap ) != 0 )
        {
          warning ( "no s'ha pogut inicialitzar el dispositiu 'haptic': %s",
        	    SDL_GetError () );
          SDL_HapticClose ( _joy.hap );
          _joy.hap= NULL;
          return;
        }
      
    }
  
} /* end try_open_joy */


static void
close_joy (void)
{
  
  if ( _joy.dev != NULL && SDL_JoystickGetAttached ( _joy.dev ) )
    {
      if ( _joy.hap != NULL ) SDL_HapticClose ( _joy.hap );
      SDL_JoystickClose ( _joy.dev );
    }
  _joy.dev= NULL;
  _joy.hap= NULL;
  
} /* end close_joy */


static void
process_joy_event_xbox360 (
        		   const SDL_Event *event,
        		   gboolean        *esc,
        		   GBC_Bool        *button_pressed,
        		   GBC_Bool        *direction_pressed
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
        	_jbuttons.stick|= GBC_LEFT;
        	_jbuttons.stick&= ~GBC_RIGHT;
        	*direction_pressed= GBC_TRUE;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons.stick&= ~GBC_LEFT;
        	_jbuttons.stick|= GBC_RIGHT;
        	*direction_pressed= GBC_TRUE;
              }
            else
              { _jbuttons.stick&= ~GBC_LEFT; _jbuttons.stick&= ~GBC_RIGHT; }
            break;
          case 1: /* UP_DOWN */
            if ( event->jaxis.value < -10000 )
              {
        	_jbuttons.stick|= GBC_UP;
        	_jbuttons.stick&= ~GBC_DOWN;
        	*direction_pressed= GBC_TRUE;
              }
            else if ( event->jaxis.value > 10000 )
              {
        	_jbuttons.stick&= ~GBC_UP;
        	_jbuttons.stick|= GBC_DOWN;
        	*direction_pressed= GBC_TRUE;
              }
            else
              { _jbuttons.stick&= ~GBC_UP; _jbuttons.stick&= ~GBC_DOWN; }
            break;
          }
        break;
        
    case SDL_JOYBUTTONDOWN:
      switch ( event->jbutton.button )
        {
        case 0:
          _jbuttons.buttons|= GBC_BUTTON_B;
          *button_pressed= GBC_TRUE;
          break;
        case 1:
          _jbuttons.buttons|= GBC_BUTTON_A;
          *button_pressed= GBC_TRUE;
          break;
        case 6:
          _jbuttons.buttons|= GBC_SELECT;
          *button_pressed= GBC_TRUE;
          break; /* COMPROVAR !!! */
        case 7:
          _jbuttons.buttons|= GBC_START;
          *button_pressed= GBC_TRUE;
          break;
        case 8: *esc= TRUE; break;
        default: break;
        }
      break;
      
    case SDL_JOYBUTTONUP:
      switch ( event->jbutton.button )
        {
        case 0: _jbuttons.buttons&= ~GBC_BUTTON_B; break;
        case 1: _jbuttons.buttons&= ~GBC_BUTTON_A; break;
        case 6: _jbuttons.buttons&= ~GBC_SELECT; break;
        case 7: _jbuttons.buttons&= ~GBC_START; break;
        default: break;
        }
      break;
      
    case SDL_JOYHATMOTION:
      if ( event->jhat.hat != 0 ) break;
      if ( event->jhat.value&0x1 )
        { _jbuttons.buttons|= GBC_UP; *direction_pressed= GBC_TRUE; }
      else
        _jbuttons.buttons&= ~GBC_UP;
      if ( event->jhat.value&0x2 )
        { _jbuttons.buttons|= GBC_RIGHT; *direction_pressed= GBC_TRUE; }
      else
        _jbuttons.buttons&= ~GBC_RIGHT;
      if ( event->jhat.value&0x4 )
        { _jbuttons.buttons|= GBC_DOWN; *direction_pressed= GBC_TRUE; }
      else
        _jbuttons.buttons&= ~GBC_DOWN;
      if ( event->jhat.value&0x8 )
        { _jbuttons.buttons|= GBC_LEFT; *direction_pressed= GBC_TRUE; }
      else
        _jbuttons.buttons&= ~GBC_LEFT;
      break;
      
    default: break;
      
    }
  
} /* end process_joy_event_xbox360 */


static void
process_joy_event (
        	   const SDL_Event *event,
        	   gboolean        *esc,
        	   GBC_Bool        *button_pressed,
        	   GBC_Bool        *direction_pressed
        	   )
{
  
  /* ATENCIÓ!! Tots els events de joystick tenen els tres primer camps
     iguals. */
  if ( _joy.dev == NULL || _joy.index != event->jdevice.which ) return;
  switch ( _joy.type )
    {
    case PAD_JOY_XBOX360:
    default:
      process_joy_event_xbox360 ( event, esc, button_pressed,
        			  direction_pressed  );
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
  _joy.hap= NULL;
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
  if ( (ret&(GBC_UP|GBC_DOWN)) == (GBC_UP|GBC_DOWN) ) ret&= ~GBC_DOWN;
  if ( (ret&(GBC_LEFT|GBC_RIGHT)) == (GBC_LEFT|GBC_RIGHT) ) ret&= ~GBC_RIGHT;
  
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
           const SDL_Event *event,
           GBC_Bool        *button_pressed,
           GBC_Bool        *direction_pressed
           )
{
  
  gboolean esc;
  
  
  esc= FALSE;
  switch ( event->type )
    {
      
      /* Tecles. */
    case SDL_KEYDOWN:
      if ( event->key.keysym.sym == SDLK_ESCAPE ) esc= TRUE;
      else if ( event->key.keysym.sym == _keys->up )
        { _kbuttons|= GBC_UP; *direction_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->down )
        { _kbuttons|= GBC_DOWN; *direction_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->left )
        { _kbuttons|= GBC_LEFT; *direction_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->right )
        { _kbuttons|= GBC_RIGHT; *direction_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->button_A )
        { _kbuttons|= GBC_BUTTON_A; *button_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->button_B )
        { _kbuttons|= GBC_BUTTON_B; *button_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->select )
        { _kbuttons|= GBC_SELECT; *button_pressed= GBC_TRUE; }
      else if ( event->key.keysym.sym == _keys->start )
        { _kbuttons|= GBC_START; *button_pressed= GBC_TRUE; }
      break;
      
    case SDL_KEYUP:
      if ( event->key.keysym.sym == _keys->up )         _kbuttons&= ~GBC_UP;
      else if ( event->key.keysym.sym == _keys->down )  _kbuttons&= ~GBC_DOWN;
      else if ( event->key.keysym.sym == _keys->left )  _kbuttons&= ~GBC_LEFT;
      else if ( event->key.keysym.sym == _keys->right ) _kbuttons&= ~GBC_RIGHT;
      else if ( event->key.keysym.sym == _keys->button_A )
        _kbuttons&= ~GBC_BUTTON_A;
      else if ( event->key.keysym.sym == _keys->button_B )
        _kbuttons&= ~GBC_BUTTON_B;
      else if ( event->key.keysym.sym == _keys->select )
        _kbuttons&= ~GBC_SELECT;
      else if ( event->key.keysym.sym == _keys->start ) _kbuttons&= ~GBC_START;
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
      process_joy_event ( event, &esc, button_pressed, direction_pressed );
      break;
      
    default: break;
      
    }
  
  return esc;
  
} /* end pad_key_event */


void
pad_update_rumble (
                   const int  level,
                   void      *udata
                   )
{

  /* NOTA: Vaig a implementar el rumble com vibracions de 250ms, el 0
     no farà res. */
  if ( _joy.hap == NULL ) return;
  switch ( level )
    {
    case 1: SDL_HapticRumblePlay ( _joy.hap, 0.25, 250 ); break;
    case 2: SDL_HapticRumblePlay ( _joy.hap, 0.50, 250 ); break;
    case 3: SDL_HapticRumblePlay ( _joy.hap, 0.75, 250 ); break;
    case 0:
    default: break;
    }
  
} /* end pad_update_rumble */
