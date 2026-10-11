/* SPDX-License-Identifier: MIT; input-only constructor, no native algorithm. */
#include "service_routines_cases.h"
#include "service_routines_source.h"
#include "service_routines_fixed_inputs.h"
unsigned service_routines_kind(uint32_t n){return (unsigned)(n/4096u);}
void service_routines_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t n,enum service_routines_mission *mission,enum service_routines_entry *entry,agc_word fixed[3]){
 const service_routines_bindings *b;uint16_t local=(uint16_t)(n%4096u),i,q,slot,source,dest,offset;unsigned kind=service_routines_kind(n);static const agc_register corners[8]={0,0177777,040000,077777,0100000,0137777,1,0177776};static const agc_word alias[8]={061,062,063,01044,0100,0140,0,0};
 *mission=kind<11u||kind==24u?SERVICE_ROUTINES_LM:SERVICE_ROUTINES_CM;*entry=(enum service_routines_entry)(kind>=24u?SERVICE_GENTRAN:kind<11u?kind:kind-11u);b=&service_routines_source[*mission];
 for(i=0;i<2048u;++i)s->words[i]=(agc_word)(((uint32_t)i*71u+(local%16u)*917u+012345u)&AGC_MASK);
 q=(agc_word)(0100u+local%960u);s->words[0]=local<8u?corners[local]:(agc_register)((uint32_t)local*997u);s->words[1]=(agc_register)((uint32_t)local*313u);s->words[2]=q;s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));s->words[7]=0;io->superbank=(agc_word)(((local>>8)&7u)*020u);io->interrupt_enabled=(agc_word)((local>>5)&1u);fixed[0]=fixed[1]=fixed[2]=0;
 switch(*entry){case SERVICE_UPENT2:s->words[5]=b->upent2;break;case SERVICE_DOWNENT2:s->words[5]=b->downent2;break;case SERVICE_UPFLAG:s->words[5]=b->upflag;break;case SERVICE_DOWNFLAG:s->words[5]=b->downflag;break;case SERVICE_DELAYJOB:s->words[5]=b->delayjob;break;case SERVICE_TCGETCAD:s->words[5]=b->tcgetcad;break;case SERVICE_WAKER:s->words[5]=b->waker;break;case SERVICE_WAKER_FINISH:s->words[5]=(agc_word)(b->waker+4u);break;case SERVICE_GENTRAN:s->words[5]=b->gentran;break;case SERVICE_GENTRAN_LOOP:s->words[5]=(agc_word)(b->gentran+1u);break;case SERVICE_B5OFF:s->words[5]=b->b5off;break;case SERVICE_TRFAILOF:s->words[5]=b->trfailof;break;default:s->words[5]=b->trfailon;break;}
 if(*entry==SERVICE_UPENT2||*entry==SERVICE_DOWNENT2){if(*mission==SERVICE_ROUTINES_LM)s->words[4]=020000;}
 if(*entry==SERVICE_UPFLAG||*entry==SERVICE_DOWNFLAG){i=q<01400u?q:(agc_word)(s->words[3]+(q&0377u));while(i<0100u){++q;i=q<01400u?q:(agc_word)(s->words[3]+(q&0377u));}s->words[2]=q;s->words[i]=(agc_word)(local%210u);if((local&15u)==15u){const uint16_t (*f)[2]=*mission==SERVICE_ROUTINES_LM?service_flag_fixed_0:service_flag_fixed_1;s->words[2]=f[(local>>4)&31u][0];fixed[0]=f[(local>>4)&31u][1];}}
 if(*entry>=SERVICE_DELAYJOB&&*entry<=SERVICE_WAKER_FINISH){s->words[4]=0;slot=(agc_word)(local%(b->delay_count+2u));for(i=0;i<=b->delay_count;++i)s->words[b->delayloc+i]=local&1u?1:077776;if(slot<=b->delay_count)s->words[b->delayloc+slot]=local&2u?0:AGC_NEG_ZERO;s->words[070]=(agc_word)(local%(b->delay_count+1u));s->words[0]=local<8u?corners[local]:(agc_register)((uint32_t)local*997u);s->words[b->delayloc-1u]=local&1u?1:0;if(*entry==SERVICE_TCGETCAD){s->words[0133]=(agc_word)(local*11u&01777u);s->words[0134]=(agc_word)(((uint32_t)local*131u)&AGC_MASK);}if(*entry==SERVICE_WAKER)s->words[3]=(agc_word)((local%(b->delay_count+1u))*0400u);}
 if(*entry==SERVICE_GENTRAN||*entry==SERVICE_GENTRAN_LOOP){
  offset=(agc_word)(local%16u);s->words[0]=offset;s->words[2]=q=01200;source=(agc_word)(0100u+(local/16u)%0300u);dest=(agc_word)(0500u+(local/16u)%0300u);if((local&7u)<6u){dest=alias[local&7u];source=0110;}
  if((local&7u)==6u){source=01400;dest=01500;offset=(agc_word)(local%64u);s->words[0]=offset;}
  if((local&7u)==7u){source=(agc_word)(04000u+(local>>3));fixed[2]=(*mission==SERVICE_ROUTINES_LM?service_data_fixed_0:service_data_fixed_1)[local>>3];}
  /* Base addresses yield one source/destination effective word. Exact
   * ITEMP1/2/3 and flag aliases include all CCS classes from copied data. */
  s->words[q]=(agc_word)((source-offset)&AGC_MASK);s->words[q+1u]=(agc_word)((dest-offset)&AGC_MASK);
  i=source<01400u?source:(agc_word)(s->words[3]+(source&0377u));if(source<02000u)s->words[i]=(agc_word)(local%4u==0?0u:local%4u==1?AGC_NEG_ZERO:local%4u==2?1u:AGC_MASK-1u);
 }
 if(kind>=24u){
  offset=(agc_word)(local%64u);s->words[0]=offset;s->words[2]=q=01200;s->words[5]=b->gentran;source=(agc_word)(0400u+(local>>6)%0400u);dest=(agc_word)(source+((local&0100u)?010u:0200u));
  if(local&0200u){source=01400;dest=01500;if(s->words[3]==0)s->words[3]=0400;}
  s->words[q]=source;s->words[q+1u]=dest;
 }
 s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));
}
