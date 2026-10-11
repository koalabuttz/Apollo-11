/* SPDX-License-Identifier: MIT */
#include "cm_startup.h"
static void clear_group(apollo_overlay_state *s,uint8_t group) {
 s->words[0752u+group*2u]=AGC_NEG_ZERO;s->words[0753u+group*2u]=0;
}
static void clean(apollo_overlay_state *s,cm_native_io *io,enum cm_startup_entry entry) {
 agc_word a=s->words[0764],l=s->words[0765];
 if(entry==CM_STARTUP_MR_KLEAN) { io->interrupt_enabled=0;clear_group(s,1); }
 if(entry!=CM_STARTUP_V37_KLEAN) { clear_group(s,3);clear_group(s,0); }
 clear_group(s,2);clear_group(s,4);clear_group(s,5);
 s->words[0]=agc_extend(a);s->words[1]=agc_extend(l);s->words[2]=3;
}
enum cm_startup_status cm_startup_begin(apollo_overlay_state *s,cm_native_io *io,
 cm_startup_ports *ports,enum cm_startup_entry entry) {
 static const uint16_t zero_addresses[]={01360,01362,01501,01316,0332,0371,01071,01130,01126,01125,01044};
 uint8_t i;agc_word old10,old11;
 if(s==0 || io==0 || ports==0 || (unsigned)entry>(unsigned)CM_STARTUP_V37_KLEAN ||
    s->words[6]!=012003 || io->interrupt_enabled>1 || io->superbank>0160 ||
    (io->superbank&017)!=0) return CM_STARTUP_OUT_OF_SCOPE;
 if(entry>=CM_STARTUP_MR_KLEAN) { clean(s,io,entry);return CM_STARTUP_OK; }
 old10=s->words[0106];old11=s->words[0107];
 if(old10>AGC_MASK || old11>AGC_MASK) return CM_STARTUP_OUT_OF_SCOPE;
 if(entry==CM_STARTUP_SLAP1) {
  cm_executive e;cm_waitlist w;cm_restart_state record;
  cm_native_gather_executive(s,io,&e);cm_native_gather_waitlist(s,io,&w);cm_native_gather_restart(s,io,&record);
  cm_rs_startsub(&record,&e,&w);
  cm_native_commit_executive(s,io,&e);cm_native_commit_waitlist(s,io,&w);cm_native_commit_restart(s,io,&record);
  s->words[01036]=(agc_word)((s->words[01036]&050u)+040000u);
  s->words[01365]=s->words[0375]=s->words[0376]=s->words[0377]=s->words[01154]=0;
  s->words[01302]=agc_negate(012000);
 }
 for(i=0;i<sizeof zero_addresses/sizeof zero_addresses[0];++i) s->words[zero_addresses[i]]=0;
 ports->channel5=ports->channel6=0;io->channel11=io->channel12=io->channel13=io->channel14=0;
 if((agc_word)(agc_negate(s->words[01036])&050)==0) io->channel12=050;
 clean(s,io,CM_STARTUP_MR_KLEAN);
 s->words[01011]=AGC_NEG_ZERO;s->words[0366]=030000;
 s->words[01320]=037411;s->words[01303]=077776;s->words[01331]=0130;s->words[01321]=016000;
 s->words[01312]=03153;s->words[01313]=012066;
 for(i=0;i<12;++i) {
  if(i==1) s->words[075]&=04000;
  else if(i==3) s->words[077]&=010000;
  else if(i==5) s->words[0101]=0200;
  else if(i==8) s->words[0104]&=06200;
  else s->words[074u+i]=0;
 }
 s->words[0]=agc_extend(old10);s->words[1]=agc_extend(old11);s->words[2]=3;
 return CM_STARTUP_OK;
}
