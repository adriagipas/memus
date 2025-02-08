/*
 * Copyright 2022-2025 Adrià Giménez Pastor.
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
 *  session.c - Implementació de 'session.h'.
 *
 */

#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "session.h"




/*********/
/* ESTAT */
/*********/

static gchar *_name;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_session (void)
{

  if ( _name != NULL )
    g_free ( _name );
  
} // end close_session


void
init_session (
              const gchar    *name,
              const gboolean  verbose
              )
{
  
  const gchar *p;

  
  if ( name != NULL )
    {
      for ( p= name; *p != '\0'; ++p )
        if ( *p != '_' && (*p < 'a' || *p > 'z') && (*p < '0' || *p > '9') )
          error ( "%s no és un nom de sessió vàlid, sols"
                  " s'accepten noms amb caràcters [a-z_0-9]",
                  name );
      _name= g_strdup ( name );
      if ( verbose )
        fprintf ( stderr, "Executant amb la sessió: %s\n", _name );
    }
  else
    {
      _name= NULL;
      if ( verbose )
        fprintf ( stderr, "Utilitzant la sessió per defecte\n" );
    }
  
} // end init_session


const gchar *
session_get_name (void)
{
  return _name;
} // end session_get_name
