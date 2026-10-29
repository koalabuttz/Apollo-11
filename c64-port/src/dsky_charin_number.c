/* SPDX-License-Identifier: MIT */
#include "dsky_charin_number.h"
#include "agc_multiword.h"
/* Source SHORTMP's TP multiply, retaining MPAC+2 and register debris. */
static void ten_times(uint16_t *w){agc_register_pair r;agc_dp pair;agc_register major;
 w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);
 r=agc_multiply(0,10);pair.high=0;pair.low=w[0156];agc_double_add(&r,&pair);w[0155]=pair.high;w[0156]=pair.low;
 major=agc_extend(w[0154]);w[0154]=agc_correct(r.a);r=agc_multiply(major,10);
 pair.high=w[0154];pair.low=w[0155];agc_double_add(&r,&pair);w[0154]=pair.high;w[0155]=pair.low;w[0]=r.a;w[1]=r.l;
}
static agc_word rotate(agc_word a){return (agc_word)(((a<<1)&AGC_MASK)|(a>>14));}
static void fraction(uint16_t *w){agc_word minor=w[0155],major=w[0154];agc_register_pair r;agc_dp pair;
 w[0116]=02322;w[0155]=0;r=agc_multiply(agc_extend(minor),013261);w[0156]=agc_correct(r.a);
 r=agc_multiply(agc_extend(013261),major);pair.high=w[0155];pair.low=w[0156];agc_double_add(&r,&pair);w[0155]=pair.high;w[0156]=pair.low;
 w[0135]=05174;r=agc_multiply(agc_extend(minor),05174);pair.high=w[0155];pair.low=w[0156];agc_double_add(&r,&pair);w[0155]=pair.high;w[0156]=pair.low;w[0154]=agc_correct(r.a);
 r=agc_multiply(agc_extend(major),05174);pair.high=w[0154];pair.low=w[0155];agc_double_add(&r,&pair);w[0154]=pair.high;w[0155]=pair.low;w[0]=r.a;w[1]=r.l;
}
enum p66_bias_status dsky_charin_number(apollo_overlay_state *s,dsky_charin_io *io){
 static const uint16_t relay[10]={025,3,031,033,017,036,034,023,035,037},critical[5]={18,16,10,5,0};
 uint16_t *w=s->words,key=w[0154],count=w[0777],relative=0,reg=0,digit,decimal=(uint16_t)(w[01000]&3u),end=0;agc_register a;
 if((key<1 || key>9) && key!=020)return P66_BIAS_OUT_OF_SCOPE;
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->control.superbank!=0100 || io->control.interrupt_enabled>1 || io->channel11>077777)return P66_BIAS_OUT_OF_SCOPE;
 if((count&040000u)==0){
  if(count>19 || count==15 || decimal>2 || (key>=8 && key<=9 && decimal==0))return P66_BIAS_OUT_OF_SCOPE;
  relative=count<5 ? 4 : count<10 ? 3 : count<15 ? 2 : count<18 ? 1 : 0;reg=(uint16_t)(01001u+relative);
  if((decimal!=0 && w[reg]>(relative<2 ? 99u : 9999u)) || w[01023+(count>>1)]==0)return P66_BIAS_OUT_OF_SCOPE;
 }
 digit=key==020 ? 0 : key;
 /* Full CHARIN lock and key-release-light path before literal NUM branch. */
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=w[01012];w[01012]=1;io->wrote_channel11=0;
 if(w[01042]!=0){io->channel11=(uint16_t)(io->channel11|020u);io->wrote_channel11=1;w[0154]=io->channel11;}else w[0154]=0;
 w[0117]=digit;
 if((count&040000u)!=0){w[0]=agc_extend(agc_diminished_abs(count));w[2]=key==8 || key==9 ? (count==077777 ? 02166u : 02165u) : 02202u;w[5]=05155;w[7]=0;return P66_BIAS_OK;}
 w[0137]=relative;if((w[01015]&040000u)==0)w[01015]=0;
 w[0124]=relay[digit];w[0143]=count;w[2]=02216;(void)dsky_pinball_write(s,&io->control,DSKY_DSPIN);
 /* Octal CYL edits rotate on each source store/read. Decimal retains the
  * entire TP SHORTMP debris and the fifth-character overflow path. */
 if(decimal==0){agc_word value=w[reg];w[reg]=0;value=rotate(rotate(rotate(value)));w[022]=agc_negate(value);a=agc_add_register(agc_extend(value),agc_extend(digit));w[reg]=agc_correct(a);}
 else{
  w[0154]=w[reg];w[reg]=(uint16_t)(decimal-1u);w[0155]=0;ten_times(w);a=agc_add_register(agc_extend(w[0155]),agc_extend(digit));w[0155]=agc_correct(a);
  if(agc_overflow(a)!=0){a=agc_add_register(1,agc_extend(w[0154]));w[0154]=agc_correct(a);end=1;}else w[reg]=agc_correct(a);
 }
 if(end || count==critical[relative]){
  if(decimal!=0 && relative>=2){agc_word middle,third;
   fraction(w);middle=w[0155];third=w[0156];
   if(decimal==2){w[1]=agc_extend(third);w[0156]=middle;w[0155]=agc_negate(third);w[reg]=agc_negate(middle);w[01004+relative]=agc_negate(third);}
   else{w[0156]=1;w[0155]=third;w[reg]=middle;w[01004+relative]=third;}
  }
  a=agc_extend(agc_negate(count));
 }else a=agc_extend(agc_diminished_abs(count));
 w[0777]=agc_correct(a);w[0]=a;w[2]=02315;w[5]=05155;w[7]=0;return P66_BIAS_OK;
}
