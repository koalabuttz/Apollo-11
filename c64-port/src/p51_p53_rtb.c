/* SPDX-License-Identifier: MIT */
/* Native P51-P53 RTB routines (include/p51_p53_rtb.h). Each instruction
 * below is the original's, on the canonical image. */
#include "p51_p53_rtb.h"
#include "agc_image_ops.h"
#include "rtb_angles.h"
#define W s->words
uint8_t p51_p53_rtb(apollo_overlay_state *s,p66_bias_io *io,enum p51_p53_rtb_kind kind){
 static const agc_word at[3]={03413,02324,03275},fb[3]={030000u,032000u,030000u},danzig[3]={06061,06030,06030};
 unsigned k=(unsigned)kind,i,fl;
 if(!s||!io||k>2u||W[5]!=at[k]||W[4]!=fb[k]||io->interrupt_enabled>1u||W[7]||W[3]>03400u||(W[3]&0377u))return RTB_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return RTB_OUT_OF_SCOPE;
 if(k==P51_CM_SET1PDT){
  /* CA TIME1; TS 1/PIPADT; TCF DANZIG. */
  ai_ca(s,025);(void)ai_ts(s,01074);W[5]=danzig[k];return RTB_OK;
 }
 fl=W[0120];
 if(fl<0401u||fl>0661u)return RTB_OUT_OF_SCOPE;
 /* INHINT; CA CDUX; INDEX FIXLOC; TS 1; CA CDUY; INDEX FIXLOC; TS 2;
  * CA CDUZ; INDEX FIXLOC; TS 3; RELINT; TC DANZIG. */
 io->interrupt_enabled=0u;
 for(i=0;i<3u;++i){ai_ca(s,032u+i);(void)ai_ts(s,fl+1u+i);}
 io->interrupt_enabled=1u;
 ai_tc(s,(agc_word)(at[k]+014u));W[5]=danzig[k];
 return RTB_OK;
}
