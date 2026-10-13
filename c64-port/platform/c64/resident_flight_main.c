/* SPDX-License-Identifier: MIT */
/* The ABI 2 flight resident's entry: boot, then resident steps until the
 * program reaches a point that neither the kernel nor the module store
 * resolves. */
#include "resident_flight.h"
int main(void) {
    if(!rk_boot()) while(rk_resident_step(AGC_TRANSFER_NONE)==EXEC_OK) {}
    for(;;) {}
}
