/* SPDX-License-Identifier: MIT */
#include "cm_startup_cases.h"
#include <stdio.h>
static void emit(void *u,agc_word w) { (void)u;printf(" %u",w); }
int main(void) {
 uint16_t n;apollo_overlay_state s;cm_native_io io;cm_startup_ports p;
 for(n=0;n<640;++n) {
  enum cm_startup_status status;cm_startup_prepare(&s,&io,&p,n);
  status=cm_startup_begin(&s,&io,&p,(enum cm_startup_entry)(n/128u));
  printf("%u",n);cm_startup_words(&s,&io,&p,status,emit,0);putchar('\n');
 }return 0;
}
