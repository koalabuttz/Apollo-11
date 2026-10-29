/* SPDX-License-Identifier: MIT
 * Native original DSPSIGN/+ON/-ON, DSPRND, SHORTMP, TRACE1/DSPIN composition. */
#include "dsky_decimal.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t a){agc_dp d;d.high=w[a];d.low=w[a+1u];agc_double_add(r,&d);w[a]=d.high;w[a+1u]=d.low;}
static void shortmp(uint16_t *w){agc_register_pair r;agc_word minor=w[0155],major=w[0154];w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(minor),10);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(major),10);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;}
enum p66_bias_status dsky_decimal_display(apollo_overlay_state *s,p66_bias_io *io,uint8_t rounded){
 static const uint16_t codes[10]={025,003,031,033,017,036,034,023,035,037};
 uint16_t *w=s->words,caller=w[2],count=w[0777],minus,plus;uint8_t i,negative=(uint8_t)((w[0154]&040000u)!=0);
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->superbank!=0100 || io->interrupt_enabled>1 || rounded>1 || caller<0100 || caller>07777 || (count!=14 && count!=9 && count!=4) || ((w[0154]^w[0155])&040000u)!=0)return P66_BIAS_OUT_OF_SCOPE;
 for(i=0;i<11;++i)if(w[01023+i]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[0115]=caller;w[0144]=rounded!=0 ? 03231 : 03265;
 minus=count==14 ? 5 : count==9 ? 3 : 0;plus=(uint16_t)(minus+1u);
 if(negative!=0){w[0154]=agc_negate(w[0154]);}
 /* +ON/-ON derives INREL2/3/4 then toggles both actual sign relays. */
 w[0137]=count==14 ? 2 : count==9 ? 3 : 4;w[1]=negative!=0 ? 03210 : 03214;
 w[0123]=0;w[0122]=02000;w[0124]=0;w[0]=negative!=0 ? plus : minus;w[2]=02426;(void)dsky_pinball_write(s,io,DSKY_11DSPIN);
 w[0124]=02000;w[0]=negative!=0 ? minus : plus;w[2]=02432;(void)dsky_pinball_write(s,io,DSKY_11DSPIN);
 if(negative!=0)w[0155]=agc_negate(w[0155]);
 if(rounded!=0){agc_register_pair r={0,02476};accumulate(&r,w,0154);if(r.a!=0 && r.a!=0177777){w[0154]=037777;w[0155]=037777;}w[0]=r.a;w[1]=r.l;}
 for(i=0;i<5;++i){uint16_t code;w[0137]=(uint16_t)(4u-i);shortmp(w);code=codes[w[0154]==077777 ? 0 : w[0154]];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;
  w[0124]=code;w[0143]=(uint16_t)(count-i);w[0777]=(uint16_t)(count-i==0 ? 0 : count-i-1u);w[0]=agc_diminished_abs((uint16_t)(count-i));w[2]=03253;(void)dsky_pinball_write(s,io,DSKY_DSPIN);
 }
 w[0]=0177754;w[1]=0;w[2]=0116;w[5]=caller;w[0777]=077754;w[7]=0;return P66_BIAS_OK;
}
