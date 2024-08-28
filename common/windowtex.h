/*
 * Copyright 2020-2024 Adrià Giménez Pastor.
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
 *  windowtex.h - Finestra que gestiona diferents textures.
 *
 */

#ifndef __WINDOWTEX_H__
#define __WINDOWTEX_H__

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

typedef struct
{
  SDL_Texture *tex;
  int          w,h;
} tex_t;

// Valors normalitzats [0,1] respecte a la finestra. Gastar tota la
// finestra seria x0=0,x1=1,y0=0,y1=1
typedef struct
{
  double x0,x1;
  double y0,y1;
} draw_area_t;
  
void
close_windowtex (void);

// with i height són la grandària de la finestra. Important, si
// keep_ratio_fs és cert, quan està en pantalla completa s'assumix que
// la grandària s'escala perquè la finestra ocupe tot el que puga de
// la pantalla però mantenint l'aspect ratio.
void
init_windowtex (
                const int   width,
                const int   height,
                const char *title,
                const int  *icon,
                const bool  vsync,
                const bool  keep_ratio_fs,
                const bool  fullscreen
                );

// ATENCIÓ!!! No redibuixa.
void
windowtex_set_wsize (
                     const int width,
                     const int height
                     );

// ATENCIÓ!!! No redibuixa.
void
windowtex_set_fullscreen (
                          const bool fullscreen
                          );

void
windowtex_set_title (
                     const char *title
                     );

void
windowtex_show_error (
                      const char *title,
                      const char *message
                      );

// Utilitza el format per defecte.
tex_t *
windowtex_create_tex (
                      const int width,
                      const int height
                      );

// Crea una textura emprant el format específicat.
tex_t *
windowtex_create_tex_fmt (
                          const int    width,
                          const int    height,
                          const Uint32 fmt
                          );

void
windowtex_draw_begin (void);

void
windowtex_draw_end (void);

// Si src és NULL es dibuixa tot el rectangle.
// Si area és NULL es dibuixa en tota la finestra.
void
windowtex_draw_tex (
                    const tex_t       *tex,
                    const SDL_Rect    *src,
                    const draw_area_t *area
                    );

// Torna utilitzant el format per defecte de la finestra. Necessita
// que windowtex estiga inicialitzada.
uint32_t
windowtex_get_color (
                    const uint8_t r,
                    const uint8_t g,
                    const uint8_t b,
                    const uint8_t a
                    );

void
windowtex_show (void);

// Textures //

void
tex_free (
          tex_t *tex
          );

// NOTA!!! Assumeix que hi ha prou espai i que el format del píxel
// conincideix amb el de la textura.
void
tex_copy_fb (
             tex_t          *tex,
             const uint32_t *fb,
             const int       w,
             const int       h
             );

// Utilitza una paletta de colors.
// NOTA!! No fa cap comprovació dels índex.
void
tex_copy_fb_pal (
                 tex_t          *tex,
                 const int      *fb,
                 const uint32_t *pal,
                 const int       w,
                 const int       h
                 );

#endif // __WINDOWTEX_H__
