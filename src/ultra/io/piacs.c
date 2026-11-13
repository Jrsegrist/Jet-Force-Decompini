/* libultra src/io/piacs.c, built from decompals/ultralib (lib/ultralib). */
/* The game's object has __osPiAccessQueue ahead of piAccessBuf in .bss: */
/* declaring the queue first gives that order. */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
OSMesgQueue __osPiAccessQueue ALIGNED(0x8);
#include "src/io/piacs.c"
