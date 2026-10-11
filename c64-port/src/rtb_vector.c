/* SPDX-License-Identifier: MIT */
#include "rtb_vector.h"
#include "agc_multiword.h"
static void pair(agc_word *h,agc_word *l)
{
 int sign=agc_to_int(*h);agc_register a;int overflow;
 if(sign==0){*h=0;return;}
 if(sign<0){*h=agc_negate(*h);*l=agc_negate(*l);}
 a=agc_add_register(040000,agc_extend(*l));*l=agc_correct(a);
 overflow=agc_overflow(a);a=(agc_register)(overflow>0?1u:overflow<0?0177776u:0u);
 a=agc_add_register(a,agc_extend(037777));*h=agc_correct(agc_add_register(a,agc_extend(*h)));
 if(sign<0){*h=agc_negate(*h);*l=agc_negate(*l);}
}
static void vector(uint16_t *w)
{
 pair(&w[0154],&w[0155]);pair(&w[0157],&w[0160]);pair(&w[0161],&w[0162]);
 w[0]=w[2];w[1]=agc_extend(agc_correct(w[1]));w[5]=w[2];w[2]=1;
}
enum rtb_status rtb_vector(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission,enum rtb_vector_entry entry)
{
 static const uint16_t starts[2][5]={{03010,03772,03726,03724,03705},{03010,03772,03726,03724,03653}};
 uint16_t *w=image->words,fb,base,cadr;unsigned i;agc_word count;agc_register_pair product;agc_dp destination;
 if((unsigned)mission>1u||(unsigned)entry>4u)return RTB_OUT_OF_SCOPE;
 fb=entry==RTB_VECAGREE?0u:(uint16_t)(mission==RTB_LM?020000u:044000u);
 base=w[0120];
 if(w[5]!=starts[mission][entry]||w[4]!=fb||w[3]>03400u||(w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||io->interrupt_enabled>1u||(io->superbank&~0160u))return RTB_OUT_OF_SCOPE;
 if(entry==RTB_VECAGREE){if(w[2]<02000u||w[2]>07777u)return RTB_OUT_OF_SCOPE;vector(w);return RTB_OK;}
 if(base!=0401u&&base!=0455u&&base!=0531u&&base!=0605u&&base!=0661u)return RTB_OUT_OF_SCOPE;
 if(entry==RTB_PULSEIMU_FRONT){
  w[0]=agc_extend(w[base+38u]);w[1]=agc_extend(agc_correct(w[1]));
  w[0133]=(uint16_t)(starts[mission][entry]+4u);w[0134]=fb;
  /* Mission-specific actual source CADR supplied as a fixed compilation constant. */
  cadr=(uint16_t)(mission==RTB_LM?017323u:017206u);
  w[4]=(uint16_t)(cadr&076000u);w[6]=(uint16_t)(w[4]|(w[3]>>8));
  w[2]=(uint16_t)(mission==RTB_LM?04631u:04675u);w[5]=(uint16_t)(02000u+(cadr&01777u));return RTB_OK;
 }
 if(entry==RTB_NORMUNIT_FRONT||entry==RTB_NORMUNX1_FRONT){w[0]=(agc_word)(base+(entry==RTB_NORMUNX1_FRONT?1u:0u));w[0156]=w[0];}
 w[0133]=(uint16_t)(entry==RTB_VECSGNAG?03774u:03733u);w[0134]=fb;
 w[2]=(uint16_t)(mission==RTB_LM?04631u:04675u);vector(w);
 /* Actual SWRETURN restores containing bank and swaps its prior zero bank. */
 w[0134]=0;w[2]=0134;w[4]=fb;w[6]=(uint16_t)(fb|(w[3]>>8));
 if(entry==RTB_VECSGNAG){w[2]=03775;w[5]=(uint16_t)(mission==RTB_LM?06061u:06030u);return RTB_OK;}
 count=0;
 if(agc_to_int(w[0154])==0&&agc_to_int(w[0157])==0&&agc_to_int(w[0161])==0){
  static const unsigned offset[3]={0,3,5};
  for(i=0;i<3u;++i){unsigned at=0154u+offset[i];product=agc_multiply(agc_extend(w[at+1u]),020000);destination.high=w[at];destination.low=w[at+1u];agc_double_add(&product,&destination);w[at]=destination.high;w[at+1u]=destination.low;w[1]=product.l;}
  count=13;
 }
 w[w[0156]+37u]=count;w[0]=count;
 /* Literal POSTJUMP leaves A unchanged, Q=subaddress(UNIT+1), bank zero. */
 w[2]=01024;w[4]=0;w[6]=(uint16_t)(w[3]>>8);w[5]=03024;w[1]=agc_extend(agc_correct(w[1]));return RTB_OK;
}
