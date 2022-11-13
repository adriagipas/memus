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
static int _fb[WIDTH*HEIGHT];

/* Configuració. */
static conf_t *_conf;

/* Estil. */
static struct
{
  
  int x,y;
  int fgcolor;
  int selcolor;
  
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
draw_menu (
           const menuitem_t menu[],
           const int        N,
           const int        selected
           )
{
  
  int i, y;
  
  
  draw_background ();
  for ( i= 0, y= _style.y; i < N; ++i, ++y )
    if ( i == selected )
      {
        tiles8b_draw_string ( _fb, WIDTH, menu[i].get_text (), _style.x+1,
        		      y, _style.selcolor, 0, BG_TRANS );
      }
    else tiles8b_draw_string ( _fb, WIDTH, menu[i].get_text (), _style.x+1,
        		       y, _style.fgcolor, 0, BG_TRANS );
  screen_update ( _fb, NULL );
  
} /* end draw_menu */


static void
draw_change_tvmode (
        	    const int selected
        	    )
{

  static const char *ENTRIES[]=
    {
      "PAL",
      "NTSC"
    };
  const int INIT_Y= 4;
  
  int i,y,len,x;


  draw_background ();
  for ( i= 0, y= INIT_Y; i < 2; ++i, ++y )
    {
      len= strlen ( ENTRIES[i] );
      x= (WIDTH/8-len)/2;
      if ( i == selected )
        tiles8b_draw_string ( _fb, WIDTH, ENTRIES[i], x, y,
        		      _style.selcolor, 0, BG_TRANS );
      else tiles8b_draw_string ( _fb, WIDTH, ENTRIES[i], x, y,
        			 _style.fgcolor, 0, BG_TRANS );
    }
  screen_update ( _fb, NULL );
  
} /* end draw_change_tvmode */


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
change_tvmode_get_text (void)
{
  return "CANVIA MODE TV I REINICIA";
} /* end change_tvmode_get_text */


static int
change_tvmode_action (void)
{

  const int N= 2;
  
  NES_TVMode tvmode;
  int sel,buttons;
  

  /* Obté entrada seleccionada. */
  tvmode= tvmode_get_val ();
  sel= tvmode==NES_PAL ? 0 : 1;
  
  /* Menú. */
  for (;;)
    {
      mpad_clear ();
      draw_change_tvmode ( sel );
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
    case 0: tvmode= NES_PAL; break;
    case 1: tvmode= NES_NTSC; break;
    default: tvmode= 0;
    }
  tvmode_set_val ( tvmode );
  
  return REINIT;
  
} /* end change_tvmode_action */




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
      {change_scaler_get_text,change_scaler_action},
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
      {config_keys1_get_text,config_keys1_action},
      {config_keys2_get_text,config_keys2_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_ingame_mainmenu_bs[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
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
      {help_get_text,help_action},
      {reset_get_text,reset_action},
      {change_tvmode_get_text,change_tvmode_action},
      {quit_get_text,quit_action}
    };

  static const menuitem_t menu_mainmenu_bs[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {change_scaler_get_text,change_scaler_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menumode_t menus[]=
    {
      { 10, menu_ingame_mainmenu },
      { 9, menu_ingame_nomainmenu },
      { 7, menu_mainmenu },
      { 7, menu_ingame_mainmenu_bs },
      { 6, menu_ingame_nomainmenu_bs },
      { 4, menu_mainmenu_bs }
    };
  
  const gulong delay1= 200000;
  const gulong delay2= 100000;
  
  int current, buttons, ret, N;
  gulong delay;
  const menuitem_t *menu;
  

  N= menus[mode+_bg_off].N;
  menu= menus[mode+_bg_off].items;
  init_background ();
  current= 0;
  delay= delay1;
  mpad_clear ();
  hud_hide ();
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
  switch ( ret )
    {
    case RESUME: return MENU_RESUME;
    case RESET: return MENU_RESET;
    case QUIT: return MENU_QUIT;
    case REINIT: return MENU_REINIT;
    default: return MENU_QUIT_MAINMENU;
    }
  
} /* end menu_run */
