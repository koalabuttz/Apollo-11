/* SPDX-License-Identifier: MIT */
#include "gimbal_complete_cases.h"
#define GIMBAL_COMPLETE_FROZEN_IMPLEMENTATION
#if GIMBAL_COMPLETE_TARGET_CM
#include "gimbal_complete_vectors_cm.h"
#define MISSION APOLLO_FIXED_CM
#else
#include "gimbal_complete_vectors_lm.h"
#define MISSION APOLLO_FIXED_LM
#endif
static apollo_overlay_state state;
static uint16_t input_exceptions[8];
static uint16_t expected_words[79];
/* Reconstruct the input-only pattern for unowned words, retaining the eight
 * COF/AM exceptions before execution. This saves a second 4096-byte image. */
static uint16_t initial_word(uint16_t n,uint16_t i){uint16_t local=(uint16_t)(n%5120u),bank=(uint16_t)((local/64u)%8u),cof=MISSION==APOLLO_FIXED_LM?03270:03342,am=MISSION==APOLLO_FIXED_LM?03335:03364;
 if(i>=cof&&i<cof+6u)return input_exceptions[i-cof];
 if(i>=am&&i<am+2u)return input_exceptions[6u+i-am];
 if(i==(MISSION==APOLLO_FIXED_LM?01325u:01130u))return (uint16_t)(2u*(local%4u));
 switch(i){case 3:return (uint16_t)(bank<<8);case 4:return 044000;case 5:return MISSION==APOLLO_FIXED_LM?06070:06037;
 case 6:case 0165:return (uint16_t)(044000u|bank);case 7:case 023:case 067:return 0;case 0115:return 040000;
 case 0120:return (uint16_t)(0401u+(local/1024u)*054u);default:return (uint16_t)((251u*i+(local%16u)*947u+99u)&077777u);}}
int main(void){volatile unsigned char *mailbox=(volatile unsigned char *)0xc000;
 gimbal_io io;frozen_words reader;uint16_t row,n,i,k,base,matrix,br,tm,expected,checked=0,super,irq,out;
 __asm__("sei");*(volatile unsigned char *)1u&=0xfeu;mailbox[0]=0xff;
 for(row=0;row<GIMBAL_COMPLETE_FROZEN_ROWS;++row){n=gimbal_complete_target_ids[row];gimbal_complete_prepare(&state,&io,n);
 for(i=0;i<6u;++i)input_exceptions[i]=state.words[(MISSION==APOLLO_FIXED_LM?03270u:03342u)+i];
 for(i=0;i<2u;++i)input_exceptions[6u+i]=state.words[(MISSION==APOLLO_FIXED_LM?03335u:03364u)+i];
 base=state.words[0120];matrix=MISSION==APOLLO_FIXED_LM?02230:03425;br=MISSION==APOLLO_FIXED_LM?03325:03310;tm=MISSION==APOLLO_FIXED_LM?03333:03316;
 if(!gimbal_complete_reference_begin(&reader,row))goto failed;
 for(k=0;k<79u;++k)if(!frozen_words_next(&reader,&expected_words[k]))goto failed;
 if(!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_finished(&reader))goto failed;
 if(!gimbal_complete(&state,&io,MISSION))goto failed;
 k=0;for(i=0;i<2048u;++i){expected=initial_word(n,i);
 if(k<35u&&i==gimbal_complete_changed[k])expected=expected_words[k++];
 if(i>=base&&i<base+10u)expected=expected_words[35u+i-base];
 if(i==base+046u)expected=expected_words[45];if(i==base+052u)expected=expected_words[46];
 if(i>=matrix&&i<matrix+18u)expected=expected_words[47u+i-matrix];
 if(i>=br&&i<br+6u)expected=expected_words[65u+i-br];
 if(i>=tm&&i<tm+2u)expected=expected_words[71u+i-tm];
 if(i>=03274u&&i<03302u)expected=expected_words[73u+i-03274u];
 if(state.words[i]!=expected)goto failed;}
 if(k!=35u||io.superbank!=super||io.interrupt_enabled!=irq)goto failed;++checked;}
 for(i=0;i<(MISSION==APOLLO_FIXED_LM?10u:30u);++i){if(!gimbal_fixed_read(MISSION,(uint16_t)((MISSION==APOLLO_FIXED_LM?18937u:18936u)+i),&out)||out!=gimbal_complete_constant_words[i])goto failed;}
 mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xa5;for(;;){}
 failed:mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xee;for(;;){}
}
