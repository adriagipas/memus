/*
 * Copyright 2008-2009,2022 Adrià Giménez Pastor.
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
 *  error.c - Implementació de 'error.h'.
 *
 */


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
print_msg (
           const char *type,
           const char *format,
           va_list     ap
           )
{
  
  fprintf ( stderr, "%s ", type );
  vfprintf ( stderr, format, ap );
  fprintf ( stderr, "\n" );
  
} /* end verror */


/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
error (
       const char *format,
       ...
       )
{

  va_list ap;


  va_start ( ap, format );
  print_msg ( "[EE]", format, ap );
  va_end ( ap );

  exit ( EXIT_FAILURE );

} /* end error */


void
warning (
         const char *format,
         ...
         )
{

  va_list ap;


  va_start ( ap, format );
  print_msg ( "[WW]", format, ap );
  va_end ( ap );

} /* end warning */


void
verror (
        const char *format,
        va_list     ap
        )
{
  
  print_msg ( "Error", format, ap );
  exit ( EXIT_FAILURE );
  
} /* end verror */


void
vwarning (
          const char *format,
          va_list     ap
          )
{
  print_msg ( "[WW]", format, ap );
} /* end vwarning */
