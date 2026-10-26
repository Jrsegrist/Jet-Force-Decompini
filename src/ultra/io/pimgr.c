/* libultra src/io/pimgr.c, built from decompals/ultralib (lib/ultralib). */
/* The PI manager thread's stack is 0x180 bytes in this game's SDK. */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#undef OS_PIM_STACKSIZE
#define OS_PIM_STACKSIZE 0x180
#include "src/io/pimgr.c"
