/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "effects.h"
#include "hud.h"
#include "menu.h"
#include "mpad.h"
#include "screen.h"
#include "tiles8b.h"




/*********/
/* TIPUS */
/*********/

/* Valor que torna action. */
enum {
  CONTINUE,
  RESUME,
  QUIT,
  QUIT_MAINMENU
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
  effect_fade ( _background, 4, 4, WIDTH-8, HEIGHT-8, 0.80 );
  
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
        /*
        tiles8b_draw_string ( _fb, WIDTH, ">", _style.x, y,
        		      _style.selcolor, 0, BG_TRANS );
        */
        tiles8b_draw_string ( _fb, WIDTH, menu[i].get_text (), _style.x+1, y,
        		      _style.selcolor, 0, BG_TRANS );
      }
    else tiles8b_draw_string ( _fb, WIDTH, menu[i].get_text (), _style.x+1, y,
        		       _style.fgcolor, 0, BG_TRANS );
  screen_update ( _fb, NULL );
  
} /* end draw_menu */




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
      "FINESTRA X4",
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
config_keys_get_text (void)
{
  return "AJUSTA EL TECLAT";
} /* end config_keys_get_text */


static int
config_keys_action (void)
{
  
  static const char * const text[]=
    {
      "PRESSIONA ESQUERRA",
      "PRESSIONA DRETA",
      "PRESSIONA AMUNT",
      "PRESSIONA AVALL",
      "PRESSIONA BOTp A",
      "PRESSIONA BOTp B",
      "PRESSIONA SELECT",
      "PRESSIONA START",
    };
  
  SDL_Keycode aux[8], i;
  gboolean stop;
  SDL_Event event;
  
  
  mpad_clear ();
  for ( i= 0; i < 8; ++i )
    {
      
      /* Dibuixa. */
      draw_background ();
      tiles8b_draw_string ( _fb, WIDTH, "TECLAT", 1, 6,
        		    _style.fgcolor, 0, BG_TRANS );
      tiles8b_draw_string ( _fb, WIDTH, text[i], 1, 7,
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
  _conf->keys.left= aux[0];
  _conf->keys.right= aux[1];
  _conf->keys.up= aux[2];
  _conf->keys.down= aux[3];
  _conf->keys.button_A= aux[4];
  _conf->keys.button_B= aux[5];
  _conf->keys.select= aux[6];
  _conf->keys.start= aux[7];
  
  return CONTINUE;
  
} /* end config_keys_action */


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
  tiles8b_draw_string ( _fb, WIDTH, "AJUDA", 7, 3,
                        _style.selcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "DESA EST.: CTR+1-5", 1, 6,
                        _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "LLIG EST.: ALT+1-5", 1, 8,
                        _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "CAP. PAN.: CTR+S", 1, 10,
                        _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "SURT:      CTR+Q", 1, 12,
                        _style.fgcolor, 0, BG_TRANS );
  tiles8b_draw_string ( _fb, WIDTH, "MENu:      ESC", 1, 14,
                        _style.fgcolor, 0, BG_TRANS );
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
quit_mmenu_get_text (void)
{
  return "TORNA AL MENu";
} /* end quit_get_text */


static int
quit_mmenu_action (void)
{
  return QUIT_MAINMENU;
} /* end quit_mmenu_action */




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
  
  _style.x= 1;
  _style.y= 5;
#if 0
  /* VELLS!!! */
  _style.bgcolor= 0x7FFF; /* Blanc. */
  _style.fgcolor= 0x0000; /* Negre. */
  _style.selcolor= 0x181F; /* Taronja menú Mario. */
#endif
  _style.fgcolor= 0x7FFF; /* Blanc. */
  _style.selcolor= 0x001F; /* Roig. */
  
} /* end init_menu */


void
menu_change_conf (
                  conf_t *conf
                  )
{
  _conf= conf;
} /* end menu_change_conf */


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
      {config_keys_get_text,config_keys_action},
      {help_get_text,help_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };
  
  static const menuitem_t menu_ingame_nomainmenu[]=
    {
      {resume_get_text,resume_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {config_keys_get_text,config_keys_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menuitem_t menu_mainmenu[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {screen_size_get_text,screen_size_action},
      {change_scaler_get_text,change_scaler_action},
      {config_keys_get_text,config_keys_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menuitem_t menu_ingame_mainmenu_bg[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
      {help_get_text,help_action},
      {quit_mmenu_get_text,quit_mmenu_action},
      {quit_get_text,quit_action}
    };
  
  static const menuitem_t menu_ingame_nomainmenu_bg[]=
    {
      {resume_get_text,resume_action},
      {change_scaler_get_text,change_scaler_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menuitem_t menu_mainmenu_bg[]=
    {
      {quit_mmenu_get_text,quit_mmenu_action},
      {change_scaler_get_text,change_scaler_action},
      {help_get_text,help_action},
      {quit_get_text,quit_action}
    };
  
  static const menumode_t menus[]=
    {
      { 7, menu_ingame_mainmenu },
      { 6, menu_ingame_nomainmenu },
      { 6, menu_mainmenu },
      { 5, menu_ingame_mainmenu_bg },
      { 4, menu_ingame_nomainmenu_bg },
      { 4, menu_mainmenu_bg }
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
      if ( buttons&K_QUIT ) return MENU_QUIT;
      else if ( buttons&K_ESCAPE )
        return mode==MENU_MODE_MAINMENU ? MENU_QUIT_MAINMENU : MENU_RESUME;
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
  
  /* Ací no pot ser CONTINUE. */
  return
    (ret==RESUME) ? MENU_RESUME :
    ((ret==QUIT) ? MENU_QUIT : MENU_QUIT_MAINMENU);
  
} /* end menu_run */
