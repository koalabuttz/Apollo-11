/* SPDX-License-Identifier: MIT */
/* lm-lunar-landing on the C64: the Luminary099 THE_LUNAR_LANDING cases
 * chosen by tools/lane_f_c64_rows.py (every routine, outcome and path the
 * host cases take) through the routines, against digests frozen from the
 * original program (tools/lm_lunar_landing_freeze.py). Two PRGs
 * (LLD_C64_GROUP): group 2 runs the rows of DDUMCALC, P63SPOT2 and
 * the order pairs they reach, group 1 all others. The undo image lives at LLD_UNDO_ADDRESS, outside the
 * program.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_lunar_landing_cases.h"
#include "lm_lunar_landing_c64_vectors.h"
static int in_group(uint16_t n){uint16_t k=lld_routine_keys[n%LLD_ROUTINES];
 /* DDUMCALC (32,3102) and the order pairs src/lm_lunar_landing.c keeps
  * under LLD_G2 (32,3104-3143, 3163-3201, 3215-3222). */
 unsigned a=k-032u*1024u+02000u;
 int heavy=k/1024u==032u&&(a==03102u||(a>=03104u&&a<=03143u)||(a>=03163u&&a<=03201u)||(a>=03215u&&a<=03222u));
 return LLD_C64_GROUP==2?heavy:!heavy;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * image lives in the RAM under KERNAL at $e000 (the undo image at $f000). */
 apollo_overlay_state *image=(apollo_overlay_state *)0xe000u;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<LLD_C64_COUNT;++n){if(!in_group(lld_c64_case[n]))continue;
  ++checks;if(lld_digest(image,lld_c64_case[n])!=lld_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
