/*
 * Copyright 2016-2022 Adrià Giménez Pastor.
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
 *  windowfb.c - Implementació de 'windowfb.h'.
 *
 *  Adrià Giménez Pastor, 2016,2019,2021,2022.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "windowfb.h"




/*************/
/* CONSTANTS */
/*************/


/*********/
/* ESTAT */
/*********/

static struct
{
 
  SDL_Rect       bounds;         /* Grandària del display. */
  int            screen_size;    /* Grandària de la finestra. */
  int            wwidth,wwidth0;
  int            wheight,wheight0;
  gboolean       fullscreen;     /* Valors decodificats de screen_size. */
  SDL_Window    *win;            /* Finestra */
  SDL_Renderer  *renderer;
  SDL_Texture   *fb;
  Uint32         pfmt;
  int            fbwidth;
  int            fbheight;
  SDL_Rect       coords;
  int            desp_r,desp_g,desp_b,desp_a;
  bool           cursor_enabled;
  bool           vsync;
  
} _sdl;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static int
get_desp (
          const Uint32 mask
          )
{

  switch ( mask )
    {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    default: return -1;
    }
  
} // end get_desp


static void
calc_desp_rgba (void)
{

  Uint32 rmask, gmask, bmask, amask;
  int bpp;
  

  // Obté màscares
  if ( !SDL_PixelFormatEnumToMasks( _sdl.pfmt, &bpp, &rmask,
                                    &gmask, &bmask, &amask ) )
    error ( "no es poden obtindre les màscares RGBA: %s", SDL_GetError () );
  if ( bpp != 32 ) goto error;
  
  // Obté desplaçaments.
  _sdl.desp_r= get_desp ( rmask );
  _sdl.desp_g= get_desp ( gmask );
  _sdl.desp_b= get_desp ( bmask );
  _sdl.desp_a= get_desp ( amask );
  if ( amask == 0 ) _sdl.desp_a= 32; // Indica que no es gasta _sdl.desp_a
  if ( _sdl.desp_r == -1 || _sdl.desp_g == -1 ||
       _sdl.desp_b == -1 || _sdl.desp_a == -1 )
    goto error;

  return;
  
 error:
  error ( "format de píxel no suportat" );
  
} // end calc_desp_rgba


static void
draw (void)
{

  SDL_SetRenderDrawColor ( _sdl.renderer, 0, 0, 0, 0xff );
  SDL_RenderFillRect ( _sdl.renderer, NULL );
  SDL_RenderCopy ( _sdl.renderer, _sdl.fb, NULL, &_sdl.coords );
  SDL_RenderPresent ( _sdl.renderer );
  
} // end draw


static void
set_wsize (
           const int width,
           const int height
           )
{

  _sdl.wwidth0= width;
  _sdl.wheight0= height;
  _sdl.fullscreen= (width<=0 || height<=0);
  if ( _sdl.fullscreen )
    {
      _sdl.wwidth= _sdl.bounds.w-_sdl.bounds.x;
      _sdl.wheight= _sdl.bounds.h-_sdl.bounds.y;
    }
  else
    {
      _sdl.wwidth= width;
      _sdl.wheight= height;
    }
  
} /* end set_wsize */


static void
init_sdl (
          const int       wwidth,
          const int       wheight,
          const int       fbwidth,
          const int       fbheight,
          const char     *title,
          const int      *icon,
          const gboolean  vsync
          )
{
  
  SDL_Surface *aux;
  

  assert ( title != NULL );
  
  // Obté les dimensions del desktop.
  if ( SDL_GetDisplayBounds ( 0, &_sdl.bounds ) != 0 )
    error ( "no s'han pogut obtenir les dimensions de la pantalla: %s",
            SDL_GetError () );
  
  // Crea la finestra.
  set_wsize ( wwidth, wheight );
  _sdl.win= SDL_CreateWindow ( title,
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               _sdl.wwidth, _sdl.wheight,
        		       0 );
  if ( _sdl.win == NULL )
    error ( "no s'ha pogut crear la finestra: %s", SDL_GetError () );
  _sdl.pfmt= SDL_GetWindowPixelFormat ( _sdl.win );
  if ( _sdl.pfmt == SDL_PIXELFORMAT_UNKNOWN )
    error ( "format de píxel desconegut: %s", SDL_GetError () );
  if ( _sdl.fullscreen )
    {
      if ( SDL_SetWindowFullscreen ( _sdl.win,
                                     SDL_WINDOW_FULLSCREEN_DESKTOP ) != 0 )
        error ( "ha fallat el mode pantalla completa: %s", SDL_GetError () );
    }
  
  // Fixa la icona
  if ( icon != NULL )
    {
      aux= SDL_CreateRGBSurfaceFrom ( (void *) &(icon[2]), icon[0], icon[1],
        			      32, icon[0]*4,
        			      0x00FF0000, 0x0000FF00,
        			      0x000000FF, 0xFF000000 );
      if ( aux == NULL )
        error ( "no s'ha pogut fixar la icona: %s", SDL_GetError () );
      SDL_SetWindowIcon ( _sdl.win, aux );
      SDL_FreeSurface ( aux );
    }
  
  // Amaga el cursor.
  _sdl.cursor_enabled= false;
  SDL_ShowCursor ( 0 );

  // Crea el renderer
  _sdl.vsync= vsync;
  _sdl.renderer= SDL_CreateRenderer ( _sdl.win, -1,
                                      SDL_RENDERER_ACCELERATED |
                                      (vsync ? SDL_RENDERER_PRESENTVSYNC : 0) );
  if ( _sdl.renderer == NULL )
    error ( "ha fallat la creació del renderer: %s", SDL_GetError () );

  // Crea el framebuffer.
  _sdl.fbwidth= fbwidth;
  _sdl.fbheight= fbheight;
  _sdl.fb= SDL_CreateTexture ( _sdl.renderer, _sdl.pfmt,
                               SDL_TEXTUREACCESS_STREAMING,
                               fbwidth, fbheight );
  if ( _sdl.fb == NULL )
    error ( "no s'ha pogut crear el framebuffer: %s", SDL_GetError () );

  // Calcula els desplaçaments.
  calc_desp_rgba ();
  
} // end init_sdl


static void
update_coords (void)
{

  double fbratio, wratio;
  
  
  fbratio= _sdl.fbwidth / (double) _sdl.fbheight;
  wratio= (double) _sdl.wwidth / (double) _sdl.wheight;
  if ( wratio >= fbratio ) // Marges horizontals
    {
      _sdl.coords.w= (int) ((fbratio/wratio)*_sdl.wwidth + 0.5);
      if ( _sdl.coords.w > _sdl.wwidth ) _sdl.coords.w= _sdl.wwidth;
      _sdl.coords.h= _sdl.wheight;
      _sdl.coords.x= (_sdl.wwidth-_sdl.coords.w)/2;
      _sdl.coords.y= 0;
    }
  else // Marges verticals
    {
      _sdl.coords.w= _sdl.wwidth;
      _sdl.coords.h= (int) ((wratio/fbratio)*_sdl.wheight + 0.5);
      if ( _sdl.coords.h > _sdl.wheight ) _sdl.coords.h= _sdl.wheight;
      _sdl.coords.x= 0;
      _sdl.coords.y= (_sdl.wheight-_sdl.coords.h)/2;
    }
  
} // end update_coords


static void
update_fbsize (
               const int width,
               const int height
               )
{

  // Allibera memòria.
  SDL_DestroyTexture ( _sdl.fb );

  // Crea nou framebuffer.
  _sdl.fb= SDL_CreateTexture ( _sdl.renderer, _sdl.pfmt,
                               SDL_TEXTUREACCESS_STREAMING,
                               width, height );
  if ( _sdl.fb == NULL )
    error ( "no s'ha pogut crear el framebuffer: %s", SDL_GetError () );
  _sdl.fbwidth= width;
  _sdl.fbheight= height;
  update_coords ();
  
} // end update_fbsize


// Aquesta funció agafa dos coordenades x/y que fan referència a la
// finestra i les reajusta perquè facen referència al framebuffer. Si
// estan fora del framebuffer torna false.
static bool
translate_xy_cursor_coords (
                            Sint32 *x,
                            Sint32 *y
                            )
{

  int xf,yf;
  bool ret;
  double f;

  
  xf= _sdl.coords.x + _sdl.coords.w;
  yf= _sdl.coords.y + _sdl.coords.h;
  if ( *x >= _sdl.coords.x && *x < xf &&
       *y >= _sdl.coords.y && *y < yf &&
       xf > _sdl.coords.x && yf > _sdl.coords.y )
    {
      f= _sdl.fbwidth / (double) (xf-_sdl.coords.x);
      *x= (Sint32) ((*x-_sdl.coords.x + 1)*f + 0.5) - 1;
      f= _sdl.fbheight / (double) (yf-_sdl.coords.y);
      *y= (Sint32) ((*y-_sdl.coords.y + 1)*f + 0.5) - 1;
      ret= true;
    }
  else ret= false;

  return ret;
  
} // end translate_xy_cursor_coords




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_windowfb (void)
{
  
  SDL_DestroyTexture ( _sdl.fb );
  SDL_DestroyRenderer ( _sdl.renderer );
  SDL_DestroyWindow ( _sdl.win );
  
} // end close_windowfb


void
init_windowfb (
               const int       wwidth,
               const int       wheight,
               const int       fbwidth,
               const int       fbheight,
               const char     *title,
               const int      *icon,
               const gboolean  vsync
               )
{

  // Finestra.
  init_sdl ( wwidth, wheight, fbwidth, fbheight, title, icon, vsync );

  // Coordenades inicials.
  update_coords ();

} // end init_windowfb


gboolean
windowfb_next_event (
        	     SDL_Event *event
        	     )
{

  while ( SDL_PollEvent ( event ) )
    switch ( event->type )
      {
      case SDL_WINDOWEVENT:
        if ( event->window.event == SDL_WINDOWEVENT_EXPOSED )
          draw ();
        else return TRUE;
        break;

        // Si està habilitat el suport del cursor, tots els events
        // fora de l'àrea del framebuffer s'ignoren, i en els que
        // estan dins abans de tornar-los es reajusten les coordenades
        // perquè facen referència al framebuffer i no a la finestra.
      case SDL_MOUSEMOTION:
        if ( _sdl.cursor_enabled )
          {
            if ( translate_xy_cursor_coords ( &(event->motion.x),
                                              &(event->motion.y) ) )
              return TRUE;
          }
        break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if ( _sdl.cursor_enabled )
          {
            if ( translate_xy_cursor_coords ( &(event->button.x),
                                              &(event->button.y) ) )
              return TRUE;
          }
        break;
      case SDL_MOUSEWHEEL:
        if ( _sdl.cursor_enabled ) return TRUE;
        break;
      default: return TRUE;
      }
  
  return FALSE;
  
} // end windowfb_next_event


void
windowfb_set_wsize (
        	    const int width,
        	    const int height
        	    )
{

  // Comprova dimensió anterior.
  if ( width == _sdl.wwidth0 && height == _sdl.wheight0 )
    return;
  
  // Obté noves dimensions i redimensiona.
  if ( _sdl.fullscreen ) SDL_SetWindowFullscreen ( _sdl.win, 0 );
  set_wsize ( width, height );
  SDL_SetWindowSize ( _sdl.win, _sdl.wwidth, _sdl.wheight );
  if ( _sdl.fullscreen )
    SDL_SetWindowFullscreen ( _sdl.win, SDL_WINDOW_FULLSCREEN_DESKTOP );
  
  // Recalcula posició.
  update_coords ();
  
  // Dibuixa.
  draw ();
  
} // end windowfb_set_wsize


void
windowfb_set_fbsize (
        	     const int width,
        	     const int height
        	     )
{

  if ( width == _sdl.fbwidth && height == _sdl.fbheight )
    return;
  update_fbsize ( width, height );
  
} // end windowfb_set_fbsize


void
windowfb_set_title (
        	    const char *title
        	    )
{
  SDL_SetWindowTitle ( _sdl.win, title );
} // end windowfb_set_title


void
windowfb_set_vsync (
                    const bool vsync
                    )
{

  if ( vsync != _sdl.vsync )
    {

      // Allibera memòria.
      SDL_DestroyTexture ( _sdl.fb );
      SDL_DestroyRenderer ( _sdl.renderer );
      
      // Crea el renderer
      _sdl.vsync= vsync;
      _sdl.renderer= SDL_CreateRenderer ( _sdl.win, -1,
                                          SDL_RENDERER_ACCELERATED |
                                          (vsync ?
                                           SDL_RENDERER_PRESENTVSYNC : 0) );
      if ( _sdl.renderer == NULL )
        error ( "ha fallat la creació del renderer: %s", SDL_GetError () );
      
      // Crea el framebuffer.
      _sdl.fb= SDL_CreateTexture ( _sdl.renderer, _sdl.pfmt,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   _sdl.fbwidth, _sdl.fbheight );
      if ( _sdl.fb == NULL )
        error ( "no s'ha pogut crear el framebuffer: %s", SDL_GetError () );
      
    }
  
} // end windowfb_set_vsync


void
windowfb_update (
        	 const int      *fb,
        	 const uint32_t *palette
        	 )
{

  int r,c,i,pitch;
  uint8_t *buffer;
  

  if ( SDL_LockTexture ( _sdl.fb, NULL, (void **) &buffer, &pitch ) != 0 )
    error ( "no s'ha pogut actualitzar la textura: %s", SDL_GetError () );
  for ( r= i= 0; r < _sdl.fbheight; ++r )
    {
      for ( c= 0; c < _sdl.fbwidth; ++c, ++i )
        ((uint32_t *) buffer)[c]= palette[fb[i]];
      buffer+= pitch;
    }
  SDL_UnlockTexture ( _sdl.fb );
  draw ();
  
} // end windowfb_update


void
windowfb_update_no_pal (
        		const uint32_t *fb
        		)
{

  int r,c,i,pitch;
  uint8_t *buffer;
  

  if ( SDL_LockTexture ( _sdl.fb, NULL, (void **) &buffer, &pitch ) != 0 )
    error ( "no s'ha pogut actualitzar la textura: %s", SDL_GetError () );
  for ( r= i= 0; r < _sdl.fbheight; ++r )
    {
      for ( c= 0; c < _sdl.fbwidth; ++c, ++i )
        ((uint32_t *) buffer)[c]= fb[i];
      buffer+= pitch;
    }
  SDL_UnlockTexture ( _sdl.fb );
  draw ();
  
} // end windowfb_update_no_pal


void
windowfb_redraw (void)
{
  draw ();
} // end windowfb_redraw


void
windowfb_show_error (
        	     const char *title,
        	     const char *message
        	     )
{
  SDL_ShowSimpleMessageBox ( SDL_MESSAGEBOX_WARNING,
                             title, message, _sdl.win );
} // end windowfb_show_error


void
windowfb_hide (void)
{
  SDL_HideWindow ( _sdl.win );
} // end windowfb_hide


void
windowfb_show (void)
{
  SDL_ShowWindow ( _sdl.win );
} // end windowfb_show


void
windowfb_raise (void)
{

  SDL_ShowWindow ( _sdl.win );
  SDL_RaiseWindow ( _sdl.win );
  
} // end windowfb_raise


uint32_t
windowfb_get_color (
                    const uint8_t r,
                    const uint8_t g,
                    const uint8_t b
                    )
{

  uint32_t ret;


  if ( _sdl.desp_a == 32 )
    ret=
      (r<<_sdl.desp_r) |
      (g<<_sdl.desp_g) |
      (b<<_sdl.desp_b);
  else
    ret=
      (r<<_sdl.desp_r) |
      (g<<_sdl.desp_g) |
      (b<<_sdl.desp_b) |
      (0xff<<_sdl.desp_a);
  
  return ret;
  
} // end windofb_get_color


void
windowfb_enable_cursor (
                        const bool val,
                        const bool show
                        )
{

  if ( val )
    {
      _sdl.cursor_enabled= true;
      SDL_ShowCursor ( show ? 1 : 0 );
    }
  else
    {
      _sdl.cursor_enabled= false;
      SDL_ShowCursor ( 0 );
    }
  
} // end windowfb_enable_cursor
