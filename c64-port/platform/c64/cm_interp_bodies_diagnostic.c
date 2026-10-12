/* SPDX-License-Identifier: MIT */
/* cm-interp-bodies on the C64: the frozen vector cases of the Comanche055
 * interpretive bodies INTERP_BODY_LO..INTERP_BODY_HI-1, each rebuilt from its
 * xorshift fill and edits, against the digest of the original program
 * (tests/cm_interp_vectors.h). The body ranges are separate programs so each
 * fits below the $c000 mailbox.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_interp_vectors.h"
#include "interp_vector_check.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n,edit=0;
 for(n=0;n<INTERP_VECTOR_CASES;++n){
  ++checks;if(!interp_vector_case(&image,n,&edit))++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
