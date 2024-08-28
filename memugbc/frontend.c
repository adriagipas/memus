/*
 * Copyright 2014-2024 Adrià Giménez Pastor.
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
 *  frontend.c - Implementació de 'frontend.h'
 *
 */


#include <glib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "error.h"
#include "frontend.h"
#include "hud.h"
#include "menu.h"
#include "pad.h"
#include "rom.h"
#include "screen.h"
#include "sound.h"
#include "sram.h"
#include "state.h"
#include "suspend.h"
#include "tiles8b.h"
#include "t8biso.h"




/*********/
/* ESTAT */
/*********/

/* Per a indicar que es vol eixir. */
static gboolean _quit;

// Bios i verbose
static const GBCu8 **_bios;
static int _verbose;




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

  const double FACTOR= 255.0/31.0;
  
  int r, c, red, green, blue, color;
  const int *fb;
  SDL_Surface *img;
  char *data;
  int *pixels;
  GString *buffer;
  GDateTime *dt;
  gchar *time_str;
  

  /* Inicialitza. */
  img= NULL;
  buffer= NULL;
  
  /* Obté fb. */
  fb= screen_get_last_fb ();
  
  /* Crea imatge en memòria. */
  img= SDL_CreateRGBSurface ( 0, WIDTH, HEIGHT, 32, 0x000000ff,
                              0x0000ff00, 0x00ff0000, 0xff000000 );
  if ( img == NULL )
    {
      warning ( "CreateRGBSurface ha fallat: %s", SDL_GetError () );
      goto error;
    }
  for ( r= 0, data= (char *) img->pixels;
        r < HEIGHT;
        ++r, data+= img->pitch )
    {
      pixels= (int *) data;
      for ( c= 0; c < WIDTH; ++c )
        {
          
          color= *(fb++);
          red= (int) ((color&0x1F)*FACTOR + 0.5);
          green= (int) (((color>>5)&0x1F)*FACTOR + 0.5);
          blue= (int) (((color>>10)&0x1F)*FACTOR + 0.5);
          pixels[c]= 0xff000000 | (blue<<16) | (green<<8) | red;
        }
    }

  /* Crea el nom del fitxer. */
  buffer= g_string_new ( NULL );
  dt= g_date_time_new_now_local ();
  if ( dt == NULL )
    error ( "error inesperat al cridar a g_date_time_new_now_local" );
  time_str= g_date_time_format_iso8601 ( dt );
  g_date_time_unref ( dt );
  g_string_printf ( buffer, "%s/memugbc-screenshot-%s.png",
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

  if ( write_screenshot () == 0 )
    {
      hud_show_msg ( "CAPTURA PANTALLA" );
      hud_flash ();
    }
  else hud_show_msg ( "ERROR!" );
  
} /* end take_screenshot */


static void
save_state (
            const int num
            )
{
  
  static char buffer[20];
  
  FILE *f;
  
  
  f= state_open_write ( num );
  if ( f == NULL )
    {
      warning ( "no es pot obrir el fitxer de l'estat per a la posició %d",
                num );
      return;
    }
  if ( GBC_save_state ( f ) != 0 )
    screen_show_error ( "Desa estat",
                        "No s'ha pogut desar l'estat" );
  else
    {
      sprintf ( buffer, "ESTAT DESAT EN %d", num );
      hud_show_msg ( buffer );
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
  
  
  f= state_open_read ( num );
  if ( f == NULL )
    {
      warning ( "no hi ha cap estat desat en la posició %d", num );
      return;
    }
  if ( GBC_load_state ( f ) != 0 )
    screen_show_error ( "Llig estat",
                        "No s'ha pogut llegir l'estat, o l'estat no és vàlid" );
  else
    {
      sprintf ( buffer, "ESTAT LLEGIT DE %d", num );
      hud_show_msg ( buffer );
    }
  fclose ( f );
  
} /* end load_state */


static void
suspend_clear (
               const gboolean verbose
               )
{
  suspend_write_rom_file_name ( "", verbose );
} // end suspend_clear


static void
suspend (
         const gchar    *rom_fn,
         const gboolean  verbose
         )
{
  
  FILE *f;
  
  
  // Neteja el nom de la rom.
  if ( !suspend_write_rom_file_name ( "", verbose ) )
    return;
  
  // Intenta escriure l'estat
  f= get_suspend_state_file ( true, verbose );
  if ( f  == NULL ) return;
  if ( GBC_save_state ( f ) != 0 )
    {
      warning ( "No s'ha pogut desar l'estat" );
      fclose ( f );
      return;
    }
  fclose ( f );

  // Desa el nom de la rom.
  suspend_write_rom_file_name ( rom_fn, verbose );
  
} // end suspend


static void
check_signals (
               GBC_Bool *stop,
               GBC_Bool *button_pressed,
               GBC_Bool *direction_pressed,
               void     *udata
               )
{
  
  SDL_Event event;
  
  
  *stop= *direction_pressed= *button_pressed= GBC_FALSE;
  while ( screen_next_event ( &event ) )
    switch ( event.type )
      {

      case SDL_QUIT:
        _quit= TRUE;
        *stop= GBC_TRUE;
        return;

      case SDL_MOUSEBUTTONDOWN:
        if ( event.button.button == SDL_BUTTON_RIGHT )
          {
            *stop= GBC_TRUE;
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
        	*stop= GBC_TRUE;
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
        if ( pad_event ( &event, button_pressed, direction_pressed ) )
          {
            *stop= GBC_TRUE;
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
  GBC_Bool stop;
  

  screen_enable_cursor ( true );
  stop= GBC_FALSE;
  t0= g_get_monotonic_time ();
  cc_iter= (int) ((GBC_CICLES_PER_SEC/1000000.0)*SLEEP + 0.5);
  cc= 0;
  delay= 0;
  for (;;)
    {

      // Executa.
      cc+= cc_iter;
      while ( cc > 0 )
        {
          cc-= GBC_iter ( &stop );
          if ( stop ) return;
        }
      
      // Delay
      tf= g_get_monotonic_time ();
      delay+= SLEEP-(tf-t0); t0= tf;
      if ( delay >= SLEEP ) g_usleep ( SLEEP );
      else if ( delay < MIN_DELAY ) delay= MIN_DELAY;
      
    }

} // end loop


static int
frontend_run_common (
                     const GBC_Rom     *rom,
                     const char        *rom_id,
                     const char        *sram_fn,
                     const char        *state_prefix,
                     const menu_mode_t  menu_mode,
                     const gchar       *rom_fn,
                     bool               resume_exec,
                     menu_response_t   *response,
                     const int          verbose
                     )
{
  
  static const GBC_Frontend frontend=
    {
      _warning,
      sram_get_external_ram,
      screen_update,
      check_signals,
      pad_check_buttons,
      sound_play,
      pad_update_rumble,
      NULL
    };
  
  GBC_Error err;
  FILE *f;
  
  
  *response= MENU_QUIT_MAINMENU;
  _quit= FALSE;
  init_sram ( rom_id, sram_fn, _verbose );
  init_state ( rom_id, state_prefix, _verbose );
  err= GBC_init ( *_bios, rom, &frontend, NULL );
  switch ( err )
    {
    case GBC_EUNKMAPPER:
      warning ( "el mapper de la ROM és desconegut" );
      goto error;
    case GBC_WRONGLOGO:
      warning ( "la ROM no ha passat el test del logotip" );
      goto error;
    case GBC_WRONGCHKS:
      warning ( "la ROM no ha passat el test del checksum" );
      goto error;
    case GBC_WRONGRAMSIZE:
      warning ( "la grandària de la RAM no està suportada" );
      goto error;
    case GBC_WRONGROMSIZE:
      warning ("la grandària de la ROM no està suportada" );
      goto error;
    case GBC_NOERROR: break;
    }
  if ( resume_exec )
    {
      f= get_suspend_state_file ( false, verbose );
      if ( f == NULL ) goto error_resum_exec;
      if ( GBC_load_state ( f ) != 0 )
        {
          warning ( "No s'ha pogut llegir l'estat, o l'estat no és vàlid" );
          fclose ( f );
          goto error_resum_exec;
        }
      fclose ( f );
    }
  for (;;)
    {
      pad_clear ();
      loop ();
      if ( _quit )
        {
          *response= MENU_QUIT;
          if ( rom_fn != NULL ) suspend ( rom_fn, verbose );
          break;
        }
      *response= menu_run ( menu_mode );
      if ( *response != MENU_RESUME )
        {
          if ( rom_fn != NULL )
            {
              if ( *response == MENU_QUIT ) suspend ( rom_fn, verbose );
              else                          suspend_clear ( verbose );
            }
          break;
        }
    }
  close_sram ();

  return 0;

 error_resum_exec:
  suspend_clear ( verbose );
  close_sram ();
  *response= MENU_QUIT_MAINMENU;
  return 0;
 error:
  close_sram ();
  return -1;
  
} // end frontend_run_common




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


void
frontend_reconfigure (
                      conf_t     *conf,
                      const char *title
                      )
{

  screen_change_scaler ( conf->scaler );
  screen_change_size ( conf->screen_size );
  screen_change_title ( title );
  menu_change_conf ( conf );
  pad_change_conf ( conf );
  
} /* end frontend_reconfigure */


int
frontend_run (
              const GBC_Rom     *rom,
              const char        *rom_id,
              const char        *sram_fn,
              const char        *state_prefix,
              const menu_mode_t  menu_mode,
              const gchar       *rom_fn,
              menu_response_t   *response,
              const int          verbose
              )
{
  return frontend_run_common ( rom, rom_id, sram_fn, state_prefix,
                               menu_mode, rom_fn, false, response,
                               verbose );
} // end frontend_run


menu_response_t
frontend_resume (
                 const menu_mode_t menu_mode,
                 const gboolean    verbose
                 )
{
  
  gchar *rom_fn;
  GBC_Rom rom;
  GBC_RomHeader header;
  const gchar *rom_id;
  menu_response_t ret;
  

  // Prepara.
  rom_fn= NULL;
  rom.banks= NULL;
  
  // Recupera el nom de la rom.
  rom_fn= suspend_read_rom_file_name ( verbose );
  if ( rom_fn == NULL ) goto error;

  // Intenta carregar la rom
  if ( load_rom ( rom_fn, &rom, verbose ) != 0 )
    goto error;
  GBC_rom_get_header ( &rom, &header );
  rom_id= get_rom_id ( &rom, &header, verbose );
  
  // Executa
  if ( frontend_run_common ( &rom, rom_id, NULL, NULL, menu_mode,
                             rom_fn, true, &ret, verbose ) != 0 )
    goto error;
  
  // Allibera recursos.
  g_free ( rom_fn );
  GBC_rom_free ( rom );
  
  return ret;
  
 error:
  GBC_rom_free ( rom );
  if ( rom_fn != NULL ) g_free ( rom_fn );
  return MENU_QUIT_MAINMENU;
  
} // end frontend_resume


void
init_frontend (
               conf_t          *conf,
               const char      *title,
               const GBCu8    **bios,
               const gboolean   big_screen,
               const int        verbose
               )
{

  int ret;
  
  
  _verbose= verbose;
  _bios= bios;
  
  ret= SDL_Init ( SDL_INIT_AUDIO|SDL_INIT_VIDEO|
        	  SDL_INIT_JOYSTICK|SDL_INIT_EVENTS|
        	  SDL_INIT_HAPTIC );
  if ( ret != 0 )
    error ( "no s'ha pogut inicialitzar SDL: %s", SDL_GetError () );
  SDL_DisableScreenSaver ();
  init_screen ( conf, title, big_screen );
  init_sound ();
  init_pad ( conf );
  init_tiles8b ();
  init_t8biso ();
  init_menu ( conf, bios, big_screen, verbose );
  init_hud ();
  init_suspend ();
  
} // end init_frontend
