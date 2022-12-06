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
 *  conf.c - Implementació de 'conf.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "dirs.h"
#include "error.h"




/**********/
/* MACROS */
/**********/

#define GROUP "main"




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_default_conf_file_name (void)
{
  return g_build_filename ( get_confdir (), "default.cfg", NULL );
} // end get_default_conf_file_name


static void
set_default_values (
        	    conf_t *conf
        	    )
{
  
  conf->keys[0].up= SDLK_w;
  conf->keys[0].left= SDLK_a;
  conf->keys[0].down= SDLK_s;
  conf->keys[0].right= SDLK_d;
  conf->keys[0].button_triangle= SDLK_o;
  conf->keys[0].button_circle= SDLK_l;
  conf->keys[0].button_cross= SDLK_k;
  conf->keys[0].button_square= SDLK_i;
  conf->keys[0].button_L2= SDLK_q;
  conf->keys[0].button_R2= SDLK_e;
  conf->keys[0].button_L1= SDLK_u;
  conf->keys[0].button_R1= SDLK_p;
  conf->keys[0].start= SDLK_SPACE;
  conf->keys[0].select= SDLK_RETURN;

  conf->keys[1].up= SDLK_UP;
  conf->keys[1].left= SDLK_LEFT;
  conf->keys[1].down= SDLK_DOWN;
  conf->keys[1].right= SDLK_RIGHT;
  conf->keys[1].button_triangle= SDLK_KP_8;
  conf->keys[1].button_circle= SDLK_KP_6;
  conf->keys[1].button_cross= SDLK_KP_2;
  conf->keys[1].button_square= SDLK_KP_4;
  conf->keys[1].button_L2= SDLK_KP_1;
  conf->keys[1].button_R2= SDLK_KP_3;
  conf->keys[1].button_L1= SDLK_KP_7;
  conf->keys[1].button_R1= SDLK_KP_9;
  conf->keys[1].start= SDLK_KP_5;
  conf->keys[1].select= SDLK_KP_0;
  
  conf->screen_size= SCREEN_SIZE_WIN_X1;
  conf->vsync= TRUE;
  conf->controllers[0]= PSX_CONTROLLER_STANDARD;
  conf->controllers[1]= PSX_CONTROLLER_NONE;
  conf->bios_fn= NULL;
  conf->tvres_is_pal= true;
  conf->memc_fn[0]= NULL;
  conf->memc_fn[1]= NULL;
  
} // end set_default_values


// Si existeix el camp el guarda en dest.
static void
get_key_value (
               GKeyFile    *f,
               const gchar *name,
               SDL_Keycode *dest
               )
{
  
  gchar *val;
  SDL_Keycode aux;
  
  
  val= g_key_file_get_string ( f, GROUP, name, NULL );
  if ( val == NULL ) return;
  aux= SDL_GetKeyFromName ( val );
  if ( aux == SDLK_UNKNOWN )
    warning ( "'%s' no és un nom de tecla vàlid,"
              " '%s' tindrà el seu valor per defecte",
              val, name );
  else *dest= aux;
  g_free ( val );
  
} // end get_key_value


static void
get_controller_value (
                      GKeyFile       *f,
                      const gchar    *name,
                      PSX_Controller *dest
                      )
{

  gchar *val;
  
  
  val= g_key_file_get_string ( f, GROUP, name, NULL );
  if ( val == NULL ) return;
  if ( !strcmp ( val, "none" ) ) *dest= PSX_CONTROLLER_NONE;
  else if ( !strcmp ( val, "standard" ) ) *dest= PSX_CONTROLLER_STANDARD;
  else warning ( "'%s' no és un dispositiu conegut,"
        	 " '%s' tindrà el seu valor per defecte",
        	 val, name );
  g_free ( val );
  
} // end get_controller_value


static void
read_conf (
           conf_t     *conf,
           const char *name,
           const int   verbose
           )
{
  
  GKeyFile *f;
  gboolean ret;
  GError *err;
  gchar *val;
  
  
  if ( verbose )
    fprintf ( stderr, "Llegint fitxer de configuració: %s\n", name );
  
  // Obri.
  f= g_key_file_new ();
  err= NULL;
  ret= g_key_file_load_from_file ( f, name, G_KEY_FILE_NONE, &err );
  if ( !ret )
    error ( "no s'ha pogut obrir el fitxer de configuració '%s': %s",
            name, err->message );
  
  // Fixa valors.
  get_key_value ( f, "key-pad1-up", &(conf->keys[0].up) );
  get_key_value ( f, "key-pad1-down", &(conf->keys[0].down) );
  get_key_value ( f, "key-pad1-left", &(conf->keys[0].left) );
  get_key_value ( f, "key-pad1-right", &(conf->keys[0].right) );
  get_key_value ( f, "key-pad1-button-triangle",
                  &(conf->keys[0].button_triangle) );
  get_key_value ( f, "key-pad1-button-circle", &(conf->keys[0].button_circle) );
  get_key_value ( f, "key-pad1-button-cross", &(conf->keys[0].button_cross) );
  get_key_value ( f, "key-pad1-button-square", &(conf->keys[0].button_square) );
  get_key_value ( f, "key-pad1-button-L2", &(conf->keys[0].button_L2) );
  get_key_value ( f, "key-pad1-button-R2", &(conf->keys[0].button_R2) );
  get_key_value ( f, "key-pad1-button-L1", &(conf->keys[0].button_L1) );
  get_key_value ( f, "key-pad1-button-R1", &(conf->keys[0].button_R1) );
  get_key_value ( f, "key-pad1-start", &(conf->keys[0].start) );
  get_key_value ( f, "key-pad1-select", &(conf->keys[0].select) );
  
  get_key_value ( f, "key-pad2-up", &(conf->keys[1].up) );
  get_key_value ( f, "key-pad2-down", &(conf->keys[1].down) );
  get_key_value ( f, "key-pad2-left", &(conf->keys[1].left) );
  get_key_value ( f, "key-pad2-right", &(conf->keys[1].right) );
  get_key_value ( f, "key-pad2-button-triangle",
                  &(conf->keys[1].button_triangle) );
  get_key_value ( f, "key-pad2-button-circle", &(conf->keys[1].button_circle) );
  get_key_value ( f, "key-pad2-button-cross", &(conf->keys[1].button_cross) );
  get_key_value ( f, "key-pad2-button-square", &(conf->keys[1].button_square) );
  get_key_value ( f, "key-pad2-button-L2", &(conf->keys[1].button_L2) );
  get_key_value ( f, "key-pad2-button-R2", &(conf->keys[1].button_R2) );
  get_key_value ( f, "key-pad2-button-L1", &(conf->keys[1].button_L1) );
  get_key_value ( f, "key-pad2-button-R1", &(conf->keys[1].button_R1) );
  get_key_value ( f, "key-pad2-start", &(conf->keys[1].start) );
  get_key_value ( f, "key-pad2-select", &(conf->keys[1].select) );
  
  val= g_key_file_get_string ( f, GROUP, "screen-size", NULL );
  if ( val != NULL )
    {
      if ( !strcmp ( val, "win_x1" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1;
      else if ( !strcmp ( val, "win_x1.5" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X1_5;
      else if ( !strcmp ( val, "win_x2" ) )
        conf->screen_size= SCREEN_SIZE_WIN_X2;
      else if ( !strcmp ( val, "fullscreen" ) )
        conf->screen_size= SCREEN_SIZE_FULLSCREEN;
      else warning ( "grandària de finestra no suportada: %s", val );
      g_free ( val );
    }
  conf->vsync= g_key_file_get_boolean ( f, GROUP, "vsync", &err );
  get_controller_value ( f, "pad1", &(conf->controllers[0]) );
  get_controller_value ( f, "pad2", &(conf->controllers[1]) );
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  conf->bios_fn= g_key_file_get_string ( f, GROUP, "bios", NULL );
  conf->tvres_is_pal= g_key_file_get_boolean ( f, GROUP, "tvres-is-pal", &err );
  if ( conf->memc_fn[0] != NULL ) g_free ( conf->memc_fn[0] );
  conf->memc_fn[0]= g_key_file_get_string ( f, GROUP, "memc1", NULL );
  if ( conf->memc_fn[1] != NULL ) g_free ( conf->memc_fn[1] );
  conf->memc_fn[1]= g_key_file_get_string ( f, GROUP, "memc2", NULL );
  
  // Allibera memòria.
  g_key_file_free ( f );
  
} // end read_conf


static void
set_key_value (
               GKeyFile    *f,
               const gchar *name,
               SDL_Keycode  val
               )
{
  
  const char *aux;
  
  
  aux= SDL_GetKeyName ( val );
  assert ( aux != NULL );
  g_key_file_set_string ( f, GROUP, name, aux );
  
} // end set_key_value


static void
set_controller_value (
                      GKeyFile             *f,
                      const gchar          *name,
                      const PSX_Controller  val
                      )
{
  
  const char *aux;
  

  aux= NULL;
  switch ( val )
    {
    case PSX_CONTROLLER_STANDARD: aux= "standard"; break;
    case PSX_CONTROLLER_NONE: aux= "none"; break;
    default: break;
    }
  assert ( aux != NULL );
  g_key_file_set_string ( f, GROUP, name, aux );
  
} // end set_controller_value


void
write_conf_common (
        	   const conf_t *conf,
        	   const gchar  *name,
        	   const int     verbose
        	   )
{
  
  const gchar *val;
  GKeyFile *f;
  gboolean ret;
  GError *err;
  
  
  // Inicialitza.
  if ( verbose )
    fprintf ( stderr, "Escrivint configuració en '%s'\n", name );
  f= g_key_file_new ();
  
  // Tecles.
  set_key_value ( f, "key-pad1-up", conf->keys[0].up );
  set_key_value ( f, "key-pad1-down", conf->keys[0].down );
  set_key_value ( f, "key-pad1-left", conf->keys[0].left );
  set_key_value ( f, "key-pad1-right", conf->keys[0].right );
  set_key_value ( f, "key-pad1-button-triangle",
                  conf->keys[0].button_triangle );
  set_key_value ( f, "key-pad1-button-circle", conf->keys[0].button_circle );
  set_key_value ( f, "key-pad1-button-cross", conf->keys[0].button_cross );
  set_key_value ( f, "key-pad1-button-square", conf->keys[0].button_square );
  set_key_value ( f, "key-pad1-button-L2", conf->keys[0].button_L2 );
  set_key_value ( f, "key-pad1-button-R2", conf->keys[0].button_R2 );
  set_key_value ( f, "key-pad1-button-L1", conf->keys[0].button_L1 );
  set_key_value ( f, "key-pad1-button-R1", conf->keys[0].button_R1 );
  set_key_value ( f, "key-pad1-start", conf->keys[0].start );
  set_key_value ( f, "key-pad1-select", conf->keys[0].select );
  
  set_key_value ( f, "key-pad2-up", conf->keys[1].up );
  set_key_value ( f, "key-pad2-down", conf->keys[1].down );
  set_key_value ( f, "key-pad2-left", conf->keys[1].left );
  set_key_value ( f, "key-pad2-right", conf->keys[1].right );
  set_key_value ( f, "key-pad2-button-triangle",
                  conf->keys[1].button_triangle );
  set_key_value ( f, "key-pad2-button-circle", conf->keys[1].button_circle );
  set_key_value ( f, "key-pad2-button-cross", conf->keys[1].button_cross );
  set_key_value ( f, "key-pad2-button-square", conf->keys[1].button_square );
  set_key_value ( f, "key-pad2-button-L2", conf->keys[1].button_L2 );
  set_key_value ( f, "key-pad2-button-R2", conf->keys[1].button_R2 );
  set_key_value ( f, "key-pad2-button-L1", conf->keys[1].button_L1 );
  set_key_value ( f, "key-pad2-button-R1", conf->keys[1].button_R1 );
  set_key_value ( f, "key-pad2-start", conf->keys[1].start );
  set_key_value ( f, "key-pad2-select", conf->keys[1].select );
  
  // Grandària pantalla.
  switch ( conf->screen_size )
    {
    case SCREEN_SIZE_WIN_X1: val= "win_x1"; break;
    case SCREEN_SIZE_WIN_X1_5: val= "win_x1.5"; break;
    case SCREEN_SIZE_WIN_X2: val= "win_x2"; break;
    case SCREEN_SIZE_FULLSCREEN: val= "fullscreen"; break;
    default: val= NULL;
    }
  assert ( val != NULL );
  if ( val != NULL ) g_key_file_set_string ( f, GROUP, "screen-size", val );
  
  // Vsync.
  g_key_file_set_boolean ( f, GROUP, "vsync", conf->vsync );

  // Controllers.
  set_controller_value ( f, "pad1", conf->controllers[0] );
  set_controller_value ( f, "pad2", conf->controllers[1] );

  // Bios.
  if ( conf->bios_fn != NULL )
    g_key_file_set_string ( f, GROUP, "bios", conf->bios_fn );

  // Vsync.
  g_key_file_set_boolean ( f, GROUP, "tvres-is-pal", conf->tvres_is_pal );

  // Memorycard.
  if ( conf->memc_fn[0] != NULL )
    g_key_file_set_string ( f, GROUP, "memc1", conf->memc_fn[0] );
  if ( conf->memc_fn[1] != NULL )
    g_key_file_set_string ( f, GROUP, "memc2", conf->memc_fn[1] );
  
  // Escriu.
  err= NULL;
  ret= g_key_file_save_to_file ( f, name, &err );
  if ( !ret )
    error ( "no s'ha pogut escriure la configuració en '%s': %s",
            name, err->message );
  
  // Allibera.
  g_key_file_free ( f );
  
} // end write_conf_common




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
free_conf (
           conf_t *conf
           )
{
  
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  if ( conf->memc_fn[0] != NULL ) g_free ( conf->memc_fn[0] );
  if ( conf->memc_fn[1] != NULL ) g_free ( conf->memc_fn[1] );
  
} // end free_conf


void
get_conf (
          conf_t     *conf,
          const char *conf_fn, // Pot ser NULL.
          const int   verbose
          )
{
  
  const gchar *name;
  
  
  set_default_values ( conf );
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  if ( g_file_test ( name, G_FILE_TEST_IS_REGULAR ) ) // Existeix
    read_conf ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} // end get_conf


void
write_conf (
            const conf_t *conf,
            const char   *conf_fn,
            const int     verbose
            )
{
  
  const gchar *name;
  
  
  name= conf_fn==NULL ? get_default_conf_file_name () : conf_fn;
  write_conf_common ( conf, name, verbose );
  if ( conf_fn == NULL ) g_free ( (void *) name );
  
} // end write_conf


void
conf_set_bios_fn (
                  conf_t      *conf,
                  const gchar *bios_fn
                  )
{
  
  if ( conf->bios_fn != NULL ) g_free ( conf->bios_fn );
  conf->bios_fn= bios_fn==NULL ? NULL : g_strdup ( bios_fn );
  
} // end conf_set_bios_fn


void
conf_set_memc_fn (
                  conf_t      *conf,
                  const int    ind,
                  const gchar *memc_fn
                  )
{
  
  if ( conf->memc_fn[ind] != NULL ) g_free ( conf->memc_fn[ind] );
  conf->memc_fn[ind]= memc_fn==NULL ? NULL : g_strdup ( memc_fn );
  
} // end conf_set_memc_fn
