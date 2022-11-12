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
 *  rom.c - Implementació de 'rom.h'.
 *
 */


#include <assert.h>
#include <ctype.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "GBC.h"
#include "error.h"
#include "rom.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

int
load_rom (
          const char *fn,
          GBC_Rom    *rom,
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
  if ( size == 0 || size%(GBC_BANK_SIZE) != 0 )
    {
      warning ( "la grandària de '%s' no és possible en una rom de GBC", fn );
      goto error;
    }
  rom->nbanks= size/GBC_BANK_SIZE;
  GBC_rom_alloc ( *rom );
  if ( rom->banks == NULL ) { cerror_w (); goto error; }
  rewind ( f );
  if ( fread ( rom->banks, GBC_BANK_SIZE, rom->nbanks, f ) != rom->nbanks )
    {
      warning ( "no s'ha pogut llegir el contingut de '%s'", fn );
      goto error;
    }
  fclose ( f );

  return 0;

 error:
  GBC_rom_free ( *rom );
  if ( f != NULL ) fclose ( f );
  return -1;
  
} /* end load_rom */


const char *
get_rom_id (
            const GBC_Rom       *rom,
            const GBC_RomHeader *header,
            const int            verbose
            )
{

  /* 16max (title) + 32 (md5) + 4 (global checksum) + 2 (checksum) +
     3max (version) + 4 (:) + 1 (0). */
  static char buffer[16+32+4+2+3+4+1];
  gchar *chk;
  size_t len;
  int i;
  char *p, aux[11];
  const char *q;
  
  
  chk= g_compute_checksum_for_data ( G_CHECKSUM_MD5,
        			     (const guchar *) rom->banks,
        			     rom->nbanks*GBC_BANK_SIZE );
  len= strlen ( chk );
  assert ( len == 32 );
  p= buffer;
  for ( q= header->title; *q; ++q )
    *(p++)= isalnum ( *q ) ? *q : '_';
  *(p++)= '-';
  for ( i= 0; i < 32; ++i ) *(p++)= chk[i];
  *(p++)= '-';
  sprintf ( aux, "%04x", header->global_checksum );
  for ( q= aux; *q; ++q ) *(p++)= *q;
  *(p++)= '-';
  sprintf ( aux, "%02x", header->checksum );
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
