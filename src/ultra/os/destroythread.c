/* libultra src/os/destroythread.c, built from decompals/ultralib (lib/ultralib). */
/* The game's osDestroyThread walks the active queue the older way, which */
/* ultralib keeps for GCC builds: defining __GNUC__ after the headers selects it. */
#include "PR/os_internal.h"
#include "PRinternal/osint.h"
#define __GNUC__ 1
#include "src/os/destroythread.c"
