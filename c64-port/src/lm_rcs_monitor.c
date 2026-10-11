/* SPDX-License-Identifier: MIT */
#include "lm_rcs_monitor.h"
static const agc_word mask5[8]={040,020,0100,0200,010,1,4,2};
static const agc_word mask6[8]={010,020,4,0200,1,2,040,0100};
static void executive_words(executive *e,uint16_t *w,uint8_t read)
{
    uint8_t i,j;
    for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){
        uint16_t address=(uint16_t)(0154+i*12u+j);
        if(read)e->core[i][j]=w[address];else w[address]=e->core[i][j];
    }
    for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){
        uint16_t address=(uint16_t)(0400+i*44u+j);
        if(read)e->vac[i][j]=w[address];else w[address]=e->vac[i][j];
    }
    if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}
    else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}
}
enum exec_status lm_rcs_monitor(apollo_overlay_state *image,p66_bias_io *io,agc_word channel32)
{
    uint16_t *w=image->words,current,changed,bit,old_l,old_bank;
    unsigned position=0;uint8_t slot;executive e;enum exec_status status;
    if(w[5]!=03156||w[4]!=014000||w[3]>03400||(w[3]&0377u)!=0||
       w[6]!=(014000u|(w[3]>>8))||w[7]!=0||w[01276]>0377||
       channel32>077777||io->interrupt_enabled!=0||io->superbank>0160||
       (io->superbank&017u)!=0||(w[067]!=077777&&(w[067]>84u||w[067]%12u!=0)))
        return EXEC_OUT_OF_SCOPE;
    executive_words(&e,w,1);
    for(slot=0;slot<EXEC_CORE_COUNT;++slot)if(e.core[slot][EXEC_PRIORITY]==0)return EXEC_OUT_OF_SCOPE;
    for(slot=0;slot<EXEC_CORE_COUNT;++slot)if(e.core[slot][EXEC_PRIORITY]==077777)break;
    if(slot==0&&w[067]==0)return EXEC_OUT_OF_SCOPE;
    current=(uint16_t)((channel32^077777u)&0377u);changed=(uint16_t)(current^w[01276]);
    w[0]=changed;w[1]=agc_extend(changed);w[2]=current;
    if(!changed){w[5]=05270;return EXEC_OK;}
    bit=0200;
    while(!(changed&bit)){bit>>=1;++position;}
    if(w[01276]&bit){w[01262]=(uint16_t)(w[01262]&agc_negate(mask5[position]));w[01263]=(uint16_t)(w[01263]&agc_negate(mask6[position]));w[01276]=(uint16_t)(w[01276]&agc_negate(bit));}
    else{w[01262]=(uint16_t)(w[01262]|mask5[position]);w[01263]=(uint16_t)(w[01263]|mask6[position]);w[01276]=(uint16_t)(w[01276]|bit);}
    old_l=w[066];old_bank=slot<EXEC_CORE_COUNT?e.core[slot][EXEC_BANK]:0;
    status=exec_admit(&e,027000,02454,040106,0);executive_words(&e,w,0);
    if((unsigned)status==(unsigned)EXEC_OK){w[061]=014000;w[062]=(uint16_t)(7u-slot);w[0]=014000;w[1]=agc_extend(old_bank);w[2]=06743;w[5]=05270;}
    else{w[061]=old_l;w[062]=0;w[0]=03240;w[1]=014000;w[2]=02703;w[4]=02000;w[6]=(uint16_t)(02000u|(w[3]>>8));w[5]=05716;}
    return status;
}
uint8_t lm_rcs_monitor_constant_read(uint16_t address,uint16_t *out)
{
    if(out==0)return 0;
    if(address==6816){*out=02454;return 1;}
    if(address==6817){*out=040106;return 1;}
    if(address>=6819&&address<6827){*out=mask5[address-6819];return 1;}
    if(address>=6827&&address<6835){*out=mask6[address-6827];return 1;}
    return 0;
}
