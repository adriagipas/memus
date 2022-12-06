/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
#include <stdint.h>
#include <stdlib.h>
#include <SDL.h>

#include "error.h"
#include "sound.h"




/**********/
/* MACROS */
/**********/

#define NBUFF 4



/*********/
/* TIPUS */
/*********/

typedef struct
{
  
  int16_t      *v;
  volatile int  full;
  
} buffer_t;




/*********/
/* ESTAT */
/*********/

static struct
{
  buffer_t buffers[NBUFF];
  int      buff_in;
  int      buff_out;
  int      pos;
  int      size;
  int      nsamples;
  double   ratio;
  double   pos2;
  char     silence;
} _abuf; // Buffers d'audio.

static SDL_AudioDeviceID _dev;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
fill_audio_data (
                 void  *userdata,
                 Uint8 *stream,
                 int    len
                 )
{
  
  int i;
  const int16_t *buffer;
  
  
  assert ( _abuf.size == len );
  if ( _abuf.buffers[_abuf.buff_out].full )
    {
      buffer= _abuf.buffers[_abuf.buff_out].v;
      for ( i= 0; i < len; ++i )
        stream[i]= ((Uint8 *) buffer)[i];
      _abuf.buffers[_abuf.buff_out].full= 0;
      _abuf.buff_out= (_abuf.buff_out+1)%NBUFF;
    }
  else
    {
      /*printf("XOF!!\n");*/
      for ( i= 0; i < len; ++i ) stream[i]= _abuf.silence;
    }
  
} // end fill_audio_data




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_sound (void)
{
  
  SDL_PauseAudioDevice ( _dev, 1 );
  SDL_CloseAudioDevice ( _dev );
  g_free ( _abuf.buffers[0].v );
  
} // end close_sound


void
init_sound (void)
{
  
  SDL_AudioSpec aspec,specs;
  int n;
  Uint8 *mem;
  
  
  // Inicialitza els buffers d'audio i l'offset.
  _abuf.buff_out= _abuf.buff_in= 0;
  for ( n= 0; n < NBUFF; ++n ) _abuf.buffers[n].full= 0;
  
  // Obri el dispositiu d'audio.
  aspec.freq= 44100;
  aspec.format= AUDIO_S16SYS;
  aspec.channels= 2;
  aspec.samples= 2048;
  aspec.size= 8192;
  aspec.callback= fill_audio_data;
  aspec.userdata= NULL;
  _dev= SDL_OpenAudioDevice ( NULL, 0, &aspec, &specs, 0 );
  if ( _dev == 0 )
    error ( "no s'ha pogut obrir el dispositiu de so: %s", SDL_GetError () );
  assert ( aspec.format == specs.format &&
           aspec.channels == specs.channels );
  SDL_PauseAudioDevice ( _dev, 0 );

  // Inicialitza estat.
  mem= g_new ( Uint8, specs.size*NBUFF );
  for ( n= 0; n < NBUFF; ++n, mem+= specs.size )
    _abuf.buffers[n].v= (int16_t *) mem;
  _abuf.silence= (char) specs.silence;
  _abuf.pos= 0;
  _abuf.size= specs.size;
  _abuf.nsamples= _abuf.size/2;
  if ( specs.freq > 44100 )
    error ( "init_sound - Frequüencia massa gran: %d\n", specs.freq );
  _abuf.ratio= 44100 / (double) specs.freq;
  _abuf.pos2= 0.0;
    
} // end init_sound


void
sound_play (
            const int16_t  samples[PSX_AUDIO_BUFFER_SIZE*2],
            void          *udata
            )
{
  
  int nofull, j;
  int16_t *buffer;
  
  
  for (;;)
    {
      
      while ( _abuf.buffers[_abuf.buff_in].full ) SDL_Delay ( 1 );
      buffer= _abuf.buffers[_abuf.buff_in].v;
      
      j= (int) (_abuf.pos2 + 0.5);
      while ( (nofull= (_abuf.pos != _abuf.nsamples)) &&
              j < PSX_AUDIO_BUFFER_SIZE )
        {
          buffer[_abuf.pos++]= samples[2*j];
          buffer[_abuf.pos++]= samples[2*j+1];
          _abuf.pos2+= _abuf.ratio;
          j= (int) (_abuf.pos2 + 0.5);
        }
      if ( !nofull )
        {
          _abuf.pos= 0;
          _abuf.buffers[_abuf.buff_in].full= 1;
          _abuf.buff_in= (_abuf.buff_in+1)%NBUFF;
        }
      if ( j >= PSX_AUDIO_BUFFER_SIZE )
        {
          _abuf.pos2-= PSX_AUDIO_BUFFER_SIZE;
          break;
        }
      
    }
  
} // end sound_play
