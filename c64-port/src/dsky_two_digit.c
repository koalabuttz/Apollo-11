/* SPDX-License-Identifier: MIT
 * Original DSPDECVN/SHORTMP/TRACE1 composed with verified generic DSPIN. */
#include "dsky_two_digit.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t a){agc_dp d;d.high=w[a];d.low=w[a+1u];agc_double_add(r,&d);w[a]=d.high;w[a+1u]=d.low;}
static void shortmp(uint16_t *w){agc_register_pair r;agc_word minor=w[0155],major=w[0154];w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(minor),10);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(major),10);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;}
enum p66_bias_status dsky_two_digit(apollo_overlay_state *s,p66_bias_io *io){
 static const uint16_t codes[10]={025,003,031,033,017,036,034,023,035,037};
 uint16_t *w=s->words,caller=w[2],count=w[0777];uint8_t i;agc_register_pair r;
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->superbank!=0100 || io->interrupt_enabled>1 || w[0]>99 || caller<0100 || caller>07777 || count>21 || !(count&1u) || w[01023+(count>>1)]==0)return P66_BIAS_OUT_OF_SCOPE;
 r=agc_multiply(w[0],0244);w[1]=agc_extend(w[0154]);w[0154]=agc_correct(r.l);w[0155]=0;w[0115]=caller;
 for(i=0;i<2;++i){uint16_t code;w[0137]=(uint16_t)(1u-i);shortmp(w);code=codes[w[0154]==077777 ? 0 : w[0154]];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0124]=code;w[0143]=(uint16_t)(count-i);w[0777]=(uint16_t)(count-i-1u);w[2]=03253;(void)dsky_pinball_write(s,io,DSKY_DSPIN);}
 w[0]=0177754;w[2]=0116;w[5]=caller;w[0777]=077754;w[7]=0;return P66_BIAS_OK;
}
