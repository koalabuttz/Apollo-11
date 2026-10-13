/* SPDX-License-Identifier: MIT */
/* resident-kernel size probe (tools/resident_kernel_size.py): links only the
 * kernel's routine tables, so the LTO + gc-sections link holds the kernel,
 * the instruction helpers it calls and the transfer-ABI operand reader. With
 * RK_EMPTY it links nothing, for the C runtime baseline. */
#include "resident_kernel.h"
const void *volatile keep[4];
int main(void){
#if RK_EMPTY
 keep[0]=0;
#elif RK_LM
 keep[0]=&lm_kernel_routines;keep[1]=&lm_kernel_job_routines;keep[2]=&lm_kernel_task_routines;keep[3]=&lm_kernel_idle_routines;
#else
 keep[0]=&cm_kernel_routines;keep[1]=&cm_kernel_job_routines;keep[2]=&cm_kernel_idle_routines;
#endif
 return 0;
}
