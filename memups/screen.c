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

#include "PSX.h"
#include "error.h"
#include "icon.h"
#include "lock.h"
#include "screen.h"
#include "windowtex.h"




/**********/
/* MACROS */
/**********/

#define PAL_WIDTH 768
#define PAL_HEIGHT 576

#define NTSC_WIDTH 640
#define NTSC_HEIGHT 480




/*********/
/* ESTAT */
/*********/

static struct
{
  int  screen_size;
  bool is_pal;
  bool fullscreen;
  int  width,height;
} _wsize;

// Estat frame buffer.
static struct
{

  tex_t                    *tex;
  PSX_UpdateScreenGeometry  old_g;
  const SDL_Rect           *src; // Pot estar a NULL.
  draw_area_t               area;
  
} _fb;

static bool _cursor_enabled;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
decode_screen_size_tvmode (
                           const int  screen_size,
                           const bool is_pal
                           )
{

  
  // Fixa grandaria base
  if ( is_pal )
    {
      _wsize.width= PAL_WIDTH;
      _wsize.height= PAL_HEIGHT;
    }
  else
    {
      _wsize.width= NTSC_WIDTH;
      _wsize.height= NTSC_HEIGHT;
    }

  // Decodifica screen_size.
  switch ( screen_size )
    {
    case SCREEN_SIZE_WIN_X1:
      _wsize.fullscreen= false;
      break;
    case SCREEN_SIZE_WIN_X1_5:
      _wsize.width=  (int) (_wsize.width*1.5);
      _wsize.height= (int) (_wsize.height*1.5);
      _wsize.fullscreen= false;
      break;
    case SCREEN_SIZE_WIN_X2:
      _wsize.width*= 2;
      _wsize.height*= 2;
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
  _wsize.is_pal= is_pal;
  
} // end decode_screen_size_tvmode


static void
draw (void)
{

  windowtex_draw_begin ();
  screen_draw_body ();
  windowtex_draw_end ();
  
} // end draw


static void
update_fb (
           const PSX_UpdateScreenGeometry *g
           )
{

  static SDL_Rect src;
  
  double x0,x1,y0,y1,tmp;
  int c0,cf,r0,rf;
  
  if ( !(g->width > 1 &&
         g->height > 1 &&
         g->x0 < g->x1 &&
         g->y0 < g->y1) )printf("%g %g %g %g\n",g->x0,g->x1,g->y0,g->y1);
  assert ( g->width > 1 &&
           g->height > 1 &&
           g->x0 < g->x1 &&
           g->y0 < g->y1 );

  // Crea textura.
  if ( _fb.tex == NULL || _fb.tex->w != g->width || _fb.tex->h != g->height )
    {
      if ( _fb.tex != NULL ) tex_free ( _fb.tex );
      _fb.tex= windowtex_create_tex_fmt ( g->width, g->height,
                                          SDL_PIXELFORMAT_RGBA32 );
    }
  
  // Actualitza valors area i src.
  // --> Copia
  _fb.old_g= *g;
  
  // --> Centra horizontalment (verticalment millor no!)
  tmp= g->x1-g->x0;
  x0= (1.0-tmp)/2.0;
  x1= x0 + tmp;
  y0= g->y0; y1= g->y1;

  // --> Determina el rectàngle a dibuixar de la textura i clip
  if ( x0 < 0.0 || x1 > 1.0 || g->y0 < 0.0 || g->y1 > 1.0 )
    {
      
      // Horizontal
      tmp= x1-x0;
      if ( x0 < 0.0 )
        {
          c0= (int) (((0-x0)/tmp*(g->width-1)) + 0.5);
          if ( c0 >= g->width ) c0= g->width-1;
          else if ( c0 < 0 ) c0= 0;
          x0= 0.0;
        }
      else c0= 0;
      if ( x1 > 1.0 )
        {
          cf= (int) (((1.0-x0)/tmp*(g->width-1)) + 0.5);
          if ( cf >= g->width ) cf= g->width-1;
          else if ( cf < 0 ) cf= 0;
          x1= 1.0;
        }
      else cf= g->width-1;

      // Vertical
      tmp= y1-y0;
      if ( y0 < 0.0 )
        {
          r0= (int) (((0-y0)/tmp*(g->height-1)) + 0.5);
          if ( r0 >= g->height ) r0= g->height-1;
          else if ( r0 < 0 ) r0= 0;
          y0= 0.0;
        }
      else r0= 0;
      if ( y1 > 1.0 )
        {
          rf= (int) (((1.0-y0)/tmp*(g->height-1)) + 0.5);
          if ( rf >= g->height ) rf= g->height-1;
          else if ( rf < 0 ) rf= 0;
          y1= 1.0;
        }
      else rf= g->height-1;

      // Assigna valors a src.
      src.x= c0;
      src.w= cf-c0+1;
      src.y= r0;
      src.h= rf-r0+1;
      _fb.src= &src;
      
    }
  else _fb.src= NULL;

  // --> Fixa valors àrea de dibuix.
  _fb.area.x0= x0;
  _fb.area.x1= x1;
  _fb.area.y0= y0;
  _fb.area.y1= y1;
  
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

  
  if ( _fb.tex != NULL )
    {

      // Àrea del ratolí.
      if ( mouse_area == NULL )
        {
          area.area= &_fb.area;
          area.w= _fb.tex->w;
          area.h= _fb.tex->h;
        }
      else area= *mouse_area;
      
      // Tradueix.
      aw= area.area->x1-area.area->x0;
      ah= area.area->y1-area.area->y0;
      xr= ((double) *x) / _wsize.width;
      yr= ((double) *y) / _wsize.height;
      if ( xr >= area.area->x0 && xr <= area.area->x1 &&
           yr >= area.area->y0 && yr <= area.area->y1 )
        {
          *x= (Sint32) (((xr-area.area->x0)/aw)*area.w + 0.5);
          if ( *x >= area.w ) *x= area.w-1;
          else if ( *x < 0 ) *x= 0;
          *y= (Sint32) (((yr-area.area->y0)/ah)*area.h + 0.5);
          if ( *y >= area.h ) *y= area.h-1;
          else if ( *y < 0 ) *y= 0;
          ret= true;
        }
      else ret= false;
      
    }
  else ret= false;
  
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
             const char   *title,
             const bool    big_screen
             )
{
  

  // Finestra.
  decode_screen_size_tvmode ( conf->screen_size, conf->tvres_is_pal );
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
  decode_screen_size_tvmode ( screen_size, _wsize.is_pal );
  windowtex_set_wsize ( _wsize.width, _wsize.height );
  windowtex_set_fullscreen ( _wsize.fullscreen );
  draw ();
  
} // end screen_change_size


void
screen_change_tvmode (
                      const bool is_pal
                      )
{

  if ( is_pal == _wsize.is_pal ) return;

  // Obté noves dimensions i redimensiona.
  decode_screen_size_tvmode ( _wsize.screen_size, is_pal );
  windowtex_set_wsize ( _wsize.width, _wsize.height );
  windowtex_set_fullscreen ( _wsize.fullscreen );
  draw ();
  
} // end screen_change_tvmode


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
      _fb.tex= windowtex_create_tex_fmt ( w, h, SDL_PIXELFORMAT_RGBA32 );
      tex_clear ( _fb.tex );
    }
  
} // end screen_change_vsync


void
screen_update (
               const uint32_t                 *fb,
               const PSX_UpdateScreenGeometry *g,
               void                           *udata
               )
{
  
  if ( _fb.tex == NULL ||
       g->width != _fb.tex->w ||
       g->height != _fb.tex->h ||
       g->x0 != _fb.old_g.x0 ||
       g->x1 != _fb.old_g.x1 ||
       g->y0 != _fb.old_g.y0 ||
       g->y1 != _fb.old_g.y1 )
    update_fb ( g );
  tex_copy_fb ( _fb.tex, fb, g->width, g->height );
  draw ();
  
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
    windowtex_draw_tex ( _fb.tex, _fb.src, &_fb.area );
  
} // end screen_draw_body


void
screen_enable_cursor (
                      const bool enable
                      )
{
  
  _cursor_enabled= enable;
  windowtex_show_cursor ( false );
  
} // end screen_enable_cursor
