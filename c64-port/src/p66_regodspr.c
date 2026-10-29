/* SPDX-License-Identifier: MIT */
#include "p66_regodspr.h"
static void executive_words(executive *e,uint16_t *w,uint8_t read) {
 uint8_t i,j;
 for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}
 for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}
 if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}
 else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}
}
enum exec_status p66_regodspr(apollo_overlay_state *image,p66_bias_io *io){
 uint16_t *w=image->words;executive e;uint8_t slot;agc_word priority=(agc_word)(w[0167]&037000u),old_newlow=w[066];enum exec_status result;
 if(w[5]!=02470u||w[0]!=01474u||w[2]!=04631u||w[3]!=03400u||w[4]!=020000u||w[6]!=020007u||w[7]!=0||w[0133]!=03476u||w[0134]!=062000u||w[0163]!=0||w[067]!=0||w[0167]>=040000u||priority==0||io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 executive_words(&e,w,1);for(slot=1;slot<EXEC_CORE_COUNT;++slot)if(e.core[slot][EXEC_PRIORITY]==AGC_NEG_ZERO)break;
 w[0155]=01474;w[0160]=020000;w[072]=0;
 result=exec_admit(&e,priority,02546,020067,0);executive_words(&e,w,0);
 if(result==EXEC_OK){uint16_t base=(uint16_t)(0154u+slot*12u);
  w[061]=020000;w[062]=(uint16_t)(7u-slot);
  w[0155]=01474;w[0157]=063476;w[0160]=023400;
  w[base+1u]=w[0155];w[base+2u]=w[0156];w[base+3u]=w[0157];w[base+4u]=w[0160];
  /* DXCH writes the new core while A/L receive its prior words;
   * current MPAC retains the saved source arguments. */
  w[0157]=063476;w[0160]=023400;w[0161]=(agc_word)(slot*12u);
  w[0]=2;w[1]=02701;w[2]=01476;w[4]=062000;w[6]=062007;w[5]=05155;io->interrupt_enabled=1;
 }else{w[061]=old_newlow;w[062]=0;w[0]=02654;w[1]=020000;w[2]=02703;w[0155]=01474;w[0160]=020000;w[4]=02000;w[6]=02007;w[5]=05716;io->interrupt_enabled=0;}
 return result;
}
