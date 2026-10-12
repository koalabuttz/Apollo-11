/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include "cm_rcs_dap_vectors.h"
unsigned cm_rcs_dap_check(void);
/* Mailbox contract (tools/vice_run.py): write $c001-$c004 first, then $a5 to $c000 last. */
int main(void) {
    volatile uint8_t *mail = (volatile uint8_t *)0xc000;
    unsigned failures = cm_rcs_dap_check(), checks = CM_RCS_DAP_FROZEN_ROWS;
    mail[1] = (uint8_t)failures; mail[2] = (uint8_t)(failures >> 8);
    mail[3] = (uint8_t)checks; mail[4] = (uint8_t)(checks >> 8);
    mail[0] = 0xa5;
    for (;;) {}
}
