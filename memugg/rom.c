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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  rom.c - Implementació de 'rom.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "GG.h"
#include "error.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

int
load_rom (
          const char *fn,
          GG_Rom     *rom,
          const int   verbose
          )
{

  FILE *f;
  long size;
  

  f= NULL;
  rom->banks= NULL;
  
  if ( verbose )
    fprintf ( stderr, "Carregant la ROM de '%s'\n", fn );
  
  f= fopen ( fn, "rb" );
  if ( f == NULL )
    {
      warning ( "no s'ha pogut obrir '%s'", fn );
      goto error;
    }
  if ( fseek ( f, 0, SEEK_END ) == -1 ) { cerror_w (); goto error; }
  size= ftell ( f );
  if ( size == -1 ) { cerror_w (); goto error; }
  if ( size == 0 || size%GG_BANK_SIZE != 0 )
    {
      warning ( "la grandària de '%s' no es possible en una rom de GG", fn );
      goto error;
    }
  rom->nbanks= size/GG_BANK_SIZE;
  GG_rom_alloc ( *rom );
  if ( rom->banks == NULL ) { cerror_w (); goto error; }
  rewind ( f );
  if ( fread ( rom->banks, GG_BANK_SIZE, rom->nbanks, f ) != rom->nbanks )
    {
      warning ( "no s'ha pogut llegir el contingut de '%s'", fn );
      goto error;
    }
  fclose ( f );

  return 0;
  
 error:
  GG_rom_free ( *rom );
  if ( f != NULL ) fclose ( f );
  return -1;
  
} /* end load_rom */


const char *
get_rom_id (
            const GG_Rom       *rom,
            const GG_RomHeader *header,
            const int           verbose
            )
{
  
  /* 32 (md5) + 10 (codi) + 3 (versio) + 2 (:) + 1 (0). */
  static char buffer[32+1+10+1+3+1];
  gchar *chk;
  size_t len;
  int i;
  char *p, aux[11];
  const char *q;
  
  
  chk= g_compute_checksum_for_data ( G_CHECKSUM_MD5,
        			     (const guchar *) rom->banks,
        			     rom->nbanks*GG_BANK_SIZE );
  len= strlen ( chk );
  assert ( len == 32 );
  p= buffer;
  for ( i= 0; i < 32; ++i ) *(p++)= chk[i];
  *(p++)= '-';
  sprintf ( aux, "%d", header->code );
  for ( q= aux; *q; ++q ) *(p++)= *q;
  *(p++)= '-';
  sprintf ( aux, "%d", header->version );
  for ( q= aux; *q; ++q ) *(p++)= *q;
  *(p++)= '\0';
  g_free ( chk );
  
  if ( verbose )
    fprintf ( stderr, "L'identificador de la ROM és: %s\n", buffer );
  
  return buffer;
  
} /* end get_rom_id */
