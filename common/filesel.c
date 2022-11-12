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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  filesel.c - Implementació de 'filesel.h'.
 *
 */

#include "error.h"
#include "filesel.h"



/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gboolean
test_path (
           const filesel_t *fsel,
           const gchar     *path
           )
{
  return ( path != NULL &&
           g_path_is_absolute ( path ) &&
           g_file_test ( path, G_FILE_TEST_IS_DIR ) &&
           !g_regex_match ( fsel->pat_dots, path, 0, NULL ) &&
           !g_regex_match ( fsel->pat_sep, path, 0, NULL ) );
} /* end test_path */


static void
set_cdir (
          filesel_t   *fsel,
          const gchar *path
          )
{

  if ( fsel->cdir != NULL ) g_free ( fsel->cdir );

  /* Si no és un path vàlid fiquem el HOME. */
  if ( !test_path ( fsel, path ) )
    {
      fsel->cdir= g_strdup ( g_get_home_dir () );
      if ( !test_path ( fsel, fsel->cdir ) )
        error ( "l'usuari no té 'HOME'" );
    }
  else fsel->cdir= g_strdup ( path );
  
} /* end set_cdir */


static void
clean_lists (
             filesel_t *fsel
             )
{

  g_slist_free_full ( fsel->dirs, g_free );
  g_slist_free_full ( fsel->files, g_free );
  fsel->dirs= NULL;
  fsel->files=NULL;
  
} /* end clean_lists */


/* Torna -1 en cas d'error.  */
static int
populate_lists (
        	filesel_t *fsel,
        	GError    **err
        	)
{

  GDir *dir;
  const gchar *p;
  gchar *aux;
  
  
  /* Obri. */
  dir= g_dir_open ( fsel->cdir, 0, err );
  if ( dir == NULL ) return -1;

  /* Ompli. */
  while ( (p= g_dir_read_name ( dir )) != NULL )
    {
      aux= g_build_path ( G_DIR_SEPARATOR_S, fsel->cdir, p, NULL );
      if ( g_file_test ( aux, G_FILE_TEST_IS_DIR ) &&
           !g_regex_match ( fsel->pat_hidden, aux, 0, NULL ) )
        fsel->dirs= g_slist_prepend ( fsel->dirs, aux );
      else if ( g_file_test ( aux, G_FILE_TEST_IS_REGULAR ) &&
        	g_regex_match ( fsel->pat, aux, 0, NULL ) )
        fsel->files= g_slist_prepend ( fsel->files, aux );
      else g_free ( aux );
    }
  fsel->dirs= g_slist_sort ( fsel->dirs, (GCompareFunc) g_strcmp0 );
  fsel->files= g_slist_sort ( fsel->files, (GCompareFunc) g_strcmp0 );
  
  /* Allibera memòria. */
  g_dir_close ( dir );
  
  return 0;
  
} /* end populate_lists */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
filesel_change_dir (
        	    filesel_t   *fsel,
        	    const gchar *path
        	    )
{
  
  GError *err;
  
  
  set_cdir ( fsel, path );
  clean_lists ( fsel );
  err= NULL;
  if ( populate_lists ( fsel, &err ) == -1 )
    {
      /* NULL indica que estem intentant obrir el HOME. Un error ací
         es considera fatal. Un error en un altre directori genera un
         warning i prova a obrir el HOME. */
      if ( path == NULL ) error ( err->message );
      else
        {
          warning ( err->message );
          g_error_free ( err );
          filesel_change_dir ( fsel, NULL );
        }
    }
  
} /* end filesel_change_dir */


void
filesel_change_parent_dir (
        		   filesel_t *fsel
        		   )
{

  gchar *aux;


  aux= g_path_get_dirname ( fsel->cdir );
  filesel_change_dir ( fsel, aux );
  g_free ( aux );
  
} /* end filesel_change_parent_dir */


void
filesel_free (
              filesel_t *fsel
              )
{

  clean_lists ( fsel );
  if ( fsel->cdir != NULL ) g_free ( fsel->cdir );
  if ( fsel->pat_hidden != NULL ) g_regex_unref ( fsel->pat_hidden );
  if ( fsel->pat_sep != NULL ) g_regex_unref ( fsel->pat_sep );
  if ( fsel->pat_dots != NULL ) g_regex_unref ( fsel->pat_dots );
  if ( fsel->pat != NULL ) g_regex_unref ( fsel->pat );
  g_free ( fsel );
  
} /* end filesel_free */

#include <stdio.h>
filesel_t *
filesel_new (
             const gchar *path,
             const gchar *pattern
             )
{

  filesel_t *new;
  GError *err;
  

  /* Prepara. */
  new= g_new ( filesel_t, 1 );
  new->pat= NULL;
  new->pat_dots= NULL;
  new->pat_sep= NULL;
  new->pat_hidden= NULL;
  new->cdir= NULL;
  new->dirs= NULL;
  new->files= NULL;

  /* Crea patrons. */
  err= NULL;
  new->pat= g_regex_new ( pattern,
        		  G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        		  G_REGEX_MATCH_NOTEMPTY,
        		  &err );
  if ( new->pat == NULL ) goto error_pat;
  new->pat_dots= g_regex_new ( "(/[.][.]$)|(/[.][.]/)",
        		       G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        		       G_REGEX_MATCH_NOTEMPTY,
        		       &err );
  if ( new->pat_dots == NULL ) goto error_pat;
#if G_DIR_SEPARATOR == '\\'
  new->pat_sep= g_regex_new ( "[\\\\][^\\\\]+[\\\\]$",
        		      G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        		      G_REGEX_MATCH_NOTEMPTY,
        		      &err );
#else
  new->pat_sep= g_regex_new ( "["G_DIR_SEPARATOR_S"][^"G_DIR_SEPARATOR_S
        		      "]+["G_DIR_SEPARATOR_S"]$",
        		      G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        		      G_REGEX_MATCH_NOTEMPTY,
        		      &err );
#endif
  if ( new->pat_sep == NULL ) goto error_pat;
#if G_DIR_SEPARATOR == '\\'
  new->pat_hidden= g_regex_new ( "[\\\\][.][^\\\\]+$",
        			 G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        			 G_REGEX_MATCH_NOTEMPTY,
        			 &err );
#else
  new->pat_hidden= g_regex_new ( "["G_DIR_SEPARATOR_S"][.][^"
        			 G_DIR_SEPARATOR_S"]+$",
        			 G_REGEX_CASELESS|G_REGEX_OPTIMIZE,
        			 G_REGEX_MATCH_NOTEMPTY,
        			 &err );
#endif
  if ( new->pat_hidden == NULL ) goto error_pat;
  
  /* Fixa el directori. */
  filesel_change_dir ( new, path );
  
  return new;

 error_pat:
  filesel_free ( new );
  error ( err->message );
  return NULL; /* CALLA! */
  
} /* end filesel_new */
