/* SPDX-License-Identifier: MIT */
#include "dsky_sign_output.h"
#include "dsky_pinball.h"
static uint8_t sign_store(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{
 uint16_t *w=s->words;p66_bias_io arithmetic;
 if(w[01023u+w[0]]==0){w[0114]=w[2];w[0141]=w[0];w[0143]=2;w[0]=0;w[2]=mission==DSKY_NV_LM?03354u:03261u;w[5]=mission==DSKY_NV_LM?05705u:05747u;io->interrupt_enabled=0;return 0;}
 arithmetic.superbank=0100;arithmetic.interrupt_enabled=io->interrupt_enabled;(void)dsky_pinball_write(s,&arithmetic,DSKY_11DSPIN);io->interrupt_enabled=arithmetic.interrupt_enabled;return 1;
}
enum p66_bias_status dsky_sign_output(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,uint8_t negative,enum dsky_sign_output_route *route)
{
 uint16_t *w;agc_word caller,minus,plus,count;unsigned i;
 if(!s||!io||!route||(unsigned)mission>1u||negative>1u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;caller=w[2];count=w[0777];
 if(w[5]!=(agc_word)((mission==DSKY_NV_LM?02413u:02314u)+(negative?020u:0u))||w[4]!=060000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(060000u|(w[3]>>8))||w[7]||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||caller<0100u||caller>07777u||count>14u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=caller;w[0137]=count<5u?4u:count<10u?3u:2u;minus=count<5u?0u:count<10u?3u:5u;plus=(agc_word)(minus+1u);
 w[0123]=0;w[0122]=negative?minus:plus;w[0124]=0;w[0]=negative?plus:minus;w[2]=mission==DSKY_NV_LM?02426u:02327u;
 if(!sign_store(s,io,mission)){*route=DSKY_SIGN_CCSHOLE;return P66_BIAS_OK;}
 w[0124]=02000;w[0122]=02000;w[0]=negative?minus:plus;w[2]=mission==DSKY_NV_LM?02432u:02333u;
 if(!sign_store(s,io,mission)){*route=DSKY_SIGN_CCSHOLE;return P66_BIAS_OK;}
 w[2]=2;w[5]=caller;*route=DSKY_SIGN_CALLER;return P66_BIAS_OK;
}
