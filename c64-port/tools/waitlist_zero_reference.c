/* SPDX-License-Identifier: GPL-2.0-or-later
 * Independent unchanged WAITLIST assembly, input constructor only. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "waitlist_zero_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(int argc,char **argv) {
 unsigned n,i,cycles,pc,result; static agc_t pristine;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1])) return 2;
 for(n=0;n<WL_ZERO_CASE_COUNT;++n) {
  waitlist w; exec_address task; memcpy(&machine,&pristine,sizeof machine);
  wl_zero_prepare(&w,(uint16_t)n,&task); InhibitAlarms=1;
  set(026,w.time3); set(0734,w.ruptagn);
  for(i=0;i<8;++i) set(01400+i,w.interval[i]);
  for(i=0;i<9;++i) { set(01410+2*i,w.task[i].location); set(01411+2*i,w.task[i].bank); }
  set(RegBB,06005); set(RegFB,06000); set(RegEB,02400); machine.AllowInterrupt=0;
  machine.Erasable[0][RegA]=20; set(RegQ,01000); set(01000,task.location); set(01001,task.bank); set(RegZ,05203); result=0;
  for(cycles=0;cycles<4000;++cycles) {
   pc=get(RegZ);
   if(!machine.PendFlag && !machine.ExtraDelay) {
    if(pc==05215) break;
    if(pc==05716) { result=(unsigned)machine.Fixed[1][get(RegQ)&01777]&AGC_MASK; break; }
   }
   machine.ScalerCounter=0; if(agc_engine(&machine)) return 2;
  }
  if(cycles==4000) return 2;
  w.time3=(agc_word)get(026); w.ruptagn=(agc_word)get(0734); w.alarm=(agc_word)result;
  for(i=0;i<8;++i) w.interval[i]=(agc_word)get(01400+i);
  for(i=0;i<9;++i) { w.task[i].location=(agc_word)get(01410+2*i); w.task[i].bank=(agc_word)get(01411+2*i); }
  printf("%u %u",n,result); wl_zero_words(&w,emit,0); putchar('\n');
  fprintf(stderr,"waitlist-zero%u %ucycles PC%05o\n",n,cycles,pc);
 }
 return 0;
}
