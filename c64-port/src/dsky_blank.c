/* SPDX-License-Identifier: MIT */
#include "dsky_blank.h"
static enum p66_bias_status blank_body(apollo_overlay_state *s,p66_bias_io *io,enum dsky_blank_entry entry,uint8_t cm){
 static const uint16_t singular[]={14,5,4},paired[]={13,9,3},first[]={14,9,4},bit[]={020,010,4};
 uint16_t *w=s->words,i,a,l,start=cm ? 03412u : 03505u,ret=cm ? 05230u : 05166u;
 if((w[4]&~076000u)||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||io->interrupt_enabled>1u||io->superbank>0160u||(io->superbank&017u))return P66_BIAS_OUT_OF_SCOPE;
 if(entry==DSKY_BLANK_RETURN){
  a=agc_correct(w[0]);l=agc_correct(w[1]);
  if(w[5]!=ret||w[0]!=agc_extend(a)||(a&~076167u)||w[1]!=agc_extend(l)||l<0100u||l>07777u)return P66_BIAS_OUT_OF_SCOPE;
  io->superbank=(uint16_t)(a&0160u);w[6]=(uint16_t)(a&076007u);w[4]=(uint16_t)(a&076000u);w[3]=(uint16_t)((a&7u)<<8);w[2]=2;w[5]=l;return P66_BIAS_OK;
 }
 if(entry!=DSKY_BLNKSUB1||w[5]!=start||w[4]!=060000u||io->superbank!=0100u||w[0123]>7u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=0;i<3;++i)if((w[0123]&(1u<<i))&&w[01023+(singular[i]>>1)]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0132]=w[0777];
 for(i=0;i<3;++i)if(w[0123]&(1u<<i)){
  uint16_t relative=(uint16_t)(i+2u);
  w[1]=(uint16_t)(start+6u+4u*i);w[0137]=relative;w[01001+relative]=0;w[01004+relative]=0;w[0124]=0;w[01000]=(uint16_t)(w[01000]&agc_negate(bit[i])&077774u);w[0143]=singular[i];w[2]=cm ? 02460u : 02557u;(void)dsky_pinball_write(s,io,DSKY_DSPIN);
  w[0777]=paired[i];w[2]=cm ? 02464u : 02563u;(void)dsky_pinball_write(s,io,DSKY_2BLANK);
  w[0777]=(uint16_t)(paired[i]-2u);w[2]=cm ? 02467u : 02566u;(void)dsky_pinball_write(s,io,DSKY_2BLANK);
  w[0777]=first[i];w[0]=first[i];
 }
 w[0777]=w[0132];a=w[0130];l=w[0131];w[0130]=w[0777];w[0131]=agc_correct(w[1]);w[0]=agc_extend(a);w[1]=agc_extend(l);w[2]=(uint16_t)(start+18u);w[5]=ret;return P66_BIAS_OK;
}
enum p66_bias_status dsky_blank(apollo_overlay_state *s,p66_bias_io *io,enum dsky_blank_entry entry){return blank_body(s,io,entry,0);}
enum exec_status cm_blank(apollo_overlay_state *s,cm_native_io *io,enum dsky_blank_entry entry){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=blank_body(s,&p,entry,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
