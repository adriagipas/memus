/*
 * Copyright 2015-2023 Adrià Giménez Pastor.
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
 *  rom.c - Implementació de 'rom.h'.
 *
 */


#include <ctype.h>
#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "MD.h"
#include "error.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

int
load_rom (
          const char     *fn,
          MD_Rom         *rom,
          const gboolean  verbose
          )
{
  
  FILE *f;
  long size;
  MD_Error err;
  

  f= NULL;
  rom->bytes= NULL;
  rom->words= NULL;
  
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
  if ( size == 0 || size%2 != 0 )
    {
      warning ( "la grandària de '%s' no és possible en una rom de MD", fn );
      goto error;
    }
  rom->nwords= size/2;
  MD_rom_alloc ( *rom );
  if ( rom->bytes == NULL ) { cerror_w (); goto error; }
  rewind ( f );
  if ( fread ( rom->bytes, size, 1, f ) != 1 )
    {
      warning ( "no s'ha pogut llegir el contingut de '%s'", fn );
      goto error;
    }
  fclose ( f );
  err= MD_rom_prepare ( rom );
  if ( err == MD_EMEM )
    {
      warning ( "no hi ha prou memòria per a carregar la rom: '%s'", fn );
      goto error;
    }

  return 0;

 error:
  MD_rom_free ( rom );
  if ( f != NULL ) fclose ( f );
  return -1;
  
} /* end load_rom */


const char *
get_rom_id (
            const MD_Rom       *rom,
            const MD_RomHeader *header,
            const int           verbose
            )
{
  
  /* 49(title ascii) + 32 (md5) + 15 (type) + 4 (checksum) + 3 (:) + 1
     (0).*/
  static char buffer[49+32+15+4+3+1];
  gchar *chk;
  size_t len;
  int i;
  char *p, aux[11];
  const char *q;
  
  
  p= buffer;
  for ( q= header->int_name; *q; ++q )
    *(p++)= isalnum ( *q ) ? *q : '_';
  *(p++)= '-';
  chk= g_compute_checksum_for_data ( G_CHECKSUM_MD5,
                                     (const guchar *) rom->bytes,
                                     rom->nwords*2 );
  len= strlen ( chk );
  assert ( len == 32 );
  p= buffer;
  for ( i= 0; i < 32; ++i ) *(p++)= chk[i];
  *(p++)= '-';
  for ( q= header->type_snumber; *q; ++q )
    *(p++)= isalnum ( *q ) ? *q : '_';
  *(p++)= '-';
  sprintf ( aux, "%04x", header->checksum );
  for ( q= aux; *q; ++q ) *(p++)= *q;
  *(p++)= '\0';
  
  if ( verbose )
    fprintf ( stderr, "L'identificador de la ROM és: %s\n", buffer );
  
  return buffer;
  
} /* end get_rom_id */
