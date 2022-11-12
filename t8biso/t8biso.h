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
 * along with adriagipas/memus.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
 *  t8biso.h - Tiles de 8 bits amb 2 colors, més finet que tiles8b i
 *             accepta gran part de la codificacií ISO-8859-1.
 *
 */

#ifndef __T8BISO_H__
#define __T8BISO_H__

#define T8BISO_FG_TRANS 0x1
#define T8BISO_BG_TRANS 0x2

void
close_t8biso (void);

void
init_t8biso (void);

/* Escriu assumint que la codificació de 'string' és ISO-8859-1. */
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
        	    );

/* Escriu assumint que la codificació de 'string' és UTF-8. */
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
        		 );


/* BANNER */

/* PRIVAT */
typedef struct
{

  int *fb;
  int  fb_size;     /* En amplaria en tiles. */
  int  msg_width;   /* Amplària en píxels del missatge. */
  int  width;       /* Amplària en píxels del banner. */
  int  pos;         /* posició del següent píxel. */
  int  state;
  int  counter;     /* Conta pasos. */
  
} t8biso_banner_t;

/* Sols esborra la memòria interna. */
void
t8biso_banner_free (
        	    const t8biso_banner_t *banner
        	    );

/* Inicilialitza la memòria. */
void
t8biso_banner_init (
        	    t8biso_banner_t *banner
        	    );

void
t8biso_banner_set_msg (
        	       t8biso_banner_t *banner,
        	       const char      *msg,
        	       const int        width /* En píxels. */
        	       );

/* Assumix que cap en el fb. Cada vegada que es crida a aquesta funció
   actualitza l'estat del banner. Està dissenyat per a ser cridat cada
   20ms. */
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
        	    );

/* Pausa el banner, posicionant-lo al principi. No afecta als que no
   tenen moviment. */
void
t8biso_banner_pause (
        	     t8biso_banner_t *banner
        	     );

/* Torna a ficar en moviment al banner. No afecta als que no tenen
   moviment. */
void
t8biso_banner_resume (
        	      t8biso_banner_t *banner
        	      );

#endif /* __T8BISO_H__ */
