/*
 * Copyright 2015-2025 Adrià Giménez Pastor.
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
 *  mpad.c - Implementació de 'mpad.h'.
 *
 */


#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "mpad.h"
#include "screen.h"




/*********/
/* ESTAT */
/*********/

static int _state= 0;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
mpad_clear (void)
{
  _state= 0;
} // end mpad_clear


int
mpad_check_buttons (
                    const mouse_area_t    *mouse_area,
                    mpad_mouse_callback_t  cb,
                    void                  *udata
                    )
{
  
  SDL_Event event;
  
  
  // Comprova tecles.
  while ( screen_next_event ( &event, mouse_area ) )
    switch ( event.type )
      {
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEWHEEL:
        if ( cb != NULL ) cb ( &event, udata );
        break;
      case SDL_QUIT: return K_QUIT;
      case SDL_KEYDOWN:
        if ( (event.key.keysym.mod&(KMOD_LCTRL|KMOD_LGUI)) ==
             (KMOD_LCTRL|KMOD_LGUI) &&
             event.key.keysym.sym == SDLK_q )
          return K_QUIT;
        else
          {
            switch ( event.key.keysym.sym )
              {
              case SDLK_ESCAPE: return K_ESCAPE;
              case SDLK_DOWN: _state|= K_DOWN; break;
              case SDLK_UP: _state|= K_UP; break;
              case SDLK_SPACE: _state|= K_BUTTON1; break;
              case SDLK_RETURN: _state|= K_BUTTON2; break;
              }
          }
        break;
      case SDL_KEYUP:
        switch ( event.key.keysym.sym )
          {
          case SDLK_DOWN: _state&= ~K_DOWN; break;
          case SDLK_UP: _state&= ~K_UP; break;
          case SDLK_SPACE: _state&= ~K_BUTTON1; break;
          case SDLK_RETURN: _state&= ~K_BUTTON2; break;
          }
        break;
      default:
      }
  
  // Reajusta.
  if ( (_state&(K_UP|K_DOWN)) == (K_UP|K_DOWN) ) _state&= ~K_DOWN;
  
  return _state;
  
} // end mpad_check_buttons
