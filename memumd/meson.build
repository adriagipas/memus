# Endianisme
if host_machine.endian()=='little'
   ENDFLAGS='-D__LITTLE_ENDIAN__'
else
   ENDFLAGS='-D__BIG_ENDIAN__'
endif

MEMUMD_FILES= files('background.c',
                    'conf.c',
                    'dirs.c',
                    'eeprom.c',
                    'effects.c',
                    'frontend.c',
                    'hud.c',
                    'icon.h',
                    'mainmenu.c',
                    'menu.c',
                    'model.c',
                    'mpad.c',
                    'pad.c',
                    'rom.c',
                    'screen.c',
                    'sound.c',
                    'sram.c',
                    'state.c',
                    'background.h',
                    'conf.h',
                    'dirs.h',
                    'eeprom.h',
                    'effects.h',
                    'frontend.h',
                    'hud.h',
                    'main.c',
                    'mainmenu.h',
                    'menu.h',
                    'model.h',
                    'mpad.h',
                    'pad.h',
                    'rom.h',
                    'screen.h',
                    'sound.h',
                    'sram.h',
                    'state.h',
                    'aux.c')

MEMUMD= executable('memumd',
                   MEMUMD_FILES,
                   dependencies : [GLIB2,SDL2,SDL2_IMG],
                   include_directories : [COMMON_H,TILES16B_H,T8BISO_H,
                                          MD_H,Z80_H],
                   link_with : [COMMON,TILES16B,T8BISO,MD,Z80],
                   c_args : ENDFLAGS,
                   install : true)
