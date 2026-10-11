/* SPDX-License-Identifier: MIT
 * Fixed NEWMODEX/MMDSPLAY/SETUPDSP return path from STARTP66; direct native
 * Executive admission, no AGC instruction decoder or placeholder callback. */
#include "p66_major_mode.h"
static void executive_words(executive *e,uint16_t *w,uint8_t read) {
 uint8_t i,j;
 for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}
 for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}
 if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}
 else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}
}
enum exec_status p66_major_mode_start(apollo_overlay_state *image,p66_bias_io *io) {
 uint16_t *w=image->words;executive e;uint8_t slot;agc_word old_l=w[066],old_bank;enum exec_status status;
 if(w[2]!=02501 || w[1]!=01400 || w[3]!=03400 || w[4]!=062000 || w[6]!=062007 ||
    w[067]!=0 || w[0167]==0 || w[0167]>=040000 || io->superbank>060 ||
    (io->superbank&017u)!=0 || io->interrupt_enabled>1)return EXEC_OUT_OF_SCOPE;
 executive_words(&e,w,1);
 for(slot=0;slot<EXEC_CORE_COUNT;++slot)if(e.core[slot][EXEC_PRIORITY]==AGC_NEG_ZERO)break;
 old_bank=slot<EXEC_CORE_COUNT ? e.core[slot][EXEC_BANK] : 0;
 w[01011]=66; /* Literal original DEC66, including original signed-word form. */
 w[070]=02502;w[071]=062007;
 status=exec_admit(&e,030000,03534,060102,0);executive_words(&e,w,0);
 if(status==EXEC_OK){
  w[061]=020000;w[062]=(uint16_t)(7u-slot);
  /* SETUPDSP DXCH RUPTREG1 preserves ENDFIND's old slot BANKSET debris. */
  w[070]=020000;w[071]=old_bank;
  w[0]=02224;w[1]=020000;w[2]=06743;w[5]=02502;io->interrupt_enabled=1;
 }else{
  w[061]=old_l;w[062]=0;w[0]=02217;w[1]=020000;w[2]=02703;
  w[3]=0;w[4]=02000;w[5]=05716;w[6]=02000;io->interrupt_enabled=0;
 }
 w[7]=0;
 return status;
}
