Z80_H= include_directories('Z80/src')
Z80= static_library('Z80',
                    'Z80/src/Z80.h',
                    'Z80/src/z80.c',
                    'Z80/src/z80_dis.c',
                    include_directories : [Z80_H])

GG_H= include_directories('GG/src')
GG= static_library('GG',
                   'GG/src/control.c',
                   'GG/src/GG.h',
                   'GG/src/io.c',
                   'GG/src/main.c',
                   'GG/src/mem.c',
                   'GG/src/psg.c',
                   'GG/src/rom.c',
                   'GG/src/vdp.c',
                   'GG/src/z80_reti.c',
                    include_directories : [GG_H,Z80_H])

GBC_H= include_directories('GBC/src')
GBC= static_library('GBC',
                    'GBC/src/apu.c',
                    'GBC/src/cpu_dis.c',
                    'GBC/src/joypad.c',
                    'GBC/src/main.c',
                    'GBC/src/mem.c',
                    'GBC/src/timers.c',
                    'GBC/src/cpu.c',
                    'GBC/src/GBC.h',
                    'GBC/src/lcd.c',
                    'GBC/src/mapper.c',
                    'GBC/src/rom.c',
                    include_directories : [GBC_H])
