/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "waitlist_entries_check.h"
#include "waitlist_entries_cases.h"
#include "lm_exec_routines.h"
#include "cm_exec_routines.h"
#include "p66_bias.h"

/* A fixed word per physical address, so BRUPT can name the word the RESUME
 * check reads under one superbank and not another. */
static bool rope_word(const agc_native_program *p,uint16_t physical,agc_word *w){(void)p;*w=(agc_word)((physical*0123u+07u)&077777u);return true;}

/* TASKOVER's CA BANKRUPT; EXTEND; WRITE SUPERBNK runs before its RESUME, so
 * the RESUME check reads ZRUPT-1 under BANKRUPT's superbank (T3RUPT's ROR
 * SUPERBNK), not the current channel 7 (port auditor on #144): BANKRUPT
 * 060100, ZRUPT 02101, channel 7 060. Returns failures. */
static unsigned long taskover_superbank(uint16_t *checks){unsigned long failures=0;unsigned mission,other;
 for(mission=0;mission<2u;++mission)for(other=0;other<2u;++other){
  static apollo_overlay_state s;agc_hardware hw={0};agc_native_program p={0};agc_routine_table t;agc_routine call;enum exec_status st,want;
  p66_bias_io lm={0};cm_native_io cm={0};uint16_t i;
  for(i=0;i<APOLLO_SHARED_WORDS;++i)s.words[i]=0;
  s.words[5]=(agc_word)(mission?CM_EXEC_TASKOVER:LM_EXEC_TASKOVER);s.words[0734]=AGC_NEG_ZERO;
  s.words[016]=060100u;s.words[015]=02101u;s.words[4]=02000u;s.words[6]=02000u;
  /* BRUPT: the word at 02100 under BANKRUPT's superbank 0100 (resumes), or
   * under the current 060 (refused: yaAGC resumes at the other word). */
  rope_word(&p,agc_fixed_physical(02100u,060000u,other?0060u:0100u),&s.words[017]);
  want=other?EXEC_OUT_OF_SCOPE:EXEC_OK;
  hw.in_interrupt=1;t=mission?cm_exec_routines:lm_exec_task_routines;
  p.basic=&t;p.basic_count=1;p.hardware=&hw;p.fixed_data=rope_word;p.interpreter_fetch=AGC_TRANSFER_NONE;
  lm.superbank=0060u;cm.superbank=0060u;
  call=agc_transfer_resolve(&p,&s,0060u);
  st=call?call(&p,&s,mission?(void *)&cm:(void *)&lm):EXEC_UNMAPPED;
  ++*checks;
  if(st!=want){++failures;printf("FAIL: %s TASKOVER RESUME, BANKRUPT 060100, channel 7 060, BRUPT under %s: status %d, want %d\n",
   mission?"CM":"LM",other?"060":"0100",(int)st,(int)want);}
 }
 return failures;}

/* The thin resident kernel's DLY2 -1 (kinds 2 and 3) on every case the
 * certified routines take reaches the same frozen digest. Returns failures. */
extern int wle_kernel;
static unsigned long kernel_dly2(uint16_t *checks,unsigned *taken){unsigned long failures=0;uint16_t n;
 for(n=0;n<wle_cases();++n){static apollo_overlay_state s;static wle_io_storage io;agc_hardware hw;uint8_t k=wle_kind(n);uint32_t d;
  if(k!=2u&&k!=3u)continue;
  wle_kernel=0;
  if(!wle_run(&s,&io,&hw,n))continue;
  wle_kernel=1;d=wle_digest(&s,n);wle_kernel=0;
  ++*taken;++*checks;
  if(d!=wle_frozen_digest(n)){++failures;printf("FAIL: kernel DLY2 -1 case %u (%s base %04x): digest %08lx, want %08lx\n",(unsigned)n,
   k==2u?"LM":"CM",(unsigned)wle_case_base_of(n),(unsigned long)d,(unsigned long)wle_frozen_digest(n));}}
 return failures;}

int main(void){uint16_t n,checks=0;unsigned long failures=0;unsigned taken=0;
 for(n=0;n<wle_cases();++n)failures+=wle_check(n,&checks);
 failures+=kernel_dly2(&checks,&taken);
 printf("waitlist-entries: resident kernel DLY2 -1 on %u cases the certified routines take\n",taken);
 failures+=taskover_superbank(&checks);
 printf("waitlist-entries: %u cases; %u checks, %lu failures\n",(unsigned)wle_cases(),(unsigned)checks,failures);
 return failures!=0;}
