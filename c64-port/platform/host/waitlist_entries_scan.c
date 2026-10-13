/* SPDX-License-Identifier: MIT */
/* Lists the base cases of each waitlist-entries kind (built with WLE_SCAN):
 * "kind base" for every T3 vector case of lm-exec-jobs / cm-exec-services,
 * every DELAYJOB entry case of service-transfer (twice: its own image
 * and a sane waitlist; LM also a sane waitlist ending WAIT2's search on
 * CCS -0), and the NOQBRSM +1 seeds. */
#include <stdio.h>
#include "waitlist_entries_cases.h"
#include "lm_exec_jobs_cases.h"
#include "cm_exec_services_cases.h"
#include "service_transfer_cases.h"
int main(void){static apollo_overlay_state s;static lm_exj_case lc;static cm_exs_case cc;p66_bias_io lio;cm_native_io cio;uint16_t n,sb,ih;
 for(n=lm_exj_first(LM_EXJ_DISPATCH);n<LM_EXJ_CASES&&lm_exj_cohort(n)==LM_EXJ_DISPATCH;++n){lm_exj_input(&s,&lio,&lc,n);if(s.words[5]==04014u)printf("0 %u\n",(unsigned)n);}
 for(n=cm_exs_first(CM_EXS_DISPATCH);n<CM_EXS_CASES&&cm_exs_cohort(n)==CM_EXS_DISPATCH;++n){cm_exs_input(&s,&cio,&cc,n);if(s.words[5]==04014u)printf("1 %u\n",(unsigned)n);}
 for(n=0;n<svt_cases();++n){svt_input(&s,&sb,&ih,n);if(s.words[5]==03735u&&!svt_mission(n))printf("2 %u\n2 %u\n2 %u\n",(unsigned)n,(unsigned)(n|WLE_SANE),(unsigned)(n|WLE_SANE|WLE_EDGE));
  if(s.words[5]==03732u&&svt_mission(n))printf("3 %u\n3 %u\n",(unsigned)n,(unsigned)(n|WLE_SANE));}
 for(n=0;n<WLE_RESUME_CASES;++n)printf("4 %u\n",(unsigned)n);
 return 0;}
