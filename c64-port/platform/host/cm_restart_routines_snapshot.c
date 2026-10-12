/* SPDX-License-Identifier: MIT */
/* Native cm-restart-routines cases in the oracle's format
 * (tools/cm_restart_routines_reference.c): argv[1] select, compose or newphase. */
#include <stdio.h>
#include <string.h>
#include "cm_restart_routines_cases.h"
static void image(const apollo_overlay_state *s){unsigned i;for(i=0;i<2048u;++i)printf(" %u",(unsigned)s->words[i]&(i==0||i==1?0177777u:077777u));putchar('\n');}
int main(int argc,char **argv){unsigned n,i;apollo_overlay_state s;
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"select"))for(n=0;n<CM_RR_SELECT_CASES;++n){enum cm_rr_exit e=cm_restart_routines_select(&s,cm_rr_select_input(&s,(uint16_t)n));
  if(e==CM_RR_INVALID){fprintf(stderr,"CM restart select %u refused\n",n);return 1;}
  printf("%u %u",n,(unsigned)(e==CM_RR_SEL_TASK?1u:e==CM_RR_SEL_LONGCALL?2u:e==CM_RR_SEL_DISPLAY?3u:0u));image(&s);}
 else if(!strcmp(argv[1],"compose"))for(n=0;n<CM_RR_COMPOSE_CASES;++n){cm_rr_log log;uint16_t steps;
  if(cm_rr_compose(&s,(uint16_t)n,&log,&steps)!=EXEC_OK){fprintf(stderr,"CM restart composition %u stopped\n",n);return 1;}
  printf("%u %u",n,(unsigned)log.calls);
  for(i=0;i<log.calls;++i)printf(" %u %u %u %u %u %u",(unsigned)log.call[i].service,(unsigned)log.call[i].a,(unsigned)log.call[i].l,(unsigned)log.call[i].q,(unsigned)log.call[i].cadr,(unsigned)log.call[i].bbcon);
  image(&s);}
 else if(!strcmp(argv[1],"newphase"))for(n=0;n<CM_RR_NEWPHASE_CASES;++n){uint16_t steps;cm_native_io io;
  if(cm_rr_newphase(&s,&io,(uint16_t)n,&steps)!=EXEC_OK){fprintf(stderr,"CM NEWPHASE %u stopped\n",n);return 1;}
  printf("%u %u",n,(unsigned)io.interrupt_enabled);image(&s);}
 else return 2;
 return 0;}
