/* SPDX-License-Identifier: MIT */
/* Native Luminary099 INTEGRATION_INITIALIZATION state-vector copies (bank
 * 13): MOVEACSM, MOVEPCSM, MOVEALEM, MOVEPLEM and SETBANK, as RTB runs
 * them. Each instruction below is the original's, on the canonical image. */
#include "lm_integration_moves.h"
#include "agc_image_ops.h"
#include "rtb_angles.h"
#define W s->words
/* Erasable (EB 3 window): DIFEQCNT 1500, RRECT 1502, RRECTCSM 1554,
 * RRECTLEM 1626. INTBANK BBCON INTEGRV (13,3042) = 026063; FORTYONE 051. */
uint8_t lm_integration_move(apollo_overlay_state *s,p66_bias_io *io,enum lm_integration_move_kind kind){
 static const uint16_t at[4]={02674,02747,02723,02770},from[4]={01502,01502,01554,01626},to[4]={01554,01626,01502,01502};
 unsigned k=(unsigned)kind,i;
 if(!s||!io||k>3u||W[5]!=at[k]||W[4]!=026000u||io->interrupt_enabled>1u||W[7]||W[3]>03400u||(W[3]&0377u))return RTB_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return RTB_OUT_OF_SCOPE;
 /* TC SETBANK; SETBANK CAF INTBANK; TS BBANK; CAF FORTYONE; TC Q. */
 ai_tc(s,(agc_word)(at[k]+1u));ai_caf(s,026063);(void)ai_ts(s,6);ai_caf(s,051);ai_tc(s,3);
 /* TS DIFEQCNT; INDEX DIFEQCNT, CA from; INDEX DIFEQCNT, TS to; CCS
  * DIFEQCNT; TCF +1 (positive); TC DANZIG (+0). */
 for(;;){
  unsigned n;
  (void)ai_ts(s,01500);n=ai_word(s,01500);
  ai_ca(s,from[k]+n);(void)ai_ts(s,to[k]+n);
  if(ai_ccs(s,01500)!=0u)break;
 }
 ai_tc(s,(agc_word)(at[k]+011u));W[5]=06061;
 return RTB_OK;
}
uint8_t lm_integration_tcdanzig(apollo_overlay_state *s){
 if((W[5]&077777u)!=03753u||(W[4]&076000u)!=056000u)return RTB_OUT_OF_SCOPE;
 ai_tc(s,03754);W[5]=06061;return RTB_OK;
}
