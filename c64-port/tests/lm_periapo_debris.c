/* SPDX-License-Identifier: MIT */
#include "lm_periapo_cases.h"
#define W m->words
uint32_t lm_periapo_random(uint32_t *seed){*seed^=*seed<<13;*seed^=*seed>>17;*seed^=*seed<<5;return *seed;}
/* Each routine's bank, LOC and Luminary call returns (QPRET). */
static const agc_word q_periapo1[5]={070043,070444,071473,072517,072706},q_periapo[1]={070254},
 q_lemstore[1]={046366},q_csmstore[1]={046374},q_vecshift[3]={070241,070356,071164},
 q_shiftr1[11]={070045,070053,070256,070273,070323,070446,071215,071475,072521,072527,072710},
 q_maxchk[6]={045600,045612,070047,070055,072523,072531};
void lm_periapo_debris(apollo_overlay_state *m,p66_bias_io *io,uint32_t n){
 uint32_t seed=UINT32_C(0x50455249)+(n/8u)*UINT32_C(0x9e3779b9);
 unsigned i,vac,kind=(unsigned)(n%8u),moon=(unsigned)((n/8u)&1u),k=(unsigned)(n/16u);
 const agc_word *q;unsigned count;agc_word fb=046000,loc,mode;
 for(i=0;i<2048u;++i)W[i]=(agc_word)(lm_periapo_random(&seed)&AGC_MASK);
 W[0]=(agc_register)(n*257u);W[1]=(agc_register)(n*977u);W[2]=(agc_word)(n&AGC_MASK);
 switch(kind){
 case 0:case 1:case 2:loc=02316;q=q_periapo1;count=5;mode=077776;break;
 case 3:loc=02326;q=q_periapo;count=1;mode=k%2u?077776u:0u;break;
 case 4:loc=02416;q=q_vecshift;count=3;mode=077776;break;
 case 5:loc=02426;q=q_shiftr1;count=11;mode=0;break;
 case 6:fb=044000;loc=03636;q=q_maxchk;count=6;mode=0;break;
 default:
  if(moon){loc=02406;q=q_csmstore;}else{loc=02376;q=q_lemstore;}
  count=1;mode=k%2u?077776u:0u;break;
 }
 /* The caller's CALL or STCALL: NEWOPS at the routine. */
 W[3]=(agc_word)(((n/32u)%8u)*0400u);W[4]=fb;W[6]=(agc_word)(fb|(W[3]>>8));W[0165]=W[6];
 W[5]=06070;W[0164]=loc;W[0115]=(agc_word)(fb&040000u);W[023]=0;W[7]=0;W[067]=0;W[0163]=mode;
 vac=0401u+((n/256u)%5u)*054u;W[0120]=(agc_word)vac;
 /* PUSHLOC: above the pushed position vector where the routine pops it. */
 W[0166]=(agc_word)(vac+6u+((n/64u)%8u)*2u);
 /* RTX1/RTX2 (E5,1776-1777) as the callers' SETs leave them, Earth (-2, 0)
  * or Moon (-10, 2); X1 from RTX1. */
 W[02776]=moon?077765u:077775u;W[02777]=moon?2u:0u;
 W[vac+046u]=W[02776];W[vac+052u]=q[k%count];
 /* OVFIND as overflow leaves it: usually clear, sometimes +-1. */
 W[0121]=(n/1024u)%8u==3u?1u:(n/1024u)%8u==6u?077776u:0u;
 io->interrupt_enabled=(uint16_t)((n/2048u)&1u);
 /* SUPERBNK 3 for the bank 30-37 callers, sometimes another value. */
 io->superbank=(uint16_t)((n/4096u)%4u==0u?((n/16384u)%8u)*020u:060u);
}
