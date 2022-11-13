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
 *  rom.c - Implementació de 'rom.h'.
 *
 */


#include <ctype.h>
#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "NES.h"
#include "error.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

int
load_rom (
          const char     *fn,
          NES_Rom        *rom,
          const gboolean  verbose
          )
{
  
  FILE *f;

  
  f= NULL;
  
  if ( verbose )
    fprintf ( stderr, "Carregant la ROM de '%s'\n", fn );
  
  f= fopen ( fn, "rb" );
  if ( f == NULL )
    {
      warning ( "no s'ha pogut obrir '%s'", fn );
      goto error;
    }
  if ( NES_rom_load_from_ines ( f, rom ) != 0 )
    {
      warning ( "'%s' no és un fitxer iNES vàlid" );
      goto error;
    }
  fclose ( f );
  
  return 0;

 error:
  if ( f != NULL ) fclose ( f );
  return -1;
  
} /* end load_rom */


const char *
get_rom_id (
            const NES_Rom *rom,
            const int      verbose
            )
{
  
  /* 32 (md5) + 1 (0).*/
  static char buffer[32+1];
  gchar *chk;
  size_t len;
  int i;
  char *p;
  
  
  p= buffer;
  chk= g_compute_checksum_for_data ( G_CHECKSUM_MD5,
                                     (const guchar *) rom->prgs,
                                     rom->nprg*NES_PRG_SIZE );
  len= strlen ( chk );
  assert ( len == 32 );
  p= buffer;
  for ( i= 0; i < 32; ++i ) *(p++)= chk[i];
  *(p++)= '\0';
  
  if ( verbose )
    fprintf ( stderr, "L'identificador de la ROM és: %s\n", buffer );
  
  return buffer;
  
} /* end get_rom_id */
