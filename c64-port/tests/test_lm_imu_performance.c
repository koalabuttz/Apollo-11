/* SPDX-License-Identifier: MIT */
/* lm-imu-performance: Luminary099 IMU_PERFORMANCE_TEST_2 and
 * IMU_PERFORMANCE_TESTS_4 as native routines, against digests frozen from
 * the original program; every routine runs in most of its states, and each
 * refusal of include/lm_imu_performance.h leaves the image and io
 * unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_imu_performance_cases.h"
#include "lm_imu_performance_vectors.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
#define B37(a) (uint16_t)(037u*1024u+((a)-02000u))
static uint16_t routine(uint16_t key){uint16_t r;for(r=0;r<IPT_ROUTINES;++r)if(ipt_routine_keys[r]==key)return r;return 0xffffu;}
/* The QPRET the CALL at loc (pair, operand) stores, through lmi_call. */
static agc_word call_qpret(agc_word loc,agc_word pair,agc_word operand){static apollo_overlay_state t;
 memset(&t,0,sizeof t);
 t.words[4]=076000u;t.words[3]=02400u;t.words[6]=076005u;t.words[0165]=076005u;t.words[0115]=040000u;
 t.words[0120]=0401u;t.words[0166]=0405u;t.words[0164]=loc;t.words[5]=06070u;
 lmi_newops(&t,pair);lmi_misc(&t,operand,02507u);lmi_call(&t);
 return t.words[0401u+052u];
}
typedef void (*patch)(apollo_overlay_state *,ipt_case *,lm_ipt_io *);
#define FIX(s) ((s)->words[0120])
/* Runs case n with patch applied; 1 when refused with nothing changed. */
static int refused(uint16_t n,patch p){static apollo_overlay_state s,before,undo;ipt_case c;lm_ipt_io io,io0;
 agc_routine_table basic[1],interp[1];agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 ipt_input(&s,&c,n);
 io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;io.hiscalar=c.hiscalar;io.loscalar=c.loscalar;io.undo=&undo;
 p(&s,&c,&io);before=s;io0=io;
 basic[0]=lm_imu_performance_routines;interp[0]=lm_imu_performance_interpretive;
 program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
 program.interpreter_fetch=(uint16_t)(2048u+06070u-04000u);program.loc=0164u;program.bankset=0165u;
 call=agc_transfer_resolve(&program,&s,c.superbank);
 return call&&call(&program,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io);
}
static void edop(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[023]=0101u;}
static void fixloc(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;FIX(s)=0400u;}
static void pushloc(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[0166]=(agc_word)(FIX(s)+043u);}
static void fbank(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[4]=(agc_word)(s->words[4]-02000u);s->words[6]=(agc_word)(s->words[6]-02000u);}
static void no_undo(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)s;(void)c;io->undo=0;}
static void newjob_minus(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[067]=077776u;}
static void newjob_zero(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[067]=0;}
static void qplace_intpret(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[02417]=06037u;}
static void qplace_data(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[02417]=010000u;}
static void loscalar(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)s;(void)c;io->loscalar=037777u;}
static void qpret(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[FIX(s)+052u]=0100u;}
static void pipindex(apollo_overlay_state *s,ipt_case *c,lm_ipt_io *io){(void)c;(void)io;s->words[03]=02400u;s->words[06]=(agc_word)(s->words[4]|012u);s->words[02415]=02000u;}
int main(void){static apollo_overlay_state s;unsigned n,k,v;uint16_t ie;static unsigned stopped[IPT_ROUTINES];
 check(sorted(&lm_imu_performance_routines)&&lm_imu_performance_routines.count==IPT_BASIC_ROUTINES,"basic table",0);
 check(sorted(&lm_imu_performance_interpretive)&&lm_imu_performance_interpretive.count==IPT_INTERPRETIVE_ROUTINES,"interpretive table",0);
 check(IPT_VECTORS==IPT_CASES,"vector count",IPT_VECTORS);
 check(call_qpret(02532u,077624u,076401u)==IPT_QPRET_ERTHRVSE_2532&&call_qpret(03030u,077624u,076401u)==IPT_QPRET_ERTHRVSE_3030&&
  call_qpret(02452u,077624u,076420u)==IPT_QPRET_EARTHR_2452,"genuine QPRET",0);
 for(n=0;n<IPT_CASES;++n){unsigned kind=ipt_run(&s,(uint16_t)n,&ie);
  check(kind==IPT_STOPPED||kind==IPT_REFUSED,"kind",n);
  check(ipt_digest(&s,(uint16_t)n)==ipt_expected[n],"digest",n);
  if(kind==IPT_STOPPED)++stopped[n%IPT_ROUTINES];
 }
 /* Every routine runs in at least half of its states. */
 for(k=0;k<IPT_ROUTINES;++k)check(stopped[k]*2u>=IPT_STATES,"routine",k);
 {static const struct {uint16_t key;patch p;const char *what;} r[]={
   {B37(02021),edop,"EDOP"},{B37(02605),fixloc,"FIXLOC"},{B37(03030),pushloc,"PUSHLOC"},
   {B37(02040),fbank,"FB not BANKSET"},{B37(02002),no_undo,"no undo image"},
   {B37(02043),newjob_minus,"NEWJOB negative at DANZIG"},{B37(02342),newjob_zero,"CHECKG wait"},
   {B37(02467),qplace_intpret,"QPLACE TC INTPRET"},{B37(02467),qplace_data,"QPLACE not a TC"},
   {(uint16_t)(2048u+04112u-04000u),loscalar,"FINETIME LOSCALAR 37777"},{B37(02414),qpret,"RVQ QPRET erasable"},
   {B37(02354),pipindex,"INDEX PIPINDEX"}};
  for(k=0;k<sizeof r/sizeof *r;++k){uint16_t i=routine(r[k].key);
   check(i!=0xffffu,r[k].what,k);
   for(v=0;v<4u;++v)check(i!=0xffffu&&refused((uint16_t)(i+v*IPT_ROUTINES),r[k].p),r[k].what,v);}
 }
 printf("lm-imu-performance: %u cases; %u checks, %u failures\n",IPT_CASES,checks,failures);
 return failures!=0u;
}
