/*
 * Copyright 2025 Adrià Giménez Pastor.
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
 *  cmos.c - Implementació de 'cmos.h'.
 *
 */


#include <errno.h>
#include <glib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dirs.h"
#include "error.h"
#include "cmos.h"




/**********/
/* MACROS */
/**********/

#define CMOS_RAM_SIZE 256




/*********/
/* ESTAT */
/*********/

// Verbositat
static int _verbose;

// Memòria.
static uint8_t _mem[CMOS_RAM_SIZE];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_cmos_ram_file_name (void)
{
  return g_build_filename ( get_sharedir (), "cmos.ram", NULL );
} // end get_cmos_ram_file_name


static gchar *
get_tmp_file_name (void)
{
  return g_build_filename ( get_sharedir (), "cmos.tmp", NULL );
} // end get_cmos_tmp_file_name 




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_cmos (void)
{
  
  FILE *fd;
  gchar *name;
  gchar *tmp;
  
  
  // Obté el nom.
  name= get_cmos_ram_file_name ();
  if ( _verbose )
    fprintf ( stderr, "Escrivint la memòria de la CMOS en '%s'\n", name );
  
  // Escriu.
  tmp= get_tmp_file_name ();
  if ( g_file_test ( tmp, G_FILE_TEST_IS_REGULAR ) )
    error ( "el fitxer temporal '%s' ja existeix", tmp );
  fd= fopen ( tmp, "wb" );
  if ( fd == NULL )
    error ( "no s'ha pogut crear el fitxer temporal '%s': %s",
            tmp, strerror ( errno ) );
  if ( fwrite ( _mem, CMOS_RAM_SIZE, 1, fd ) != 1 )
    {
      fclose ( fd );
      remove ( tmp );
      error ( "no s'ha pogut escriure la CMOS: %s", strerror ( errno ) );
    }
  if ( fclose ( fd ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut acabar d'escriure la CMOS: %s",
              strerror ( errno ) );
    }
  if ( rename ( tmp, name ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut escriure la CMOS en '%s': %s",
              name, strerror ( errno ) );
    }
  
  // Allibera memòria.
  g_free ( tmp );
  g_free ( name );
  
} // end close_cmos


void
init_cmos (
           const bool verbose
           )
{
  _verbose= verbose;
} // end init_sram


uint8_t *
cmos_get_ram (
              void *udata
              )
{
  
  FILE *f;
  gchar *name;
  
  
  // Busca si existeix el fitxer i carrega'l o inicialitza a 0 si no
  // està.
  name= get_cmos_ram_file_name ();
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) )
    {
      if ( _verbose )
        fprintf ( stderr, "Llegint la memòria de la CMOS de '%s'\n", name );
      f= fopen ( name, "rb" );
      if ( f == NULL )
        error ( "no s'ha pogut obrir '%s': %s", name, strerror ( errno )  );
      if ( fread ( _mem, CMOS_RAM_SIZE, 1, f ) != 1 )
        error ( "no s'ha pogut llegir la CMOS de '%s': %s",
        	name, strerror ( errno ) );
      fclose ( f );
    }
  else memset ( _mem, 0, CMOS_RAM_SIZE );
  g_free ( name );
  
  return &_mem[0];
  
} // end cmos_get_ram
