# Endianisme
if host_machine.endian()=='little'
   ENDFLAGS='-D__LITTLE_ENDIAN__'
else
   ENDFLAGS='-D__BIG_ENDIAN__'
endif

MEMUPS_FILES= files('main.c',
                    'dirs.h',
                    'dirs.c',
                    'conf.h',
                    'conf.c',
                    'frontend.h',
                    'frontend.c',
                    'load_bios.h',
                    'load_bios.c',
                    'lock.h',
                    'lock.c',
                    'screen.h',
                    'screen.c',
                    'icon.h',
                    'sound.h',
                    'sound.c',
                    'pad.h',
                    'pad.c',
                    'mpad.h',
                    'mpad.c',
                    'menu.h',
                    'menu.c',
                    'fchooser.h',
                    'fchooser.c',
                    'cd.h',
                    'cd.c',
                    'memc.h',
                    'memc.c',
                    'session.h',
                    'session.c')

MEMUPS= executable('memups',
                   MEMUPS_FILES,
                   dependencies : [GLIB2,SDL2,SDL2_IMG,DBUS],
                   include_directories : [COMMON_H,TILES8B_H,TILES16B_H,
                                          T8BISO_H,PSX_H,CD_H],
                   link_with : [COMMON,TILES8B,TILES16B,T8BISO,PSX,CD],
                   link_args : '-lm',
                   c_args : ENDFLAGS,
                   install : true)
