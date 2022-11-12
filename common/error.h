/*
 * Copyright 2008,2015,2022 Adrià Giménez Pastor.
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
 *  error.h - Per a gestionar els errors i avisos.
 *
 */

#ifndef __CPROG_ERROR_H__
#define __CPROG_ERROR_H__

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>


/* MACROS */

/* Error de la llibreria C. */
#define cerror()                     \
        error ( (errno != 0) ?       \
                strerror ( errno ) : \
                "'errno' is 0" )

/* com cerror pero fà un warning. */
#define cerror_w()                     \
        warning ( (errno != 0) ?       \
        	  strerror ( errno ) : \
        	  "'errno' is 0" )


/* FUNCIONS */

/* Mostra un missatge d'error i ix. */
void
error (
       const char *format,
       ...
       );

/* Mostra un missatge d'avís. */
void
warning (
         const char *format,
         ...
         );

/* Equivalent a 'error' però amb 'va_list'. */
void
verror (
        const char *format,
        va_list     ap
        );

/* Equivalent a 'warning' però amb 'va_list'. */
void
vwarning (
          const char *format,
          va_list     ap
          );


#endif /* __CPROG_ERROR_H__ */
