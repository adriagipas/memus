/*
 * Copyright 2016-2025 Adrià Giménez Pastor.
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
 *  menu.c - Implementació de 'menu.h'.
 *
 */
/*
 * NOTA: Com el framebuffer sempre és igual de gran i l'únic que
 * canvia és que a vegades no es pinten la primera i última fila, la
 * idea és no gastar eixes files.
 */

#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "NES.h"
#include "cursor.h"
#include "effects.h"
#include "hud.h"
#include "menu.h"
#include "mpad.h"
#include "pad.h"
#include "screen.h"
#include "tiles8b.h"
#include "tvmode.h"




/**********/
/* MACROS */
/**********/

#define WIDTH 256
#define HEIGHT 240

#define MOUSE_COUNTER 200




/*************/
/* CONSTANTS */
/*************/

static const char *TV_ENTRIES[]=
  {
    "PAL",
    "NTSC"
  };
const int TV_INIT_Y= 4;




/*********/
/* TIPUS */
/*********/

typedef struct menuitem menuitem_t;

typedef struct
{
  int               N;
  int               current;
  const menuitem_t *menu;
  int               ret_mouse;
  int               mouse_x;
  int               mouse_y;
  bool              mouse_hide;
  int               mouse_counter;
} menu_state_t;


/* Valor que torna action. */
enum {
  NO_ACTION,
  CONTINUE,
  RESUME,
  QUIT,
  RESET,
  QUIT_MAINMENU,
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




/*********/
/* ESTAT */
/*********/

/* Offset mode big screen. */
static int _bg_off;

/* Frame buffer. */
static int _fb[WIDTH*HEIGHT];

/* Configuració. */
static conf_t *_conf;

/* Estil. */
static struct
{
  
  int x,y;
  int fgcolor;
  int selcolor;
  int cursor_black;
  int cursor_white;
  
} _style;

/* Fons. */
static int _background[WIDTH*HEIGHT];




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
init_background (void)
{
  
  memcpy ( _background, screen_get_last_fb (), sizeof(_background) );
  effect_fade ( _background, 4, 12, WIDTH-8, HEIGHT-24, -3 );
  
} /* end init_background */


static void
draw_background (void)
{
  memcpy ( _fb, _background, sizeof(_background) );
} /* end draw_background */


static void
draw_cursor (
             menu_state_t *mst
             )
{

  if ( !mst->mouse_hide )
    {
      cursor_draw ( _fb, WIDTH, HEIGHT,
                    mst->mouse_x, mst->mouse_y,
                    _style.cursor_black, _style.cursor_white );
      if ( --(mst->mouse_counter) == 0 )
        mst->mouse_hide= true;
    }
  
} // end draw_cursor


static void
draw_menu (
           menu_state_t *mst
           )
{
  
  int i, y;
  
  
  draw_background ();
  for ( i= 0, y= _style.y; i < mst->N; ++i, ++y )
    if ( i == mst->current )
      {
        tiles8b_draw_string ( _fb, WIDTH, mst->menu[i].get_text (), _style.x+1,
        		      y, _style.selcolor, 0, BG_TRANS );
      }
    else tiles8b_draw_string ( _fb, WIDTH, mst->menu[i].get_text (), _style.x+1,
        		       y, _style.fgcolor, 0, BG_TRANS );
  draw_cursor ( mst );
  screen_update ( _fb, NULL );
  
} // end draw_menu


// Torna -1 si no s'està apuntat a ninguna
static int
translate_mousexy2entry (
                         const int x,
                         const int y,
                         const int nentries
                         )
{

  int x0,xf,y0,yf,ret;


  x0= (_style.x+1)*8;
  xf= WIDTH - (_style.x+1)*8;
  y0= _style.y*8;
  yf= y0 + nentries*8;
  if ( x >= x0 && x < xf && y >= y0 && y < yf )
    ret= (y-y0)/8;
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


static void
draw_change_tvmode (
        	    menu_state_t *mst
        	    )
{

  int i,y,len,x;

  
  draw_background ();
  for ( i= 0, y= TV_INIT_Y; i < mst->N; ++i, ++y )
    {
      len= strlen ( TV_ENTRIES[i] );
      x= (WIDTH/8-len)/2;
      if ( i == mst->current )
        tiles8b_draw_string ( _fb, WIDTH, TV_ENTRIES[i], x, y,
        		      _style.selcolor, 0, BG_TRANS );
      else tiles8b_draw_string ( _fb, WIDTH, TV_ENTRIES[i], x, y,
        			 _style.fgcolor, 0, BG_TRANS );
    }
  draw_cursor ( mst );
  screen_update ( _fb, NULL );
  
} // end draw_change_tvmode


// Torna -1 si no s'està apuntat a ninguna
static int
translate_mousexy2select_tvmode (
                                 const int x,
                                 const int y,
                                 const int N
                                 )
{
  
  int x0,xf,y0,yf,ret,len;
  
  
  y0= TV_INIT_Y*8;
  yf= y0 + N*8;
  if ( y >= y0 && y < yf )
    {
      ret= (y-y0)/8;
      if ( ret < 0 ) ret= 0;
      else if ( ret >= N  ) ret= N-1;
      len= strlen ( TV_ENTRIES[ret] );
      x0= ((WIDTH/8-len)/2)*8;
      xf= x0 + len*8;
      if ( x < x0 || x >= xf ) ret= -1;
    }
  else ret= -1;
  
  return ret;
  
} // translate_mousexy2select_tvmode


static void
mouse_cb_change_tvmode (
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
              sel= translate_mousexy2select_tvmode ( event->button.x,
                                                     event->button.y,
                                                     mst->N );
              if ( sel != -1 )
                {
                  mst->current= sel;
                  if ( event->button.clicks > 1 )
                    {
                      mst->ret_mouse= QUIT; // <-- PER A INDICAR REINICI
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
  
} // end mouse_cb_change_tvmode


static int
config_keys_action (
        	    const int pad
        	    )
{
  
  static const char * const text[]=
    {
      "PRESSIONA ESQUERRA",
      "PRESSIONA DRETA",
      "PRESSIONA AMUNT",
      "PRESSIONA AVALL",
      "PRESSIONA BOTp B",
      "PRESSIONA BOTp A",
      "PRESSIONA SELECT",
      "PRESSIONA START"
    };
  
  int aux[8], i;
  gboolean stop;
  SDL_Event event;
  
  
  for ( i= 0; i < 8; ++i )
    {
      
      /* Dibuixa. */
      draw_background ();
      tiles8b_draw_string ( _fb, WIDTH, "TECLAT", 2, 5,
        		    _style.fgcolor, 0, BG_TRANS );
      tiles8b_draw_string ( _fb, WIDTH, text[i], 2, 6,
        		    _style.selcolor, 0, BG_TRANS );
      screen_update ( _fb, NULL );
      
      /* Llig events. */
      stop= FALSE;
      for (;;)
        {
          while ( !stop && screen_next_event ( &event ) )
            switch ( event.type )
              {
              case SDL_QUIT: return QUIT;
              case SDL_KEYDOWN:
        	if ( event.key.keysym.sym == SDLK_ESCAPE ) return CONTINUE;
        	aux[i]= event.key.keysym.sym;
        	stop= TRUE;
        	break;
              default: break;
              }
          if ( stop ) break;
          g_usleep ( 100000 );
        }
      
    }
  _conf->keys[pad].left= aux[0];
  _conf->keys[pad].right= aux[1];
  _conf->keys[pad].up= aux[2];
  _conf->keys[pad].down= aux[3];
  _conf->keys[pad].button_B= aux[4];
  _conf->keys[pad].button_A= aux[5];
  _conf->keys[pad].select= aux[6];
  _conf->keys[pad].start= aux[7];
  
  return CONTINUE;
  
} /* end config_keys_action */




/*************/
/* MENUITEMS */
/*************/

static const char *
resume_get_text (void)
{
  return "REPReN PARTIDA";
} /* end resume_get_text */


static int
resume_action (
               menu_state_t *mst
               )
{
  return RESUME;
} // end resume_action


static const char *
screen_size_get_text (void)
{
  
  static const char * const text[]=
    {
      "FINESTRA",
      "FINESTRA X2",
      "FINESTRA X3",
      "PANTALLA COMPLETA"
    };
  
  return text[_conf->screen_size];
  
} /* end screen_size_get_text */


static int
screen_size_action (
                    menu_state_t *mst
                    )
{
  
  if ( ++(_conf->screen_size) == SCREEN_SIZE_SENTINEL )
    _conf->screen_size= SCREEN_SIZE_WIN_X1;
  screen_change_size ( _conf->screen_size );
  
  return CONTINUE;
  
} // end screen_size_action


static const char *
change_scaler_get_text (void)
{
  
  static const char * const text[]=
    {
      "FILTRE: CAP",
      "FILTRE: SCALE2X"
    };
  
  return text[_conf->scaler];
  
} /* end change_scaler_get_text */


static int
change_scaler_action (
                      menu_state_t *mst
                      )
{
  
  if ( ++(_conf->scaler) == SCALER_SENTINEL )
    _conf->scaler= SCALER_NONE;
  screen_change_scaler ( _conf->scaler );
  
  return CONTINUE;
  
} // end change_scaler_action


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
  screen_change_vsync ( _conf->vsync );
  
  return CONTINUE;
  
} // end change_vsync_action


static const char *
config_keys1_get_text (void)
{
  return "AJUSTA EL TECLAT PER AL PAD1";
} /* end config_keys1_get_text */


static int
config_keys1_action (
                     menu_state_t *mst
                     )
{
  return config_keys_action ( 0 );
} // end config_keys1_action


static const char *
config_keys2_get_text (void)
{
  return "AJUSTA EL TECLAT PER AL PAD2";
} /* end config_keys2_get_text */


static int
config_keys2_action (
                     menu_state_t *mst
                     )
{
  return config_keys_action ( 1 );
} // end config_keys2_action


static const char *
help_get_text (void)
{
  return "AJUDA";
} /* end help_get_text */


static void
draw_help (
           menu_state_t *mst
           )
{
  
  draw_background ();
  tiles8b_draw_string ( _fb, WIDTH, "AJUDA",
        		13, 4, _style.selcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "DESA ESTAT:       CTRL+1-5",
        		2, 7, _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "LLIG ESTAT:       ALT+1-5",
        		2, 9, _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "CAPTURA PANTALLA: CTRL+S",
        		2, 11, _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "SURT:             CTRL+Q",
        		2, 13, _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "MENu:             ESC",
        		2, 15, _style.fgcolor, 0, BG_TRANS );
  draw_cursor ( mst );
  screen_update ( _fb, NULL );
  
} // end draw_help


static int
help_action (
             menu_state_t *mst
             )
{

  SDL_Event event;

  
  // Llig events.
  for (;;)
    {
      draw_help ( mst );
      g_usleep ( 10000 );
      while ( screen_next_event ( &event ) )
        switch ( event.type )
          {
          case SDL_QUIT: return QUIT;
          case SDL_KEYDOWN:
            if ( event.key.keysym.sym == SDLK_ESCAPE ) return CONTINUE;
            else if ( event.key.keysym.mod&KMOD_CTRL &&
                      event.key.keysym.sym == SDLK_q )
              return QUIT;
            break;
          case SDL_MOUSEBUTTONDOWN:
            mouse_update_pos ( mst, event.button.x, event.button.y );
            if ( event.button.button == SDL_BUTTON_RIGHT )
              return CONTINUE;
            break;
          case SDL_MOUSEBUTTONUP:
            mouse_update_pos ( mst, event.button.x, event.button.y );
            break;
          case SDL_MOUSEMOTION:
            mouse_update_pos ( mst, event.motion.x, event.motion.y );
            break;
          default: break;
          }
    }
  
  return CONTINUE;
      
} // end help_action


static const char *
quit_get_text (void)
{
  return "SURT";
} /* end quit_get_text */


static int
quit_action (
             menu_state_t *mst
             )
{
  return QUIT;
} // end quit_action


static const char *
reset_get_text (void)
{
  return "REINICIA";
} /* end reset_get_text */


static int
reset_action (
              menu_state_t *mst
              )
{
  return RESET;
} // end reset_action


static const char *
quit_mmenu_get_text (void)
{
  return "TORNA AL MENu";
} /* end quit_get_text */


static int
quit_mmenu_action (
                   menu_state_t *mst
                   )
{
  return QUIT_MAINMENU;
} // end quit_mmenu_action


static const char *
change_tvmode_get_text (void)
{
  return "CANVIA MODE TV I REINICIA";
} /* end change_tvmode_get_text */


static int
change_tvmode_action (
                      menu_state_t *mst
                      )
{

  const int N= 2;
  
  NES_TVMode tvmode;
  int sel,buttons;
  menu_state_t mst_tv;
  
  
  // Obté entrada seleccionada.
  tvmode= tvmode_get_val ();
  sel= tvmode==NES_PAL ? 0 : 1;
  
  // Menú.
  mst_tv.N= N;
  mst_tv.mouse_x= mst->mouse_x;
  mst_tv.mouse_y= mst->mouse_y;
  mst_tv.mouse_hide= mst->mouse_hide;
  mst_tv.current= sel;
  for (;;)
    {
      mpad_clear ();
      draw_change_tvmode ( &mst_tv );
      g_usleep ( 20000 );
      mst_tv.ret_mouse= NO_ACTION;
      buttons= mpad_check_buttons ( mouse_cb_change_tvmode, &mst_tv );
      if ( mst_tv.ret_mouse != NO_ACTION )
        {
          if ( mst_tv.ret_mouse != RESUME ) break;
        }
      if ( buttons&K_QUIT ) return QUIT;
      else if ( buttons&K_ESCAPE ||
                mst_tv.ret_mouse == RESUME ) return CONTINUE;
      else if ( buttons&K_BUTTON ) break;
      else if ( buttons&K_UP )
        {
          if ( mst_tv.current == 0 ) mst_tv.current= N-1;
          else --mst_tv.current;
        }
      else if ( buttons&K_DOWN ) mst_tv.current= (mst_tv.current+1)%N;
    }
      
  // Actualitza mode.
  switch ( mst_tv.current )
    {
    case 0: tvmode= NES_PAL; break;
    case 1: tvmode= NES_NTSC; break;
    default: tvmode= 0;
    }
  tvmode_set_val ( tvmode );

  // Recupera
  mst->mouse_x= mst_tv.mouse_x;
  mst->mouse_y= mst_tv.mouse_y;
  mst->mouse_hide= mst_tv.mouse_hide;
  
  return REINIT;
  
} // end change_tvmode_action




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
init_menu (
           conf_t         *conf,
           const gboolean  big_screen
           )
{
  
  _conf= conf;
  _bg_off= big_screen ? MENU_MODE_SENTINEL : 0;
  
  _style.x= 2;
  _style.y= 4;
  _style.selcolor= 0x16; /* Roig. */
  _style.fgcolor= 0x30; /* Blanc. */
  _style.cursor_black= 0x0F;
  _style.cursor_white= 0x20;
  
} // end init_menu


menu_response_t
menu_run (
          const menu_mode_t mode
          )
{

  static const menuitem_t menu_ingame_mainmenu[]=
    {
      {resume_get_text,resume_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_tvmode_get_text,change_tvmode_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_nomainmenu[]=
    {
      {resume_get_text,resume_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_tvmode_get_text,change_tvmode_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_mainmenu[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_mainmenu_bs[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_tvmode_get_text,change_tvmode_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_nomainmenu_bs[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_tvmode_get_text,change_tvmode_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_mainmenu_bs[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menumode_t menus[]=
    {
      { 11, menu_ingame_mainmenu },
      { 10, menu_ingame_nomainmenu },
      { 8, menu_mainmenu },
      { 8, menu_ingame_mainmenu_bs },
      { 7, menu_ingame_nomainmenu_bs },
      { 5, menu_mainmenu_bs }
    };
  
  const gulong delay1= 200000;
  const gulong delay2= 100000;

  menu_state_t mst;
  int buttons, ret;
  gulong delay;
  
  
  mst.N= menus[mode+_bg_off].N;
  mst.menu= menus[mode+_bg_off].items;
  mst.mouse_x= 0;
  mst.mouse_y= 0;
  mst.mouse_hide= true;
  init_background ();
  mst.current= 0;
  delay= delay1;
  mpad_clear ();
  hud_hide ();
  for (;;)
    {
      draw_menu ( &mst );
      g_usleep ( 10000 );
      mst.ret_mouse= NO_ACTION;
      buttons= mpad_check_buttons ( mouse_cb, &mst );
      if ( mst.ret_mouse != NO_ACTION )
        {
          if ( mst.ret_mouse != CONTINUE ) { ret= mst.ret_mouse; break; }
          continue;
        }
      if ( buttons&K_QUIT ) { ret= QUIT; break; }
      else if ( buttons&K_ESCAPE )
        {
          ret= mode==MENU_MODE_MAINMENU ? QUIT_MAINMENU : RESUME;
          break;
        }
      else if ( buttons&K_BUTTON )
        {
          ret= mst.menu[mst.current].action ( &mst );
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
    case RESUME: return MENU_RESUME;
    case RESET: return MENU_RESET;
    case QUIT: return MENU_QUIT;
    case REINIT: return MENU_REINIT;
    default: return MENU_QUIT_MAINMENU;
    }
  
} // end menu_run 
