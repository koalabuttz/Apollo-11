/* SPDX-License-Identifier: MIT */
/* lm-landing-displays on the C64: the Luminary099 LANDING_ANALOG_DISPLAYS cases chosen
 * by tools/lane_f_c64_rows.py (every entry, outcome and branch the host
 * cases take) through the trampoline,
 * against digests frozen from the original program
 * (tools/lm_landing_displays_freeze.py).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "lm_landing_displays_cases.h"
#include "lm_landing_displays_c64_vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 for(n=0;n<LM_LAD_C64_COUNT;++n){++checks;if(lm_lad_digest(&image,lm_lad_c64_case[n])!=lm_lad_c64[n])++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
