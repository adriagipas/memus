/*
 * Copyright 2021-2022 Adrià Giménez Pastor.
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
 *  ui_file_chooser.h - Element que representa una llista per a navegar pel
 *                      sistema de fitxers.
 *
 */

#ifndef __UI_FILE_CHOOSER_H__
#define __UI_FILE_CHOOSER_H__

#include <stdbool.h>

#include "filesel.h"
#include "ui_element.h"
#include "ui_scrollbar.h"

#define UI_FILE_CHOOSER_ROWS 13

typedef struct ui_fchooser_render_node ui_fchooser_render_node_t;

struct ui_fchooser_render_node
{

  int                        N; // Número de columnes actuals
  int                        capacity; // Capacitat medit en columnes
  int                       *v; // té renderitza el text sense seleccionar
  ui_fchooser_render_node_t *next;
  
};

typedef struct
{

  ui_fchooser_render_node_t  v[UI_FILE_CHOOSER_ROWS+1];
  ui_fchooser_render_node_t *free_nodes;
  
} ui_fchooser_render_mng_t;

typedef struct
{

  bool                       is_dir;
  const gchar               *path; // Punter a filesel_t. NULL significa ".."
  gchar                     *path_name;
  ui_fchooser_render_node_t *p; // Pot ser NULL
  
} ui_fchooser_node_t;

typedef struct
{

  UI_ELEMENT_CLASS;

  int                        _x,_y;
  int                        _w,_h; // No inclou scrollbars
  bool                       _show_files;
  bool                       _empty_entry;
  filesel_t                 *_fsel;
  ui_fchooser_render_mng_t  *_render;
  
  ui_fchooser_node_t        *_nodes;
  guint                      _N;
  size_t                     _size_nodes;
  
  const gchar               *_cdir; // Punter a fsel.
  gchar                     *_cdir_name;
  ui_fchooser_render_node_t *_cdir_render_node;

  guint                      _first;
  guint                      _last;
  guint                      _current;
  int                        _ccol; // Columna (mesurada en caràcters) actual
  int                        _max_width_chars; // Amplària màxima en caràcters

  ui_scrollbar_t            *_vsb,*_hsb;

  ui_action_t               *_action;
  void                      *_udata;
  
} ui_file_chooser_t;

#define UI_FILE_CHOOSER(PTR) ((ui_file_chooser_t *) (PTR))

ui_file_chooser_t *
ui_file_chooser_new (
                     const int    x,
                     const int    y,
                     const gchar *cdir, // Pot ser NULL
                     const bool   show_files,
                     const bool   empty_entry,
                     const gchar *selector, // Sols s'aplica si
                                            // show_files és false
                     ui_action_t *action,
                     void        *user_data
                     );

const gchar *
ui_file_chooser_get_current_dir (
                                 ui_file_chooser_t *self
                                 );

// Torna NULL si ara mateixa no hi ha cap fitxer regular seleccionat
// ara mateixa. El PATH "" vol dir --EMPTY--
const gchar *
ui_file_chooser_get_current_file (
                                  ui_file_chooser_t *self
                                  );

#endif // __UI_FILE_CHOOSER_H__
