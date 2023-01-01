/*
 *  sram.h - Memòria estàtica.
 *
 *  Adrià Giménez Pastor, 2014.
 *
 */

#ifndef __SRAM_H__
#define __SRAM_H__

#include "MD.h"

void
close_sram (void);

void
init_sram (
           const char *rom_id,
           const char *sram_fn,
           const int   verbose
           );

MD_Word *
sram_get_static_ram (
                     const int  num_words,
                     void      *udata
                     );

#endif /* __SRAM_H__ */
