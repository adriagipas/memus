/*
 * Copyright 2020-2025 Adrià Giménez Pastor.
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
 *  screen.c - Implementació de 'screen.h'.
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "PC.h"
#include "error.h"
#include "icon.h"
#include "lock.h"
#include "screen.h"
#include "windowtex.h"




/**********/
/* MACROS */
/**********/

#define MAX_WIDTH 1600
#define MAX_HEIGHT 1200




/*********/
/* ESTAT */
/*********/

static struct
{
  int  screen_size;
  bool fullscreen;
  int  width,height;
} _wsize;

// Estat frame buffer.
static struct
{

  tex_t       *tex;
  draw_area_t  area;
  
} _fb;

static uint32_t _frame[MAX_WIDTH*MAX_HEIGHT];

static bool _cursor_enabled;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
decode_screen_size (
                    const int screen_size
                    )
{
  // Decodifica screen_size.
  switch ( screen_size )
    {
    case SCREEN_SIZE_WIN_640_480:
      _wsize.width= 640;
      _wsize.height= 480;
      _wsize.fullscreen= false;
      break;
    case SCREEN_SIZE_WIN_800_600:
      _wsize.width= 800;
      _wsize.height= 600;
      _wsize.fullscreen= false;
      break;
    case SCREEN_SIZE_WIN_960_720:
      _wsize.width= 960;
      _wsize.height= 720;
      _wsize.fullscreen= false;
      break;
    case SCREEN_SIZE_FULLSCREEN:
      _wsize.fullscreen= true;
      break;
    default:
      error ( "WTF!! decode_screen_size - screen_size desconegut" );
    }

  // Desa valors.
  _wsize.screen_size= screen_size;
  
} // end decode_screen_size


static void
render_frame (
              const PC_RGB *fb,
              const int     width,
              const int     height,
              const int     line_stride
              )
{

  int r,c;
  uint8_t *p;

  
  assert ( width <= MAX_WIDTH && height <= MAX_HEIGHT );
  p= (uint8_t *) &_frame[0];
  for ( r= 0; r < height; ++r )
    {
      for ( c= 0; c < width; ++c )
        {
          *(p++)= ((uint8_t) fb[c].r);
          *(p++)= ((uint8_t) fb[c].g);
          *(p++)= ((uint8_t) fb[c].b);
          *(p++)= 0xFF;
        }
      fb+= line_stride;
    }
  
} // end render_frame


static void
draw (void)
{
  windowtex_draw_begin ();
  screen_draw_body ();
  windowtex_draw_end ();
  
} // end draw


static void
update_fb (
           const int width,
           const int height
           )
{

  assert ( width > 0 && height > 0 );
  
  // Crea textura.
  if ( _fb.tex == NULL || _fb.tex->w != width || _fb.tex->h != height )
    {
      if ( _fb.tex != NULL ) tex_free ( _fb.tex );
      _fb.tex= windowtex_create_tex_fmt ( width, height,
                                          SDL_PIXELFORMAT_RGBA32, true );
      SDL_SetTextureScaleMode ( _fb.tex->tex, SDL_ScaleModeLinear );
    }
  
  // Fixa valors àrea de dibuix.
  _fb.area.x0= 0.0;
  _fb.area.x1= 1.0;
  _fb.area.y0= 0.0;
  _fb.area.y1= 1.0;
  
} // end update_fb


// Aquesta funció agafa dos coordenades x/y que fan referència a la
// finestra i les reajusta perquè facen referència al framebuffer. Si
// estan fora del framebuffer torna false.
static bool
translate_xy_cursor_coords (
                            Sint32             *x,
                            Sint32             *y,
                            const mouse_area_t *mouse_area
                            )
{
  
  bool ret;
  double xr,yr,aw,ah;
  mouse_area_t area;
  SDL_Rect win_geo;
  
  
  // Àrea del ratolí.
  if ( mouse_area == NULL )
    {
      if ( _fb.tex != NULL )
        {
          area.area= &_fb.area;
          area.w= _fb.tex->w;
          area.h= _fb.tex->h;
          ret= true;
        }
      else ret= false;
    }
  else
    {
      area= *mouse_area;
      ret= true;
    }

  // Clipeja coordenades i obté grandària real.
  win_geo= windowtex_get_win_geometry ();
  (*x) -= win_geo.x;
  if ( *x < 0 ) *x= 0;
  else if ( *x >= win_geo.w ) *x= win_geo.w-1;
  (*y) -= win_geo.y;
  if ( *y < 0 ) *y= 0;
  else if ( *y >= win_geo.h ) *y= win_geo.h-1;
  
  // Tradueix.
  if ( ret )
    {
      aw= area.area->x1-area.area->x0;
      ah= area.area->y1-area.area->y0;
      xr= ((double) *x) / win_geo.w;
      yr= ((double) *y) / win_geo.h;
      if ( xr >= area.area->x0 && xr <= area.area->x1 &&
           yr >= area.area->y0 && yr <= area.area->y1 )
        {
          *x= (Sint32) (((xr-area.area->x0)/aw)*area.w + 0.5);
          if ( *x >= area.w ) *x= area.w-1;
          else if ( *x < 0 ) *x= 0;
          *y= (Sint32) (((yr-area.area->y0)/ah)*area.h + 0.5);
          if ( *y >= area.h ) *y= area.h-1;
          else if ( *y < 0 ) *y= 0;
        }
      else ret= false;
    }
  
  return ret;
  
} // end translate_xy_cursor_coords




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_screen (void)
{

  if ( _fb.tex != NULL ) tex_free ( _fb.tex );
  close_windowtex ();
  
} // end close_screen


void
init_screen (
             const conf_t *conf,
             const char   *title
             )
{
  

  // Finestra.
  decode_screen_size ( conf->screen_size );
  init_windowtex ( _wsize.width, _wsize.height,
                   title, ICON, conf->vsync, true,
                   _wsize.fullscreen );

  // Inicialitza frame buffer.
  _fb.tex= NULL;
  
  // Altres.
  _cursor_enabled= false;
  
  windowtex_show ();
  
} // end init_screen


bool
screen_next_event (
        	   SDL_Event          *event,
                   const mouse_area_t *mouse_area
        	   )
{

  if ( lock_check_signals () )
    windowtex_raise ();
  
  while ( SDL_PollEvent ( event ) )
    switch ( event->type )
      {
      case SDL_WINDOWEVENT:
        if ( event->window.event == SDL_WINDOWEVENT_EXPOSED )
          draw ();
        else return true;
        break;

        // Si està habilitat el suport del cursor, tots els events
        // fora de l'àrea del framebuffer s'ignoren, i en els que
        // estan dins abans de tornar-los es reajusten les coordenades
        // perquè facen referència al framebuffer i no a la finestra.
      case SDL_MOUSEMOTION:
        if ( _cursor_enabled )
          {
            if ( translate_xy_cursor_coords ( &(event->motion.x),
                                              &(event->motion.y),
                                              mouse_area ) )
              return true;
          }
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if ( _cursor_enabled )
          {
            if ( translate_xy_cursor_coords ( &(event->button.x),
                                              &(event->button.y),
                                              mouse_area ) )
              return true;
          }
        break;
      case SDL_MOUSEWHEEL:
        if ( _cursor_enabled ) return true;
        break;
      default: return true;
      }
  
  return false;

} // end screen_next_event


void
screen_change_size (
        	    const int screen_size
        	    )
{
  
  if ( screen_size == _wsize.screen_size ) return;
  
  // Obté noves dimensions i redimensiona.
  decode_screen_size ( screen_size );
  windowtex_set_wsize ( _wsize.width, _wsize.height );
  windowtex_set_fullscreen ( _wsize.fullscreen );
  draw ();
  
} // end screen_change_size


void
screen_change_title (
                     const char *title
                     )
{
  windowtex_set_title ( title );
} // end screen_change_title


void
screen_change_vsync (
                     const bool vsync
                     )
{
  
  int w,h;
  bool create_tex;
  
  
  if ( _fb.tex != NULL )
    {
      w= _fb.tex->w;
      h= _fb.tex->h;
      tex_free ( _fb.tex );
      create_tex= true;
    }
  else create_tex= false;
  windowtex_set_vsync ( vsync );
  if ( create_tex )
    {
      _fb.tex= windowtex_create_tex_fmt ( w, h, SDL_PIXELFORMAT_RGBA32, false );
      tex_clear ( _fb.tex );
    }
  
} // end screen_change_vsync


void
screen_update (
               void         *udata,
               const PC_RGB *fb,
               const int     width,
               const int     height,
               const int     line_stride
               )
{

  // IGNORA GRANDÀRIES MOLT MENUDES
  if ( width >= 100 && height >= 100 )
    {
      if ( _fb.tex == NULL || width != _fb.tex->w || height != _fb.tex->h )
        update_fb ( width, height );
      render_frame ( fb, width, height, line_stride );
      tex_copy_fb ( _fb.tex, _frame, width, height );
      draw ();
    }
  
} // end screen_update


void
screen_show_error (
                   const char *title,
                   const char *message
                   )
{
  windowtex_show_error ( title, message );
} // end screen_show_error


void
screen_draw_body (void)
{

  if ( _fb.tex != NULL )
    windowtex_draw_tex ( _fb.tex, NULL, &_fb.area );
  
} // end screen_draw_body


void
screen_enable_cursor (
                      const bool enable
                      )
{
  
  _cursor_enabled= enable;
  windowtex_show_cursor ( false );
  
} // end screen_enable_cursor


void
screen_grab_cursor (
                    const bool grab
                    )
{
  windowtex_grab_cursor ( grab );
} // end screen_grab_cursor
