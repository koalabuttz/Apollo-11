/* SPDX-License-Identifier: MIT
 * Direct fixed source DSPMMJB composition, not an AGC instruction decoder. */
#include "p66_mode_display.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t a){agc_dp d;d.high=w[a];d.low=w[a+1u];agc_double_add(r,&d);w[a]=d.high;w[a+1u]=d.low;}
static void shortmp(uint16_t *w){
 agc_register_pair r;agc_word minor=w[0155],major=w[0154];
 w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;
 r=agc_multiply(agc_extend(minor),10);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);
 r=agc_multiply(agc_extend(major),10);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;
}
static void digit(uint16_t *w,uint16_t code,uint16_t left){
 agc_word old=w[01035],mag=(old&040000u)!=0 ? agc_negate(old) : old;
 uint16_t mask=left!=0 ? 01740 : 037;
 w[0124]=(uint16_t)(left!=0 ? code<<5 : code);w[0143]=left;w[0141]=10;w[021]=10;w[0114]=03253;w[0142]=mag;
 if((mag&mask)!=w[0124]){w[01035]=agc_negate((agc_word)((mag&~mask)+w[0124]));if((old&040000u)==0)w[01016]=agc_add_word(w[01016],1);}
}
enum p66_bias_status p66_mode_display(apollo_overlay_state *s,p66_bias_io *io){
 static const uint16_t relay[10]={025,003,031,033,017,036,034,023,035,037};
 uint16_t *w=s->words,mode=w[01011],saved=w[0777];uint8_t i;
 if(w[3]!=01000 || w[4]!=060000 || w[6]!=060002 || io->superbank!=0100 || io->interrupt_enabled>1 || (mode<040000 && mode>99) || (mode<040000 && w[01035]==0))return P66_BIAS_OUT_OF_SCOPE;
 w[0777]=025;w[0140]=saved;
 if(mode==077777){
  uint16_t old=w[01035];w[021]=012;w[01035]=073777;if((old&040000u)==0)w[01016]=agc_add_word(w[01016],1);w[0]=agc_extend(old);io->interrupt_enabled=1;
 }else if(mode>=040000){w[0]=agc_diminished_abs(mode);}
 else{
  agc_register_pair r=agc_multiply(mode,0244);w[1]=agc_extend(w[0154]);w[0154]=agc_correct(r.l);w[0155]=0;w[0115]=03542;
  for(i=0;i<2;++i){uint16_t code;
   w[0137]=(uint16_t)(1u-i);shortmp(w);code=relay[w[0154]];
   w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0777]=(uint16_t)(024u-i);
   digit(w,code,(uint16_t)(1u-i));io->interrupt_enabled=1;
  }
  w[0777]=077754;w[0]=0177754;
 }
 w[0140]=agc_correct(w[0]);w[0]=agc_extend(saved);w[0777]=saved;w[2]=03547;w[5]=05155;w[7]=0;
 return P66_BIAS_OK;
}
