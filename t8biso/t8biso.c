/*
 * Copyright 2015-2022 Adrià Giménez Pastor.
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
 * along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  t8biso.c - Implementació de 't8biso.h'.
 *
 */


#include <assert.h>
#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "t8biso.h"




/*************/
/* CONSTANTS */
/*************/

static const unsigned char TILES[113][8]=
  {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x10,0x38,0x38,0x38,0x10,0x10,0x00,0x10},
    {0x30,0x68,0x68,0x32,0x5A,0xCC,0x7A,0x00},
    {0x20,0x20,0x40,0x00,0x00,0x00,0x00,0x00},
    {0x06,0x08,0x10,0x10,0x10,0x08,0x06,0x00},
    {0xC0,0x20,0x10,0x10,0x10,0x20,0xC0,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x40},
    {0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00},
    {0x00,0x02,0x04,0x08,0x10,0x20,0x40,0x00},
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00},
    {0x08,0x18,0x28,0x08,0x08,0x08,0x3E,0x00},
    {0x3C,0x42,0x02,0x0C,0x30,0x40,0x7E,0x00},
    {0x3C,0x42,0x02,0x1C,0x02,0x42,0x3C,0x00},
    {0x0C,0x14,0x24,0x44,0x44,0x7E,0x04,0x00},
    {0x7E,0x40,0x7C,0x02,0x02,0x42,0x3C,0x00},
    {0x3C,0x40,0x40,0x7C,0x42,0x42,0x3C,0x00},
    {0x7E,0x02,0x04,0x08,0x10,0x10,0x10,0x00},
    {0x3C,0x42,0x42,0x3C,0x42,0x42,0x3C,0x00},
    {0x3C,0x42,0x42,0x3E,0x02,0x02,0x3C,0x00},
    {0x00,0x00,0x10,0x00,0x00,0x10,0x00,0x00},
    {0x00,0x00,0x10,0x00,0x00,0x10,0x20,0x10},
    {0x7C,0xC6,0xC6,0x1C,0x10,0x10,0x00,0x10},
    {0x10,0x28,0x28,0x44,0x7C,0x82,0x82,0x00},
    {0xF8,0x84,0x84,0xFC,0x82,0x82,0xFC,0x00},
    {0x3C,0x42,0x80,0x80,0x80,0x42,0x3C,0x00},
    {0xF8,0x84,0x82,0x82,0x82,0x84,0xF8,0x00},
    {0xFE,0x80,0x80,0xFC,0x80,0x80,0xFE,0x00},
    {0xFE,0x80,0x80,0xFC,0x80,0x80,0x80,0x00},
    {0x3C,0x42,0x80,0x9E,0x82,0x42,0x3C,0x00},
    {0x82,0x82,0x82,0xFE,0x82,0x82,0x82,0x00},
    {0x7C,0x10,0x10,0x10,0x10,0x10,0x7C,0x00},
    {0x7E,0x08,0x08,0x08,0x88,0x88,0x70,0x00},
    {0x84,0x88,0x90,0xB0,0xC8,0x84,0x82,0x00},
    {0x80,0x80,0x80,0x80,0x80,0x80,0xFE,0x00},
    {0x82,0xC6,0xAA,0x92,0x82,0x82,0x82,0x00},
    {0x82,0xC2,0xA2,0x92,0x8A,0x86,0x82,0x00},
    {0x38,0x44,0x82,0x82,0x82,0x44,0x38,0x00},
    {0xFC,0x82,0x82,0xFC,0x80,0x80,0x80,0x00},
    {0x38,0x44,0x82,0x82,0x8A,0x44,0x3A,0x00},
    {0xFC,0x82,0x82,0xFC,0x88,0x84,0x82,0x00},
    {0x78,0x84,0x80,0x7C,0x02,0x82,0x7C,0x00},
    {0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x00},
    {0x82,0x82,0x82,0x82,0x82,0x42,0x3E,0x00},
    {0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x00},
    {0x82,0x92,0xAA,0xAA,0xC6,0xC6,0x82,0x00},
    {0xC6,0x44,0x28,0x10,0x28,0x44,0xC6,0x00},
    {0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x00},
    {0xFE,0x04,0x08,0x10,0x20,0x40,0xFE,0x00},
    {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E,0x00},
    {0xE0,0x20,0x20,0x20,0x20,0x20,0xE0,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00},
    {0x00,0x00,0x38,0x04,0x3C,0x44,0x3E,0x00},
    {0x40,0x40,0x40,0x7C,0x42,0x42,0x7C,0x00},
    {0x00,0x00,0x3C,0x42,0x40,0x42,0x3C,0x00},
    {0x02,0x02,0x02,0x3E,0x42,0x42,0x3E,0x00},
    {0x00,0x00,0x3C,0x42,0x7E,0x40,0x3E,0x00},
    {0x0C,0x12,0x10,0x7E,0x10,0x10,0x10,0x00},
    {0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x7C},
    {0x40,0x40,0x40,0x78,0x44,0x44,0x44,0x00},
    {0x00,0x10,0x00,0x10,0x10,0x10,0x10,0x00},
    {0x00,0x08,0x00,0x08,0x08,0x08,0x08,0x30},
    {0x40,0x40,0x46,0x58,0x60,0x58,0x46,0x00},
    {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00},
    {0x00,0x00,0x68,0x54,0x54,0x54,0x54,0x00},
    {0x00,0x00,0x3C,0x22,0x22,0x22,0x22,0x00},
    {0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00},
    {0x00,0x00,0x7C,0x42,0x42,0x7C,0x40,0x40},
    {0x00,0x00,0x3E,0x42,0x42,0x3E,0x02,0x02},
    {0x00,0x00,0x4E,0x50,0x60,0x40,0x40,0x00},
    {0x00,0x00,0x3C,0x40,0x3C,0x02,0x7C,0x00},
    {0x00,0x10,0x7C,0x10,0x10,0x10,0x0C,0x00},
    {0x00,0x00,0x44,0x44,0x44,0x44,0x3C,0x00},
    {0x00,0x00,0x44,0x44,0x44,0x28,0x10,0x00},
    {0x00,0x00,0x82,0x92,0x92,0xAA,0x44,0x00},
    {0x00,0x00,0xC4,0x28,0x10,0x28,0x46,0x00},
    {0x00,0x00,0x44,0x44,0x44,0x3C,0x04,0x78},
    {0x00,0x00,0x7E,0x04,0x18,0x20,0x7E,0x00},
    {0x10,0x00,0x10,0x10,0x38,0x38,0x38,0x10},
    {0x00,0x30,0x48,0x48,0x30,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00},
    {0x10,0x00,0x10,0x10,0x70,0xC6,0xC6,0x7C},
    {0x10,0x08,0x10,0x28,0x44,0xFE,0x82,0x00},
    {0x10,0x20,0x10,0x28,0x44,0xFE,0x82,0x00},
    {0x28,0x10,0x28,0x44,0x7C,0x82,0x82,0x00},
    {0x3C,0x42,0x80,0x80,0x42,0x3C,0x08,0x10},
    {0x10,0x08,0xFE,0x80,0xFC,0x80,0xFE,0x00},
    {0x10,0x20,0xFE,0x80,0xFC,0x80,0xFE,0x00},
    {0x10,0x08,0x7C,0x10,0x10,0x10,0x7C,0x00},
    {0x10,0x20,0x7C,0x10,0x10,0x10,0x7C,0x00},
    {0x32,0x4C,0xC2,0xA2,0x92,0x8A,0x86,0x00},
    {0x10,0x08,0x7C,0x82,0x82,0x82,0x7C,0x00},
    {0x10,0x20,0x7C,0x82,0x82,0x82,0x7C,0x00},
    {0x28,0x38,0x44,0x82,0x82,0x44,0x38,0x00},
    {0x00,0x00,0x44,0x28,0x10,0x28,0x44,0x00},
    {0x10,0x08,0x82,0x82,0x82,0x42,0x3E,0x00},
    {0x10,0x20,0x82,0x82,0x82,0x42,0x3E,0x00},
    {0x28,0x82,0x82,0x82,0x82,0x42,0x3E,0x00},
    {0x20,0x10,0x38,0x04,0x3C,0x44,0x3E,0x00},
    {0x08,0x10,0x38,0x04,0x3C,0x44,0x3E,0x00},
    {0x28,0x00,0x38,0x04,0x3C,0x44,0x3E,0x00},
    {0x00,0x00,0x3C,0x42,0x40,0x4A,0x3C,0x10},
    {0x10,0x08,0x3C,0x42,0x7E,0x40,0x3E,0x00},
    {0x08,0x10,0x3C,0x42,0x7E,0x40,0x3E,0x00},
    {0x20,0x10,0x00,0x10,0x10,0x10,0x10,0x00},
    {0x08,0x10,0x00,0x10,0x10,0x10,0x10,0x00},
    {0x32,0x4C,0x3C,0x22,0x22,0x22,0x22,0x00},
    {0x20,0x10,0x3C,0x42,0x42,0x42,0x3C,0x00},
    {0x08,0x10,0x3C,0x42,0x42,0x42,0x3C,0x00},
    {0x28,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00},
    {0x20,0x10,0x44,0x44,0x44,0x44,0x3C,0x00},
    {0x08,0x10,0x44,0x44,0x44,0x44,0x3C,0x00},
    {0x28,0x00,0x44,0x44,0x44,0x44,0x3C,0x00}
  };

static const unsigned char MAP[256]=
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 3, 4, 5, 0, 0, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0, 0, 0, 22, 0, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 0, 50, 0, 51, 0, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
    77, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 78, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 79, 0, 0, 0, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 81, 82,
    83, 0, 0, 84, 0, 0, 85, 86, 87, 0, 0, 88, 89, 0, 0, 0, 90, 91, 92, 0, 0,
    93, 94, 0, 95, 96, 0, 97, 0, 0, 0, 98, 99, 0, 0, 100, 0, 0, 101, 102, 103,
    0, 0, 104, 105, 0, 0, 0, 106, 107, 108, 0, 0, 109, 0, 0, 110, 111, 0, 112,
    0, 0, 0
  };



/*********/
/* TIPUS */
/*********/

enum {
  BANNER_FITS,
  BANNER_BEGIN,
  BANNER_MOVING,
  BANNER_END,
  BANNER_PAUSED
};




/*********/
/* ESTAT */
/*********/

/* Per a convertir. */
static struct
{
  
  GIConv  cd;
  char   *buf;
  size_t  bsize;
  
} _toiso;




/*********************/
/* FUNCIONS PRIVADES */
/*********************/

static void
close_toiso (void)
{

  g_iconv_close ( _toiso.cd );
  g_free ( _toiso.buf );
  
} /* end toiso */


static void
init_toiso (void)
{

  _toiso.cd= g_iconv_open ( "ISO-8859-1", "UTF-8" );
  if ( _toiso.cd == (GIConv) -1 )
    error ( "t8biso: no s'ha pogut crear el conversor de caràcters" );
  _toiso.buf= g_new ( char, 1 );
  _toiso.bsize= 1;
  
} /* end init_toiso */


static const char *
to_iso (
        const char *string
        )
{

  int len;
  gsize ileft, oleft, ret;
  char *itmp, *otmp;
  
  
  /* Obté longitut i prepara buffer. */
  len= strlen ( string );
  if ( _toiso.bsize < (len+1) )
    {
      _toiso.bsize= len+1;
      _toiso.buf= g_renew ( char, _toiso.buf, _toiso.bsize );
    }

  /* Fa la transformació. */
  ileft= oleft= len;
  itmp= (char *) string; otmp= &(_toiso.buf[0]);
  ret= g_iconv ( _toiso.cd, &itmp, &ileft, &otmp, &oleft );
  if ( ret == (gsize) -1 )
    {
      warning ( "t8biso: no s'ha pogut transformar a"
        	" ISO-8859-1 la cadena: '%s'", string );
      return string;
    }
  else
    {
      _toiso.buf[len-oleft]= '\0';
      return _toiso.buf;
    }
  
} /* end to_iso */


static void
draw_tile (
           int                 *fb,
           const int            fb_width,
           const unsigned char  tile,
           const int            x,
           const int            y,
           const int            fgcolor,
           const int            bgcolor,
           const int            flags
           )
{
  
  int offset, r, c;
  const unsigned char *pat;
  unsigned char byte;
  
  
  pat= TILES[tile];
  offset= y*8*fb_width + x*8;
  for ( r= 0; r < 8; ++r )
    {
      byte= pat[r];
      for ( c= 0; c < 8; ++c )
        {
          if ( byte&0x80 )
            {
              if ( !(flags&T8BISO_FG_TRANS) ) fb[offset+c]= fgcolor;
            }
          else
            {
              if ( !(flags&T8BISO_BG_TRANS) ) fb[offset+c]= bgcolor;
            }
          byte<<= 1;
        }
      offset+= fb_width;
    }
  
} /* end draw_tile */




/**********************/
/* FUNCIONS PÚBLIQUES */
/**********************/

void
close_t8biso (void)
{

  close_toiso ();
  
} /* end close_t8biso */


void
init_t8biso (void)
{

  init_toiso ();
  
} /* end init_t8biso */


void
t8biso_draw_string (
        	    int        *fb,
        	    const int   fb_width,
        	    const char *string,
        	    int         x,
        	    const int   y,
        	    const int   fgcolor,
        	    const int   bgcolor,
        	    const int   flags
        	    )
{
  
  const char *p;
  
  
  for ( p= string; *p; ++p, ++x )
    draw_tile ( fb, fb_width, MAP[(unsigned char) *p],
                x, y, fgcolor, bgcolor, flags );
  
} /* end t8biso_draw_string */


void
t8biso_draw_string_utf8 (
        		 int        *fb,
        		 const int   fb_width,
        		 const char *string,
        		 int         x,
        		 const int   y,
        		 const int   fgcolor,
        		 const int   bgcolor,
        		 const int   flags
        		 )
{
  t8biso_draw_string ( fb, fb_width, to_iso ( string ),
        	       x, y, fgcolor, bgcolor, flags );
} /* end t8biso_draw_string_utf8 */


void
t8biso_banner_free (
        	    const t8biso_banner_t *banner
        	    )
{
  g_free ( banner->fb );
} /* end t8biso_banner_free */


void
t8biso_banner_init (
        	    t8biso_banner_t *banner
        	    )
{

  banner->fb_size= 1;
  banner->fb= g_new ( int, 8*8*1 );
  
} /* end t8biso_banner_init */


void
t8biso_banner_set_msg (
        	       t8biso_banner_t *banner,
        	       const char      *msg,
        	       const int        width
        	       )
{

  const char *str;
  int len;
  

  assert ( width > 0 );
  
  /* Obté cadena en iso. */
  str=  to_iso ( msg );
  len= strlen ( str );
  
  /* Reserva memòria. */
  if ( len > banner->fb_size )
    {
      banner->fb= g_renew ( int, banner->fb, 8*8*len );
      banner->fb_size= len;
    }

  /* Inicialitza. */
  banner->msg_width= len*8;
  t8biso_draw_string ( banner->fb, banner->msg_width, str, 0, 0, 1, 0, 0 );
  banner->width= width;
  banner->pos= 0;
  banner->counter= 0;
  banner->state= banner->msg_width <= width ? BANNER_FITS : BANNER_BEGIN;
  
} /* end t8biso_banner_set_msg */


void
t8biso_banner_draw (
        	    t8biso_banner_t *banner,
        	    int             *fb,
        	    const int        fb_width,
        	    const int        x, /* píxels */
        	    const int        y, /* píxels */
        	    const int        fgcolor,
        	    const int        bgcolor,
        	    const int        flags
        	    )
{

  static const int DESP= 1;
  
  int r, c, end, color;
  const int *src, *q;
  int *dst, *p;


  /* Actualitza l'estat. */
  /* NOTA: Assumisc que cada pas són 20ms. */
  switch ( banner->state )
    {

    case BANNER_BEGIN:
      if ( ++(banner->counter) == 50 ) /* 1s */
        {
          banner->state= BANNER_MOVING;
          banner->pos= 0;
        }
      break;

    case BANNER_MOVING:
      banner->pos+= DESP;
      if ( banner->pos >= banner->msg_width )
        {
          banner->state= BANNER_END;
          banner->counter= 0;
        }
      break;

    case BANNER_END:
      if ( ++(banner->counter) == 25 ) /* <1s */
        {
          banner->state= BANNER_BEGIN;
          banner->pos= 0;
          banner->counter= 0;
        }
      break;
      
    case BANNER_FITS:
    case BANNER_PAUSED:
    default:
      break;
      
    }
  
  /* Dibuixa. */
  end= MIN ( banner->msg_width, banner->pos + banner->width );
  dst= fb + y*fb_width + x;
  src= banner->fb + banner->pos;
  for ( r= 0; r < 8; ++r )
    {
      for ( c= banner->pos, p= dst, q= src; c < end; ++c, ++p, ++q )
        {
          color= *q;
          if ( color )
            {
              if ( !(flags&T8BISO_FG_TRANS) ) *p= fgcolor;
            }
          else
            {
              if ( !(flags&T8BISO_BG_TRANS) ) *p= bgcolor;
            }
        }
      dst+= fb_width;
      src+= banner->msg_width;
    }
  
} /* end t8biso_banner_draw */


void
t8biso_banner_pause (
        	     t8biso_banner_t *banner
        	     )
{

  if ( banner->state == BANNER_FITS ) return;

  banner->state= BANNER_PAUSED;
  banner->pos= 0;
  banner->counter= 0;
  
} /* end t8biso_banner_pause */


void
t8biso_banner_resume (
        	      t8biso_banner_t *banner
        	      )
{

  if ( banner->state == BANNER_PAUSED )
    banner->state= BANNER_BEGIN;
  
} /* end t8biso_banner_resume */
