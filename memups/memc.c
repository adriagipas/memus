/*
 * Copyright 2020-2022 Adrià Giménez Pastor.
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
 *  memc.c - Implementació de 'memc.h'.
 *
 */

#include <assert.h>
#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cd.h"
#include "conf.h"
#include "dirs.h"
#include "error.h"
#include "fchooser.h"
#include "memc.h"

#include "PSX.h"




/**********/
/* MACROS */
/**********/

#define MEMCARD_SIZE (128*1024)




/*********/
/* TIPUS */
/*********/

typedef enum
  {
   STAT_OK= 0,
   STAT_ERR_READ, // Algun error durant la lectura
   STAT_ERR_SIZE
  } status_t;




/*********/
/* ESTAT */
/*********/

// Configuració.
static conf_t *_conf;

// memoria.
static uint8_t _mc[2][MEMCARD_SIZE];

// Verbositat.
static bool _verbose;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static gchar *
get_tmp_file_name (void)
{
  
  GString *buffer;
  gchar *aux;
  
  
  aux= g_build_filename ( get_sharedir (), "memups", NULL );
  buffer= g_string_new ( aux );
  g_free ( aux );
  g_string_append ( buffer, ".tmp_memc_ps1" );
  
  return g_string_free ( buffer, FALSE );
  
} // end get_tmp_file_name


static status_t
read_memc (
           const int   ind,
           const char *fn
           )
{

  FILE *f;
  long size;

  
  if ( _verbose )
    fprintf ( stderr, "Carregant la memory card en %d de '%s'\n",
              ind+1, fn );
  
  f= fopen ( fn, "rb" );
  if ( f == NULL ) goto error_read;
  if ( fseek ( f, 0, SEEK_END ) == -1 ) goto error_read;
  size= ftell ( f );
  if ( size == -1 ) goto error_read;
  if ( size != MEMCARD_SIZE ) goto error_size;
  rewind ( f );
  if ( fread ( _mc[ind], MEMCARD_SIZE, 1, f ) != 1 ) goto error_read;
  fclose ( f );
  
  return STAT_OK;

 error_read:
  if ( f != NULL ) fclose ( f );
  return STAT_ERR_READ;
 error_size:
  if ( f != NULL ) fclose ( f );
  return STAT_ERR_SIZE;
  
} // end read_memc


// No passa res si no hi ha memc
static void
write_memc (
            const int ind
            )
{

  FILE *fd;
  const char *name;
  char *tmp;

  
  if ( _conf->memc_fn[ind] == NULL ) return;
  
  // Obté el nom.
  name= _conf->memc_fn[ind];
  if ( _verbose )
    fprintf ( stderr, "Escrivint la memòria de la memory card de %d en '%s'\n",
              ind+1, name );
  
  // Escriu.
  tmp= get_tmp_file_name ();
  if ( g_file_test ( tmp, G_FILE_TEST_IS_REGULAR ) )
    error ( "el fitxer temporal '%s' ja existeix", tmp );
  fd= fopen ( tmp, "wb" );
  if ( fd == NULL )
    error ( "no s'ha pogut crear el fitxer temporal '%s': %s",
            tmp, strerror ( errno ) );
  if ( fwrite ( _mc[ind], MEMCARD_SIZE, 1, fd ) != 1 )
    {
      fclose ( fd );
      remove ( tmp );
      error ( "no s'ha pogut escriure la memory card: %s", strerror ( errno ) );
    }
  if ( fclose ( fd ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut acabar d'escriure la memory card: %s",
              strerror ( errno ) );
    }
  if ( rename ( tmp, name ) != 0 )
    {
      remove ( tmp );
      error ( "no s'ha pogut escriure la memory card en '%s': %s",
              name, strerror ( errno ) );
    }
  
  // Allibera memòria.
  g_free ( tmp );
  
} // end write_memc




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_memc (void)
{

  write_memc ( 0 );
  write_memc ( 1 );
  
} // end close_memc


void
init_memc (
           conf_t    *conf,
           const int  verbose
           )
{

  int i;
  status_t ret;
  
  
  // Inicialitza.
  _conf= conf;
  _verbose= verbose;

  // Intenta llegir els memorycard de la bios.
  for ( i= 0; i < 2; ++i )
    if ( conf->memc_fn[i] != NULL )
      {
        ret= read_memc ( i, conf->memc_fn[i] );
        switch ( ret )
          {
          case STAT_ERR_READ:
            warning ( "no s'ha pogut llegir correctament '%s'",
                      conf->memc_fn[i] );
            break;
          case STAT_ERR_SIZE:
            warning ( "'%s' no té la grandària adecuada per ser"
                      " un fitxer de memory card",
                      conf->memc_fn[i] );
            break;
          default: break;
          }
        if ( ret != STAT_OK )
          {
            conf_set_memc_fn ( conf, i, NULL );
            warning ( "s'ha esborrat la memory card %d del"
                      " fitxer de configuració", i+1 );
          }
      }
  
} // end init_memc


bool
memc_set (
          const int  port, // 0 o 1
          bool      *quit
          )
{

  status_t st;
  bool ret,stop;
  const char *fn;
  uint8_t copy[MEMCARD_SIZE];
  
  
  init_fchooser ( "memc", "INSERTA MEMORY CARD",
                  "[.](sav|mcr|mcd)$", true, _verbose );
  *quit= false;
  ret= stop= false;
  while ( !stop )
    {
      fn= fchooser_run ( quit );
      if ( *quit || fn == NULL ) { ret= false; stop= true; }
      else if ( fn[0] == '\0' ) // Lleva el memory card
        {
          if ( _conf->memc_fn[port] != NULL )
            {
              if ( _verbose  )
                fprintf ( stderr, "Llevant el memory card del port %d\n",
                          port+1 );
              write_memc ( port );
            }
          conf_set_memc_fn ( _conf, port, NULL );
          memc_replug ();
          stop= true;
        }
      else
        {
          memcpy ( copy, _mc[port], MEMCARD_SIZE );
          st= read_memc ( port, fn );
          switch ( st )
            {
            case STAT_ERR_READ:
              warning ( "no s'ha pogut llegir correctament '%s'", fn );
              break;
            case STAT_ERR_SIZE:
              warning ( "'%s' no té la grandària adecuada per ser"
                        " un fitxer de memory card", fn );
              break;
            case STAT_OK:
              if ( _conf->memc_fn[port] != NULL )
                {
                  if ( _verbose  )
                    fprintf ( stderr, "Llevant el memory card del port %d\n",
                              port+1 );
                  write_memc ( port );
                }
              conf_set_memc_fn ( _conf, port, fn );
              stop= true;
              ret= true;
              break;
            default:
              printf("memc_set - WTF!!!\n");
            }
          if ( st!=STAT_OK )
            {
              memcpy ( _mc[port], copy, MEMCARD_SIZE );
              if ( fchooser_error_dialog () == -1 )
                *quit= stop= true;
            }
          memc_replug ();
        }
    }
  close_fchooser ( "memc" );
  
  return ret;
  
} // end memc_set


void
memc_replug (void)
{

  uint8_t *mem1,*mem2;

  
  mem1= _conf->memc_fn[0]!=NULL ? &(_mc[0][0]) : NULL;
  mem2= _conf->memc_fn[1]!=NULL ? &(_mc[1][0]) : NULL;
  PSX_plug_mem_cards ( mem1, mem2 );
  
} // memc_replug
