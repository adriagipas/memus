/*
 * Copyright 2025 Adrià Giménez Pastor.
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
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "cmos.h"
#include "error.h"
#include "frontend.h"
#include "load_bios.h"
#include "load_hdd.h"
#include "load_vgabios.h"
#include "menu.h"
#include "screen.h"
#include "sound.h"
#include "tiles8b.h"
#include "tiles16b.h"
#include "t8biso.h"




/*********/
/* ESTAT */
/*********/

// Verbositat.
static bool _verbose;

// Configuració
static conf_t *_conf;

// Per a indicar que es vol eixir.
static bool _quit;

// Per a indicar que des de el menú s'ha fet un reset.
static bool _reset;

// Frontend.
static PC_Frontend _frontend;

// Dispositius IDE connectats.
static PC_IDEDevice _ide_devices[2][2];

// BIOS
static uint8_t *_bios;
static size_t _bios_size;

// Disquets.
static PC_File *_fd[2];

// Configuració. De moment valors fixes.
static PC_Config _config=
  {
    .flags= PC_CFG_QEMU_COMPATIBLE,
    .ram_size= PC_RAM_SIZE_32MB,
    .qemu_boot_order= {
      .check_floppy_sign= true,
      .order= {
        PC_QEMU_BOOT_ORDER_FLOPPY,
        PC_QEMU_BOOT_ORDER_HD,
        PC_QEMU_BOOT_ORDER_NONE }
    },
    .pci_devs= {
      {
        .dev= PC_PCI_DEVICE_SVGA_CIRRUS_CLGD5446,
        .optrom= NULL,
        .optrom_size= 0
      },
      {
        .dev= PC_PCI_DEVICE_NULL
      }
    },
    .cpu_model= IA32_CPU_P5_66MHZ, // <-- NO VA la disquetera AMB 60Hz!!!
    .diskettes= {
      PC_DISKETTE_1M44,
      PC_DISKETTE_1M2,
      PC_DISKETTE_NONE,
      PC_DISKETTE_NONE
    },
    .host_mouse= {
      .resolution= 25.0
    }
  };




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
  
} // end _warning


static void
get_current_time (
                  void    *udata,
                  uint8_t *ss,
                  uint8_t *mm,
                  uint8_t *hh,
                  uint8_t *day_week,
                  uint8_t *day_month,
                  uint8_t *month,
                  int     *year
                  )
{

  GTimeZone *tz;
  GDateTime *date;


  tz= NULL;
  date= NULL;
  tz= g_time_zone_new_local ();
  if ( tz == NULL ) goto error;
  date= g_date_time_new_now ( tz );
  if ( date == NULL ) goto error;
  *ss= g_date_time_get_second ( date );
  *mm= g_date_time_get_minute ( date );
  *hh= g_date_time_get_hour ( date );
  *day_week= g_date_time_get_day_of_week ( date );
  *day_month= g_date_time_get_day_of_month ( date );
  *month= g_date_time_get_month ( date );
  *year= g_date_time_get_year ( date );
  g_time_zone_unref ( tz );
  g_date_time_unref ( date );
  
  return;
  
 error:
  warning ( udata, "no s'ha pogut obtindre l'hora i data actuals,"
            " es fixa a l'inici de la UNIX EPOCH" );
  *ss= 0;
  *mm= 0;
  *hh= 0;
  *day_week= 4;
  *day_month= 1;
  *month= 1;
  *year= 1970;
  if ( tz == NULL ) g_time_zone_unref ( tz );
  if ( date == NULL ) g_date_time_unref ( date );
  
} // end get_current_time


static void
write_sea_bios_debug_port (
                           const char  c,
                           void       *udata
                           )
{

  static int pos= 0;


  if ( pos == 0 ) fprintf ( stderr, "[BIOS::DEBUG] " );
  fputc ( c, stderr );
  if ( c == '\n' ) pos= 0;
  else             ++pos;
  fflush ( stderr );
  
} // end write_sea_bios_debug_port


static PC_Scancode
get_scancode (
              const SDL_Event *event
              )
{

  PC_Scancode ret;


  // NOTA!! No vaig a implementar les combinacions rares!!!! Tampoc
  // PC_KBDSP_BLOQ_DESPL, PC_KBDSP_PAUSA, PC_KBDSP_WIN_DERECHA
  //
  // FALTEN ELS DOS BOTONS DE TILDES !!! ES MAPEJEN AL MATEIX BOTO!!!
  switch ( event->key.keysym.sym )
    {
    case SDLK_ESCAPE: ret= PC_KBDSP_ESC; break;
    case SDLK_F1: ret= PC_KBDSP_F1; break;
    case SDLK_F2: ret= PC_KBDSP_F2; break;
    case SDLK_F3: ret= PC_KBDSP_F3; break;
    case SDLK_F4: ret= PC_KBDSP_F4; break;
    case SDLK_F5: ret= PC_KBDSP_F5; break;
    case SDLK_F6: ret= PC_KBDSP_F6; break;
    case SDLK_F7: ret= PC_KBDSP_F7; break;
    case SDLK_F8: ret= PC_KBDSP_F8; break;
    case SDLK_F9: ret= PC_KBDSP_F9; break;
    case SDLK_F10: ret= PC_KBDSP_F10; break;
    case SDLK_F11: ret= PC_KBDSP_F11; break;
    case SDLK_F12: ret= PC_KBDSP_F12; break;
    case SDLK_PRINTSCREEN: ret= PC_KBDSP_IMP_PNT; break;
    case SDLK_BACKSPACE: ret= PC_KBDSP_RETROCESO; break;
    case SDLK_EXCLAIM: ret= PC_KBDSP_ABRE_EXCLAMACION; break;
    case SDLK_QUOTE: ret= PC_KBDSP_COMILLAS; break;
    case SDLK_0: ret= PC_KBDSP_0; break;
    case SDLK_1: ret= PC_KBDSP_1; break;
    case SDLK_2: ret= PC_KBDSP_2; break;
    case SDLK_3: ret= PC_KBDSP_3; break;
    case SDLK_4: ret= PC_KBDSP_4; break;
    case SDLK_5: ret= PC_KBDSP_5; break;
    case SDLK_6: ret= PC_KBDSP_6; break;
    case SDLK_7: ret= PC_KBDSP_7; break;
    case SDLK_8: ret= PC_KBDSP_8; break;
    case SDLK_9: ret= PC_KBDSP_9; break;
    case SDLK_TAB: ret= PC_KBDSP_TABULADOR; break;
      //case SDLK_COMPOSE: ret= PC_KBDSP_ACCENT_OBERT; break;
    case SDLK_PLUS: ret= PC_KBDSP_SIGNO_MAS; break;
    case SDLK_RETURN: ret= PC_KBDSP_ENTRAR; break;
    case SDLK_CAPSLOCK: ret= PC_KBDSP_BLOQ_MAYUS; break;
    case 241: ret= PC_KBDSP_ENYE; break;
      //case SDLK_COMPOSE: ret= PC_KBDSP_TILDE; break;
    case 231: ret= PC_KBDSP_C_TRENCADA; break;
    case SDLK_LSHIFT: ret= PC_KBDSP_MAYUS; break;
    case SDLK_LESS: ret= PC_KBDSP_MENOR; break;
    case SDLK_COMMA: ret= PC_KBDSP_COMA; break;
    case SDLK_MINUS: ret= PC_KBDSP_GUION; break;
    case SDLK_PERIOD: ret= PC_KBDSP_PUNTO; break;
    case SDLK_RSHIFT: ret= PC_KBDSP_MAYUS_DERECHA; break;
    case SDLK_LCTRL: ret= PC_KBDSP_CONTROL; break;
    case SDLK_LGUI: ret= PC_KBDSP_WINDOWS; break;
    case SDLK_LALT: ret= PC_KBDSP_ALT; break;
    case SDLK_SPACE: ret= PC_KBDSP_ESPACIO; break;
    case SDLK_MODE: ret= PC_KBDSP_ALT_GR; break;
    case SDLK_MENU: ret= PC_KBDSP_MENU; break;
    case SDLK_RCTRL: ret= PC_KBDSP_CONTROL_DERECHA; break;
    case SDLK_RALT: ret= PC_KBDSP_ALT_GR; break;
    case SDLK_a: ret= PC_KBDSP_A; break;
    case SDLK_b: ret= PC_KBDSP_B; break;
    case SDLK_c: ret= PC_KBDSP_C; break;
    case SDLK_d: ret= PC_KBDSP_D; break;
    case SDLK_e: ret= PC_KBDSP_E; break;
    case SDLK_f: ret= PC_KBDSP_F; break;
    case SDLK_g: ret= PC_KBDSP_G; break;
    case SDLK_h: ret= PC_KBDSP_H; break;
    case SDLK_i: ret= PC_KBDSP_I; break;
    case SDLK_j: ret= PC_KBDSP_J; break;
    case SDLK_k: ret= PC_KBDSP_K; break;
    case SDLK_l: ret= PC_KBDSP_L; break;
    case SDLK_m: ret= PC_KBDSP_M; break;
    case SDLK_n: ret= PC_KBDSP_N; break;
    case SDLK_o: ret= PC_KBDSP_O; break;
    case SDLK_p: ret= PC_KBDSP_P; break;
    case SDLK_q: ret= PC_KBDSP_Q; break;
    case SDLK_r: ret= PC_KBDSP_R; break;
    case SDLK_s: ret= PC_KBDSP_S; break;
    case SDLK_t: ret= PC_KBDSP_T; break;
    case SDLK_u: ret= PC_KBDSP_U; break;
    case SDLK_v: ret= PC_KBDSP_V; break;
    case SDLK_w: ret= PC_KBDSP_W; break;
    case SDLK_x: ret= PC_KBDSP_X; break;
    case SDLK_y: ret= PC_KBDSP_Y; break;
    case SDLK_z: ret= PC_KBDSP_Z; break;
    case SDLK_DELETE: ret= PC_KBDSP_SUPR; break;
    case SDLK_END: ret= PC_KBDSP_FIN; break;
    case SDLK_PAGEDOWN: ret= PC_KBDSP_AV_PAG; break;
    case SDLK_INSERT: ret= PC_KBDSP_INSERT; break;
    case SDLK_HOME: ret= PC_KBDSP_INICIO; break;
    case SDLK_PAGEUP: ret= PC_KBDSP_RE_PAG; break;
    case SDLK_UP: ret= PC_KBDSP_ARRIBA; break;
    case SDLK_DOWN: ret= PC_KBDSP_ABAJO; break;
    case SDLK_RIGHT: ret= PC_KBDSP_DERECHA; break;
    case SDLK_LEFT: ret= PC_KBDSP_IZQUIERDA; break;
    case SDLK_KP_0: ret= PC_KBDSP_NUM_0; break;
    case SDLK_KP_1: ret= PC_KBDSP_NUM_1; break;
    case SDLK_KP_2: ret= PC_KBDSP_NUM_2; break;
    case SDLK_KP_3: ret= PC_KBDSP_NUM_3; break;
    case SDLK_KP_4: ret= PC_KBDSP_NUM_4; break;
    case SDLK_KP_5: ret= PC_KBDSP_NUM_5; break;
    case SDLK_KP_6: ret= PC_KBDSP_NUM_6; break;
    case SDLK_KP_7: ret= PC_KBDSP_NUM_7; break;
    case SDLK_KP_8: ret= PC_KBDSP_NUM_8; break;
    case SDLK_KP_9: ret= PC_KBDSP_NUM_9; break;
    case SDLK_KP_PERIOD: ret= PC_KBDSP_NUM_PUNTO; break;
    case SDLK_KP_ENTER: ret= PC_KBDSP_NUM_ENTRAR; break;
    case SDLK_KP_PLUS: ret= PC_KBDSP_NUM_SUMA; break;
    case SDLK_NUMLOCKCLEAR: ret= PC_KBDSP_BLOQ_NUM; break;
    case SDLK_KP_DIVIDE: ret= PC_KBDSP_NUM_DIV; break;
    case SDLK_KP_MULTIPLY: ret= PC_KBDSP_NUM_MUL; break;
    case SDLK_KP_MINUS: ret= PC_KBDSP_NUM_RESTA; break;
    case 186: ret= PC_KBDSP_SUPER_O; break;
    default: ret= PC_SCANCODE_ALL;
    }

  return ret;
  
} // end get_scancode


static void
check_signals (
               bool *stop,
               bool *reset
               )
{
  
  SDL_Event event;
  PC_Scancode key;
  
  
  *stop= false;
  *reset= _reset; _reset= false;
  while ( screen_next_event ( &event, NULL ) )
    switch ( event.type )
      {
        
      case SDL_WINDOWEVENT:
        if ( event.window.event == SDL_WINDOWEVENT_FOCUS_LOST )
          PC_kbd_clear ();
        break;
        
      case SDL_QUIT:
        _quit= TRUE;
        *stop= true;
        return;

        // Tecles ràpides. No hi ha BREAK APOSTA!!! D'aquesta manera
        // es passa al pad.
      case SDL_KEYDOWN:
        if ( (event.key.keysym.mod&(KMOD_LCTRL|KMOD_LGUI)) ==
             (KMOD_LCTRL|KMOD_LGUI))
          {
            switch ( event.key.keysym.sym )
              {
              case SDLK_q:
                _quit= true;
                *stop= true;
                return;
              case SDLK_ESCAPE:
              case SDLK_RETURN:
                *stop= true;
                return;
              default: break;
              }
            break;
          }
        else
          {
            key= get_scancode ( &event );
            if ( key != PC_SCANCODE_ALL )
              {
                PC_kbd_press ( key );
                if ( key == PC_KBDSP_BLOQ_MAYUS )
                  PC_kbd_release ( key );
              }
            else fprintf(stderr,"KEY_DOWN key:%d mod:%d\n",
                         event.key.keysym.sym,
                         event.key.keysym.mod);
          }
        break;

      case SDL_KEYUP:
        key= get_scancode ( &event );
        if ( key != PC_SCANCODE_ALL )
          {
            if ( key == PC_KBDSP_BLOQ_MAYUS )
              PC_kbd_press ( key );
            PC_kbd_release ( key );
          }
        else fprintf(stderr,"KEY_UP key:%d mod:%d\n",
                     event.key.keysym.sym,
                     event.key.keysym.mod);
        break;

      case SDL_MOUSEMOTION:
        PC_mouse_motion ( (int32_t) event.motion.xrel,
                          (int32_t) event.motion.yrel );
        break;

      case SDL_MOUSEBUTTONUP:
        switch ( event.button.button )
          {
          case SDL_BUTTON_LEFT:
            PC_mouse_button_release ( PC_MOUSE_BUTTON_LEFT );
            break;
          case SDL_BUTTON_MIDDLE:
            PC_mouse_button_release ( PC_MOUSE_BUTTON_MIDDLE );
            break;
          case SDL_BUTTON_RIGHT:
            PC_mouse_button_release ( PC_MOUSE_BUTTON_RIGHT );
            break;
          }
        break;

      case SDL_MOUSEBUTTONDOWN:
        switch ( event.button.button )
          {
          case SDL_BUTTON_LEFT:
            PC_mouse_button_press ( PC_MOUSE_BUTTON_LEFT );
            break;
          case SDL_BUTTON_MIDDLE:
            PC_mouse_button_press ( PC_MOUSE_BUTTON_MIDDLE );
            break;
          case SDL_BUTTON_RIGHT:
            PC_mouse_button_press ( PC_MOUSE_BUTTON_RIGHT );
            break;
          }
        break;
        
      default: break;
        
      }
  
} // end check_signals


static void
reset_sim (void)
{

  int i;

  
  // En realitat reinicie.
  PC_close ();
  PC_init ( _bios, _bios_size, _ide_devices,
            &_frontend, NULL, &_config );
  for ( i= 0; i < 2; ++i )
    PC_fd_insert_floppy ( _fd[i], i );
  
} // end reset_sim


static void
loop (void)
{

  const gint64 SLEEP= 1000;
  const gint64 MIN_DELAY= -10000;

  int cc,cc_iter;
  gint64 t0,tf,delay;
  bool stop,reset;


  screen_enable_cursor ( true );
  stop= reset= false;
  t0= g_get_monotonic_time ();
  cc_iter= (int) ((PC_ClockFreq/1000000.0)*SLEEP + 0.5);
  cc= 0;
  delay= 0;
  for (;;)
    {

      // Executa.
      cc+= cc_iter;
      while ( cc > 0 )
        cc-= PC_jit_iter ( cc );
      check_signals ( &stop, &reset );
      if ( stop ) return;
      else if ( reset ) reset_sim ();
      
      // Delay
      tf= g_get_monotonic_time ();
      delay+= SLEEP-(tf-t0); t0= tf;
      if ( delay >= SLEEP ) g_usleep ( SLEEP );
      else if ( delay < MIN_DELAY ) delay= MIN_DELAY;
      
    }
  
} // end loop


static void
remove_fd (
           const int drv
           )
{

  PC_fd_insert_floppy ( NULL, drv );
  if ( _fd[drv] != NULL )
    {
      PC_file_free ( _fd[drv] );
      _fd[drv]= NULL;
    }
  
} // end remove_fd




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_frontend (void)
{

  int i;
  
  
  for ( i= 0; i < 2; ++i )
    if ( _fd[i] != NULL )
      PC_file_free ( _fd[i] );
  close_menu ();
  PC_cdrom_free ( _ide_devices[1][0].cdrom.cdrom );
  close_cmos ();
  close_t8biso ();
  close_sound ();
  close_screen ();
  SDL_Quit ();
  
} // end close_frontend


menu_response_t
frontend_run (
              const gchar *disc_D,
              const gchar *disc_A,
              const gchar *disc_B
              )
{
  
  menu_response_t ret;

  PC_Error err;
  
  
  ret= MENU_QUIT;
  _quit= _reset= false;

  // Inicialitza el simulador.
  do {
    
    // Carrega BIOS.
    if ( !load_bios ( _conf, &_bios, &_bios_size, _verbose ) )
      goto quit;
    
    // Carrega VGABIOS.
    if ( !load_vgabios ( _conf,
                         &_config.pci_devs[0].optrom,
                         &_config.pci_devs[0].optrom_size,
                         _verbose ) )
      goto quit;
    
    // Carrega HDD.
    _ide_devices[0][0].hdd.f= load_hdd ( _conf, _verbose );
    if ( _ide_devices[0][0].hdd.f == NULL ) goto quit;
    
    // Inicialitza
    err= PC_init ( _bios, _bios_size, _ide_devices,
                   &_frontend, NULL, &_config );
    switch ( err )
      {
      case PC_BADBIOS:
        warning ( "'%s' no és una BIOS vàlida", _conf->bios_fn );
        remove_bios ( _conf );
        break;
      case PC_BADOPTROM:
        warning ( "'%s' no és una VGA BIOS vàlida", _conf->vgabios_fn );
        remove_vgabios ( _conf );
        break;
      case PC_UNK_CPU_MODEL:
        error ( "Unknown CPU model" );
        break;
      case PC_HDD_WRONG_SIZE:
        warning ( "'%s' no té una grandària vàlida per a ser un disc dur ",
                  _conf->hdd_fn );
        remove_hdd ( _conf );
      case PC_NOERROR: break;
      default:
        error ( "frontend_run - PC_init - WTF!!" );
        break;
      }
    
  } while ( err != PC_NOERROR );

  // Inserta discs.
  if ( disc_D != NULL )
    {
      if ( !frontend_set_disc ( disc_D, DISC_D_CDROM ) )
        warning ( "no s'ha pogut inserir el disc '%s'", disc_D );
    }
  if ( disc_A != NULL )
    {
      if ( !frontend_set_disc ( disc_A, DISC_A_FLOPPY_1M44 ) )
        warning ( "no s'ha pogut inserir el disc '%s'", disc_A );
    }
  if ( disc_B != NULL )
    {
      if ( !frontend_set_disc ( disc_B, DISC_B_FLOPPY_1M2 ) )
        warning ( "no s'ha pogut inserir el disc '%s'", disc_B );
    }
  
  // Executa loop.
  for (;;)
    {
      PC_kbd_clear ();
      screen_grab_cursor ( true );
      loop ();
      screen_grab_cursor ( false );
      if ( _quit ) { ret= MENU_QUIT; break; }
      ret= menu_run ();
      if ( ret == MENU_RESET ) _reset= TRUE;
      else if ( ret != MENU_RESUME ) break;
    }
  
  // Allibera memòria.
  PC_close ();
  free_hdd ();
  free_vgabios ();
  free_bios ();
  
  return ret;

 quit:
  free_hdd ();
  free_vgabios ();
  free_bios ();
  return MENU_QUIT;
  
} // end frontend_run


void
init_frontend (
               conf_t     *conf,
               const char *title,
               const bool  verbose
               )
{
  
  int ret;
  

  _verbose= verbose;
  _conf= conf;
  
  // Inicialitza.
  ret= SDL_Init ( SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_INIT_EVENTS );
  if ( ret != 0 )
    error ( "no s'ha pogut inicialitzar SDL: %s", SDL_GetError () );
  SDL_DisableScreenSaver ();
  init_screen ( conf, title );
  init_sound ();
  init_tiles8b ();
  init_tiles16b ();
  init_t8biso ();
  init_cmos ( verbose );
  init_menu ( conf, verbose );
  
  // Inicialitza frontend.
  _frontend.warning= _warning;
  _frontend.write_sb_dbg_port= write_sea_bios_debug_port;
  _frontend.update_screen= screen_update;
  _frontend.play_sound= sound_play;
  _frontend.get_cmos_ram= cmos_get_ram;
  _frontend.get_current_time= get_current_time;
  _frontend.trace= NULL;

  // Dispositius IDE.
  _ide_devices[0][0].hdd.type= PC_IDE_DEVICE_TYPE_HDD;
  _ide_devices[0][0].hdd.f= NULL; // Temporal
  _ide_devices[0][1].type= PC_IDE_DEVICE_TYPE_NONE;
  _ide_devices[1][0].cdrom.type= PC_IDE_DEVICE_TYPE_CDROM;
  _ide_devices[1][0].cdrom.cdrom= PC_cdrom_new ();
  if ( _ide_devices[1][0].cdrom.cdrom == NULL )
    error ( "no s'ha pogut reservar memòria per al cdrom" );
  _ide_devices[1][1].type= PC_IDE_DEVICE_TYPE_NONE;

  // Disquets.
  _fd[0]= NULL;
  _fd[1]= NULL;
  
} // end init_frontend


bool
frontend_set_disc (
                   const gchar *file_name,
                   const int    type
                   )
{

  bool ok;
  char *err;
  int fd_drv;
  PC_File *f;
  PC_Error err_ret;

  
  ok= false;
  switch ( type )
    {
    case DISC_A_FLOPPY_1M44:
    case DISC_B_FLOPPY_1M2:
      fd_drv= type==DISC_A_FLOPPY_1M44 ? 0 : 1;
      if ( file_name == NULL ) remove_fd ( fd_drv );
      else
        {
          f= PC_file_new_from_file ( file_name, true );
          if ( f != NULL )
            {
              remove_fd ( fd_drv );
              err_ret= PC_fd_insert_floppy ( f, fd_drv );
              if ( err_ret != PC_NOERROR )
                {
                  warning ( "no s'ha pogut inserir '%s' en la unitat"
                            " %s: grandària invàlida",
                            file_name, type==DISC_A_FLOPPY_1M44 ? "A" : "B" );
                  PC_file_free ( f );
                }
              else
                {
                  printf("INSERIT %s\n",file_name);
                  _fd[fd_drv]= f;
                  ok= true;
                }
            }
          else
            warning ( "no s'ha pogut inserir '%s' en la unitat %s:"
                      " error de lectura",
                      file_name, type==DISC_A_FLOPPY_1M44 ? "A" : "B" );
        }
      break;
    case DISC_D_CDROM:
      ok= PC_cdrom_insert_disc ( _ide_devices[1][0].cdrom.cdrom,
                                 file_name, &err );
      if ( !ok )
        {
          warning ( "no s'ha pogut insertar '%s' en la unitat D: %s",
                    file_name, err );
          free ( err );
        }
      break;
    default:
      error ( "no s'ha pogut insertar el disc '%s': "
              "dispositiu (%d) desconegut",
              file_name, type );
    }

  return ok;
  
} // end frontend_set_disc


bool
frontend_disc_is_empty (
                        const int type
                        )
{

  bool ret;
  
  
  switch ( type )
    {
    case DISC_A_FLOPPY_1M44:
      ret= _fd[0]==NULL;
      break;
    case DISC_B_FLOPPY_1M2:
      ret= _fd[1]==NULL;
      break;
    case DISC_D_CDROM:
      ret= (_ide_devices[1][0].cdrom.cdrom->current == NULL);
      break;
    default:
      ret= false;
      error ( "no s'ha pogut comprovar si el dispositiu %d està"
              " buit: dispositiu desconegut", type );
    }

  return ret;
  
} // end frontend_disc_is_empty


void
frontend_change_bios (
                      uint8_t      *bios,
                      const size_t  bios_size
                      )
{

  _bios= bios;
  _bios_size= bios_size;
  reset_sim ();
  
} // end frontend_change_bios


void
frontend_change_vgabios (
                         uint8_t      *bios,
                         const size_t  bios_size
                         )
{

  _config.pci_devs[0].optrom= bios;
  _config.pci_devs[0].optrom_size= bios_size;
  reset_sim ();
  
} // end frontend_change_vgabios


void
frontend_change_hdd (
                     PC_File *hdd
                     )
{

  _ide_devices[0][0].hdd.f= hdd;
  reset_sim ();
  
} // end frontend_change_hdd
