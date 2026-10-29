/* SPDX-License-Identifier: MIT */
#include "lm_basic_wake.h"
static void executive_words(executive *e,uint16_t *w,uint8_t read){uint8_t i,j;for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154u+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400u+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}}
enum exec_status lm_basic_wake(apollo_overlay_state *s,p66_bias_io *io){
 uint16_t *w=s->words,caller=w[2],oldl=w[1],oldfb=w[4],wake=agc_correct(w[0]),savedbank=0;
 uint8_t i,slot=8;executive e;enum exec_status status;
 if(w[5]!=05137u||w[0]!=agc_extend(wake)||caller<0100u||caller>07777u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||w[0167]==0||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||(w[067]!=AGC_NEG_ZERO&&(w[067]>84u||w[067]%12u)))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<8u;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return EXEC_OUT_OF_SCOPE;if(slot==8u&&p>=040000u&&p<AGC_NEG_ZERO&&(loc==wake||((loc==0||loc==AGC_NEG_ZERO)&&(wake==0||wake==AGC_NEG_ZERO)))){slot=i;savedbank=w[0165u+12u*i];}}
 executive_words(&e,w,1);status=exec_wake_basic(&e,wake);if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND)return status;
 if(status==EXEC_OK)e.newloc[1]=agc_correct(oldl);
 executive_words(&e,w,0);w[061]=oldfb;w[062]=(uint16_t)(slot<8u?7u-slot:0u);
 w[0]=agc_extend(oldfb);w[1]=slot<8u?agc_extend(savedbank):agc_extend(agc_correct(oldl));w[2]=06743;w[5]=caller;io->interrupt_enabled=0;
 return status;
}
