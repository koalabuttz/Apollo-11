/* SPDX-License-Identifier: MIT */
/* resident-kernel: the thin Executive and Waitlist (src/lm_kernel.c,
 * src/cm_kernel.c) on every case of the certified service milestones
 * (tests/resident_kernel_cases.c). Each case's digest must equal the
 * original program's (the milestone's frozen yaAGC vector), and the final
 * image must equal the certified routines' word for word. Cases the
 * certified routines refuse (the original runs into data: DPBIT14, TC 1203)
 * have no oracle and are counted separately. Build with -DRK_LM=1 or
 * -DRK_CM=1; RK_SHOW=<octal entry Z> prints that entry's mismatches. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resident_kernel_cases.h"
#if RK_LM
#include "lm_exec_services_vectors.h"
#include "lm_exec_jobs_vectors.h"
#include "lm_exec_idle_vectors.h"
#define MISSION "lm"
#include "lm_kernel_c64_rows.h"
static unsigned certified_services(apollo_overlay_state *c,uint16_t n){uint16_t steps;return lm_exs_run(c,n,&steps);}
static unsigned certified_jobs(apollo_overlay_state *c,uint16_t n){p66_bias_io io;lm_exj_case k;uint16_t steps;return lm_exj_run(c,&io,&k,n,&steps);}
static unsigned certified_idle(apollo_overlay_state *c,uint16_t n){p66_bias_io io;agc_hardware hw;return lm_exi_run(c,&io,&hw,n);}
#define EXPECTED_SERVICES lm_exs_expected
#define EXPECTED_JOBS lm_exj_expected
#define EXPECTED_IDLE lm_exi_expected
#else
#include "cm_exec_services_vectors.h"
#include "cm_exec_jobs_vectors.h"
#include "cm_exec_idle_vectors.h"
#define MISSION "cm"
#include "cm_kernel_c64_rows.h"
static unsigned certified_services(apollo_overlay_state *c,uint16_t n){cm_native_io io;cm_exs_case k;uint16_t steps;return cm_exs_run(c,&io,&k,n,&steps);}
static unsigned certified_jobs(apollo_overlay_state *c,uint16_t n){cm_native_io io;return cm_exj_run(c,&io,n);}
static unsigned certified_idle(apollo_overlay_state *c,uint16_t n){cm_native_io io;return cm_exi_run(c,&io,n);}
#define EXPECTED_SERVICES cm_exs_expected
#define EXPECTED_JOBS cm_exj_expected
#define EXPECTED_IDLE cm_exi_expected
#endif
static unsigned checks,failures,shown;
static void diff(const char *what,unsigned n,agc_word entry,const apollo_overlay_state *a,const apollo_overlay_state *b){unsigned i,k=0;
 {const char *f=getenv("RK_SHOW");if(f&&strtoul(f,0,8)!=entry)return;}
 if(shown>=12u)return;
 ++shown;
 fprintf(stderr,"%s case %u (from %04o): kernel/certified",what,n,entry);
 for(i=0;i<2048u&&k<8u;++i)if(a->words[i]!=b->words[i]){fprintf(stderr," [%04o] %06o/%06o",i,a->words[i],b->words[i]);++k;}
 fprintf(stderr,"\n");
}
typedef struct {unsigned same,refused,cases;} tally;
static void module(tally *t,const char *what,uint16_t cases,uint32_t (*kernel)(apollo_overlay_state *,uint16_t),
 unsigned (*certified)(apollo_overlay_state *,uint16_t),const uint32_t *expected){uint16_t n;
 for(n=0;n<cases;++n){apollo_overlay_state s,c;uint32_t d;agc_word entry;
  ++t->cases;
  if(certified(&c,n)==2u){++t->refused;continue;}
  d=kernel(&s,n);entry=rk_entry;
  checks+=2u;
  if(d==expected[n]&&!memcmp(&s,&c,sizeof s)){++t->same;continue;}
  if(d!=expected[n])++failures;
  if(memcmp(&s,&c,sizeof s))++failures;
  diff(what,n,entry,&s,&c);
 }
}
int main(void){tally t[3];unsigned i;static const char *names[3]={"services","jobs","idle"};
 memset(t,0,sizeof t);
 module(&t[0],MISSION "-exec-services",RK_SERVICES_CASES,rk_services_digest,certified_services,EXPECTED_SERVICES);
 module(&t[1],MISSION "-exec-jobs",RK_JOBS_CASES,rk_jobs_digest,certified_jobs,EXPECTED_JOBS);
 module(&t[2],MISSION "-exec-idle",RK_IDLE_CASES,rk_idle_digest,certified_idle,EXPECTED_IDLE);
 /* The C64 rows' frozen digests are this corpus's. */
 for(i=0;i<RK_C64_ROWS;++i){apollo_overlay_state s;uint16_t n=rk_c64_case[i];uint32_t d;
  if(n<RK_SERVICES_CASES)d=rk_services_digest(&s,n);
  else if((n=(uint16_t)(n-RK_SERVICES_CASES))<RK_JOBS_CASES)d=rk_jobs_digest(&s,n);
  else d=rk_idle_digest(&s,(uint16_t)(n-RK_JOBS_CASES));
  ++checks;if(d!=rk_c64_digest[i])++failures;}
 printf("%s: resident-kernel %s:",failures||!checks?"FAIL":"PASS",MISSION);
 for(i=0;i<3u;++i)printf(" %s %u/%u (%u refused by the certified routines)",names[i],t[i].same,t[i].cases-t[i].refused,t[i].refused);
 printf("; %u C64 rows; %u checks, %u failures\n",(unsigned)RK_C64_ROWS,checks,failures);
 return failures!=0u||checks==0u;
}
