/*
 * Copyright 2016-2022 Adrià Giménez Pastor.
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
 *  sram.c - Implementació de 'sram.h'.
 *
 */


#include <assert.h>
#include <errno.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dirs.h"
#include "error.h"
#include "sram.h"




/**********/
/* MACROS */
/**********/

#define SIZE 0x2000




/*********/
/* ESTAT */
/*********/

/* Identificador de ROM i verbositat. */
static const char *_rom_id;
static int _verbose;

/* Fitxer opcional on emmagatzemar la memòria estàtica. Pot ser
   NULL. */
static const char *_sram_fn;

/* Memòria. */
static NESu8 *_mem;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_sram_file_name (void)
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_sharedir (), _rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".sram" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_sram_file_name */


static gchar *
get_tmp_file_name (void)
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_sharedir (), _rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".tmp" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_tmp_file_name */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_sram (void)
{
  
  FILE *fd;
  const char *name;
  char *tmp;
  
  
  if ( _mem == NULL ) return;
  
  /* Obté el nom. */
  name= _sram_fn==NULL ? get_sram_file_name () : _sram_fn;
  if ( _verbose )
    fprintf ( stderr, "Escrivint la memòria estàtica en '%s'\n", name );
  
  /* Escriu. */
  tmp= get_tmp_file_name ();
  if ( g_file_test ( tmp, G_FILE_TEST_IS_REGULAR ) )
    error ( "el fitxer temporal '%s' ja existeix", tmp );
  fd= fopen ( tmp, "wb" );
  if ( fd == NULL )
    error ( "no s'ha pogut crear el fitxer temporal '%s': %s",
            tmp, strerror ( errno ) );
  if ( fwrite ( _mem, SIZE, 1, fd ) != 1 )
    {
      fclose ( fd );
      remove ( tmp );
      error ( "no s'ha pogut escriure la SRAM: %s", strerror ( errno ) );
    }
  if ( fclose ( fd ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut acabar d'escriure la SRAM: %s",
              strerror ( errno ) );
    }
  if ( rename ( tmp, name ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut escriure la SRAM en '%s': %s",
              name, strerror ( errno ) );
    }
  
  /* Allibera memòria. */
  g_free ( tmp );
  if ( name != _sram_fn ) g_free ( (void *) name );
  g_free ( _mem );
  _mem= NULL;
  
} /* end close_sram */


void
init_sram (
           const char *rom_id,
           const char *sram_fn,
           const int   verbose
           )
{
  
  _rom_id= rom_id;
  _sram_fn= sram_fn;
  _verbose= verbose;
  _mem= NULL;
  
} /* end init_sram */


NESu8 *
sram_get_static_ram (void)

{
  
  FILE *f;
  const gchar *name;
  long size;
  
  
  assert ( _mem == NULL );
  
  _mem= g_new ( NESu8, SIZE );
  
  /* Busca si existeix el fitxer i carrega'l o inicialitza a 0 si no
     està. */
  name= _sram_fn == NULL ? get_sram_file_name () : _sram_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) )
    {
      if ( _verbose )
        fprintf ( stderr, "Llegint la SRAM de '%s'\n", name );
      f= fopen ( name, "rb" );
      if ( f == NULL )
        error ( "no s'ha pogut obrir '%s': %s", name, strerror ( errno )  );
      if ( fseek ( f, 0, SEEK_END ) == -1 ) cerror ();
      size= ftell ( f );
      if ( size == -1 ) cerror ();
      if ( size != SIZE )
        error ( "la grandària de '%s' és %ul però s'esperava %lu",
        	name, size, SIZE );
      rewind ( f );
      if ( fread ( _mem, SIZE, 1, f ) != 1 )
        error ( "no s'ha pogut llegir la SRAM de '%s': %s",
        	name, strerror ( errno ) );
      fclose ( f );
    }
  else memset ( _mem, 0, SIZE );
  if ( name != _sram_fn ) g_free ( (void *) name );
  
  return _mem;
  
} /* end sram_get_external_ram */
