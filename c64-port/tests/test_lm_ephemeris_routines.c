/* SPDX-License-Identifier: MIT */
/* LSPOS/LUNPOS and LUNVEL through the control-transfer trampoline: for every
 * sampled entry, the trampoline starts at the entry frontier, runs the
 * registered routine at each NEWOPS frontier, returns to the caller's
 * continuation (a native stand-in registered at the caller's return LOC,
 * which leaves Z at fixed 04000) and must leave exactly the image of the
 * certified lm_ephemeris composition followed by that stand-in. */
#include <stdio.h>
#include "lm_ephemeris_routines.h"
#include "lm_ephemeris_cases.h"
#include "standard_lead_routines.h"
#define CALLER_STOP ((uint16_t)2048u) /* fixed-fixed 04000 */
static int reached;
static enum exec_status caller(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;++reached;s->words[5]=04000u;return EXEC_OK;}
static apollo_overlay_state a,b,start;
int main(void){
 agc_routine_entry back={0,caller};agc_routine_table tables[2];agc_native_program program={0};
 unsigned n,i,returned=0,refused=0,aborted=0;uint32_t h1=0,h2=0;
 tables[0]=lm_ephemeris_routines;tables[1].entries=&back;tables[1].count=1;
 program.interpretive=tables;program.interpretive_count=2;program.interpreter_fetch=LM_INTERPRETER_FETCH;program.loc=0164;program.bankset=0165;
 for(n=0;n<65536u;++n){p66_bias_io ia,ib;uint8_t r;uint16_t steps=0,stop;enum exec_status st;
  lm_ephemeris_input(&a,&ia,n);start=a;lm_ephemeris_input(&b,&ib,n);
  r=lm_ephemeris(&a,&ia);
  if(r==1u){back.physical=agc_transfer_point(&program,&a,ia.superbank);
   if(back.physical==AGC_TRANSFER_NONE){fprintf(stderr,"FAIL %u: caller point\n",n);return 1;}
   reached=0;a.words[5]=04000u;stop=CALLER_STOP;++returned;}
  else if(r==2u){stop=agc_transfer_point(&program,&a,ia.superbank);++aborted;}
  else stop=AGC_TRANSFER_NONE;
  st=agc_transfer_run(&program,&b,&ib,&ib.superbank,stop,64u,&steps);
  if(r==0u){if(st!=EXEC_OUT_OF_SCOPE||steps!=1u){fprintf(stderr,"FAIL %u: refused entry ran (status %d, %u steps)\n",n,(int)st,steps);return 1;}
   for(i=0;i<2048u;++i)if(b.words[i]!=start.words[i]){fprintf(stderr,"FAIL %u: refused entry changed word %04o\n",n,i);return 1;}
   ++refused;continue;}
  if(st!=EXEC_OK||(r==1u&&reached!=1)){fprintf(stderr,"FAIL %u: status %d after %u steps\n",n,(int)st,steps);return 1;}
  for(i=0;i<2048u;++i)if(a.words[i]!=b.words[i]){fprintf(stderr,"FAIL %u: word %04o %05o, composition %05o\n",n,i,b.words[i],a.words[i]);return 1;}
  if(ia.superbank!=ib.superbank||ia.interrupt_enabled!=ib.interrupt_enabled){fprintf(stderr,"FAIL %u: io\n",n);return 1;}
  for(i=0;i<2048u;++i){h1=h1*31u+b.words[i];h2^=h1;}h1+=steps;
 }
 /* Outside the domain (a pending NEWJOB, a nonzero EDOP, MODE, word 7 or
  * 023 at entry): the entry routine refuses before changing anything. */
 for(n=0;n<1024u;++n){static const uint16_t words[5]={07,023,067,0115,0163};p66_bias_io io;uint16_t steps=0;
  lm_ephemeris_input(&b,&io,n);b.words[words[n%5u]]=(agc_word)(1u+n/5u%077777u);start=b;
  if(agc_transfer_run(&program,&b,&io,&io.superbank,AGC_TRANSFER_NONE,64u,&steps)!=EXEC_OUT_OF_SCOPE||steps!=1u){fprintf(stderr,"FAIL guard %u ran\n",n);return 1;}
  for(i=0;i<2048u;++i)if(b.words[i]!=start.words[i]){fprintf(stderr,"FAIL guard %u changed word %04o\n",n,i);return 1;}
 }
 printf("PASS %u LSPOS/LUNVEL entries through the transfer trampoline equal the certified composition (%u to the caller's native continuation, %u at the UNIT abort frontier, %u refused unchanged; 1024 out-of-domain entries refused unchanged) digest %08lx%08lx\n",returned+aborted+refused,returned,aborted,refused,(unsigned long)h1,(unsigned long)h2);
 return 0;}
