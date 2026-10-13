/* SPDX-License-Identifier: MIT */
/* waitlist-entries on the C64, one program per kind (WLE_ONLY = 0..4): every
 * fourth case of the kind, and its first two, and the kind's cover rows
 * (tests/waitlist_entries_c64_rows.h: with those the rows take every branch
 * arc the host cases take), against the frozen yaAGC digests, with the refusal. Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "waitlist_entries_check.h"
#include "waitlist_entries_cases.h"
#include "waitlist_entries_c64_rows.h"
#define MINE(k) ((k)==WLE_ONLY)
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;uint8_t k;
 /* Interrupts off and $01=$35: BASIC and KERNAL ROM banked out, so the
  * check's state at $e000 reads back as RAM. */
 __asm__("sei");*(volatile uint8_t *)1u=0x35u;
 for(n=0;n<wle_cases();++n){k=wle_kind(n);if(!MINE(k))continue;
  if(n%4u==0u||!n||k!=wle_kind((uint16_t)(n-1u))||(n>=2u&&k!=wle_kind((uint16_t)(n-2u))))failures=(uint16_t)(failures+wle_check(n,&checks));}
 for(n=0;n<WLE_C64_ROWS;++n)if(MINE(wle_kind(wle_c64_row[n]))){
  /* The cover row's digest (the host's) is the frozen one. */
  if(wle_frozen_digest(wle_c64_row[n])!=wle_c64_row_digest[n])++failures;
  failures=(uint16_t)(failures+wle_check(wle_c64_row[n],&checks));}
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
