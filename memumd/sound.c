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

#define ABUF_SIZE 22050 /* per a 44100 en estero, medit en shorts, 0.25s. */




/*************/
/* CONSTANTS */
/*************/

#define AUDIO_FREQ_PAL                                          \
  (MD_CYCLES_PER_SEC_PAL/(double) MD_CPU_CYCLES_PER_FM_SAMPLE)

#define AUDIO_FREQ_NTSC                                         \
  (MD_CYCLES_PER_SEC_NTSC/(double) MD_CPU_CYCLES_PER_FM_SAMPLE)




/*********/
/* ESTAT */
/*********/

static struct
{
  unsigned short v[ABUF_SIZE];
  unsigned int   pr; /* Seguent posició a llegir. */
  unsigned int   pw; /* Següent posició a escriure. */
  unsigned int   N;  /* Número de mostres escrites. */
} _abuf; /* Buffer d'audio. */
static SDL_AudioDeviceID _dev;
static SDL_AudioSpec _specs;
static double _offset;
static double _sfactor;




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
  
  short *p;
  
  
  /* Prepara el stream. */
  len/= 2;
  p= (short *) stream;
  
  /* Plena el que es puga del nostre buffer. */
  while ( len && _abuf.N )
    {
      *(p++)= _abuf.v[_abuf.pr];
      if ( ++(_abuf.pr) == ABUF_SIZE ) _abuf.pr= 0;
      --len;
      --(_abuf.N);
    }
  
  /* Ompli amb silencis el que quede. */
  if ( !len ) return;
  while ( len )
    {
      *(p++)= _specs.silence;
      --len;
    }
  
} /* end fill_audio_data */




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
  _abuf.pw= 0;
  _abuf.N= 0;
  _offset= 0;
  sound_change_freq ( false ); // Fica valor inicial
  
  // Obri el dispositiu d'audio.
  aspec.freq= 44100;
  aspec.format= AUDIO_S16SYS;
  aspec.channels= 2;
  aspec.samples= 1024;
  aspec.callback= fill_audio_data;
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
            const MDs16  samples[MD_FM_BUFFER_SIZE*2],
            void        *udata
            )
{
  
  unsigned long j;
  
  
  SDL_LockAudioDevice ( _dev );
  
  j= (unsigned long) (_offset+0.5);
  while ( j < MD_FM_BUFFER_SIZE && _abuf.N < (ABUF_SIZE-1) )
    {
      _abuf.v[_abuf.pw]= (unsigned short) samples[2*j];
      if ( ++_abuf.pw == ABUF_SIZE ) _abuf.pw= 0;
      _abuf.v[_abuf.pw]= (unsigned short) samples[2*j+1];
      if ( ++_abuf.pw == ABUF_SIZE ) _abuf.pw= 0;
      _abuf.N+= 2;
      _offset+= _sfactor;
      j= (unsigned long) (_offset+0.5);
    }
  if ( j < MD_FM_BUFFER_SIZE )
    {
      warning ( "S'han descartat mostres d'audio" );
      _offset= 0.0;
    }
  else _offset-= MD_FM_BUFFER_SIZE;
  
  SDL_UnlockAudioDevice ( _dev );
  
} // end sound_play


void
sound_change_freq (
                   const bool pal // false NTSC
                   )
{
  
  if ( pal )
    _sfactor= ((double) AUDIO_FREQ_PAL  / (double) RATE);
  else
    _sfactor= ((double) AUDIO_FREQ_NTSC  / (double) RATE);
  
} // end sound_change_freq
