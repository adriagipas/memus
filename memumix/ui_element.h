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
 *  ui_element.h - Element base de la interfície.
 *
 */

#ifndef __UI_ELEMENT_H__
#define __UI_ELEMENT_H__

#include <stdbool.h>
#include <SDL.h>

typedef struct ui_element ui_element_t;

typedef bool (ui_event_callback_t) (const SDL_Event *event,void *udata);
typedef void (ui_action_t) (ui_element_t *e,void *udata);

#define UI_ELEMENT_CLASS                                                \
  /* MÈTODES */                                                         \
  void (*free) (ui_element_t *self);                                    \
  void (*draw) (ui_element_t *self,int *fb,const int fb_width,          \
                const bool has_focus);                                  \
  void (*set_visible) (ui_element_t *self,const bool value);            \
  bool (*mouse_event) (ui_element_t *self,const SDL_Event *event);      \
  /* --> Pot ser NULL */                                                \
  bool (*key_event) (ui_element_t *self,const SDL_Event *event);        \
                                                                        \
  /* ATRIBUTS */                                                        \
  bool visible;

struct ui_element
{
  UI_ELEMENT_CLASS;
};

#define UI_ELEMENT(PTR) ((ui_element_t *) (PTR))

// Allibera memòria element
#define ui_element_free(UIE) (UIE)->free ( UI_ELEMENT(UIE) )

// Pinta l'element en el framebuffer
#define ui_element_draw(UIE,FB,FB_WIDTH,HAS_FOCUS)                      \
  (UIE)->draw ( UI_ELEMENT(UIE), (FB), (FB_WIDTH), (HAS_FOCUS) )

// Fa que el framebuffer siga visible
#define ui_element_set_visible(UIE,VALUE)               \
  (UIE)->set_visible ( UI_ELEMENT(UIE), (VALUE) )

// Processa un event de ratolí SDL_MOUSEMOTION, SDL_MOUSEBUTTONDOWN,
// SDL_MOUSEBUTTONUP o SDL_MOUSEWHEEL. Torna cert si l'event ha sigut
// processat.
#define ui_element_mouse_event(UIE,CP_EVENT)            \
  (UIE)->mouse_event ( UI_ELEMENT(UIE), (CP_EVENT) )

// Processa un event de teclat SDL_KEYDOWN o SDL_KEYUP. Torna cert si
// l'event ha sigut processat.
#define ui_element_key_event(UIE,CP_EVENT)            \
  (UIE)->key_event ( UI_ELEMENT(UIE), (CP_EVENT) )

// MÈTODES PER DEFECTE
void
ui_element_set_visible_default (
                                ui_element_t *e,
                                const bool    val
                                );

#endif // __UI_ELEMENT_H__
