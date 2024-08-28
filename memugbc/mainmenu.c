/*
 * Copyright 2015-2024 Adrià Giménez Pastor.
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
 *  mainmenu.c - Implementació de 'mainmenu.h'.
 *
 */


#include <glib.h>
#include <SDL.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "background.h"
#include "conf.h"
#include "error.h"
#include "fchooser.h"
#include "frontend.h"
#include "hud.h"
#include "mainmenu.h"
#include "menu.h"
#include "rom.h"
#include "screen.h"




/*********/
/* ESTAT */
/*********/

// Opcions.
static gboolean _verbose;

/* Configuració per defecte. */
conf_t *_conf;

// Background.
static int _background[WIDTH*HEIGHT];




/*********/
/* TIPUS */
/*********/

enum {
  CONTINUE,
  ERROR,
  QUIT,
};




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
init_background (void)
{

  int i;


  for ( i= 0; i < WIDTH*HEIGHT; ++i )
    _background[i]= (int) BACKGROUND[i];
  
} // end init_background


// Torna CONTINUE, ERROR o QUIT.
static int
run_romfn (
           const gchar *fn
           )
{

  GBC_Rom rom;
  GBC_RomHeader header;
  const char *rom_id;
  menu_response_t response;
  int ret;
  

  // Inicialitza.
  if ( load_rom ( fn, &rom, _verbose ) != 0 )
    return ERROR;
  GBC_rom_get_header ( &rom, &header );
  rom_id= get_rom_id ( &rom, &header, _verbose );
  /*
  get_conf ( &conf, rom_id, NULL, _conf, _verbose );
  frontend_reconfigure ( &conf, "memuGBC ");
  */
  
  // Executa.
  ret= frontend_run ( &rom, rom_id, NULL, NULL,
        	      MENU_MODE_INGAME_MAINMENU,
                      fn, &response, _verbose );
  
  // Tanca.
  /*
  write_conf ( &conf, rom_id, NULL, _verbose );
  free_conf ( &conf );
  frontend_reconfigure ( _conf, "memuGBC" );
  */
  GBC_rom_free ( rom );
  
  return ret==-1 ? ERROR : (response==MENU_QUIT ? QUIT : CONTINUE);
  
} // end run_romfn




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
main_menu (
           conf_t         *conf,
           const gboolean  verbose
           )
{

  const char *rom_fn;
  bool stop,quit;
  fchooser_t *fc;
  
  
  // Prepara.
  _verbose= verbose;
  _conf= conf;
  hud_hide ();
  init_background ();
  fc= fchooser_new ( "[.]gbc?$", false, _background, verbose );
  
  // Executa.
  if ( frontend_resume ( MENU_MODE_INGAME_MAINMENU, verbose ) != MENU_QUIT )
    {
      stop= quit= false;
      while ( !stop )
        {
          rom_fn= fchooser_run ( fc, &quit );
          if ( quit ) stop= true;
          else if ( rom_fn == NULL )
            {
              if ( menu_run ( MENU_MODE_MAINMENU ) == MENU_QUIT )
                stop= true;
            }
          else
            {
              switch ( run_romfn ( rom_fn ) )
                {
                case ERROR:
                  if ( fchooser_error_dialog ( fc ) == -1 )
                    stop= true;
                  break;
                case QUIT:
                  stop= true;
                  break;
                default: break;
                }
            }
        }
    }
  
  // Allibera.
  fchooser_free ( fc );
  
} // end main_menu
