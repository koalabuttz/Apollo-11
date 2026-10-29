/* SPDX-License-Identifier: MIT */
#include "cm_restart_native_cases.h"
#include <stdio.h>
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(void) {
 uint8_t n,step;for(n=0;n<64;++n) {
  cm_rn_run r;cm_rn_init(&r,n);
  for(step=0;step<14;++step) {
   enum exec_status status=cm_rn_step(&r,step);
   printf("%u %u",n,step);cm_rn_words(&r.state,status,r.runtime.current,r.callbacks,emit,0);putchar('\n');
   if(r.failures || r.active || r.runtime.in_callback) return 1;
   if(status==EXEC_IDLE) break;
   if(status!=EXEC_OK) return 1;
  }
 }return 0;
}
