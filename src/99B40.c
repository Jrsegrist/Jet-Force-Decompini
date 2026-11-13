/*
 * libultra's RSP task loader (sptask), an earlier form than ultralib's: a
 * yielded task is not checked for a loadable microcode address. Written from
 * the assembly; built like the game's other 2.0I os/io objects (-O1 -mips2).
 */
#include "PR/os_internal.h"
#include "PR/sptask.h"
#include "PR/rcp.h"
#include "PRinternal/osint.h"

static OSTask tmp_task;

/* Copy the task and turn each of its buffer pointers into a physical address. */
static OSTask *_VirtualToPhysicalTask(OSTask *intp) {
    OSTask *tp = &tmp_task;

    bcopy(intp, tp, sizeof(OSTask));
    if (tp->t.ucode != NULL) {
        tp->t.ucode = (u64 *) osVirtualToPhysical(tp->t.ucode);
    }
    if (tp->t.ucode_data != NULL) {
        tp->t.ucode_data = (u64 *) osVirtualToPhysical(tp->t.ucode_data);
    }
    if (tp->t.dram_stack != NULL) {
        tp->t.dram_stack = (u64 *) osVirtualToPhysical(tp->t.dram_stack);
    }
    if (tp->t.output_buff != NULL) {
        tp->t.output_buff = (u64 *) osVirtualToPhysical(tp->t.output_buff);
    }
    if (tp->t.output_buff_size != NULL) {
        tp->t.output_buff_size = (u64 *) osVirtualToPhysical(tp->t.output_buff_size);
    }
    if (tp->t.data_ptr != NULL) {
        tp->t.data_ptr = (u64 *) osVirtualToPhysical(tp->t.data_ptr);
    }
    if (tp->t.yield_data_ptr != NULL) {
        tp->t.yield_data_ptr = (u64 *) osVirtualToPhysical(tp->t.yield_data_ptr);
    }
    return tp;
}

void osSpTaskLoad(OSTask *intp) {
    OSTask *tp = _VirtualToPhysicalTask(intp);

    if (tp->t.flags & OS_TASK_YIELDED) {
        tp->t.ucode_data = tp->t.yield_data_ptr;
        tp->t.ucode_data_size = tp->t.yield_data_size;
        intp->t.flags &= ~OS_TASK_YIELDED;
    }
    osWritebackDCache(tp, sizeof(OSTask));
    __osSpSetStatus(SP_CLR_YIELD | SP_CLR_YIELDED | SP_CLR_TASKDONE | SP_SET_INTR_BREAK);
    while (__osSpSetPc(SP_IMEM_START) == -1) {
    }
    while (__osSpRawStartDma(OS_WRITE, SP_IMEM_START - sizeof(OSTask), tp, sizeof(OSTask)) == -1) {
    }
    while (__osSpDeviceBusy()) {
    }
    while (__osSpRawStartDma(OS_WRITE, SP_IMEM_START, tp->t.ucode_boot, tp->t.ucode_boot_size) == -1) {
    }
}

void osSpTaskStartGo(OSTask *tp) {
    while (__osSpDeviceBusy()) {
    }
    __osSpSetStatus(SP_SET_INTR_BREAK | SP_CLR_SSTEP | SP_CLR_BROKE | SP_CLR_HALT);
}
