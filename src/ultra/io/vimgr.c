/* libultra src/io/vimgr.c, built from decompals/ultralib (lib/ultralib). */
/* The VI manager thread's stack is 0x100 bytes in this game's SDK. */
#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#undef OS_VIM_STACKSIZE
#define OS_VIM_STACKSIZE 0x100
#include "src/io/vimgr.c"
