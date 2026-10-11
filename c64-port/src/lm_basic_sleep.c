/* SPDX-License-Identifier: MIT */
#include "lm_basic_sleep.h"
static void executive_words(executive *e,uint16_t *w,uint8_t read){uint8_t i,j;for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154u+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400u+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}}
enum exec_status lm_basic_sleep(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,exec_address *next){
 uint16_t *w=s->words,wake=agc_correct(w[0]),caller=w[2],oldl=w[1],highest=0,buf0=w[0130],buf1=AGC_NEG_ZERO,buf2=w[0132],q=caller;
 uint8_t i,winner=0;executive e;exec_address result;enum exec_status status;
 static const uint16_t calls[7]={03122,03126,03132,03136,03142,03146,03152};
 if(next==0||w[5]!=05133u||w[0]!=agc_extend(wake)||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||w[0167]==0||w[0167]>=040000u)return EXEC_OUT_OF_SCOPE;
 for(i=1;i<8u;++i){uint16_t priority=w[0167u+12u*i];if(priority==0u)return EXEC_OUT_OF_SCOPE;if(priority<040000u){buf2=(uint16_t)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03206;}else q=03201;}}
 if(winner&&(w[0165u+winner*12u]&(uint16_t)~076167u))return EXEC_OUT_OF_SCOPE;
 executive_words(&e,w,1);
 /* Source CCS(priority)-1 makes raw priority1 a zero candidate. Exclude it
  * from the typed scan, then retain its original canonical priority below. */
 for(i=1;i<8u;++i)if(w[0167u+12u*i]==1u)e.core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
 status=exec_sleep_basic(&e,wake,(agc_word)(w[6]|io->superbank),&result);if(status!=EXEC_OK&&status!=EXEC_IDLE)return status;
 for(i=1;i<8u;++i)if(w[0167u+12u*i]==1u)e.core[i][EXEC_PRIORITY]=1;
 executive_words(&e,w,0);w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[2]=q;
 if(status==EXEC_IDLE){w[4]=02000;w[6]=(uint16_t)(02000u|(w[3]>>8));w[0]=agc_extend((agc_word)(*channel11&077775u));w[1]=agc_extend(agc_correct(oldl));w[5]=03214;*channel11=(agc_word)(*channel11&077775u);}
 else{w[0]=02772;w[1]=(agc_word)(02000u|(w[3]>>8));w[5]=result.location;w[6]=(agc_word)(result.bank&076007u);w[3]=(agc_word)((w[6]&7u)*0400u);w[4]=(agc_word)(w[6]&076000u);io->superbank=(agc_word)(result.bank&0160u);}
 io->interrupt_enabled=1;*next=result;return status;
}
