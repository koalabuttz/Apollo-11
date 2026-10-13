/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_entries_cases.h"
/* Per case: "n reached superbank enable inisr w0..w2047" (refused: "n 0"). */
int main(void){static apollo_overlay_state s;static wle_io_storage io;agc_hardware hw;uint16_t n,i,sb,en;
 for(n=0;n<wle_cases();++n){if(!wle_run(&s,&io,&hw,n)){printf("%u 0\n",(unsigned)n);continue;}
  wle_io(&io,wle_kind(n),&sb,&en);printf("%u 1 %u %u %u",(unsigned)n,(unsigned)sb,(unsigned)en,(unsigned)hw.in_interrupt);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<3u?0177777u:077777u)));
  printf("\n");}
 return 0;}
