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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  windowtex.c - Implementació de 'windowtex.h'.
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
#include "windowtex.h"




/*********/
/* ESTAT */
/*********/

static struct
{

  SDL_Rect      bounds;       // Grandària del display
  bool          fullscreen;   // Valor decodificat de screen_size
  bool          vsync;
  int           width,height; // Grandària de la finestra.
  int           rwidth,rheight; // Grandària real de la finestra. Pot
                                // no coincidir amb l'anterior quan
                                // està en pantalla completa.
  bool          keep_ratio_fs;
  SDL_Window   *win;        // Finestra
  SDL_Renderer *renderer;
  Uint32        pfmt;
  int           desp_r,desp_g,desp_b,desp_a;
  SDL_Rect      coords; // Àrea de dibuix. Típicament és tota la
                        // finistra. Pot no ser-ho quan estem en
                        // pantalla completa i keep_ratio_fs és cert.
  enum
    {
     PFMT_RGBA,     // Sense importar molt A
     PFMT_UNK
    }           hpfmt; // Format de pixel per a comprovar ràpidament.
} _sdl;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
update_coords (void)
{

  double fbratio, wratio;

  
  // Cas simple.
  if ( !_sdl.keep_ratio_fs || !_sdl.fullscreen )
    {
      _sdl.coords.x= 0;
      _sdl.coords.y= 0;
      _sdl.coords.w= _sdl.rwidth;
      _sdl.coords.h= _sdl.rheight;
      return;
    }

  // Quan estem en pantalla completa i keep_ratio_fs és cert, cal
  // canviar l'àrea de dibuixar.
  fbratio= _sdl.width / (double) _sdl.height;
  wratio= (double) _sdl.rwidth / (double) _sdl.rheight;
  if ( wratio >= fbratio ) // Marges horizontals
    {
      _sdl.coords.w= (int) ((fbratio/wratio)*_sdl.rwidth + 0.5);
      if ( _sdl.coords.w > _sdl.rwidth ) _sdl.coords.w= _sdl.rwidth;
      _sdl.coords.h= _sdl.rheight;
      _sdl.coords.x= (_sdl.rwidth-_sdl.coords.w)/2;
      _sdl.coords.y= 0;
    }
  else // Marges verticals
    {
      _sdl.coords.w= _sdl.rwidth;
      _sdl.coords.h= (int) ((wratio/fbratio)*_sdl.rheight + 0.5);
      if ( _sdl.coords.h > _sdl.rheight ) _sdl.coords.h= _sdl.rheight;
      _sdl.coords.x= 0;
      _sdl.coords.y= (_sdl.rheight-_sdl.coords.h)/2;
    }
  
} // end update_coords


static void
set_wsize (
           const int width,
           const int height
           )
{

  _sdl.width= width;
  _sdl.height= height;
  if ( _sdl.fullscreen )
    {
      _sdl.rwidth= _sdl.bounds.w-_sdl.bounds.x;
      _sdl.rheight= _sdl.bounds.h-_sdl.bounds.y;
    }
  else
    {
      _sdl.rwidth= width;
      _sdl.rheight= height;
    }
  
} // end set_wsize


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


// RGBA entès com l'únic que importa és l'ordre a nivell de byte: R,
// G, B, A.
static bool
check_pfmt_is_rgba (
                    const uint32_t rmask,
                    const uint32_t gmask,
                    const uint32_t bmask,
                    const uint32_t amask
                    )
{

  uint32_t val;


  ((uint8_t *) &val)[0]= 0x11;
  ((uint8_t *) &val)[1]= 0x22;
  ((uint8_t *) &val)[2]= 0x33;
  ((uint8_t *) &val)[3]= 0x44;

  return ( (val&rmask) == 0x11 &&
           (val&gmask) == 0x22 &&
           (val&bmask) == 0x33 &&
           (amask==0 || (val&amask) == 0x44) );
  
} // end check_pfmt_is_rgba


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
  
  // Fixa hpfmt. (Forma ràpida de comprovar que no cal transormar els
  // valors)
  if ( check_pfmt_is_rgba ( rmask, gmask, bmask, amask ) )
    _sdl.hpfmt= PFMT_RGBA;
  else
    _sdl.hpfmt= PFMT_UNK;
  
  return;
  
 error:
  error ( "format de píxel no suportat" );
  
} // end calc_desp_rgba


static void
init_sdl (
          const int   width,
          const int   height,
          const char *title,
          const int  *icon,
          const bool  vsync,
          const bool  keep_ratio_fs,
          const bool  fullscreen
          )
{
  
  SDL_Surface *aux;
  

  assert ( title != NULL );
  
  // Obté les dimensions del desktop.
  if ( SDL_GetDisplayBounds ( 0, &_sdl.bounds ) != 0 )
    error ( "no s'han pogut obtenir les dimensions de la pantalla: %s",
            SDL_GetError () );
  
  // Crea la finestra.
  _sdl.fullscreen= fullscreen;
  _sdl.keep_ratio_fs= keep_ratio_fs;
  set_wsize ( width, height );
  _sdl.win= SDL_CreateWindow ( title,
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               _sdl.width, _sdl.height,
                               SDL_WINDOW_HIDDEN );
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
  SDL_ShowCursor ( 0 );
  
  // Crea el renderer
  _sdl.vsync= vsync;
  _sdl.renderer= SDL_CreateRenderer ( _sdl.win, -1,
                                      SDL_RENDERER_ACCELERATED |
                                      (vsync ? SDL_RENDERER_PRESENTVSYNC : 0) );
  if ( _sdl.renderer == NULL )
    error ( "ha fallat la creació del renderer: %s", SDL_GetError () );
  
  // Calcula els desplaçaments.
  calc_desp_rgba ();
  
} // end init_sdl




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_windowtex (void)
{

  SDL_DestroyRenderer ( _sdl.renderer );
  SDL_DestroyWindow ( _sdl.win );
  
} // end close_windowtex


void
init_windowtex (
                const int   width,
                const int   height,
                const char *title,
                const int  *icon,
                const bool  vsync,
                const bool  keep_ratio_fs,
                const bool  fullscreen
                )
{

  // Finestra.
  init_sdl ( width, height, title, icon, vsync, keep_ratio_fs, fullscreen );

  // Coordenades àrea de dibuix.
  update_coords ();
  
} // end init_windowtex


void
windowtex_set_wsize (
                     const int width,
                     const int height
                     )
{

  // Comprova dimensió anterior.
  if ( width == _sdl.width && height == _sdl.height )
    return;
  
  // Obté noves dimensions i redimensiona.
  if ( _sdl.fullscreen ) SDL_SetWindowFullscreen ( _sdl.win, 0 );
  set_wsize ( width, height );
  SDL_SetWindowSize ( _sdl.win, _sdl.width, _sdl.height );
  if ( _sdl.fullscreen )
    SDL_SetWindowFullscreen ( _sdl.win, SDL_WINDOW_FULLSCREEN_DESKTOP );
  
  // Recalcula posició.
  update_coords ();
  
} // end windowtex_set_wsize


void
windowtex_set_fullscreen (
                          const bool fullscreen
                          )
{

  if ( fullscreen == _sdl.fullscreen ) return;

  _sdl.fullscreen= fullscreen;
  set_wsize ( _sdl.width, _sdl.height );
  if ( fullscreen )
    SDL_SetWindowFullscreen ( _sdl.win, SDL_WINDOW_FULLSCREEN_DESKTOP );
  else
    SDL_SetWindowFullscreen ( _sdl.win, 0 );
  update_coords ();
  
} // end windowtex_set_fullscreen


void
windowtex_set_title (
                     const char *title
                     )
{
  SDL_SetWindowTitle ( _sdl.win, title );
} // end windowtex_set_title


void
windowtex_show_error (
                      const char *title,
                      const char *message
                      )
{
  SDL_ShowSimpleMessageBox ( SDL_MESSAGEBOX_WARNING,
                             title, message, _sdl.win );
} // end windowtex_show_error


tex_t *
windowtex_create_tex (
                      const int width,
                      const int height
                      )
{

  tex_t *ret;


  ret= g_new ( tex_t, 1 );
  ret->w= width;
  ret->h= height;
  ret->tex= SDL_CreateTexture ( _sdl.renderer, _sdl.pfmt,
                                SDL_TEXTUREACCESS_STREAMING,
                                width, height );
  if ( ret->tex == NULL )
    error ( "window_create_tex - no s'ha pogut crear una textura de %dx%d: %s",
            width, height, SDL_GetError () );

  return ret;
  
} // end windowtex_create_tex


tex_t *
windowtex_create_tex_fmt (
                          const int    width,
                          const int    height,
                          const Uint32 fmt,
                          const bool   linear_scale
                          )
{

  tex_t *ret;


  ret= g_new ( tex_t, 1 );
  ret->w= width;
  ret->h= height;
  ret->tex= SDL_CreateTexture ( _sdl.renderer, fmt,
                                SDL_TEXTUREACCESS_STREAMING,
                                width, height );
  if ( ret->tex == NULL )
    error ( "window_create_tex_fmt - no s'ha pogut"
            " crear una textura de %dx%d: %s",
            width, height, SDL_GetError () );
  if ( SDL_SetTextureScaleMode ( ret->tex,
                                 linear_scale ? SDL_ScaleModeLinear :
                                 SDL_ScaleModeNearest ) != 0 )
    error ( "window_create_tex_fmt - no s'ha pogut fixar el tipus d'escalat" );
  
  return ret;
  
} // end windowtex_create_tex_fmt


void
windowtex_draw_begin (void)
{

  SDL_RenderClear ( _sdl.renderer );
  SDL_SetRenderDrawColor ( _sdl.renderer, 0, 0, 0, 0xff );
  SDL_RenderFillRect ( _sdl.renderer, NULL );
  
} // end windowtex_draw_begin


void
windowtex_draw_end (void)
{
  SDL_RenderPresent ( _sdl.renderer );
} // end windowtex_draw_end


void
windowtex_draw_tex (
                    const tex_t       *tex,
                    const SDL_Rect    *src,
                    const draw_area_t *area
                    )
{

  SDL_Rect dst;


  if ( area == NULL )
    SDL_RenderCopy ( _sdl.renderer, tex->tex, src, &_sdl.coords );
  else
    {

      // Horizontal
      dst.w= (int) ((area->x1-area->x0)*_sdl.coords.w + 0.5);
      if ( dst.w > _sdl.coords.w ) dst.w= _sdl.coords.w;
      else if ( dst.w <= 0 ) dst.w= 1;
      dst.x= (int) (_sdl.coords.x + area->x0*_sdl.coords.w + 0.5);
      if ( dst.x >= (_sdl.coords.x + _sdl.coords.w) )
        dst.x= _sdl.coords.x + _sdl.coords.w - 1;
      else if ( dst.x < _sdl.coords.x )
        dst.x= _sdl.coords.x;

      // Vertical
      dst.h= (int) ((area->y1-area->y0)*_sdl.coords.h + 0.5);
      if ( dst.h > _sdl.coords.h ) dst.h= _sdl.coords.h;
      else if ( dst.h <= 0 ) dst.h= 1;
      dst.y= (int) (_sdl.coords.y + area->y0*_sdl.coords.h + 0.5);
      if ( dst.y >= (_sdl.coords.y + _sdl.coords.h) )
        dst.y= _sdl.coords.y + _sdl.coords.h - 1;
      else if ( dst.y < _sdl.coords.y )
        dst.y= _sdl.coords.y;

      // Copia.
      SDL_RenderCopy ( _sdl.renderer, tex->tex, src, &dst );
      
    }
  
} // end windowtex_draw_tex


void
windowtex_set_vsync (
                     const bool vsync
                     )
{
  
  if ( vsync != _sdl.vsync )
    {
      SDL_DestroyRenderer ( _sdl.renderer );
      _sdl.vsync= vsync;
      _sdl.renderer= SDL_CreateRenderer ( _sdl.win, -1,
                                          SDL_RENDERER_ACCELERATED |
                                          (vsync ?
                                           SDL_RENDERER_PRESENTVSYNC : 0) );
      if ( _sdl.renderer == NULL )
        error ( "ha fallat la creació del renderer: %s", SDL_GetError () );
    }
  
} // end windowfb_set_vsync


uint32_t
windowtex_get_color (
                    const uint8_t r,
                    const uint8_t g,
                    const uint8_t b,
                    const uint8_t a
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
  
} // end windowtex_get_color


void
windowtex_show (void)
{
  SDL_ShowWindow ( _sdl.win );
} // end windowtex_show


void
windowtex_raise (void)
{

  SDL_ShowWindow ( _sdl.win );
  SDL_RaiseWindow ( _sdl.win );
  
} // end windowtex_raise


void
windowtex_show_cursor (
                       const bool show
                       )
{
  SDL_ShowCursor ( show ? 1 : 0 );
} // end windowtex_show_cursor


void
windowtex_grab_cursor (
                       const bool grab
                       )
{
  SDL_SetRelativeMouseMode ( grab );
} // end windowtex_grab_cursor


SDL_Rect
windowtex_get_win_geometry (void)
{
  return _sdl.coords;
} // end windowtex_get_win_geometry


void
tex_free (
          tex_t *tex
          )
{

  SDL_DestroyTexture ( tex->tex );
  g_free ( tex );
  
} // end tex_free


void
tex_copy_fb (
             tex_t          *tex,
             const uint32_t *fb,
             const int       w,
             const int       h
             )
{

  int r,c,i,pitch;
  uint8_t *buffer;
  
  
  if ( SDL_LockTexture ( tex->tex, NULL, (void **) &buffer, &pitch ) != 0 )
    error ( "no s'ha pogut actualitzar la textura: %s", SDL_GetError () );

  for ( r= i= 0; r < h; ++r )
    {
      for ( c= 0; c < w; ++c, ++i )
        ((uint32_t *) buffer)[c]= fb[i];
      buffer+= pitch;
    }
  
  SDL_UnlockTexture ( tex->tex );
  
} // end tex_copy_fb


void
tex_copy_fb_pal (
                 tex_t          *tex,
                 const int      *fb,
                 const uint32_t *pal,
                 const int       w,
                 const int       h
                 )
{

  int r,c,i,pitch;
  uint8_t *buffer;
  
  
  if ( SDL_LockTexture ( tex->tex, NULL, (void **) &buffer, &pitch ) != 0 )
    error ( "no s'ha pogut actualitzar la textura: %s", SDL_GetError () );

  for ( r= i= 0; r < h; ++r )
    {
      for ( c= 0; c < w; ++c, ++i )
        ((uint32_t *) buffer)[c]= pal[fb[i]];
      buffer+= pitch;
    }
  
  SDL_UnlockTexture ( tex->tex );
  
} // end tex_copy_fb_pal


void
tex_clear (
           tex_t *tex
           )
{

  int r,c,i,pitch;
  uint8_t *buffer;
  
  
  if ( SDL_LockTexture ( tex->tex, NULL, (void **) &buffer, &pitch ) != 0 )
    error ( "no s'ha pogut actualitzar la textura: %s", SDL_GetError () );
  
  for ( r= i= 0; r < tex->h; ++r )
    {
      for ( c= 0; c < tex->w; ++c, ++i )
        ((uint32_t *) buffer)[c]= 0;
      buffer+= pitch;
    }
  
  SDL_UnlockTexture ( tex->tex );
  
} // end tex_clear
