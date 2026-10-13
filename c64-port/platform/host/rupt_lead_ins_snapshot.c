/* SPDX-License-Identifier: MIT */
/* Native rupt-lead-ins cases of one mission (argv[1]: lm or cm) in the
 * oracle's format: case, outcome kind and, unless refused, all 2048 erasable
 * words, SUPERBNK, the interrupt-enable bit and InIsr. */
#include <stdio.h>
#include <string.h>
#include "rupt_lead_ins_cases.h"
int main(int argc,char **argv){unsigned n,i,kind,cm;
 if(argc!=2)return 2;
 cm=!strcmp(argv[1],"cm");
 for(n=0;n<rli_cases(cm);++n){static apollo_overlay_state s;static rli_io io;
  kind=rli_run(cm,&s,&io,(uint16_t)n);
  if(kind>RLI_REFUSED){fprintf(stderr,"rupt-lead-ins case %u not entered at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u",n,kind);
  if(kind!=RLI_REFUSED){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<3u?0177777u:077777u));
   printf(" %u %u %u",(unsigned)(cm?io.cm.superbank:io.lm.superbank),(unsigned)(cm?io.cm.interrupt_enabled:io.lm.interrupt_enabled),(unsigned)io.hw.in_interrupt);}
  putchar('\n');
 }return 0;}
