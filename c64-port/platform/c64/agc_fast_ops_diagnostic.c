/* SPDX-License-Identifier: MIT */
/* agc-fast-ops on the C64: tests/test_agc_fast_ops.c's checks with the
 * 6502 MP/DV cores, on the fixed flight image at APOLLO_SHARED_ADDRESS
 * (RK_FIXED_IMAGE), every fast helper against its agc_image_ops helper.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#define AFO_DIAG_CASES 2000ul
#define AFO_DIAG_MULDIV 20000ul
unsigned long afo_check(unsigned long cases,unsigned long muldiv);
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;
 unsigned long failures=afo_check(AFO_DIAG_CASES,AFO_DIAG_MULDIV);
 uint16_t f=failures>0xffffu?0xffffu:(uint16_t)failures,checks=(uint16_t)(AFO_DIAG_CASES+AFO_DIAG_MULDIV);
 mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
