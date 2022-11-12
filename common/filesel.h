/*
 * Copyright 2015,2022 Adrià Giménez Pastor.
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
 *  filesel.h - La llògica d'un selector de fitxers.
 *
 */

#ifndef __FILESEL_H__
#define __FILESEL_H__

#include <glib.h>

/* NO MODIFICAR DIRECTAMENT. */
typedef struct
{
  
  GRegex *pat;
  GRegex *pat_dots;
  GRegex *pat_sep;
  GRegex *pat_hidden;
  gchar  *cdir;
  GSList *dirs;
  GSList *files;
  
} filesel_t;

/* Si el path no és valid torna al HOME. */
void
filesel_change_dir (
        	    filesel_t   *fsel,
        	    const gchar *path
        	    );

void
filesel_change_parent_dir (
        		   filesel_t *fsel
        		   );

void
filesel_free (
              filesel_t *fsel
              );

/* TORNA (const gchar *) */
#define filesel_get_current_dir(FSEL) ((const gchar *) (FSEL)->cdir)

/* TORNA (const GSList *) */
#define filesel_get_dirs(FSEL) ((const GSList *) (FSEL)->dirs)
#define filesel_get_files(FSEL) ((const GSList *) (FSEL)->files)

/* Crea un nou selector de fitxers. 'path' és el path inicial, si no
   existeix, o no és un directori, o no és absolut, o gasta "..", o
   acaba amb separador, aleshores s'estableix el directori home de
   l'usuari. pattern és el patró que tenen que complir els fitxers
   regulars per a ser llistats. En cas d'error para l'execució. */
filesel_t *
filesel_new (
             const gchar *path,
             const gchar *pattern
             );

#endif /* __FILESEL_H__ */
