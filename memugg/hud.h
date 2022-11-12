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
 *  hud.h - HUD sobre el framebuffer.
 *
 */

#ifndef __HUD_H__
#define __HUD_H__

void
init_hud (void);

/* Torna un fb actualitzat amb el HUD. Si no hi ha res que dibuixar
 * torna el mateix. S'ha de cridar en cada frame.
 */
const int *
hud_update_fb (
               const int *fb
               );

/* Mostra una barra dalt amb un missatge d'una línia gastant
 * tiles. Una nova crida a aquesta funció mostra un nou
 * missatge. L'amplaria màxima és 19 caràcters. Si no es produeix un
 * error greu.
 */
void
hud_show_msg (
              const char *msg
              );

/* Efecte de flash. */
void
hud_flash (void);

/* Oculta tots els elements del hud. */
void
hud_hide (void);

#endif /* __HUD_H__ */
