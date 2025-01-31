/*
 * Copyright 2015-2025 Adrià Giménez Pastor.
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
 *  fchooser.h - Menú per a seleccionar fitxers.
 *
 */

#ifndef __FCHOOSER_H__
#define __FCHOOSER_H__

#include <glib.h>
#include <stdbool.h>

#include "filesel.h"
#include "t8biso.h"
#include "screen.h"

// Número de entrades visible.
#define FCNVIS 10

typedef struct
{

  gboolean         is_dir;
  const gchar     *path;    // Punter a filesel_t. NULL significa ".."
  gchar           *path_name;
  t8biso_banner_t *banner;
  
} fchooser_node_t;

// Selector de fitxers.
typedef struct
{

  filesel_t       *fsel;
  
  const gchar     *cdir; // Punter a fsel.
  gchar           *cdir_name;

  fchooser_node_t *v;
  guint            size;
  guint            N;
  t8biso_banner_t  banners[FCNVIS+1]; // 0 és per a cdir.

  guint            first;
  guint            last;
  guint            current;

  int              fgcolor_cdir;
  int              fgcolor_entry;
  int              fgcolor_selected_entry;
  int              fgcolor_sc;

  bool             empty_entry;

  gboolean         verbose;

  // Punter al background
  const int       *background;

  // Frame buffer.
  int              fb[WIDTH*HEIGHT];

  int              mouse_x;
  int              mouse_y;
  bool             mouse_hide;
  int              mouse_counter;
  int              mouse_color_black;
  int              mouse_color_white;
  enum {
    MOUSE_NO_ACTION,
    MOUSE_SEL_ENTRY,
    MOUSE_ESCAPE
  }                mouse_action;
  
} fchooser_t;

void
fchooser_free (
               fchooser_t *fc
               );

fchooser_t *
fchooser_new (
              const gchar    *selector,
              const bool      empty_entry,
              const int      *background,
              const gboolean  verbose
              );

// NULL vol dir que s'ha eixit sense elegir res.
const char *
fchooser_run (
              fchooser_t *fc,
              bool       *quit
              );

// Un -1 indica que s'ha forçat l'eixida.
int
fchooser_error_dialog (
                       fchooser_t *fc
                       );

void
fchooser_hide_cursor (
                      fchooser_t *fc
                      );

#endif // __FCHOOSER_H__
