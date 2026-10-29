/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "recovery_check.h"
#include "recovery_vectors.h"
int main(void) {
 recovery_case c; recovery_abort_state before; unsigned n;
 for(n=0;n<RECOVERY_TARGET_COUNT;++n) assert(recovery_check((uint8_t)n,&c)==0);
 recovery_prepare(&c,0); before=c.state;
 assert(recovery_abort(&c.state,(enum recovery_abort_entry)5)==1);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 assert(recovery_abort(&c.state,(enum recovery_abort_entry)-1)==1);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 c.state.q=3; before=c.state;
 assert(recovery_abort(&c.state,RECOVERY_FLAGS)==1);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 /* Complete phase cleanup must leave validation-ready -0/+0 pairs, retain
  * priorities/addresses, and return the exact old sixth pair as debris. */
 recovery_prepare(&c,32768); before=c.state;
 assert(recovery_abort(&c.state,RECOVERY_MR_KLEAN)==0);
 for(n=0;n<6;++n) {
  assert(c.state.records.complement[n]==AGC_NEG_ZERO && c.state.records.phase[n]==0);
  assert(c.state.records.priority[n]==before.records.priority[n]);
  assert(c.state.records.address[n].location==before.records.address[n].location);
  assert(c.state.records.address[n].bank==before.records.address[n].bank);
 }
 assert(c.state.a==agc_extend(before.records.complement[5]));
 assert(c.state.l==agc_extend(before.records.phase[5]));
 puts("PASS: 128 complete original assembly abort-cleanup states; unsupported routes retain state");
 return 0;
}
