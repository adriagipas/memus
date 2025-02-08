# Endianisme
if host_machine.endian()=='little'
   ENDFLAGS='-D__LITTLE_ENDIAN__'
else
   ENDFLAGS='-D__BIG_ENDIAN__'
endif

MEMUPC_FILES= files('main.c',
                    'cmos.h',
                    'cmos.c',
                    'conf.h',
                    'conf.c',
                    'dirs.h',
                    'dirs.c',
                    'fchooser.h',
                    'fchooser.c',
                    'frontend.h',
                    'frontend.c',
                    'icon.h',
                    'load_bios.h',
                    'load_bios.c',
                    'load_hdd.h',
                    'load_hdd.c',
                    'load_vgabios.h',
                    'load_vgabios.c',
                    'lock.h',
                    'lock.c',
                    'menu.h',
                    'menu.c',
                    'mpad.h',
                    'mpad.c',
                    'screen.h',
                    'screen.c',
                    'session.h',
                    'session.c',
                    'set_disc.h',
                    'set_disc.c',
                    'sound.h',
                    'sound.c')

MEMUPC= executable('memupc',
                   MEMUPC_FILES,
                   dependencies : [GLIB2,SDL2,SDL2_IMG,DBUS],
                   include_directories : [COMMON_H,TILES8B_H,TILES16B_H,
                                          T8BISO_H,PC_H,CD_H,IA32_H],
                   link_with : [COMMON,TILES8B,TILES16B,T8BISO,PC,CD,IA32],
                   link_args : '-lm',
                   c_args : ENDFLAGS,
                   install : true)
