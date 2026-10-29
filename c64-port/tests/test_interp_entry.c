/* SPDX-License-Identifier: MIT */
/* interp-entry: INTPRET, INTPRET +2 and INTRSM of Luminary099 and
 * Comanche055 as routines of the interpreter tables, against digests frozen
 * from the original programs; each ends at NEWOPS with LOC and BANKSET set;
 * incoherent banks are refused with the image unchanged; INTPRET from E/CALL
 * still runs on through its CALL. */
#include <stdio.h>
#include <string.h>
#include "interp_entry_cases.h"
#include "interp_entry_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static enum exec_status call_at(unsigned cm,apollo_overlay_state *s,ie_io *io){agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 program.basic=cm?&cm_interpreter_routines:&lm_interpreter_routines;program.basic_count=1;
 call=agc_transfer_resolve(&program,s,cm?io->cm.superbank:io->lm.superbank);
 return call?call(&program,s,cm?(void *)&io->cm:(void *)&io->lm):EXEC_OUT_OF_SCOPE;}
int main(void){static apollo_overlay_state s,in,before;unsigned cm,n,kind[3]={0,0,0};
 for(cm=0;cm<2u;++cm){const uint32_t *expected=cm?ie_cm_expected:ie_lm_expected;agc_word newops=(agc_word)((cm?INTERP_ENTRY_CM_INTPRET:INTERP_ENTRY_LM_INTPRET)+031u);
  for(n=0;n<IE_CASES;++n){static ie_io io,io0;unsigned k,e=n/IE_PER_ENTRY;
   check(ie_digest(cm,&s,(uint16_t)n)==expected[n],"digest",n);
   ie_input(cm,&in,&io0,(uint16_t)n);
   k=ie_run(cm,&s,&io,(uint16_t)n);if(k<3u)++kind[k];
   if(e==1u&&agc_correct(in.words[1])>03777u){check(k==IE_REFUSED&&!memcmp(&s,&in,sizeof s)&&!memcmp(&io,&io0,sizeof io),"INTPRET +2 out of the bank refused",n);continue;}
   check(k==IE_RETURNED,"outcome",n);
   check(s.words[5]==newops&&s.words[0165]==s.words[6],"NEWOPS with BANKSET",n);
   if(e==0u)check(s.words[0164]==agc_correct(in.words[2])&&(cm?io.cm.interrupt_enabled:io.lm.interrupt_enabled)==1u,"INTPRET: LOC is the caller's word, interrupts on",n);
   if(e==1u)check(s.words[0164]==agc_correct(in.words[1]),"INTPRET +2: LOC from L",n);
   if(e==2u)check(s.words[0164]==in.words[0164]&&s.words[6]==(agc_word)(agc_correct(in.words[1])&076007u),"INTRSM: LOC kept, BBANK from L",n);}
  /* Incoherent banks: refused, nothing changed. */
  for(n=0;n<IE_CASES;n+=IE_PER_ENTRY){static ie_io io,io0;ie_input(cm,&s,&io,(uint16_t)n);s.words[4]^=02000u;before=s;io0=io;
   check(call_at(cm,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),"incoherent FB refused",n);}
  /* E/CALL's INTPRET still takes the CALL path: LOC becomes the callee, not E/CALL's word. */
  {static ie_io io;ie_input(cm,&s,&io,0);s.words[2]=cm?02064u:02624u;s.words[4]=cm?026000u:010000u;s.words[3]=0;s.words[6]=s.words[4];
   s.words[cm?0364u:01167u]=cm?026100u:010100u;s.words[0120]=0401u;
   check(call_at(cm,&s,&io)==EXEC_OK&&s.words[0164]!=agc_correct(cm?02064u:02624u),"E/CALL form takes the CALL",cm);}}
 printf("interp-entry: %u LM and %u CM cases (%u returned, %u refused); %u checks, %u failures\n",IE_CASES,IE_CASES,kind[0],kind[2],checks,failures);
 return failures!=0u;
}
