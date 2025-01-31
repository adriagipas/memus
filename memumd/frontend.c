/*
 * Copyright 2013-2025 Adrià Giménez Pastor.
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
 *  frontend.c - Implementació de 'frontend.h'
 *
 */


#include <glib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "eeprom.h"
#include "error.h"
#include "frontend.h"
#include "hud.h"
#include "menu.h"
#include "model.h"
#include "pad.h"
#include "screen.h"
#include "sound.h"
#include "sram.h"
#include "state.h"
#include "tiles16b.h"
#include "t8biso.h"




/*********/
/* ESTAT */
/*********/

/* Per a indicar que es vol eixir. */
static gboolean _quit;

/* Per a indicar que des de el menú s'ha fet un reset. */
static gboolean _reset;

/* Frontend. */
MD_Frontend _frontend;

/* Cicles per segon actuals. */
static int _ciclespersec;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
_warning (
          void       *udata,
          const char *format,
          ...
          )
{
  
  va_list ap;
  
  
  va_start ( ap, format );
  vwarning ( format, ap );
  va_end ( ap );
  
} /* end _warning */


/* Torna 0 si tot ha anat bé. */
static int
write_screenshot (void)
{

  int width, height, r, c;
  const int *fb;
  SDL_Surface *img;
  MD_RGB color;
  char *data;
  int *pixels;
  GString *buffer;
  GDateTime *dt;
  gchar *time_str;
  

  /* Inicialitza. */
  img= NULL;
  buffer= NULL;
  
  /* Obté fb. */
  screen_get_res ( &width, &height );
  fb= screen_get_last_fb ();

  /* Crea imatge en memòria. */
  img= SDL_CreateRGBSurface ( 0, width, height, 32, 0x000000ff,
        		      0x0000ff00, 0x00ff0000, 0xff000000 );
  if ( img == NULL )
    {
      warning ( "CreateRGBSurface ha fallat: %s", SDL_GetError () );
      goto error;
    }
  for ( r= 0, data= (char *) img->pixels;
        r < height;
        ++r, data+= img->pitch )
    {
      pixels= (int *) data;
      for ( c= 0; c < width; ++c )
        {
          color= MD_color2rgb ( *(fb++) );
          pixels[c]= 0xff000000 | (color.b<<16) | (color.g<<8) | color.r;
        }
    }

  /* Crea el nom del fitxer. */
  buffer= g_string_new ( NULL );
  dt= g_date_time_new_now_local ();
  if ( dt == NULL )
    error ( "error inesperat al cridar a g_date_time_new_now_local" );
  time_str= g_date_time_format_iso8601 ( dt );
  g_date_time_unref ( dt );
  g_string_printf ( buffer, "%s/memumd-screenshot-%s.png",
        	    g_get_user_special_dir ( G_USER_DIRECTORY_PICTURES ),
        	    time_str );
  g_free ( time_str );

  /* Escriu. */
  if ( IMG_SavePNG ( img, buffer->str ) < 0 )
    {
      warning ( "Error al desar '%s': %s", buffer->str, SDL_GetError ()  );
      goto error;
    }
  fprintf ( stderr, "S'ha fet una captura de pantalla en '%s'\n",
            buffer->str );
  
  /* Allibera memòria. */
  SDL_FreeSurface ( img );
  g_string_free ( buffer, TRUE );
  
  return 0;
  
 error:
  if ( img != NULL ) SDL_FreeSurface ( img );
  if ( buffer != NULL ) g_string_free ( buffer, TRUE );
  return -1;
  
} /* end write_screenshot */


static void
take_screenshot (void)
{

  int width, height;
  
  
  screen_get_res ( &width, &height );
  if ( write_screenshot () == 0 )
    {
      hud_show_msg ( "CAPTURA DE PANTALLA FETA", width );
      hud_flash ();
    }
  else hud_show_msg ( "ERROR!", width );
  
} /* end take_screenshot */


static void
save_state (
            const int num
            )
{
  
  static char buffer[20];
  
  FILE *f;
  int width, height;
  
  
  f= state_open_write ( num );
  if ( f == NULL )
    {
      warning ( "no es pot obrir el fitxer de l'estat per a la posició %d",
                num );
      return;
    }
  if ( MD_save_state ( f ) != 0 )
    screen_show_error ( "Desa estat",
                        "No s'ha pogut desar l'estat" );
  else
    {
      screen_get_res ( &width, &height );
      sprintf ( buffer, "ESTAT DESAT EN %d", num );
      hud_show_msg ( buffer, width );
    }
  fclose ( f );
  
} /* end save_state */


static void
load_state (
            const int num
            )
{
  
  static char buffer[20];
  
  FILE *f;
  int width, height;
  
  
  f= state_open_read ( num );
  if ( f == NULL )
    {
      warning ( "no hi ha cap estat desat en la posició %d", num );
      return;
    }
  if ( MD_load_state ( f ) != 0 )
    screen_show_error ( "Llig estat",
                        "No s'ha pogut llegir l'estat, o l'estat no és vàlid" );
  else
    {
      screen_get_res ( &width, &height );
      sprintf ( buffer, "ESTAT LLEGIT DE %d", num );
      hud_show_msg ( buffer, width );
    }
  fclose ( f );
  
} /* end load_state */


static void
check_signals (
               MD_Bool *stop,
               MD_Bool *reset,
               void    *udata
               )
{
  
  SDL_Event event;
  
  
  *stop= MD_FALSE;
  *reset= _reset; _reset= FALSE;
  while ( screen_next_event ( &event ) )
    switch ( event.type )
      {

      case SDL_QUIT:
        _quit= TRUE;
        *stop= MD_TRUE;
        return;

      case SDL_MOUSEBUTTONDOWN:
        if ( event.button.button == SDL_BUTTON_RIGHT )
          {
            *stop= MD_TRUE;
            return;
          }
        break;
        
        /* Tecles ràpides. No hi ha BREAK APOSTA!!! D'aquesta manera
           es passa al pad. */
      case SDL_KEYDOWN:
        if ( event.key.keysym.mod&KMOD_CTRL )
          {
            switch ( event.key.keysym.sym )
              {
              case SDLK_1: save_state ( 1 ); break;
              case SDLK_2: save_state ( 2 ); break;
              case SDLK_3: save_state ( 3 ); break;
              case SDLK_4: save_state ( 4 ); break;
              case SDLK_5: save_state ( 5 ); break;
              case SDLK_s: take_screenshot (); break;
              case SDLK_q:
                _quit= TRUE;
                *stop= MD_TRUE;
                return;
              default: break;
              }
            break;
          }
        else if ( event.key.keysym.mod&KMOD_ALT )
          {
            switch ( event.key.keysym.sym )
              {
              case SDLK_1: load_state ( 1 ); break;
              case SDLK_2: load_state ( 2 ); break;
              case SDLK_3: load_state ( 3 ); break;
              case SDLK_4: load_state ( 4 ); break;
              case SDLK_5: load_state ( 5 ); break;
              default: break;
              }
            break;
          }

      default:
        if ( pad_event ( &event ) )
          {
            *stop= MD_TRUE;
            return;
          }
        
      }
  
} /* end check_signals */


static void
loop (void)
{
  
  const gint64 SLEEP= 1000;
  const gint64 MIN_DELAY= -10000;

  int cc,cc_iter;
  gint64 t0,tf,delay;
  MD_Bool stop;
  
  
  screen_enable_cursor ( true );
  stop= MD_FALSE;
  t0= g_get_monotonic_time ();
  cc_iter= (int) ((_ciclespersec/1000000.0)*SLEEP + 0.5);
  cc= 0;
  delay= 0;
  for (;;)
    {

      // Executa.
      cc+= cc_iter;
      while ( cc > 0 )
        {
          cc-= MD_iter ( &stop );
          if ( stop ) return;
        }
      
      // Delay
      tf= g_get_monotonic_time ();
      delay+= SLEEP-(tf-t0); t0= tf;
      if ( delay >= SLEEP ) g_usleep ( SLEEP );
      else if ( delay < MIN_DELAY ) delay= MIN_DELAY;
      
    }
  
} // end loop




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_frontend (void)
{

  close_t8biso ();
  close_pad ();
  close_sound ();
  close_screen ();
  SDL_Quit ();
  
} /* end close_frontend */


menu_response_t
frontend_run (
              const MD_Rom       *rom,
              const MD_RomHeader *header,
              const char         *rom_id,
              const char         *sram_fn,
              const char         *eeprom_fn,
              const char         *state_prefix,
              const menu_mode_t   menu_mode,
              const int           verbose
              )
{
  
  menu_response_t ret;
  MDu8 model;
  
  
  ret= MENU_QUIT_MAINMENU;
  _quit= _reset= FALSE;
  init_model ( rom_id, header, verbose );
  init_eeprom ( rom_id, eeprom_fn, verbose );
  init_sram ( rom_id, sram_fn, verbose );
  init_state ( rom_id, state_prefix, verbose );
  model= model_get_val ();
  _ciclespersec= model&MD_MODEL_PAL ?
    MD_CYCLES_PER_SEC_PAL : MD_CYCLES_PER_SEC_NTSC;
  _frontend.plugged_devs= pad_get_devices (); /* Inicialitza. */
  sound_change_freq ( (model&MD_MODEL_PAL) != 0 );
  MD_init ( rom, model, &_frontend, NULL );
  for (;;)
    {
      pad_clear ();
      loop ();
      if ( _quit ) { ret= MENU_QUIT; break; }
      ret= menu_run ( menu_mode );
      if ( ret == MENU_RESET ) _reset= TRUE;
      else if ( ret == MENU_REINIT )
        {
          close_sram ();
          close_eeprom ();
          model= model_get_val ();
          _ciclespersec= model&MD_MODEL_PAL ?
            MD_CYCLES_PER_SEC_PAL : MD_CYCLES_PER_SEC_NTSC;
          _frontend.plugged_devs= pad_get_devices (); /* Guarda l'última
        						 configuració. */
          MD_close ();
          MD_init ( rom, model, &_frontend, NULL );
          init_eeprom ( rom_id, eeprom_fn, verbose );
          init_sram ( rom_id, sram_fn, verbose );
        }
      else if ( ret != MENU_RESUME ) break;
    }
  close_sram ();
  close_eeprom ();
  close_model ();
  _frontend.plugged_devs= pad_get_devices (); /* Guarda última configuració. */
  MD_close ();
  
  return ret;
  
} /* end frontend_run */


void
init_frontend (
               conf_t         *conf,
               const char     *title,
               const gboolean  big_screen
               )
{
  
  int ret;
  

  /* Inicialitza. */
  ret= SDL_Init ( SDL_INIT_AUDIO|SDL_INIT_VIDEO|
                  SDL_INIT_JOYSTICK|SDL_INIT_EVENTS );
  if ( ret != 0 )
    error ( "no s'ha pogut inicialitzar SDL: %s", SDL_GetError () );
  SDL_DisableScreenSaver ();
  init_screen ( conf, title, big_screen );
  init_sound ();
  init_pad ( conf, NULL );
  init_tiles16b ();
  init_t8biso ();
  init_menu ( conf, big_screen );
  init_hud ();
  
  /* Inicialitza frontend. */
  _frontend.warning= _warning;
  _frontend.check= check_signals;
  _frontend.sres_changed= screen_sres_changed;
  _frontend.update_screen= screen_update;
  _frontend.play_sound= sound_play;
  _frontend.get_static_ram= sram_get_static_ram;
  _frontend.get_eeprom= eeprom_get_eeprom;
  _frontend.trace= NULL;
  _frontend.plugged_devs= conf->devs;
  _frontend.check_buttons= pad_check_buttons;
  
} /* end init_frontend */
