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
 *  load_bios.c - Implementació de 'load_bios.h'.
 *
 */


#include <stddef.h>
#include <stdlib.h>

#include "error.h"
#include "load_bios.h"




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

const GBCu8 *
load_bios (
           conf_t    *conf,
           const int  verbose
           )
{

  static GBCu8 bios[0x900];
  
  FILE *f;
  long size;
  

  if ( conf->bios_fn == NULL ) return NULL;
  
  if ( verbose )
    fprintf ( stderr, "Carregant la BIOS de '%s'\n", conf->bios_fn );
  
  f= fopen ( conf->bios_fn, "rb" );
  if ( f == NULL )
    {
      warning ( "no s'ha pogut obrir '%s'", conf->bios_fn );
      goto error;
    }
  if ( fseek ( f, 0, SEEK_END ) == -1 ) { cerror_w (); goto error; }
  size= ftell ( f );
  if ( size == -1 ) { cerror_w (); goto error; }
  if ( size != 0x900 )
    {
      warning ( "la grandària de '%s' no és possible per a ser una BIOS",
        	conf->bios_fn );
      goto error;
    }
  rewind ( f );
  if ( fread ( bios, 0x900, 1, f ) != 1 )
    {
      warning ( "no s'ha pogut llegir el contingut de '%s'", conf->bios_fn );
      goto error;
    }
  fclose ( f );
  
  return &(bios[0]);

 error:
  conf_set_bios_fn ( conf, NULL );
  warning ( "s'ha deshabilitat la BIOS" );
  return NULL;
  
} /* end load_bios */
