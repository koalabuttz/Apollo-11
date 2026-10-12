/* SPDX-License-Identifier: MIT */
/* lm-p34-transfer: Luminary099 P34-35_P74-75 as routines of the
 * control-transfer ABI, against digests frozen from the original program
 * (tools/lm_p34_differential.py). Every basic routine reaches each exit the
 * source gives it, the refused stored returns leave the image unchanged,
 * and entries outside the routines' contracts are refused unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_p34_cases.h"
#include "lm_p34_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
#define COMPTGO 02432u
#define DELAYJOB 03735u
#define GOFLASH 02476u
#define PHASCHNG 05353u
#define UPFLAG 05504u
#define GOTOPOOH 06001u
#define INTPRET 06037u
#define CALLER 1u /* a return the case chose (P3XORP7X's Q or Q+1, a stored TC) */
/* Exits each basic routine must reach, in tests/lm_p34_cases.c order. */
static const agc_word exits[LM_P34_BASIC_ROUTINES][2]={
 {COMPTGO},{DELAYJOB},{GOFLASH},{GOTOPOOH},{GOTOPOOH,PHASCHNG},{PHASCHNG},{GOTOPOOH},{GOTOPOOH,PHASCHNG},{UPFLAG},{PHASCHNG},
 {INTPRET},{GOFLASH},{GOTOPOOH},{CALLER},{GOFLASH},{CALLER},{GOFLASH},{GOTOPOOH},{CALLER},{GOFLASH}};
static unsigned reached[LM_P34_BASIC_ROUTINES][2];
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* One trampoline run of an edited case image: must refuse unchanged. */
static void refuse(apollo_overlay_state *s,p66_bias_io *io,const char *what,unsigned n){
 agc_routine_table basic[1],interp[1];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
 apollo_overlay_state before=*s;p66_bias_io io0=*io;uint16_t steps;enum exec_status st;
 basic[0]=lm_p34_basic_routines;interp[0]=lm_p34_interpretive_routines;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+(06070u-04000u));program.loc=0164;program.bankset=0165;
 st=agc_transfer_run(&program,s,io,&io->superbank,0xfffeu,4,&steps);
 check(st==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&io0,sizeof io0),what,n);
}
int main(void){apollo_overlay_state s,before;p66_bias_io io;lm_p34_case c;unsigned n,i,refused=0,guards=0;
 check(sorted(&lm_p34_basic_routines)&&lm_p34_basic_routines.count==LM_P34_BASIC_ROUTINES,"basic table",0);
 check(sorted(&lm_p34_interpretive_routines)&&lm_p34_interpretive_routines.count==LM_P34_INTERPRETIVE_ROUTINES,"interpretive table",0);
 {uint16_t k;const agc_site_operand *t=lm_p34_site_table(&k);check(k==LM_P34_SITES&&t[0].physical<t[1].physical,"site table",0);}
 check(LM_P34_VECTORS==LM_P34_CASES,"vector count",LM_P34_VECTORS);
 for(n=0;n<LM_P34_CASES;++n){unsigned kind;
  lm_p34_input(&before,&io,&c,n);
  check(lm_p34_digest(lm_p34_program(),&s,n)==lm_p34_expected[n],"digest",n);
  kind=lm_p34_run(&s,&io,n);
  if(c.refused){check(kind==LM_P34_REFUSED&&!memcmp(&s,&before,sizeof s),"refused",n);++refused;continue;}
  check(kind==LM_P34_STOPPED,"stopped",n);
  if(n<LM_P34_BASIC_CASES){unsigned r=n%LM_P34_BASIC_ROUTINES;int known=0;
   for(i=0;i<2u&&exits[r][i];++i){
    int hit=s.words[5]==exits[r][i];
    if(exits[r][i]==CALLER){unsigned k;for(k=LM_P34_COMMON_STOPS;k<LM_P34_STOPS&&c.stop[k].z;++k)if(s.words[5]==c.stop[k].z)hit=1;}
    if(hit){++reached[r][i];known=1;break;}}
   check(known,"exit",n);
  }
 }
 for(n=0;n<LM_P34_BASIC_ROUTINES;++n)for(i=0;i<2u&&exits[n][i];++i)check(reached[n][i]>0u,"exit reached",n*10u+i);
 check(refused==64u,"stored returns refused",refused);
 /* Entries outside the contracts, from the first state of each kind. */
 for(n=0;n<LM_P34_BASIC_ROUTINES;++n){
  lm_p34_input(&s,&io,&c,n);s.words[3]|=1u;refuse(&s,&io,"EB not a bank",n);++guards;
  lm_p34_input(&s,&io,&c,n);s.words[6]^=1u;refuse(&s,&io,"BB incoherent",n);++guards;
  lm_p34_input(&s,&io,&c,n);s.words[7]=1u;refuse(&s,&io,"ZERO",n);++guards;
  lm_p34_input(&s,&io,&c,n);s.words[0400]|=0100000u;refuse(&s,&io,"16-bit word",n);++guards;
  lm_p34_input(&s,&io,&c,n);io.superbank=1u;refuse(&s,&io,"SUPERBNK bits",n);++guards;
  lm_p34_input(&s,&io,&c,n);io.interrupt_enabled=2u;refuse(&s,&io,"IRQ",n);++guards;
 }
 {static const uint32_t first[4]={LM_P34_BASIC_CASES,LM_P34_BASIC_CASES+LM_P34_CLUPDATE_CASES,
   LM_P34_BASIC_CASES+LM_P34_CLUPDATE_CASES+LM_P34_VN1645_CASES,LM_P34_BASIC_CASES+LM_P34_CLUPDATE_CASES+LM_P34_VN1645_CASES+3u};
  for(i=0;i<4u;++i){uint32_t k=first[i];
   lm_p34_input(&s,&io,&c,k);s.words[0120]=0400u;refuse(&s,&io,"VAC",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[0163]=2u;refuse(&s,&io,"MODE",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[0115]^=040000u;refuse(&s,&io,"INTBIT15",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[023]=1u;refuse(&s,&io,"EDOP",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[067]=1u;refuse(&s,&io,"NEWJOB",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[0165]^=1u;refuse(&s,&io,"BANKSET",k);++guards;
   lm_p34_input(&s,&io,&c,k);s.words[0166]=(agc_word)(s.words[0120]+041u);refuse(&s,&io,"PUSHLOC",k);++guards;
  }}
 printf("lm-p34-transfer: %u cases, %u refused stored returns, %u admission guards;",LM_P34_CASES,refused,guards);
 for(n=0;n<LM_P34_BASIC_ROUTINES;++n){printf(" [");for(i=0;i<2u&&exits[n][i];++i)printf("%s%u",i?" ":"",reached[n][i]);printf("]");}
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
