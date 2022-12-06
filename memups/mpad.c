/*
 * Copyright 2015-2022 Adrià Giménez Pastor.
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

#include "PSX.h"
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
} // end mpad_clear


int
mpad_check_buttons (void)
{
  
  SDL_Event event;
  int ret, aux;
  const PSX_ControllerState *state;
  

  // Comprova tecles.
  while ( screen_next_event ( &event ) )
    switch ( event.type )
      {
      case SDL_QUIT: return K_QUIT;
      case SDL_KEYDOWN:
        if ( event.key.keysym.mod&KMOD_CTRL &&
             event.key.keysym.sym == SDLK_q )
          return K_QUIT;
      default:
        if ( pad_event ( &event ) ) return K_ESCAPE;
      }
  
  // Reinterpreta les tecles del pad. */
  aux= 0;
  state= pad_get_controller_state ( 0, NULL );
  aux|= state->buttons;
  state= pad_get_controller_state ( 1, NULL );
  aux|= state->buttons;
  ret= 0;
  if ( aux&PSX_BUTTON_UP ) ret|= K_UP;
  if ( aux&PSX_BUTTON_DOWN ) ret|= K_DOWN;
  if ( aux&PSX_BUTTON_CROSS ) ret|= K_BUTTON;
  if ( aux&PSX_BUTTON_CIRCLE ) ret|= K_ESCAPE;
  if ( aux&PSX_BUTTON_START ) ret|= K_BUTTON;
  if ( (ret&(K_UP|K_DOWN)) == (K_UP|K_DOWN) ) ret&= ~K_DOWN;
  
  return ret;
  
} // end mpad_check_buttons
