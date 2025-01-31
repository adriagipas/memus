/*
 * Copyright 2016-2025 Adrià Giménez Pastor.
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
 *  mpad.c - Implementació de 'mpad.h'.
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>

#include "NES.h"
#include "mpad.h"
#include "pad.h"
#include "screen.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
mpad_clear (void)
{
  pad_clear ();
} /* end mpad_clear */


int
mpad_check_buttons (
                    mpad_mouse_callback_t  cb,
                    void                  *udata
                    )
{
  
  SDL_Event event;
  int ret, aux;
  

  // Comprova tecles.
  while ( screen_next_event ( &event ) )
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
        if ( event.key.keysym.mod&KMOD_CTRL &&
             event.key.keysym.sym == SDLK_q )
          return K_QUIT;
      default:
        if ( pad_event ( &event ) ) return K_ESCAPE;
      }
  
  // Reinterpreta les tecles del pad.
  aux= pad_check_buttons ( 0 ) | pad_check_buttons ( 1 );
  ret= 0;
  if ( aux&(1<<NES_UP) ) ret|= K_UP;
  if ( aux&(1<<NES_DOWN) ) ret|= K_DOWN;
  if ( aux&(1<<NES_A) ) ret|= K_A;
  if ( aux&(1<<NES_B) ) ret|= K_B;
  if ( aux&(1<<NES_SELECT) ) ret|= K_SELECT;
  if ( aux&(1<<NES_START) ) ret|= K_START;
  if ( (ret&(K_UP|K_DOWN)) == (K_UP|K_DOWN) ) ret&= ~K_DOWN;
  
  return ret;
  
} // end mpad_check_buttons
