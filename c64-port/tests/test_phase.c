/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "phase_check.h"
#include "phase_vectors.h"
int main(void) {
 phase_case c; phase_state before; unsigned n;
 for(n=0;n<PH_TARGET_COUNT;++n) assert(phase_check((uint8_t)n,&c)==0);
 phase_prepare(&c,0); before=c.state;
 for(n=0;n<32768;++n) {
  if((n&7)!=0 && (n&7)!=7) continue;
  c.words[0]=(agc_word)n;
  assert(phase_change(&c.state,c.words,c.count,0)==PHASE_OUT_OF_SCOPE);
  assert(memcmp(&before,&c.state,sizeof before)==0);
 }
 phase_prepare(&c,0); before=c.state;
 assert(phase_change(&c.state,c.words,0,0)==PHASE_OUT_OF_SCOPE);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 assert(phase_change(&c.state,c.words,6,0)==PHASE_OUT_OF_SCOPE);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 c.state.superbank=0100000; before=c.state;
 assert(phase_change(&c.state,c.words,5,0)==PHASE_OUT_OF_SCOPE);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 c.state.superbank=0; before=c.state;
 c.words[0]=05001;
 assert(phase_change(&c.state,c.words,1,0)==PHASE_OUT_OF_SCOPE);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 c.words[0]=077777; c.words[1]=1;
 assert(phase_change(&c.state,c.words,5,1)==PHASE_OUT_OF_SCOPE);
 assert(memcmp(&before,&c.state,sizeof before)==0);
 puts("PASS: 128 original assembly full-state phase vectors; all 8,192 invalid group words retain state");
 return 0;
}
