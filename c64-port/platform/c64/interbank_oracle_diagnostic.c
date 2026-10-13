/* SPDX-License-Identifier: MIT */
/* interbank-oracle on the C64: every third case of
 * tests/interbank_oracle_cases.c (sites and entries of both missions) and the
 * cover rows that take every host branch arc (tools/lane_a_c64_cover.py),
 * against frozen yaAGC digests, and the refusals. The image lives under the
 * KERNAL ($e000, interrupts off, $01=$35).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "interbank_oracle_check.h"
#include "interbank_oracle_cases.h"
#include "interbank_oracle_c64_vectors.h"
#include "interbank_oracle_c64_rows.h"
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<IBO_C64_COUNT;++n)failures=(uint16_t)(failures+ibo_check((uint16_t)(n*IBO_C64_STRIDE),ibo_c64[n],&checks));
 for(n=0;n<IBO_C64_ROWS;++n)failures=(uint16_t)(failures+ibo_check(ibo_c64_row[n],ibo_c64_row_digest[n],&checks));
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
