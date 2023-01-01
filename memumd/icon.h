/*
 * Copyright 2014-2022 Adrià Giménez Pastor.
 *
 * This file is part of adriagipas/memus.
 *
 * This file is a conversion of an original image from
 * http://www.ix.sakura.ne.jp/~yoshi
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
 *  icon.h - Una constant amb l'icona en format CARDINAL width,height,ARGB.
 *
 */

#ifndef __ICON_H__
#define __ICON_H__

#define ICON_SIZE (32*32+2)

static const int ICON[ICON_SIZE]=
  {
    32,
    32,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0xFF000000,
    0xFF525453,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,
    0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,
    0xFF262626,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,
    0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF43423E,0xFF000000,0x00FFFFFF,
    0xFF000000,0xFF525453,0xFF000000,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,
    0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,
    0xFF262626,0xFF262626,0xFF000000,0xFF000000,0xFF2C2E2D,0xFF2C2E2D,
    0xFF2C2E2D,0xFF2C2E2D,0xFF363635,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF3C3E3D,0xFF43423E,
    0xFF525453,0xFF0E0E0E,0xFF000000,0xFF000000,0xFF262626,0xFF262626,
    0xFF262626,0xFF262626,0xFF43423E,0xFF3C3E3D,0xFF43423E,0xFF3C3E3D,
    0xFF43423E,0xFF262626,0xFF262626,0xFF000000,0xFF2C2E2D,0xFF2C2E2D,
    0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF363635,
    0xFF363635,0xFF000000,0xFF000000,0xFF000000,0xFF43423E,0xFF3C3E3D,
    0xFF43423E,0xFF43423E,0xFF525453,0xFF0E0E0E,0xFF000000,0xFF323232,
    0xFF525453,0xFF3C3E3D,0xFF3C3E3D,0xFF262626,0xFF3C3E3D,0xFF43423E,
    0xFF3C3E3D,0xFF43423E,0xFF262626,0xFF262626,0xFF000000,0xFF2C2E2D,
    0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF363635,
    0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF000000,0xFF000000,
    0xFF000000,0xFF43423E,0xFF3C3E3D,0xFF43423E,0xFF525453,0xFF0E0E0E,
    0xFF000000,0xFF525453,0xFF0E0E0E,0xFF262626,0xFF262626,0xFF262626,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF262626,0xFF000000,
    0xFF2C2E2D,0xFF363635,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF262626,
    0xFF363635,0xFF000000,0xFF000000,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF525453,0xFF0E0E0E,0xFF000000,0xFF0E0E0E,0xFF525453,0xFF3C3E3D,
    0xFF3C3E3D,0xFF262626,0xFF43423E,0xFF43423E,0xFF43423E,0xFF262626,
    0xFF262626,0xFF000000,0xFF363635,0xFF000000,0xFF262626,0xFF262626,
    0xFF262626,0xFF262626,0xFF262626,0xFF262626,0xFF262626,0xFF262626,
    0xFF262626,0xFF000000,0xFF363635,0xFF000000,0xFF000000,0xFF000000,
    0xFF43423E,0xFF43423E,0xFF525453,0xFF0E0E0E,0xFF000000,0xFF525453,
    0xFF0E0E0E,0xFF262626,0xFF262626,0xFF262626,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF262626,0xFF000000,0xFF363635,0xFF363635,0xFF363635,
    0xFF1A1A1A,0xFF1A1A1A,0xFF1A1A1A,0xFF1A1A1A,0xFF1A1A1A,0xFF1A1A1A,
    0xFF1A1A1A,0xFF1A1A1A,0xFF1A1A1A,0xFF262626,0xFF363635,0xFF3E403F,
    0xFF2B312C,0xFF000000,0xFF43423E,0xFF43423E,0xFF525453,0xFF0E0E0E,
    0xFF000000,0xFF0E0E0E,0xFF525453,0xFF3C3E3D,0xFF3C3E3D,0xFF262626,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF262626,0xFF000000,0xFF3E403F,
    0xFF3E403F,0xFF3E403F,0xFF363635,0xFF363635,0xFF363635,0xFF363635,
    0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF363635,0xFF3E403F,
    0xFF3E403F,0xFF4D4F4E,0xFF2B312C,0xFF000000,0xFF43423E,0xFF43423E,
    0xFF525453,0xFF0E0E0E,0xFF000000,0xFF525453,0xFF0E0E0E,0xFF262626,
    0xFF262626,0xFF262626,0xFF43423E,0xFF43423E,0xFF43423E,0xFF262626,
    0xFF000000,0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF3E403F,
    0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF3E403F,
    0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF4D4F4E,0xFF2B312C,0xFF000000,
    0xFF43423E,0xFF43423E,0xFF525453,0xFF000000,0xFF000000,0xFF0E0E0E,
    0xFF525453,0xFF3C3E3D,0xFF3C3E3D,0xFF262626,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF262626,0xFF000000,0xFF000000,0xFF3E403F,0xFF3E403F,
    0xFF3E403F,0xFF3E403F,0xFF1A1A1A,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,
    0xFF1A1A1A,0xFF0E0E0E,0xFF3E403F,0xFF3E403F,0xFF4D4F4E,0xFF2B312C,
    0xFF2B312C,0xFF000000,0xFF43423E,0xFF43423E,0xFF525453,0xFF000000,
    0xFF000000,0xFF525453,0xFF0E0E0E,0xFF262626,0xFF262626,0xFF262626,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF000000,0xFF000000,
    0xFF4D4F4E,0xFF3E403F,0xFF3E403F,0xFF3E403F,0xFF0E0E0E,0xFFD8CAA0,
    0xFF887966,0xFFD8CAA0,0xFF887966,0xFF0E0E0E,0xFF3E403F,0xFF4D4F4E,
    0xFF4D4F4E,0xFF2B312C,0xFF000000,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF525453,0xFF000000,0xFF000000,0xFF0E0E0E,0xFF525453,0xFF3C3E3D,
    0xFF3C3E3D,0xFF262626,0xFF525453,0xFF43423E,0xFF525453,0xFF43423E,
    0xFF262626,0xFF000000,0xFF000000,0xFF4D4F4E,0xFF4D4F4E,0xFF3E403F,
    0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,
    0xFF4D4F4E,0xFF4D4F4E,0xFF2B312C,0xFF000000,0xFF262626,0xFF43423E,
    0xFF525453,0xFF43423E,0xFF525453,0xFF000000,0xFF000000,0xFF525453,
    0xFF323232,0xFF262626,0xFF262626,0xFF262626,0xFF43423E,0xFF525453,
    0xFF43423E,0xFF525453,0xFF43423E,0xFF2B312C,0xFF000000,0xFF000000,
    0xFF4D4F4E,0xFF4D4F4E,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,
    0xFF0E0E0E,0xFF0E0E0E,0xFF4D4F4E,0xFF2B312C,0xFF2B312C,0xFF000000,
    0xFF43423E,0xFF525453,0xFF43423E,0xFF43423E,0xFF525453,0xFF000000,
    0xFF000000,0xFF323232,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF2B312C,0xFF2B312C,0xFF000000,0xFF000000,0xFFAE5C6B,0xFF0E0E0E,
    0xFF0E0E0E,0xFF0E0E0E,0xFF0E0E0E,0xFFAE5C6B,0xFF2B312C,0xFF2B312C,
    0xFF000000,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF000000,0xFF000000,0xFF000000,0xFF505050,0xFF000000,
    0xFF000000,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF505050,0xFF505050,0xFF525453,0xFF2B312C,
    0xFF865356,0xFF865356,0xFF865356,0xFFF9B8C7,0xFF865356,0xFF865356,
    0xFF000000,0xFF525453,0xFF505050,0xFF262626,0xFF1A1A1A,0xFF1A1A1A,
    0xFF0E0E0E,0xFF0E0E0E,0xFF262626,0xFF000000,0xFF000000,0xFF323232,
    0xFF000000,0xFF565656,0xFF565656,0xFF565656,0xFF565656,0xFF505050,
    0xFF963100,0xFF505050,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,
    0xFF302E2C,0xFF505050,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF505050,0xFF262626,0xFF262626,0xFF262626,
    0xFF262626,0xFF262626,0xFF262626,0xFF262626,0xFF43423E,0xFF000000,
    0xFF000000,0xFF323232,0xFF43423E,0xFF262626,0xFF505050,0xFF565656,
    0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF302E2C,0xFF302E2C,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF525453,0xFF000000,0xFF000000,0xFF323232,0xFF43423E,0xFF262626,
    0xFF262626,0xFF626262,0xFF302E2C,0xFF93B8DD,0xFF93B8DD,0xFF93B8DD,
    0xFF302E2C,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF000000,0xFF000000,0xFF323232,
    0xFF43423E,0xFF0E0E0E,0xFF0E0E0E,0xFF626262,0xFF505050,0xFF505050,
    0xFF505050,0xFF505050,0xFF323232,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,
    0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF525453,0xFF000000,
    0xFF000000,0xFF323232,0xFF43423E,0xFF6D7170,0xFF6D7170,0xFF6D7170,
    0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,
    0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,
    0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF6D7170,0xFF9E9E9E,0xFF9E9E9E,
    0xFF9E9E9E,0xFF6D7170,0xFFF2F2F2,0xFF6D7170,0xFFF2F2F2,0xFF6D7170,
    0xFF525453,0xFF000000,0xFF000000,0xFF323232,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,0xFF43423E,
    0xFF43423E,0xFF43423E,0xFF43423E,0xFF000000,0xFF000000,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF503200,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF302E2C,
    0xFF302E2C,0xFF302E2C,0xFF43423E,0xFF43423E,0xFF302E2C,0xFF302E2C,
    0xFF43423E,0xFF43423E,0xFF302E2C,0xFF302E2C,0xFF302E2C,0xFF000000,
    0x00FFFFFF,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,0xFF000000,
    0xFF000000,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,
    0x00FFFFFF,0x00FFFFFF,0x00FFFFFF,0x00FFFFFF
  };

#endif /* __ICON_H__ */
