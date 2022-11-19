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

NES_H= include_directories('NES/src')
NES= static_library('NES',
                    'NES/src/apu.c',
                    'NES/src/cpu_dis.c',
                    'NES/src/joypads.c',
                    'NES/src/mapper.c',
                    'NES/src/mem.c',
                    'NES/src/op.h',
                    'NES/src/ppu.c',
                    'NES/src/cpu.c',
                    'NES/src/dma.c',
                    'NES/src/main.c',
                    'NES/src/mapper_names.c',
                    'NES/src/NES.h',
                    'NES/src/palette.c',
                    'NES/src/rom.c',
                    'NES/src/mappers/aorom.c',
                    'NES/src/mappers/cnrom.c',
                    'NES/src/mappers/mmc1.c',
                    'NES/src/mappers/mmc2.c',
                    'NES/src/mappers/mmc3.c',
                    'NES/src/mappers/nrom.c',
                    'NES/src/mappers/unrom.c',
                    'NES/src/mappers/aorom.h',
                    'NES/src/mappers/cnrom.h',
                    'NES/src/mappers/mmc1.h',
                    'NES/src/mappers/mmc2.h',
                    'NES/src/mappers/mmc3.h',
                    'NES/src/mappers/nrom.h',
                    'NES/src/mappers/unrom.h',
                    include_directories : [NES_H])

MIX_H= include_directories('MIX/src')
MIX= static_library('MIX',
                    'MIX/src/MIX.h',
                    'MIX/src/mix.c',
                    include_directories : [MIX_H])
