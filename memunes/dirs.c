/*
 * Copyright 2016-2025 Adrià Giménez Pastor.
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
 *  dirs.c - Implementació de 'dirs.h'.
 *
 */


#include <glib.h>
#include <stddef.h>
#include <stdlib.h>

#include "session.h"




/*********/
/* ESTAT */
/*********/

static gchar *_confdir;
static gchar *_sharedir;




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_dirs (void)
{
  
  g_free ( _confdir );
  g_free ( _sharedir );
  
} /* end close_dirs */


void
init_dirs (void)
{

  const gchar *sname;
  
  
  // Crea noms
  sname= session_get_name ();
  if ( sname == NULL )
    {
      _confdir= g_build_path ( G_DIR_SEPARATOR_S,
                               g_get_user_config_dir (),
                               "memus", "NES", NULL );
      _sharedir= g_build_path ( G_DIR_SEPARATOR_S,
                                g_get_user_data_dir (),
                                "memus", "NES", NULL );
    }
  else
    {
      _confdir= g_build_path ( G_DIR_SEPARATOR_S,
                               g_get_user_config_dir (),
                               "memus", "NES", "sessions",
                               sname, NULL );
      _sharedir= g_build_path ( G_DIR_SEPARATOR_S,
                                g_get_user_data_dir (),
                                "memus", "NES", "sessions",
                                sname, NULL );
    }

  // Construeix.
  g_mkdir_with_parents ( _confdir, 0755 );
  g_mkdir_with_parents ( _sharedir, 0755 );
  
} // end init_dirs


const gchar *
get_confdir (void)
{
  return _confdir;
} /* end get_confdir */


const gchar *
get_sharedir (void)
{
  return _sharedir;
} /* end get_sharedir */
