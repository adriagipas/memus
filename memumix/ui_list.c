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
 *  ui_list.c - Implementació de 'ui_list.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "ui_list.h"




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
free_ui_element (
                 gpointer data
                 )
{
  ui_element_free ( UI_ELEMENT(data) );
} // end free_ui_element


static void
move_focus (
            ui_list_t *self
            )
{

  const GList *p;

  
  if ( self->_tail == NULL )
    self->_focus= self->_tail;
  else // Mou al següent no buit
    {

      if ( self->_focus == NULL )
        self->_focus= self->_tail;
      
      // Avant fins principi
      for ( p= self->_focus->prev;
            p != NULL && (!UI_ELEMENT(p->data)->visible ||
                          UI_ELEMENT(p->data)->key_event==NULL);
            p= p->prev );
      
      // Final fins focus
      if ( p == NULL )
        for ( p= self->_tail;
              p != self->_focus && (!UI_ELEMENT(p->data)->visible ||
                                    UI_ELEMENT(p->data)->key_event==NULL);
              p= p->prev );

      // Assigna.
      self->_focus= p;
      
    }
  
} // end move_focus




/***********/
/* MÈTODES */
/***********/

static void
free_ (
       ui_element_t *s
       )
{

  ui_list_t *self;


  self= UI_LIST(s);
  if ( self->_v != NULL )
    g_list_free_full ( self->_v, free_ui_element );
  g_free ( self );
  
} // end free_


static void
draw (
      ui_element_t *s,
      int          *fb,
      const int     fb_width,
      const bool    has_focus
      )
{

  ui_list_t *self;
  const GList *p;
  ui_element_t *e;
  

  self= UI_LIST(s);
  if ( self->visible )
    for ( p= self->_v; p != NULL; p= p->next )
      {
        e= UI_ELEMENT(p->data);
        if ( e->visible )
          ui_element_draw ( e, fb, fb_width,
                            has_focus && p==self->_focus );
        else if ( p == self->_focus )
          self->_focus= NULL;
      }
  
} // end draw


static bool
mouse_event (
             ui_element_t    *s,
             const SDL_Event *event
             )
{

  ui_list_t *self;
  const GList *p;
  ui_element_t *e;
  

  self= UI_LIST(s);
  if ( !self->visible ) return false;
  for ( p= self->_tail; p != NULL; p= p->prev )
    {
      e= UI_ELEMENT(p->data);
      if ( e->visible &&
           ui_element_mouse_event ( e, event ) )
        {
          self->_focus= p;
          return true;
        }
    }

  // Si es fa un click fica el focus a NULL, però no cosumeix l'event.
  if ( self->visible && event->type == SDL_MOUSEBUTTONDOWN )
    self->_focus= NULL;
  
  // Callback global.
  if ( self->_global_mouse_event != NULL &&
       self->_global_mouse_event ( event, self->_udata ) )
    {
      self->_focus= NULL;
      return true;
    }
  
  return false;
  
} // end mouse_event


static bool
key_event (
           ui_element_t    *s,
           const SDL_Event *event
           )
{

  ui_list_t *self;
  ui_element_t *e;
  
  
  self= UI_LIST(s);
  if ( !self->visible ) return false;
  if ( self->_focus != NULL )
    {
      e= UI_ELEMENT(self->_focus->data);
      if ( e->visible && e->key_event != NULL )
        {
          if ( ui_element_key_event ( e, event ) )
            return true;
        }
      else self->_focus= NULL;
    }
  
  // Tecla tabulador canvia el focus.
  if ( event->type == SDL_KEYDOWN &&
       event->key.keysym.sym == SDLK_TAB )
    {
      move_focus ( self );
      return true;
    }
  
  // Callback global.
  if ( self->_global_key_event != NULL &&
       self->_global_key_event ( event, self->_udata ) )
    return true;
  
  return false;
  
} // end key_event




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

ui_list_t *
ui_list_new (
             ui_event_callback_t *g_mouse_event, // Pot ser NULL
             ui_event_callback_t *g_key_event,    // Pot ser NULL
             void                *udata
             )
{

  ui_list_t *new;


  // Reserva memòria.
  new= g_new ( ui_list_t, 1 );
  new->_focus= NULL;
  new->_v= NULL;
  new->_tail= NULL;
  new->_global_mouse_event= g_mouse_event;
  new->_global_key_event= g_key_event;
  new->_udata= udata;

  // Inicialitza base.
  new->free= free_;
  new->draw= draw;
  new->set_visible= ui_element_set_visible_default;
  new->mouse_event= mouse_event;
  new->key_event= key_event;
  new->visible= true;

  return new;
  
} // end ui_list_new


void
ui_list_add (
             ui_list_t    *self,
             ui_element_t *e
             )
{

  if ( self->_tail == NULL )
    {
      assert ( self->_v == NULL );
      self->_v= self->_tail= g_list_append ( NULL, e );
    }
  else
    {
      self->_tail= g_list_append ( self->_tail, e );
      self->_tail= self->_tail->next;
    }
  
} // end ui_list_add


void
ui_list_set_focus (
                   ui_list_t *self
                   )
{

  if ( self->_focus == NULL )
    move_focus ( self );
  
} // end ui_list_set_focus


void
ui_list_set_focus_begin (
                         ui_list_t *self
                         )
{

  const GList *p;


  for ( p= self->_v;
        p != NULL && (!UI_ELEMENT(p->data)->visible ||
                      UI_ELEMENT(p->data)->key_event==NULL);
        p= p->next );
  self->_focus= p;
  
} // end ui_list_set_focus_begin


void
ui_list_set_focus_end (
                       ui_list_t *self
                       )
{

  const GList *p;


  for ( p= self->_tail;
        p != NULL && (!UI_ELEMENT(p->data)->visible ||
                      UI_ELEMENT(p->data)->key_event==NULL);
        p= p->prev );
  self->_focus= p;
  
} // end ui_list_set_focus_end
