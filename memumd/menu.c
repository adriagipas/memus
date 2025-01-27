/*
 * Copyright 2014-2023 Adrià Giménez Pastor.
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




/*********/
/* TIPUS */
/*********/

/* Valor que torna action. */
enum {
  CONTINUE,
  RESUME,
  QUIT,
  RESET,
  QUIT_MAINMENU,
  REINIT
};

typedef struct
{
  
  const char * (*get_text) (void);
  int          (*action) (void);
  
} menuitem_t;

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
draw_menu (
           const menuitem_t menu[],
           const int        N,
           const int        selected
           )
{
  
  int i, y;
  
  
  draw_background ();
  for ( i= 0, y= _style.y + _dim.y_tile; i < N; ++i, ++y )
    if ( i == selected )
      {
        tiles16b_draw_string ( _fb, _dim.fb_width, menu[i].get_text (),
        		       _style.x+_dim.x_tile, y, _style.selcolor1,
        		       _style.selcolor2, 0, T16_BG_TRANS );
      }
    else tiles16b_draw_string ( _fb, _dim.fb_width, menu[i].get_text (),
        			_style.x+_dim.x_tile, y, _style.fgcolor1,
        			_style.fgcolor2, 0, T16_BG_TRANS );
  screen_update ( _fb, NULL );
  
} /* end draw_menu */


static void
draw_change_model (
        	   const int selected
        	   )
{

  static const char *ENTRIES[]=
    {
      "PAL",
      "NTSC OVERSEAS",
      "NTSC DOMESTIC"
    };
  const int INIT_Y= 4;
  
  int i,y,len,x;


  draw_background ();
  for ( i= 0, y= INIT_Y + _dim.y_tile; i < 3; ++i, ++y )
    {
      len= strlen ( ENTRIES[i] );
      x= (_dim.fb_width/8-len)/2;
      if ( i == selected )
        tiles16b_draw_string ( _fb, _dim.fb_width, ENTRIES[i],
        		       x, y, _style.selcolor1,
        		       _style.selcolor2, 0, T16_BG_TRANS );
      else tiles16b_draw_string ( _fb, _dim.fb_width, ENTRIES[i],
        			  x, y, _style.fgcolor1,
        			  _style.fgcolor2, 0, T16_BG_TRANS );
    }
  screen_update ( _fb, NULL );
  
} /* end draw_change_model */


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
resume_action (void)
{
  return RESUME;
} /* end resume_action */


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
screen_size_action (void)
{
  
  if ( ++(_conf->screen_size) == SCREEN_SIZE_SENTINEL )
    _conf->screen_size= SCREEN_SIZE_WIN_X1;
  screen_change_size ( _conf->screen_size );
  
  return CONTINUE;
  
} /* end screen_size_action */


static const char *
change_scaler_get_text ()
{
  
  static const char * const text[]=
    {
      "FILTRE: CAP",
      "FILTRE: SCALE2X"
    };
  
  return text[_conf->scaler];
  
} /* end change_scaler_get_text */


static int
change_scaler_action (void)
{
  
  if ( ++(_conf->scaler) == SCALER_SENTINEL )
    _conf->scaler= SCALER_NONE;
  screen_change_scaler ( _conf->scaler );
  
  return CONTINUE;
  
} /* end change_scaler_action */


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
change_vsync_action (void)
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
config_keys1_action (void)
{
  return config_keys_action ( 0 );
} /* end config_keys1_action */


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
port1_action (void)
{

  if ( _devs.dev1 == MD_IODEV_NONE ) _devs.dev1= 0;
  else ++_devs.dev1;

  return CONTINUE;
  
} /* end port1_action */


static const char *
config_keys2_get_text (void)
{
  return "AJUSTA EL TECLAT PER AL PAD2";
} /* end config_keys2_get_text */


static int
config_keys2_action (void)
{
  return config_keys_action ( 1 );
} /* end config_keys2_action */


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
port2_action (void)
{

  if ( _devs.dev2 == MD_IODEV_NONE ) _devs.dev2= 0;
  else ++_devs.dev2;

  return CONTINUE;
  
} /* end port2_action */


static const char *
help_get_text (void)
{
  return "AJUDA";
} /* end help_get_text */


static int
help_action (void)
{

  SDL_Event event;

  
  /* Dibuixa. */
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
  screen_update ( _fb, NULL );
  
  /* Llig events. */
  for (;;)
    {
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
          default: break;
          }
      g_usleep ( 100000 );
    }
  
  return CONTINUE;
      
} /* end help_action */


static const char *
quit_get_text (void)
{
  return "SURT";
} /* end quit_get_text */


static int
quit_action (void)
{
  return QUIT;
} /* end quit_action */


static const char *
reset_get_text (void)
{
  return "REINICIA";
} /* end reset_get_text */


static int
reset_action (void)
{
  return RESET;
} /* end reset_action */


static const char *
quit_mmenu_get_text (void)
{
  return "TORNA AL MENu";
} /* end quit_get_text */


static int
quit_mmenu_action (void)
{
  return QUIT_MAINMENU;
} /* end quit_mmenu_action */


static const char *
change_model_get_text (void)
{
  return "CANVIA MODEL I REINICIA";
} /* end change_model_get_text */


static int
change_model_action (void)
{

  const int N= 3;
  
  MDu8 model;
  int sel,buttons;
  

  /* Obté entrada seleccionada. */
  model= model_get_val ();
  if ( model&MD_MODEL_PAL ) sel= 0;
  else if ( model&MD_MODEL_OVERSEAS ) sel= 1;
  else sel= 2;

  /* Menú. */
  for (;;)
    {
      mpad_clear ();
      draw_change_model ( sel );
      g_usleep ( 20000 );
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) return QUIT;
      else if ( buttons&K_ESCAPE ) return CONTINUE;
      else if ( buttons&K_BUTTON ) break;
      else if ( buttons&K_UP )
        {
          if ( sel == 0 ) sel= N-1;
          else --sel;
        }
      else if ( buttons&K_DOWN ) sel= (sel+1)%N;
    }
      
  /* Actualitza mode. */
  switch ( sel )
    {
    case 0: model= MD_MODEL_PAL|MD_MODEL_OVERSEAS; break;
    case 1: model= MD_MODEL_OVERSEAS; break;
    case 2:
    default: model= 0;
    }
  model_set_val ( model );
  
  return REINIT;
  
} /* end change_model_action */




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
  
} /* end init_menu */


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
  
  int current, buttons, ret, N, width, height;
  gulong delay;
  const menuitem_t *menu;
  

  N= menus[mode+_bg_off].N;
  menu= menus[mode+_bg_off].items;
  screen_get_res ( &width, &height );
  init_dim ( width, height );
  init_background ();
  current= 0;
  delay= delay1;
  mpad_clear ();
  hud_hide ();
  _devs= pad_get_devices ();
  for (;;)
    {
      draw_menu ( menu, N, current );
      g_usleep ( 10000 );
      buttons= mpad_check_buttons ();
      if ( buttons&K_QUIT ) { ret= QUIT; break; }
      else if ( buttons&K_ESCAPE )
        {
          ret= mode==MENU_MODE_MAINMENU ? QUIT_MAINMENU : RESUME;
          break;
        }
      else if ( buttons&K_BUTTON )
        {
          ret= menu[current].action ();
          mpad_clear (); /* neteja botons. */
          if ( ret != CONTINUE ) break;
        }
      else if ( buttons&K_UP )
        {
          if ( current == 0 ) current= N-1;
          else --current;
        }
      else if ( buttons&K_DOWN ) current= (current+1)%N;
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
  
} /* end menu_run */
