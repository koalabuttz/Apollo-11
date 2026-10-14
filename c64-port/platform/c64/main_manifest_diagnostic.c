/* SPDX-License-Identifier: MIT */
/* main-assembly on the C64: both MAIN.agc manifests (src/main_manifest.c)
 * against the ropes resident in the REU (build/self-check.reu,
 * include/rope_image.h).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "main_manifest.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0;unsigned m;
 for(m=0;m<2u;++m)failures=(uint16_t)(failures+main_manifest_check(m,&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
