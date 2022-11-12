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


#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>

#include "GBC.h"
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
mpad_check_buttons (void)
{
  
  SDL_Event event;
  int ret, aux;
  GBC_Bool junk;
  
  
  /* Comprova tecles. */
  while ( screen_next_event ( &event ) )
    switch ( event.type )
      {
      case SDL_QUIT: return K_QUIT;
      case SDL_KEYDOWN:
        if ( event.key.keysym.mod&KMOD_CTRL &&
             event.key.keysym.sym == SDLK_q )
          return K_QUIT;
      default:
        if ( pad_event ( &event, &junk, &junk ) ) return K_ESCAPE;
      }
  
  /* Reinterpreta les tecles del pad. */
  aux= pad_check_buttons ( NULL );
  ret= 0;
  if ( aux&GBC_UP ) ret|= K_UP;
  if ( aux&GBC_DOWN ) ret|= K_DOWN;
  if ( aux&GBC_BUTTON_A ) ret|= K_BUTA;
  if ( aux&GBC_BUTTON_B ) ret|= K_BUTB;
  if ( aux&GBC_SELECT ) ret|= K_SELECT;
  if ( aux&GBC_START ) ret|= K_START;
  
  return ret;
  
} /* end mpad_check_buttons */
