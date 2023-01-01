# Endianisme
if host_machine.endian()=='little'
   ENDFLAGS='-D__LITTLE_ENDIAN__'
else
   ENDFLAGS='-D__BIG_ENDIAN__'
endif

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

CD_H= include_directories('CD/src')
CD= static_library('CD',
                   'CD/src/CD.h',
                   'CD/src/crc.c',
                   'CD/src/crc.h',
                   'CD/src/cue.c',
                   'CD/src/cue.h',
                   'CD/src/info.c',
                   'CD/src/new.c',
                   'CD/src/utils.c',
                   'CD/src/utils.h',
                   include_directories : [CD_H])

PSX_H= include_directories('PSX/src')
PSX= static_library('PSX',
                    'PSX/src/cd.c',
                    'PSX/src/cpu_interpreter_cop0.h',
                    'PSX/src/cpu_regs.c',
                    'PSX/src/gpu.c',
                    'PSX/src/joy.c',
                    'PSX/src/mem.c',
                    'PSX/src/stats_renderer.c',
                    'PSX/src/cpu_decode.c',
                    'PSX/src/cpu_interpreter_cop2.h',
                    'PSX/src/default_renderer.c',
                    'PSX/src/gte.c',
                    'PSX/src/main.c',
                    'PSX/src/PSX.h',
                    'PSX/src/timers.c',
                    'PSX/src/cpu_interpreter.c',
                    'PSX/src/cpu_interpreter_inst1.h',
                    'PSX/src/dma.c',
                    'PSX/src/int.c',
                    'PSX/src/mdec.c',
                    'PSX/src/spu.c',
                    c_args : ENDFLAGS,
                    include_directories : [PSX_H,CD_H])

MD_H= include_directories('MD/src')
MD= static_library('MD',
                   'MD/src/audio.c',
                   'MD/src/cpu_dis.c',
                   'MD/src/fm.c',
                   'MD/src/main.c',
                   'MD/src/mem.c',
                   'MD/src/rom.c',
                   'MD/src/unpack.c',
                   'MD/src/vdp.c',
                   'MD/src/cpu.c',
                   'MD/src/eeprom.c',
                   'MD/src/io.c',
                   'MD/src/MD.h',
                   'MD/src/psg.c',
                   'MD/src/svp.c',
                   'MD/src/unpack.h',
                   'MD/src/z80.c',
                   c_args : ENDFLAGS,
                   include_directories : [MD_H,Z80_H])
