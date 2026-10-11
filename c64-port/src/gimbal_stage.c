/* SPDX-License-Identifier: MIT
 * Compiled original entry stages and return metadata, no instruction decoder. */
#include "gimbal_stage.h"
#include "guidance_shift.h"
#include "agc_word.h"
#include "gimbal_data.inc"
uint16_t gimbal_fixed_read(enum apollo_fixed_mission mission,uint16_t address,uint16_t *out){
 const uint16_t *words;uint16_t first,count;
 if(out==0)return 0;
 if(mission==APOLLO_FIXED_LM){words=gimbal_lm_words;first=GIMBAL_LM_DATA_START;count=10;}
 else if(mission==APOLLO_FIXED_CM){words=gimbal_cm_words;first=GIMBAL_CM_DATA_START;count=30;}
 else return 0;
 if(address<first || (uint16_t)(address-first)>=count)return 0;
 *out=words[address-first];return 1;
}
uint16_t gimbal_entry(apollo_overlay_state *image,gimbal_io *io,enum apollo_fixed_mission mission){
 uint16_t *w,base,rate,start,callee,table,i,set_flag; p66_preparation transient={0};p66_scale_scratch scratch;
 if(image==0 || io==0 || (mission!=APOLLO_FIXED_LM && mission!=APOLLO_FIXED_CM))return 0;
 w=image->words;base=w[0120];start=mission==APOLLO_FIXED_LM?02746:02737;
 callee=mission==APOLLO_FIXED_LM?02527:02530;table=mission==APOLLO_FIXED_LM?02771:02770;
 rate=w[mission==APOLLO_FIXED_LM?01325:01130];
 set_flag=(uint16_t)(mission==APOLLO_FIXED_LM && w[0164]==02744);
 if(w[3]>03400u || (w[3]&0377u)!=0 || w[4]!=044000 || w[6]!=(uint16_t)(044000u|(w[3]>>8)) ||
    w[5]!=(mission==APOLLO_FIXED_LM?06070u:06037u) || w[0165]!=w[6] || w[0115]!=040000 ||
    (w[0164]!=start && !set_flag) || w[023]!=0 || w[067]!=0 || w[7]!=0 ||
    base<0401u || base>0661u || (uint16_t)(base-0401u)%054u!=0 || rate>6u || (rate&1u)!=0 ||
    io->interrupt_enabled>1u || (io->superbank&~0160u)!=0)return 0;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return 0;
 if(set_flag){
  /* Original CALCMAN3 selector42: STATE word2, bit3. SET restores RELINT. */
  w[076]=(uint16_t)(w[076]|4u);w[0131]=4;io->interrupt_enabled=1;
 }
 /* LXC,1 stores the complement in the selected VAC index register. */
 w[base+046u]=(uint16_t)(rate^077777u);
 /* DLOAD* loads the exact selected fixed pair, clears MPAC2 and DP MODE. */
 for(i=0;i<8u;++i)transient.mpac[i]=w[0154u+i];
 transient.mpac[0]=mission==APOLLO_FIXED_LM?gimbal_lm_words[rate]:gimbal_cm_words[rate];
 transient.mpac[1]=mission==APOLLO_FIXED_LM?gimbal_lm_words[rate+1u]:gimbal_cm_words[rate+1u];
 transient.mpac[2]=0;transient.mpac[7]=0;
 scratch.mptemp=w[0135];scratch.ovfind=w[0121];
 (void)p66_scalar_shift_right(&transient,&scratch,4,1);
 for(i=0;i<8u;++i)w[0154u+i]=transient.mpac[i];
 w[0135]=scratch.mptemp;
 /* Known original SR4/CALL dispatch bookkeeping. These are compiled source
  * operation/address constants, not a lookup of expected state fixtures.
  * SR4 uses SR=count-1. CALL's CYR prefix ends050006, TCQ leaves Q3.
  * CALL saves original positive FCADR of the next operation pair in QPRET;
  * GOTO/INTPRET+3 restores BANKSET and leaves signextended BIT15 in A. */
 w[021]=3;w[020]=050006;w[2]=3;w[1]=transient.rate.registers.l;w[0]=agc_extend(040000);
 w[0116]=(uint16_t)(table+rate);w[0117]=(uint16_t)(044000u+callee-02000u);
 w[0130]=base;w[base+052u]=(uint16_t)(044000u+start+5u-02000u);
 w[0164]=callee;
 return 1;
}
