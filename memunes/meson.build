MEMUNES_FILES= files('background.c',
                     'conf.c',
                     'dirs.c',
                     'effects.c',
                     'frontend.c',
                     'hud.c',
                     'lock.c',
                     'icon.h',
                     'mainmenu.c',
                     'menu.c',
                     'mpad.c',
                     'pad.c',
                     'rom.c',
                     'screen.c',
                     'session.c',
                     'sound.c',
                     'sram.c',
                     'state.c',
                     'suspend.c',
                     'tvmode.c',
                     'background.h',
                     'conf.h',
                     'dirs.h',
                     'effects.h',
                     'frontend.h',
                     'hud.h',
                     'lock.h',
                     'main.c',
                     'mainmenu.h',
                     'menu.h',
                     'mpad.h',
                     'pad.h',
                     'rom.h',
                     'screen.h',
                     'session.h',
                     'sound.h',
                     'sram.h',
                     'state.h',
                     'suspend.h',
                     'tvmode.h')

MEMUNES= executable('memunes',
                    MEMUNES_FILES,
                    dependencies : [GLIB2,SDL2,SDL2_IMG,DBUS],
                    include_directories : [COMMON_H,TILES8B_H,T8BISO_H,NES_H],
                    link_with : [COMMON,TILES8B,T8BISO,NES],
                    install : true)
