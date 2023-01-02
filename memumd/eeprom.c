/*
 * Copyright 2016-2023 Adrià Giménez Pastor.
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
 *  eeprom.c - Implementació de 'eeprom.h'.
 *
 *  Adrià Giménez Pastor, 2016.
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
#include "eeprom.h"




/*********/
/* ESTAT */
/*********/

/* Identificador de ROM i verbositat. */
static const char *_rom_id;
static int _verbose;

/* Fitxer opcional on emmagatzemar la memòria estàtica. Pot ser
   NULL. */
static const char *_eeprom_fn;

/* Memòria. */
static MDu8 *_mem;
static size_t _size;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_eeprom_file_name (void)
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_sharedir (), _rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".eeprom" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_eeprom_file_name */


static gchar *
get_tmp_file_name (void)
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_sharedir (), _rom_id, NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".tmp_eeprom" );
  
  return g_string_free ( buffer, FALSE );
  
} /* end get_tmp_file_name */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_eeprom (void)
{
  
  FILE *fd;
  const char *name;
  char *tmp;
  
  
  if ( _mem == NULL ) return;
  
  /* Obté el nom. */
  name= _eeprom_fn==NULL ? get_eeprom_file_name () : _eeprom_fn;
  if ( _verbose )
    fprintf ( stderr, "Escrivint la memòria de l'EEPROM en '%s'\n", name );
  
  /* Escriu. */
  tmp= get_tmp_file_name ();
  if ( g_file_test ( tmp, G_FILE_TEST_IS_REGULAR ) )
    error ( "el fitxer temporal '%s' ja existeix", tmp );
  fd= fopen ( tmp, "wb" );
  if ( fd == NULL )
    error ( "no s'ha pogut crear el fitxer temporal '%s': %s",
            tmp, strerror ( errno ) );
  if ( fwrite ( _mem, _size, 1, fd ) != 1 )
    {
      fclose ( fd );
      remove ( tmp );
      error ( "no s'ha pogut escriure l'EEPROM: %s", strerror ( errno ) );
    }
  if ( fclose ( fd ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut acabar d'escriure l'EEPROM: %s",
              strerror ( errno ) );
    }
  if ( rename ( tmp, name ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut escriure l'EEPROM en '%s': %s",
              name, strerror ( errno ) );
    }
  
  /* Allibera memòria. */
  g_free ( tmp );
  if ( name != _eeprom_fn ) g_free ( (void *) name );
  g_free ( _mem );
  _mem= NULL;
  
} /* end close_eeprom */


void
init_eeprom (
             const char *rom_id,
             const char *eeprom_fn,
             const int   verbose
             )
{
  
  _rom_id= rom_id;
  _eeprom_fn= eeprom_fn;
  _verbose= verbose;
  _mem= NULL;
  
} /* end init_eeprom */


MDu8 *
eeprom_get_eeprom (
        	   const size_t  nbytes,
        	   const MDu8    init_val,
        	   void         *udata
        	   )
{
  
  FILE *f;
  const gchar *name;
  long size;
  size_t i;
  
  
  assert ( _mem == NULL && nbytes > 0 );
  
  _mem= g_new ( MDu8, nbytes );
  _size= nbytes;
  
  /* Busca si existeix el fitxer i carrega'l o inicialitza a 0xFF si no
     està. */
  name= _eeprom_fn == NULL ? get_eeprom_file_name () : _eeprom_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) )
    {
      if ( _verbose )
        fprintf ( stderr, "Llegint l'EEPROM de '%s'\n", name );
      f= fopen ( name, "rb" );
      if ( f == NULL )
        error ( "no s'ha pogut obrir '%s': %s", name, strerror ( errno )  );
      if ( fseek ( f, 0, SEEK_END ) == -1 ) cerror ();
      size= ftell ( f );
      if ( size == -1 ) cerror ();
      if ( size != _size )
        error ( "la grandària de '%s' és %ul però s'esperava %lu",
        	name, size, _size );
      rewind ( f );
      if ( fread ( _mem, _size, 1, f ) != 1 )
        error ( "no s'ha pogut llegir l'EEPROM de '%s': %s",
        	name, strerror ( errno ) );
      fclose ( f );
    }
  else for ( i= 0; i < _size; ++i ) _mem[i]= init_val;
  if ( name != _eeprom_fn ) g_free ( (void *) name );
  
  return _mem;
  
} /* end eeprom_get_eeprom */