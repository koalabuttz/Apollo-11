/* SPDX-License-Identifier: MIT
 * Literal KEYCOM/ACCEPTUP source stores composed with original native NOVAC. */
#include "dsky_keycom.h"
static void executive_words(executive *e,uint16_t *w,uint8_t read){uint8_t i,j;
 for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}
 for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}
 if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}
 else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}}
enum exec_status dsky_keycom(apollo_overlay_state *image,p66_bias_io *io){uint16_t *w=image->words,key=w[0],old_l=w[066],old_bank;executive e;uint8_t slot;enum exec_status status;
 if(key>037 || w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=010000 || w[6]!=(010000u|(w[3]>>8)) || io->superbank>0160 || (io->superbank&017u)!=0 || io->interrupt_enabled>1 || (w[067]!=077777 && (w[067]>84 || w[067]%12u!=0)))return EXEC_OUT_OF_SCOPE;
 executive_words(&e,w,1);for(slot=0;slot<EXEC_CORE_COUNT;++slot)if(e.core[slot][EXEC_PRIORITY]==077777)break;old_bank=slot<EXEC_CORE_COUNT ? e.core[slot][EXEC_BANK] : 0;
 w[073]=key;w[0101]=(uint16_t)(w[0101]|040000u);
 status=exec_admit(&e,030000,02077,060101,0);executive_words(&e,w,0);
 if(status==EXEC_OK){w[061]=010000;w[062]=(uint16_t)(7u-slot);w[0154+slot*12u]=key;w[0]=key;w[1]=agc_extend(old_bank);w[2]=03317;w[5]=05270;}
 else{w[061]=old_l;w[062]=0;w[0]=03311;w[1]=010000;w[2]=02703;w[4]=02000;w[6]=(uint16_t)(02000u|(w[3]>>8));w[5]=05716;}
 w[7]=0;io->interrupt_enabled=0;return status;
}
