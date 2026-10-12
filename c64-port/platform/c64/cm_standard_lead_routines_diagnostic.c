/* SPDX-License-Identifier: MIT */
/* cm-standard-lead-transfer on the C64: every standard_lead stage input
 * entered through the routine registry, and every other E/BKCALL and E/CALL
 * round trip through SWCALL/INTPRET, native callees and back to the caller. No rope
 * reader is linked. Digests must equal the host run, which
 * test-cm-standard-lead-transfer checks against the certified stages.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_standard_lead_routines_cases.h"
#include "cm-standard-lead-transfer-vectors.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t n,failures=0,checks=0,steps;cm_native_io io;enum exec_status status;
 for(n=0;n<CM_SLR_TARGET_CASES;++n){uint16_t k=cm_slr_expected[n].n;steps=0;status=cm_slr_run(&image,&io,k,cm_slr_expected[n].stop,k<3072u?1u:16u,&steps);++checks;if(cm_slr_digest(status,steps,&image,&io)!=cm_slr_expected[n].digest)++failures;}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
