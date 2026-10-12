/* SPDX-License-Identifier: MIT */
/* Native lm_restart_select over every case, in the oracle's format. */
#include <stdio.h>
#include "lm_restart_select_cases.h"
int main(void){unsigned n,i;
 for(n=0;n<LM_RSSELECT_CASES;++n){apollo_overlay_state s;enum lm_restart_select_exit e=lm_restart_select(&s,lm_rsselect_input(&s,(uint16_t)n));
  if(e==LM_RS_SELECT_INVALID){fprintf(stderr,"LM restart select %u refused\n",n);return 1;}
  printf("%u %u",n,(unsigned)(e==LM_RS_SELECTED_TASK?1u:e==LM_RS_SELECTED_LONGCALL?2u:e==LM_RS_SELECTED_DISPLAY?3u:0u));
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i==0||i==1?0177777u:077777u));
  putchar('\n');
 }return 0;}
