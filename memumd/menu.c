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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  menu.c - Implementació de 'menu.h'.
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "MD.h"
#include "cursor.h"
#include "effects.h"
#include "hud.h"
#include "menu.h"
#include "model.h"
#include "mpad.h"
#include "pad.h"
#include "screen.h"
#include "tiles16b.h"




/**********/
/* MACROS */
/**********/

#define MAXWIDTH (320*2)
#define MAXHEIGHT (240*2)

#define WIDTH 256
#define HEIGHT 224

#define MOUSE_COUNTER 200




/*************/
/* CONSTANTS */
/*************/

static const char *CM_ENTRIES[]=
  {
    "PAL",
    "NTSC OVERSEAS",
    "NTSC DOMESTIC"
  };
const int CM_INIT_Y= 4;




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
static int _fb[MAXWIDTH*MAXHEIGHT];

/* Configuració. */
static conf_t *_conf;

/* Estil. */
static struct
{
  
  int x,y;
  int fgcolor1;
  int fgcolor2;
  int selcolor1;
  int selcolor2;
  int cursor_black;
  int cursor_white;
  
} _style;

/* Dimensions. La idea és pintar un menú de 256x224 centrat en el
   frame buffer. */
static struct
{

  int fb_width;
  int fb_height;
  int x;
  int y;
  int x_tile;
  int y_tile;
  
} _dim;

/* Fons. */
static int _background[MAXWIDTH*MAXHEIGHT];

/* Dispositius actuals. */
MD_IOPluggedDevices _devs;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
init_background (void)
{
  
  memcpy ( _background, screen_get_last_fb (),
           _dim.fb_width*_dim.fb_height*sizeof(int) );
  effect_fade ( _background, _dim.fb_width, _dim.x+4,
        	_dim.y+4, WIDTH-8, HEIGHT-8, 0.80 );
    
} /* end init_background */


static void
draw_background (void)
{
  memcpy ( _fb, _background, _dim.fb_width*_dim.fb_height*sizeof(int) );
} /* end draw_background */


static void
draw_cursor (
             menu_state_t *mst
             )
{

  if ( !mst->mouse_hide )
    {
      cursor_draw ( _fb, _dim.fb_width, _dim.fb_height,
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
  for ( i= 0, y= _style.y + _dim.y_tile; i < mst->N; ++i, ++y )
    if ( i == mst->current )
      {
        tiles16b_draw_string ( _fb, _dim.fb_width, mst->menu[i].get_text (),
        		       _style.x+_dim.x_tile, y, _style.selcolor1,
        		       _style.selcolor2, 0, T16_BG_TRANS );
      }
    else tiles16b_draw_string ( _fb, _dim.fb_width, mst->menu[i].get_text (),
        			_style.x+_dim.x_tile, y, _style.fgcolor1,
        			_style.fgcolor2, 0, T16_BG_TRANS );
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


  x0= (_style.x+_dim.x_tile)*8;
  xf= _dim.fb_width - (_style.x+_dim.x_tile)*8;
  y0= (_dim.y_tile+_style.y)*16;
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


static void
draw_change_model (
                   menu_state_t *mst
        	   )
{
  
  int i,y,len,x;


  draw_background ();
  for ( i= 0, y= CM_INIT_Y + _dim.y_tile; i < mst->N; ++i, ++y )
    {
      len= strlen ( CM_ENTRIES[i] );
      x= (_dim.fb_width/8-len)/2;
      if ( i == mst->current )
        tiles16b_draw_string ( _fb, _dim.fb_width, CM_ENTRIES[i],
        		       x, y, _style.selcolor1,
        		       _style.selcolor2, 0, T16_BG_TRANS );
      else tiles16b_draw_string ( _fb, _dim.fb_width, CM_ENTRIES[i],
        			  x, y, _style.fgcolor1,
        			  _style.fgcolor2, 0, T16_BG_TRANS );
    }
  draw_cursor ( mst );
  screen_update ( _fb, NULL );
  
} // end draw_change_model


// Torna -1 si no s'està apuntat a ninguna
static int
translate_mousexy2select_model (
                                const int x,
                                const int y,
                                const int N
                                )
{

  int x0,xf,y0,yf,ret,len;


  y0= (CM_INIT_Y + _dim.y_tile)*16;
  yf= y0 + N*16;
  if ( y >= y0 && y < yf )
    {
      ret= (y-y0)/16;
      if ( ret < 0 ) ret= 0;
      else if ( ret >= N  ) ret= N-1;
      len= strlen ( CM_ENTRIES[ret] );
      x0= ((_dim.fb_width/8-len)/2)*8;
      xf= x0 + len*8;
      if ( x < x0 || x >= xf ) ret= -1;
    }
  else ret= -1;

  return ret;
  
} // translate_mousexy2select_model


static void
mouse_cb_change_model (
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
              sel= translate_mousexy2select_model ( event->button.x,
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
  
} // end mouse_cb_change_model


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
      "PRESSIONA BOTp A",
      "PRESSIONA BOTp B",
      "PRESSIONA BOTp C",
      "PRESSIONA BOTp X",
      "PRESSIONA BOTp Y",
      "PRESSIONA BOTp Z",
      "PRESSIONA START",
    };
  
  int aux[11], i;
  gboolean stop;
  SDL_Event event;
  
  
  for ( i= 0; i < 11; ++i )
    {
      
      /* Dibuixa. */
      draw_background ();
      tiles16b_draw_string ( _fb, _dim.fb_width, "TECLAT",
        		     _dim.x_tile+2, _dim.y_tile+5,
        		     _style.fgcolor1, _style.fgcolor2,
        		     0, T16_BG_TRANS );
      tiles16b_draw_string ( _fb, _dim.fb_width, text[i],
        		     _dim.x_tile+2, _dim.y_tile+6,
        		     _style.selcolor1, _style.selcolor2,
        		     0, T16_BG_TRANS );
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
  _conf->keys[pad].button_A= aux[4];
  _conf->keys[pad].button_B= aux[5];
  _conf->keys[pad].button_C= aux[6];
  _conf->keys[pad].button_X= aux[7];
  _conf->keys[pad].button_Y= aux[8];
  _conf->keys[pad].button_Z= aux[9];
  _conf->keys[pad].start= aux[10];
  
  return CONTINUE;
  
} /* end config_keys_action */


static void
init_dim (
          const int width,
          const int height
          )
{

  _dim.fb_width= width;
  _dim.fb_height= height;
  _dim.x= (width-WIDTH)/2;
  _dim.y= (height-HEIGHT)/2;
  _dim.x_tile= _dim.x/8;
  if ( _dim.x%8 != 0 ) ++_dim.x_tile;
  _dim.y_tile= _dim.y/16;
  if ( _dim.y%16 != 0 ) ++_dim.y_tile;
  
} /* end init_dim */




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
port1_get_text (void)
{

  static const char * const text[]=
    {
      "PORT 1: PAD ESTaNDARD",
      "PORT 1: PAD 6 BOTONS",
      "PORT 1: -"
    };

  return text[_devs.dev1];
  
} /* end port1_get_text */


static int
port1_action (
              menu_state_t *mst
              )
{

  if ( _devs.dev1 == MD_IODEV_NONE ) _devs.dev1= 0;
  else ++_devs.dev1;

  return CONTINUE;
  
} // end port1_action


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
port2_get_text (void)
{

  static const char * const text[]=
    {
      "PORT 2: PAD ESTaNDARD",
      "PORT 2: PAD 6 BOTONS",
      "PORT 2: -"
    };

  return text[_devs.dev2];
  
} /* end port2_get_text */


static int
port2_action (
              menu_state_t *mst
              )
{

  if ( _devs.dev2 == MD_IODEV_NONE ) _devs.dev2= 0;
  else ++_devs.dev2;

  return CONTINUE;
  
} // end port2_action


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
  tiles16b_draw_string ( _fb, _dim.fb_width, "AJUDA",
        		 _dim.x_tile+7, _dim.y_tile+1, _style.selcolor1,
        		 _style.selcolor2, 0, T16_BG_TRANS );
  tiles16b_draw_string ( _fb, _dim.fb_width, "DESA ESTAT:       CTRL+1-5",
        		 _dim.x_tile+2, _dim.y_tile+3,
        		 _style.fgcolor1, _style.fgcolor2,
        		 0, T16_BG_TRANS );
  tiles16b_draw_string ( _fb, _dim.fb_width, "LLIG ESTAT:       ALT+1-5",
        		 _dim.x_tile+2, _dim.y_tile+5,
        		 _style.fgcolor1, _style.fgcolor2,
        		 0, T16_BG_TRANS );
  tiles16b_draw_string ( _fb, _dim.fb_width, "CAPTURA PANTALLA: CTRL+S",
        		 _dim.x_tile+2, _dim.y_tile+7,
        		 _style.fgcolor1, _style.fgcolor2,
        		 0, T16_BG_TRANS );
  tiles16b_draw_string ( _fb, _dim.fb_width, "SURT:             CTRL+Q",
        		 _dim.x_tile+2, _dim.y_tile+9,
        		 _style.fgcolor1, _style.fgcolor2,
        		 0, T16_BG_TRANS );
  tiles16b_draw_string ( _fb, _dim.fb_width, "MENu:             ESC",
        		 _dim.x_tile+2, _dim.y_tile+11,
        		 _style.fgcolor1, _style.fgcolor2,
        		 0, T16_BG_TRANS );
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
change_model_get_text (void)
{
  return "CANVIA MODEL I REINICIA";
} /* end change_model_get_text */


static int
change_model_action (
                     menu_state_t *mst
                     )
{

  const int N= 3;
  
  MDu8 model;
  int sel,buttons;
  menu_state_t mst_cm;

  // Obté entrada seleccionada.
  model= model_get_val ();
  if ( model&MD_MODEL_PAL ) sel= 0;
  else if ( model&MD_MODEL_OVERSEAS ) sel= 1;
  else sel= 2;

  // Menú.
  mst_cm.N= 3;
  mst_cm.mouse_x= mst->mouse_x;
  mst_cm.mouse_y= mst->mouse_y;
  mst_cm.mouse_hide= mst->mouse_hide;
  mst_cm.current= sel;
  for (;;)
    {
      mpad_clear ();
      draw_change_model ( &mst_cm );
      g_usleep ( 20000 );
      mst_cm.ret_mouse= NO_ACTION;
      buttons= mpad_check_buttons ( mouse_cb_change_model, &mst_cm );
      if ( mst_cm.ret_mouse != NO_ACTION )
        {
          if ( mst_cm.ret_mouse != RESUME ) break;
        }
      if ( buttons&K_QUIT ) return QUIT;
      else if ( buttons&K_ESCAPE ||
                mst_cm.ret_mouse == RESUME ) return CONTINUE;
      else if ( buttons&K_BUTTON ) break;
      else if ( buttons&K_UP )
        {
          if ( mst_cm.current == 0 ) mst_cm.current= N-1;
          else --mst_cm.current;
        }
      else if ( buttons&K_DOWN ) mst_cm.current= (mst_cm.current+1)%N;
    }
      
  // Actualitza mode.
  switch ( mst_cm.current )
    {
    case 0: model= MD_MODEL_PAL|MD_MODEL_OVERSEAS; break;
    case 1: model= MD_MODEL_OVERSEAS; break;
    case 2:
    default: model= 0;
    }
  model_set_val ( model );
  
  // Recupera
  mst->mouse_x= mst_cm.mouse_x;
  mst->mouse_y= mst_cm.mouse_y;
  mst->mouse_hide= mst_cm.mouse_hide;
  
  return REINIT;
  
} // end change_model_action




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
  _style.y= 1;
#if 0
  /* VELLS! */
  _style.bgcolor= 0x49; /* Casi negre. */
  _style.fgcolor2= 0x1F0; /* Un blau més claret. */
  _style.fgcolor1= 0x1D0; /* Un blau. */
  _style.selcolor1= 0x1F; /* Taronja . */
  _style.selcolor2= 0x3F; /* Taronja casi groc. */
#endif
  _style.selcolor2= 0x007; /* Tonalitat de roig. */
  _style.selcolor1= 0x003; /* Roig més oscur. */
  _style.fgcolor1= 0x0DB; /* Gris . */
  _style.fgcolor2= 0x1FF; /* Blanc. */
  _style.cursor_black= 0x000;
  _style.cursor_white= 0x16D;
  
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
      {change_vsync_get_text,change_vsync_action},
      {change_scaler_get_text,change_scaler_action},
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_model_get_text,change_model_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_nomainmenu[]=
    {
      {resume_get_text,resume_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_model_get_text,change_model_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_mainmenu[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
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
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_model_get_text,change_model_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_nomainmenu_bs[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_model_get_text,change_model_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_mainmenu_bs[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {change_scaler_get_text,change_scaler_action},
      {change_vsync_get_text,change_vsync_action},
      {port1_get_text,port1_action},
      {port2_get_text,port2_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menumode_t menus[]=
    {
      { 13, menu_ingame_mainmenu },
      { 12, menu_ingame_nomainmenu },
      { 10, menu_mainmenu },
      { 10, menu_ingame_mainmenu_bs },
      { 9, menu_ingame_nomainmenu_bs },
      { 7, menu_mainmenu_bs }
    };
  
  const gulong delay1= 200000;
  const gulong delay2= 100000;

  menu_state_t mst;
  int buttons, ret, width, height;
  gulong delay;
  

  mst.N= menus[mode+_bg_off].N;
  mst.menu= menus[mode+_bg_off].items;
  mst.mouse_x= 0;
  mst.mouse_y= 0;
  mst.mouse_hide= true;
  screen_get_res ( &width, &height );
  init_dim ( width, height );
  init_background ();
  mst.current= 0;
  delay= delay1;
  mpad_clear ();
  hud_hide ();
  screen_enable_cursor ( true );
  _devs= pad_get_devices ();
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
          mpad_clear (); /* neteja botons. */
          if ( ret != CONTINUE ) break;
        }
      else if ( buttons&K_UP )
        {
          if ( mst.current == 0 ) mst.current= mst.N-1;
          else --mst.current;
        }
      else if ( buttons&K_DOWN ) mst.current= (mst.current+1)%mst.N;
      if ( buttons ) /* Descansa un poc. */
        {
          g_usleep ( delay );
          delay= delay2;
        }
      else delay= delay1;
    }
  pad_set_devices ( _devs );
  switch ( ret )
    {
    case RESUME: return MENU_RESUME;
    case RESET: return MENU_RESET;
    case QUIT: return MENU_QUIT;
    case REINIT: return MENU_REINIT;
    default: return MENU_QUIT_MAINMENU;
    }
  
} // end menu_run
