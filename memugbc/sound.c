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
 *  sound.c - Implementació de 'sound.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include <SDL.h>

#include "error.h"
#include "sound.h"




/**********/
/* MACROS */
/**********/

#define RATE 44100


#define ABUF_SIZE 4096 // Un múltiple de 2




/*********/
/* ESTAT */
/*********/

static struct
{
  unsigned short v[ABUF_SIZE];
  unsigned int   pr; // Seguent posició a llegir.
  unsigned int   N;  // Número de mostres escrites.
} _abuf; // Buffer d'audio.
static SDL_AudioDeviceID _dev;
static SDL_AudioSpec _specs;
static double _offset;
static struct
{
  double        left;
  double        right;
  unsigned long N;
} _aliasing;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
aliasing_accum (
                const unsigned long prev_j,
                const unsigned long new_j,
                const double        left[GBC_APU_BUFFER_SIZE],
                const double        right[GBC_APU_BUFFER_SIZE]
                )
{
  
  unsigned long i,end;


  end= new_j < GBC_APU_BUFFER_SIZE ? new_j : GBC_APU_BUFFER_SIZE-1;
  for ( i= prev_j; i <= end; ++i )
    {
      _aliasing.left+= left[i];
      _aliasing.right+= right[i];
      ++_aliasing.N;
    }
  
} // end aliasing_accum




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_sound (void)
{
  
  SDL_PauseAudioDevice ( _dev, 1 );
  SDL_CloseAudioDevice ( _dev );
  
} // end close_sound


void
init_sound (void)
{
  
  SDL_AudioSpec aspec;
  
  
  // Inicialitza els buffers d'audio i l'offset.
  _abuf.pr= 0;
  _abuf.N= 0;
  _offset= 0;

  // Aliasing
  _aliasing.left= 0;
  _aliasing.right= 0;
  _aliasing.N= 0;
  
  // Obri el dispositiu d'audio.
  aspec.freq= RATE;
  aspec.format= AUDIO_S16SYS;
  aspec.channels= 2;
  aspec.samples= 1024;
  aspec.callback= NULL;//fill_audio_data;
  aspec.userdata= NULL;
  _dev= SDL_OpenAudioDevice ( NULL, 0, &aspec, &_specs, 0 );
  if ( _dev == 0 )
    error ( "no s'ha pogut obrir el dispositiu de so: %s", SDL_GetError () );
  assert ( aspec.freq == _specs.freq &&
           aspec.format == _specs.format &&
           aspec.channels == _specs.channels );
  SDL_PauseAudioDevice ( _dev, 0 );
  
} // end init_sound


void
sound_play (
            const double  left[GBC_APU_BUFFER_SIZE],
            const double  right[GBC_APU_BUFFER_SIZE],
            void         *udata
            )
{

  static const double SFACTOR=
    ((double) (GBC_APU_SAMPLES_PER_SEC) / (double) RATE);
  
  unsigned long j,new_j;
  int ret;

  
  j= (unsigned long) (_offset+0.5);
  aliasing_accum ( 0, j, left, right );
  while ( j < GBC_APU_BUFFER_SIZE )
    {
      if ( _aliasing.N )
        {
          _aliasing.left/= _aliasing.N;
          _aliasing.right/= _aliasing.N;
          _abuf.v[_abuf.N++]= (unsigned short) ((_aliasing.left*32768)+0.5);
          _abuf.v[_abuf.N++]= (unsigned short) ((_aliasing.right*32768)+0.5);
          _aliasing.left= 0.0;
          _aliasing.right= 0.0;
          _aliasing.N= 0;
          if ( _abuf.N == ABUF_SIZE )
            {
              ret= SDL_QueueAudio ( _dev, _abuf.v, sizeof(short)*ABUF_SIZE );
              if ( ret != 0 )
                error ( "sound_play: %s", SDL_GetError () );
              _abuf.N= 0;
            }
        }
      _offset+= SFACTOR;
      new_j= (unsigned long) (_offset+0.5);
      aliasing_accum ( j, new_j, left, right );
      j= new_j;
    }
  if ( j < GBC_APU_BUFFER_SIZE )
    {
      warning ( "S'han descartat mostres d'audio" );
      _offset= 0.0;
    }
  else _offset-= GBC_APU_BUFFER_SIZE;
  
} // end sound_play
