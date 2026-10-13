/* SPDX-License-Identifier: MIT */
#include "lm_t4rupt_rr.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
/* Erasable (M11-M32 in the EB window: EBANK= M11, E6 when T4RUPT runs). */
#define CDUX 032u
#define CDUZ 034u
#define CDUT 035u
#define COSMG 061u
#define FLAGWRD0 074u
#define FLAGWRD5 0101u
#define FLAGWRD6 0102u
#define FLAGWRD7 0103u
#define RADMODES 0110u
#define M11 01414u
#define M21 01415u
#define M31 01416u
#define M22 01417u
#define M32 01420u
/* Fixed constants (tools/lm_t4rupt_rr_source_map.py checks each against the rope). */
#define ONE 01u
#define TWO 02u
#define BIT2 02u
#define AUTOMBIT 02u
#define NORRMBIT 010u
#define AVEGFBIT 020u
#define RCDUFBIT 0100u
#define RNDVZBIT 0100u
#define MUNFLBIT 0200u
#define REPOSBIT 02000u
#define OCT10001 010001u
#define OCT05776 05776u
#define OCT32002 032002u
#define OCT20002 020002u
/* Outside the file. */
#define ALARM 05567u
#define WAITLIST 05203u
#define SETTRKF 04564u
#define RRLIMCHK 04523u
#define SPSIN 05033u
#define SPCOS 05032u
#define RESUME 05270u
#define FB06 014000u

static agc_word a15(const apollo_overlay_state *s){return agc_correct(W[0]);}
static agc_word lchan(const apollo_overlay_state *s){return agc_correct(W[1]);}
static void rxor(apollo_overlay_state *s,agc_word v){W[0]=agc_extend((agc_word)(a15(s)^v));}
static void wand(apollo_overlay_state *s,agc_word *c){*c=(agc_word)(a15(s)&*c);W[0]=agc_extend(*c);}
static enum exec_status tc(apollo_overlay_state *s,agc_word at,agc_word target){ai_tc(s,(agc_word)(at+1u));W[5]=target;return EXEC_OK;}
/* A holds an overflow (bits 16 and 15 differ). */
static int overflow(const apollo_overlay_state *s){return ((W[0]>>15)^(W[0]>>14))&1u;}

static enum exec_status rr(apollo_overlay_state *s,agc_hardware *h,agc_word z){
 switch(z){
  case 03006u:goto RRAUTCHK;case 03032u:case 03114u:case 03126u:case 03132u:goto GPMATRIX;
  case 03070u:goto TRKFLCDU;case 03071u:goto RRGIMON;case 03113u:goto MONREPOS;
  case 03134u:goto L3134;case 03137u:goto L3137;case 03142u:goto L3142;case 03151u:goto L3151;
  default:return EXEC_OUT_OF_SCOPE;}

RRAUTCHK:
 ai_ca(s,RADMODES);rxor(s,h->channel33);ai_maskf(s,AUTOMBIT);
 if(ai_bzf(s))goto RRCDUCHK;
 ai_lxch(s,RADMODES);rxor(s,lchan(s));ai_maskf(s,OCT05776);(void)ai_ts(s,RADMODES);
 ai_maskf(s,BIT2);
 if(ai_ccs(s,0u)==0u)goto RRCDUCHK_M3;   /* a masked bit: + or +0 */
 ai_caf(s,OCT10001);ai_ads(s,RADMODES);ai_caf(s,ONE);
 return tc(s,03027u,WAITLIST);           /* 2CADR RRTURNON */
RRCDUCHK_M3:
 ai_csf(s,BIT2);wand(s,&h->channel12);
RRCDUCHK:
 ai_ca(s,RADMODES);rxor(s,h->channel30);ai_maskf(s,RCDUFBIT);
 if(ai_bzf(s))goto RRGIMON;
 ai_caf(s,AUTOMBIT);ai_mask(s,RADMODES);
 if(ai_ccs(s,0u)==0u)goto GPMATRIX;      /* TCF NORRGMON */
 ai_caf(s,RCDUFBIT);ai_lxch(s,RADMODES);rxor(s,lchan(s));(void)ai_ts(s,RADMODES);
 ai_ca(s,RADMODES);ai_maskf(s,RCDUFBIT);
 if(ai_ccs(s,0u)==0u)goto TRKFLCDU;
 ai_cs(s,FLAGWRD0);ai_maskf(s,RNDVZBIT);
 if(ai_ccs(s,0u)==0u)goto TRKFLCDU;
 return tc(s,03066u,ALARM);              /* TC ALARM; OCT 00515 */
TRKFLCDU:
 return tc(s,03070u,SETTRKF);
RRGIMON:
 ai_ca(s,FLAGWRD5);ai_maskf(s,NORRMBIT);
 if(ai_ccs(s,0u)==0u)goto GPMATRIX;
 ai_cs(s,FLAGWRD7);ai_maskf(s,AVEGFBIT);
 if(ai_ccs(s,0u)!=0u){                   /* +0: servicer running */
  ai_ca(s,FLAGWRD6);ai_maskf(s,MUNFLBIT);
  if(ai_ccs(s,0u)==0u)goto GPMATRIX;}
 ai_caf(s,OCT32002);ai_mask(s,RADMODES); /* +5 */
 if(ai_ccs(s,0u)==0u)goto GPMATRIX;
 return tc(s,03111u,RRLIMCHK);           /* ADRES CDUT */
MONREPOS:
 ai_caf(s,REPOSBIT);ai_ads(s,RADMODES);ai_csf(s,OCT20002);wand(s,&h->channel12);ai_caf(s,TWO);
 return tc(s,03123u,WAITLIST);           /* 2CADR DORREPOS */
GPMATRIX:
 ai_ca(s,CDUZ);return tc(s,03133u,SPSIN);
L3134:
 (void)ai_ts(s,M11);ai_ca(s,CDUZ);return tc(s,03136u,SPCOS);
L3137:
 (void)ai_ts(s,COSMG);ai_ca(s,CDUX);return tc(s,03141u,SPSIN);
L3142:
 (void)ai_ts(s,M22);ai_cs(s,M22);ai_mp(s,COSMG);(void)ai_ts(s,M31);ai_ca(s,CDUX);return tc(s,03150u,SPCOS);
L3151:
 (void)ai_ts(s,M32);ai_mp(s,COSMG);(void)ai_ts(s,M21);
 return tc(s,03155u,RESUME);
}

static enum exec_status entry(const agc_native_program *p,apollo_overlay_state *s,void *io){agc_hardware *h;agc_word z;(void)io;
 if(!p||!s||!(h=p->hardware)||W[4]!=FB06)return EXEC_OUT_OF_SCOPE;
 z=W[5];
 /* SPSIN and SPCOS return their result in A without overflow. */
 if((z==03134u||z==03137u||z==03142u||z==03151u)&&overflow(s))return EXEC_OUT_OF_SCOPE;
 return rr(s,h,z);
}
#define B6(z) (uint16_t)(06u*1024u+((z)-02000u))
static const agc_routine_entry entries[LM_T4RUPT_RR_ENTRIES]={
 {B6(03006u),entry},{B6(03032u),entry},{B6(03070u),entry},{B6(03071u),entry},{B6(03113u),entry},{B6(03114u),entry},
 {B6(03126u),entry},{B6(03132u),entry},{B6(03134u),entry},{B6(03137u),entry},{B6(03142u),entry},{B6(03151u),entry}};
const agc_routine_table lm_t4rupt_rr_routines={entries,LM_T4RUPT_RR_ENTRIES};
