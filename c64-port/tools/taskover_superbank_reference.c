/* SPDX-License-Identifier: GPL-2.0-or-later; host-only yaAGC oracle for TASKOVER's RESUME check (#144). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "lm_exec_routines.h"
#include "cm_exec_routines.h"
#include "p66_bias.h"
/* argv: Luminary rope, Comanche rope. For each case yaAGC itself takes
 * T3RUPT in a program running at 02000-03777 of a bank from 030 to 033 with
 * one superbank, so the hardware fills ZRUPT and BRUPT from the real rope.
 * T3RUPT keeps the superbank in BANKRUPT (ROR SUPERBNK), and the task leaves
 * channel 7 at another superbank. The native TASKOVER then gets that ZRUPT,
 * BRUPT and BANKRUPT, the real rope as fixed_data and the task's channel 7.
 * It must resume (not refuse), and its result must equal yaAGC running the
 * original TASKOVER from the same state until InIsr clears: SUPERBNK, the
 * interrupt-enable bit and all 2048 erasable words. Prints one line per
 * mission and the cases where the word under the task's superbank differs
 * (there, a check under channel 7 would refuse). */
static agc_t rope[2];
static int mission_now;
static bool rope_word(const agc_native_program *p,uint16_t physical,agc_word *w){(void)p;
 if(physical>=36u*1024u)return false;*w=(agc_word)(rope[mission_now].Fixed[physical/1024u][physical%1024u]&077777u);return true;}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
int main(int argc,char **argv){static apollo_overlay_state s;unsigned m,n,i,cycles,cases[2]={0,0},differ[2]={0,0},failures=0;
 if(argc!=3)return 2;
 for(i=0;i<2u;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;rope[i]=machine;}
 InhibitAlarms=1;
 for(m=0;m<2u;++m)for(n=0;n<512u;++n){uint32_t r=0x7a5u+m*0x9e3779b9u+n*2654435761u;agc_word bank,off,eb,sb,task_sb,zrupt,brupt;
  agc_hardware hw={0};agc_native_program p={0};agc_routine_table t;agc_routine call;enum exec_status st;p66_bias_io lm={0};cm_native_io cm={0};
  bank=(agc_word)(030u+rnd(&r)%4u);off=(agc_word)(rnd(&r)%02000u);eb=(agc_word)(rnd(&r)%8u);
  /* One superbank with bit 0100 and one without, either way round, or
   * (one case in four) two without. */
  sb=(agc_word)((rnd(&r)%4u)*020u);task_sb=(agc_word)((rnd(&r)%4u)*020u);
  if(n%4u){if(n%2u)sb|=0100u;else task_sb|=0100u;}
  /* yaAGC takes T3RUPT at Z = 02000 + off under FB = bank, SUPERBNK sb. */
  machine=rope[m];for(i=0;i<2048u;++i)*memory(i)=0;
  set(4,(agc_word)(bank<<10));set(3,(agc_word)(eb<<8));set(6,(agc_word)((bank<<10)|eb));set(5,(agc_word)(02000u+off));
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)sb;machine.AllowInterrupt=1;machine.InIsr=0;machine.InterruptRequests[3]=1;
  machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
  if(!machine.InIsr)continue;      /* RELINT, INHINT or EXTEND at Z: the interrupt waits */
  zrupt=(agc_word)get(015);brupt=(agc_word)get(017);
  /* TASKOVER with RUPTAGN -0: everything else +0, as the synthetic test has it. */
  for(i=0;i<APOLLO_SHARED_WORDS;++i)s.words[i]=0;
  s.words[5]=(agc_word)(m?CM_EXEC_TASKOVER:LM_EXEC_TASKOVER);s.words[0734]=AGC_NEG_ZERO;
  s.words[015]=zrupt;s.words[017]=brupt;s.words[016]=(agc_word)((bank<<10)|eb|sb);s.words[4]=02000u;s.words[6]=02000u;
  ++cases[m];
  {agc_word w;rope_word(0,agc_fixed_physical((agc_word)(zrupt-1u),(agc_word)(bank<<10),task_sb),&w);if(w!=brupt)++differ[m];}
  hw.in_interrupt=1;t=m?cm_exec_routines:lm_exec_task_routines;mission_now=(int)m;
  p.basic=&t;p.basic_count=1;p.hardware=&hw;p.fixed_data=rope_word;p.interpreter_fetch=AGC_TRANSFER_NONE;
  lm.superbank=task_sb;cm.superbank=task_sb;
  call=agc_transfer_resolve(&p,&s,task_sb);
  st=call?call(&p,&s,m?(void *)&cm:(void *)&lm):EXEC_UNMAPPED;
  if(st!=EXEC_OK){++failures;printf("FAIL: %s case %u: ZRUPT %05o BRUPT %05o BANKRUPT %05o channel 7 %03o: status %d\n",
   m?"CM":"LM",n,zrupt,brupt,(unsigned)((bank<<10)|eb|sb),task_sb,(int)st);continue;}
  /* yaAGC from the same state. */
  machine=rope[m];
  {static apollo_overlay_state in;for(i=0;i<APOLLO_SHARED_WORDS;++i)in.words[i]=0;
   in.words[5]=(agc_word)(m?CM_EXEC_TASKOVER:LM_EXEC_TASKOVER);in.words[0734]=AGC_NEG_ZERO;
   in.words[015]=zrupt;in.words[017]=brupt;in.words[016]=(agc_word)((bank<<10)|eb|sb);in.words[4]=02000u;in.words[6]=02000u;
   for(i=0;i<2048u;++i)*memory(i)=(int16_t)in.words[i];}
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)task_sb;machine.AllowInterrupt=0;machine.InIsr=1;
  for(cycles=0;cycles<2000u&&machine.InIsr;++cycles){machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  {agc_word nsb=m?cm.superbank:lm.superbank,nen=m?(agc_word)cm.interrupt_enabled:(agc_word)lm.interrupt_enabled;
   unsigned bad=machine.InIsr!=0||hw.in_interrupt!=0||((unsigned)machine.OutputChannel7&0160u)!=nsb||(unsigned)machine.AllowInterrupt!=nen;
   for(i=0;i<2048u&&!bad;++i)bad=(((unsigned)*memory(i)^s.words[i])&(i<3u?0177777u:077777u))!=0;
   if(bad){++failures;printf("FAIL: %s case %u differs from yaAGC (word %u)\n",m?"CM":"LM",n,i?i-1u:0u);}}
 }
 for(m=0;m<2u;++m)printf("%s: %u TASKOVER RESUME cases from yaAGC's own T3RUPT, %u where the word under the task's superbank differs\n",
  m?"Comanche055":"Luminary099",cases[m],differ[m]);
 printf("%u failures\n",failures);
 return failures!=0;}
