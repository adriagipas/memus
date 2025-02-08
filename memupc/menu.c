/*
 * Copyright 2014-2025 Adrià Giménez Pastor.
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
 *  menu.c - Implementació de 'menu.h'.
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cursor.h"
#include "error.h"
#include "frontend.h"
#include "load_bios.h"
#include "load_hdd.h"
#include "load_vgabios.h"
#include "menu.h"
#include "mpad.h"
#include "PC.h"
#include "screen.h"
#include "set_disc.h"
#include "tiles16b.h"
#include "windowtex.h"




/**********/
/* MACROS */
/**********/

#define WIDTH 320
#define HEIGHT 240

// El 90% de 320x240
//#define WIDTH 288
//#define HEIGHT 216

#define MOUSE_COUNTER 200



/*************/
/* CONSTANTS */
/*************/

static const draw_area_t DRAW_AREA=
  { .x0= 0.05, .x1= 0.95, .y0= 0.05, .y1= 0.95 };

static const mouse_area_t MOUSE_AREA= {
    .area= &DRAW_AREA,
    .w= WIDTH,
    .h= HEIGHT
  };




/*********/
/* TIPUS */
/*********/

typedef struct menuitem menuitem_t;

typedef struct
{
  int                 N;
  int                 current;
  const menuitem_t   *menu;
  int                 ret_mouse;
  int                 mouse_x;
  int                 mouse_y;
  bool                mouse_hide;
  int                 mouse_counter;
} menu_state_t;

// Valor que torna action.
enum
  {
    NO_ACTION,
    CONTINUE,
    RESUME,
    QUIT,
    RESET,
    REINIT
  };

struct menuitem
{
  
  const char * (*get_text) (void);
  int          (*action) (menu_state_t *mst);
  
};

typedef struct
{
  
  int               N;
  const menuitem_t *items;
  
} menumode_t;

enum
  {
   BGCOLOR= 0,
   FGCOLOR1,
   FGCOLOR2,
   SELCOLOR1,
   SELCOLOR2,
   CURSOR_BLACK,
   CURSOR_WHITE,

   NUM_COLORS
  };




/*********/
/* ESTAT */
/*********/

// Verbose.
static bool _verbose;

// Frame buffer.
static int _fb[WIDTH*HEIGHT];

// Textura utilitzada per a dibuixar.
static tex_t *_fb_tex;

// Configuració.
static conf_t *_conf;

// Paleta de colors.
static uint32_t _pal[NUM_COLORS];



/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static uint32_t
get_color (
           const uint8_t r,
           const uint8_t g,
           const uint8_t b,
           const uint8_t a
           )
{

  uint32_t ret;


  ((uint8_t *) &ret)[0]= r;
  ((uint8_t *) &ret)[1]= g;
  ((uint8_t *) &ret)[2]= b;
  ((uint8_t *) &ret)[3]= a;
  
  return ret;
  
} // end get_color


static void
init_pal (void)
{

  _pal[BGCOLOR]= get_color ( 0x00, 0x00, 0x00, 0x7f );
  _pal[FGCOLOR1]= get_color ( 0x7f, 0x7f, 0x7f, 0xff );
  _pal[FGCOLOR2]= get_color ( 0xff, 0xff, 0xff, 0xff );
  _pal[SELCOLOR1]= get_color ( 0xa0, 0x00, 0x00, 0xff );
  _pal[SELCOLOR2]= get_color ( 0xe0, 0x00, 0x00, 0xff );
  _pal[CURSOR_BLACK]= get_color ( 0, 0, 0, 0xff );
  _pal[CURSOR_WHITE]= get_color ( 250, 250, 250, 0xff );
  
} // end init_pal


static void
redraw_fb (void)
{
  
  // Copia en textura.
  tex_copy_fb_pal ( _fb_tex, _fb, _pal, WIDTH, HEIGHT );

  // Redibuixa.
  windowtex_draw_begin ();
  screen_draw_body ();
  windowtex_draw_tex ( _fb_tex, NULL, &DRAW_AREA );
  windowtex_draw_end ();
  
} // end redraw_fb


static void
draw_cursor (
             menu_state_t *mst
             )
{

  if ( !mst->mouse_hide )
    {
      cursor_draw ( _fb, WIDTH, HEIGHT,
                    mst->mouse_x, mst->mouse_y,
                    CURSOR_BLACK, CURSOR_WHITE );
      if ( --(mst->mouse_counter) == 0 )
        mst->mouse_hide= true;
    }
  
} // end draw_cursor


static void
draw_menu (
           menu_state_t *mst
           )
{

  const int OFF_X= 1;
  const int OFF_Y= 1;
  
  int i, y;
  

  // Fons.
  for ( int i= 0; i < WIDTH*HEIGHT; ++i ) _fb[i]= BGCOLOR;
  
  // Menú.
  for ( i= 0, y= OFF_Y; i < mst->N; ++i, ++y )
    if ( i == mst->current )
      tiles16b_draw_string ( _fb, WIDTH, mst->menu[i].get_text (),
                             OFF_X, y, SELCOLOR1, SELCOLOR2, 0, T16_BG_TRANS );
    else
      tiles16b_draw_string ( _fb, WIDTH, mst->menu[i].get_text (),
                             OFF_X, y, FGCOLOR1, FGCOLOR2, 0, T16_BG_TRANS );
  draw_cursor ( mst );
  redraw_fb ();
  
} // end draw_menu


// Torna -1 si no s'està apuntat a ninguna
static int
translate_mousexy2entry (
                         const int x,
                         const int y,
                         const int nentries
                         )
{

  const int OFF_X= 1;
  const int OFF_Y= 1;
  
  int x0,xf,y0,yf,ret;


  x0= OFF_X*8;
  xf= WIDTH - OFF_X*8;
  y0= OFF_Y*16;
  yf= y0 + nentries*16;
  if ( x >= x0 && x < xf && y >= y0 && y < yf )
    ret= (y-y0)/16;
  else ret= -1;
  
  return ret;
  
} // translate_mousexy2entry


static void
mouse_update_pos (
                  menu_state_t *mst,
                  const int     x,
                  const int     y
                  )
{

  mst->mouse_x= x;
  mst->mouse_y= y;
  mst->mouse_hide= false;
  mst->mouse_counter= MOUSE_COUNTER;
  
} // end mouse_update_pos


static void
mouse_cb (
          SDL_Event *event,
          void      *udata
          )
{

  int sel;
  menu_state_t *mst;
  
  
  mst= (menu_state_t *) udata;
  if ( mst->ret_mouse == NO_ACTION )
    {
      switch ( event->type )
        {
        case SDL_MOUSEBUTTONUP:
          mouse_update_pos ( mst, event->button.x, event->button.y );
          break;
        case SDL_MOUSEMOTION:
          mouse_update_pos ( mst, event->motion.x, event->motion.y );
          break;
        case SDL_MOUSEBUTTONDOWN:
          mouse_update_pos ( mst, event->button.x, event->button.y );
          switch ( event->button.button )
            {
              // Botó esquerre
            case SDL_BUTTON_LEFT:
              sel= translate_mousexy2entry ( event->button.x,
                                             event->button.y,
                                             mst->N );
              if ( sel != -1 )
                {
                  mst->current= sel;
                  if ( event->button.clicks > 1 )
                    {
                      mst->ret_mouse= mst->menu[sel].action ( mst );
                      mpad_clear (); // neteja botons
                    }
                }
              break;
              // Botó dret.
            case SDL_BUTTON_RIGHT:
              mst->ret_mouse= RESUME;
              break;
            }
          break;
        case SDL_MOUSEWHEEL:
          if ( event->wheel.y > 0 )
            {
              if ( mst->current > 0 ) --(mst->current);
            }
          else if ( event->wheel.y < 0 )
            {
              if ( mst->current < mst->N-1 ) ++(mst->current);
            }
          break;
        }
    }
  
} // end mouse_cb




/*************/
/* MENUITEMS */
/*************/

static const char *
resume_get_text (void)
{
  return "TANCA MENu";
} // end resume_get_text


static int
resume_action (
               menu_state_t *mst
               )
{
  return RESUME;
} // end resume_action


static const char *
cd_get_text (void)
{
  return frontend_disc_is_empty ( DISC_D_CDROM ) ?
    "D: INSERIX CD-ROM" :
    "D: CANVIA CD-ROM";
} // end cd_get_text


static int
cd_action (
           menu_state_t *mst
           )
{
  
  bool quit;
  
  
  set_disc ( &quit, DISC_D_CDROM, _verbose );
  
  return quit ? QUIT : RESUME;
  
} // end cd_action


static const char *
floppyA_get_text (void)
{
  return frontend_disc_is_empty ( DISC_A_FLOPPY_1M44 ) ?
    "A: INSERIX DISQUET" :
    "A: CANVIA DISQUET";
} // end floppyA_get_text


static int
floppyA_action (
                menu_state_t *mst
                )
{
  
  bool quit;
  
  
  set_disc ( &quit, DISC_A_FLOPPY_1M44, _verbose );
  
  return quit ? QUIT : RESUME;
  
} // end floppyA_action


static const char *
floppyB_get_text (void)
{
  return frontend_disc_is_empty ( DISC_B_FLOPPY_1M2 ) ?
    "B: INSERIX DISQUET" :
    "B: CANVIA DISQUET";
} // end floppyB_get_text


static int
floppyB_action (
                menu_state_t *mst
                )
{
  
  bool quit;
  
  
  set_disc ( &quit, DISC_B_FLOPPY_1M2, _verbose );
  
  return quit ? QUIT : RESUME;
  
} // end floppyB_action


static const char *
screen_size_get_text (void)
{
  
  static const char * const text[]=
    {
     "FINESTRA 640X480",
     "FINESTRA 800X600",
     "FINESTRA 960X720",
     "PANTALLA COMPLETA"
    };
  
  return text[_conf->screen_size];
  
} // end screen_size_get_text


static int
screen_size_action (
                    menu_state_t *mst
                    )
{
  
  if ( ++(_conf->screen_size) == SCREEN_SIZE_SENTINEL )
    _conf->screen_size= SCREEN_SIZE_WIN_640_480;
  screen_change_size ( _conf->screen_size );
  
  return CONTINUE;
  
} // end screen_size_action


static const char *
change_vsync_get_text (void)
{
  
  static const char * const text[]=
    {
      "VSYNC: Si",
      "VSYNC: NO"
    };
  
  return text[_conf->vsync ? 0 : 1];
  
} // end change_vsync_get_text


static int
change_vsync_action (
                     menu_state_t *mst
                     )
{
  
  _conf->vsync= !_conf->vsync;
  tex_free ( _fb_tex );
  screen_change_vsync ( _conf->vsync );
  _fb_tex= windowtex_create_tex_fmt ( WIDTH, HEIGHT,
                                      SDL_PIXELFORMAT_RGBA32, false );
  if ( SDL_SetTextureBlendMode ( _fb_tex->tex, SDL_BLENDMODE_BLEND ) != 0 )
    error ( "change_vsync_action - no s'ha pogut crear"
            " la textura per a _fb_tex: %s",
            SDL_GetError () );
  
  return CONTINUE;
  
} // end change_vsync_action


static const char *
reset_get_text (void)
{
  return "REINICIA";
} // end reset_get_text


static int
reset_action (
              menu_state_t *mst
              )
{
  return RESET;
} // end reset_action


static const char *
bios_get_text (void)
{
  return "SELECCIONA BIOS I REINICIA";
} // end bios_get_text


static int
bios_action (
             menu_state_t *mst
             )
{
  
  bool quit,ret;
  
  
  ret= change_bios ( _conf, &quit, _verbose );
  if ( quit ) return QUIT;
  else if ( ret ) return RESET;
  else return CONTINUE;
  
  return CONTINUE;
  
} // end bios_action


static const char *
vgabios_get_text (void)
{
  return "SELECCIONA VGA BIOS I REINICIA";
} // end vgabios_get_text


static int
vgabios_action (
                menu_state_t *mst
                )
{
  
  bool quit,ret;
  
  
  ret= change_vgabios ( _conf, &quit, _verbose );
  if ( quit ) return QUIT;
  else if ( ret ) return RESET;
  else return CONTINUE;
  
  return CONTINUE;
  
} // end vgabios_action


static const char *
hdd_get_text (void)
{
  return "SELECCIONA DISC DUR I REINICIA";
} // end hdd_get_text


static int
hdd_action (
            menu_state_t *mst
            )
{
  
  bool quit,ret;
  
  
  ret= change_hdd ( _conf, &quit, _verbose );
  if ( quit ) return QUIT;
  else if ( ret ) return RESET;
  else return CONTINUE;
  
  return CONTINUE;
  
} // end hdd_action


static const char *
quit_get_text (void)
{
  return "SURT";
} // end quit_get_text


static int
quit_action (
             menu_state_t *mst
             )
{
  return QUIT;
} // end quit_action




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_menu (void)
{
  tex_free ( _fb_tex );
} // end close_menu


void
init_menu (
           conf_t     *conf,
           const bool  verbose
           )
{

  // Bàsics.
  _verbose= verbose;
  _conf= conf;
  
  // Textura.
  _fb_tex= windowtex_create_tex_fmt ( WIDTH, HEIGHT,
                                      SDL_PIXELFORMAT_RGBA32, false );
  if ( SDL_SetTextureBlendMode ( _fb_tex->tex, SDL_BLENDMODE_BLEND ) != 0 )
    error ( "init_menu - no s'ha pogut crear la textura per a _fb_tex: %s",
            SDL_GetError () );

  // Paleta.
  init_pal ();
  
} // end init_menu


menu_response_t
menu_run (void)
{

  static const menuitem_t menu_basic[]=
    {
     {resume_get_text,resume_action},
     {cd_get_text,cd_action},
     {floppyA_get_text,floppyA_action},
     {floppyB_get_text,floppyB_action},
     {screen_size_get_text,screen_size_action},
     {change_vsync_get_text,change_vsync_action},
     {reset_get_text,reset_action},
     {hdd_get_text,hdd_action},
     {bios_get_text,bios_action},
     {vgabios_get_text,vgabios_action},
     {quit_get_text,quit_action}
    };
  
  static const menumode_t menus[]=
    {
      { 11, menu_basic },
    };
  
  const gulong delay1= 200000;
  const gulong delay2= 100000;

  menu_state_t mst;
  int buttons, ret;
  gulong delay;
  

  mst.N= menus[0].N;
  mst.menu= menus[0].items;
  mst.current= 0;
  mst.mouse_x= 0;
  mst.mouse_y= 0;
  mst.mouse_hide= true;
  delay= delay1;
  mpad_clear ();
  screen_enable_cursor ( true );
  for (;;)
    {
      draw_menu ( &mst );
      g_usleep ( 10000 );
      mst.ret_mouse= NO_ACTION;
      buttons= mpad_check_buttons ( &MOUSE_AREA, mouse_cb, &mst );
      if ( mst.ret_mouse != NO_ACTION )
        {
          if ( mst.ret_mouse != CONTINUE ) { ret= mst.ret_mouse; break; }
          continue;
        }
      if ( buttons&K_QUIT ) { ret= QUIT; break; }
      else if ( buttons&K_ESCAPE ) { ret= RESUME; break; }
      else if ( buttons&K_BUTTON )
        {
          ret= mst.menu[mst.current].action ( &mst );
          draw_menu ( &mst ); // Per si canvíem alguna
                                          // cosa de la patalla.
          mpad_clear (); // neteja botons.
          if ( ret != CONTINUE ) break;
        }
      else if ( buttons&K_UP )
        {
          if ( mst.current == 0 ) mst.current= mst.N-1;
          else --mst.current;
        }
      else if ( buttons&K_DOWN ) mst.current= (mst.current+1)%mst.N;
      if ( buttons ) // Descansa un poc.
        {
          g_usleep ( delay );
          delay= delay2;
        }
      else delay= delay1;
    }
  switch ( ret )
    {
    case RESET: return MENU_RESET;
    case QUIT: return MENU_QUIT;
    case REINIT: return MENU_REINIT;
    case RESUME:
    default: return MENU_RESUME;
    }
  
} // end menu_run
