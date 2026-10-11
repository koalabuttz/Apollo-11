/* SPDX-License-Identifier: MIT */
#include "dsky_five_blank.h"
static enum p66_bias_status five_blank(apollo_overlay_state *s,p66_bias_io *io,enum dsky_five_blank_route *route,uint8_t cm){static const uint16_t singular[]={14,5,4},paired[]={13,9,3},first[]={14,9,4},bit[]={020,010,4};uint16_t *w=s->words,i,pc=w[5],base=cm ? 02437u : 02536u,plus=(uint16_t)(base+2u),count=w[0777],caller=w[2],rel,idx,relative,dspin_caller=(uint16_t)(base+021u);p66_bias_io physical;
 if(route==0||(pc!=base&&pc!=(uint16_t)(base-1u)&&pc!=plus&&pc!=(cm ? 02426u : 02525u))||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc==plus){rel=w[0137];caller=w[1];if(rel<2u||rel>4u)return P66_BIAS_OUT_OF_SCOPE;}
 else{if(pc==(uint16_t)(base-1u)){count=agc_correct(w[0]);if(w[0]!=agc_extend(count))return P66_BIAS_OUT_OF_SCOPE;}if(count>14u)return P66_BIAS_OUT_OF_SCOPE;rel=count<5u ? 4u : count<10u ? 3u : 2u;}
 if(caller<0100u||caller>07777u)return P66_BIAS_OUT_OF_SCOPE;
 if(pc==(uint16_t)(base-1u))w[0777]=count;
 w[1]=caller;w[0137]=rel;idx=(uint16_t)(rel-2u);w[01001u+rel]=0;w[01004u+rel]=0;w[0124]=0;w[01000]=(uint16_t)(w[01000]&agc_negate(bit[idx])&077774u);w[0143]=singular[idx];w[2]=dspin_caller;relative=(uint16_t)(singular[idx]>>1);
 if(w[01023u+relative]==0){w[021]=relative;w[0141]=relative;w[0114]=dspin_caller;w[0143]=(singular[idx]&1u)!=0 ? 1u : 0u;w[0]=0;w[2]=cm ? 03261u : 03354u;w[5]=cm ? 05747u : 05705u;io->interrupt_enabled=0;*route=DSKY_FIVE_BLANK_CCS_HOLE;return P66_BIAS_OK;}
 physical.superbank=0100;physical.interrupt_enabled=io->interrupt_enabled;(void)dsky_pinball_write(s,&physical,DSKY_DSPIN);w[0777]=paired[idx];w[2]=(uint16_t)(dspin_caller+4u);(void)dsky_pinball_write(s,&physical,DSKY_2BLANK);w[0777]=(uint16_t)(paired[idx]-2u);w[2]=(uint16_t)(dspin_caller+7u);(void)dsky_pinball_write(s,&physical,DSKY_2BLANK);io->interrupt_enabled=physical.interrupt_enabled;w[0777]=first[idx];w[0]=first[idx];w[2]=2;w[5]=caller;*route=DSKY_FIVE_BLANK_CALLER;return P66_BIAS_OK;
}
enum p66_bias_status dsky_five_blank(apollo_overlay_state *s,p66_bias_io *io,enum dsky_five_blank_route *route){return five_blank(s,io,route,0);}
enum exec_status cm_five_blank(apollo_overlay_state *s,cm_native_io *io,enum dsky_five_blank_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=five_blank(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
