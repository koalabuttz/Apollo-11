/* SPDX-License-Identifier: MIT
 * Direct source Pinball scale composition, no interpreter operation decoding. */
#include <string.h>
#include "dsky_scale.h"
#include "guidance_product.h"
static agc_register indicator(int positive){return positive!=0 ? 1 : 0177776;}
/* TPAGREE with READLO's third limb cleared, original carry-chain stores. */
static void agree(uint16_t *w){
 int sign=agc_to_int(w[0154]),overflow;agc_register a;agc_word limit;uint8_t i;
 if(sign==0)sign=agc_to_int(w[0155]);
 if(sign==0){w[0154]=w[0155]=w[0156]=0;return;}
 limit=sign>0 ? 037777 : 040000;a=agc_add_register(agc_extend(limit),indicator(sign>0));
 for(i=2;i>0;--i){a=agc_add_register(a,agc_extend(w[0154+i]));w[0154+i]=agc_correct(a);overflow=agc_overflow(a);a=overflow!=0 ? indicator(overflow>0) : 0;a=agc_add_register(a,agc_extend(limit));}
 a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);overflow=agc_overflow(a);
 if(overflow==0)w[0155]=agc_correct(a);
}
enum p66_bias_status dsky_scale_output(apollo_overlay_state *s,p66_bias_io *io,enum dsky_scale_entry entry){
 uint16_t *w=s->words,eb=(uint16_t)(w[3]>>8),address,logical;uint8_t i;agc_dp factor; p66_preparation p;p66_scale_scratch sc;agc_register a;
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(uint16_t)(060000|eb) || io->superbank!=0100 || io->interrupt_enabled>1 || (unsigned)entry>DSKY_DP3OUT || (w[0140]!=1 && w[0140]!=2) || (w[0140]==2 && w[0117]>2))return P66_BIAS_OUT_OF_SCOPE;
 if(w[0140]==2){address=(uint16_t)(w[0150+w[0117]]&03777u);eb=(uint16_t)(address>>8);}
 else{logical=w[0145];if(logical>01777)return P66_BIAS_OUT_OF_SCOPE;address=logical<01400 ? logical : (uint16_t)(eb*0400u+(logical&0377u));}
 if(address<010 || address>03776 || (address&0377u)==0377)return P66_BIAS_OUT_OF_SCOPE;
 w[0162]=entry==DSKY_DP1OUT ? 02711 : entry==DSKY_DP2OUT ? 02716 : 02720;w[0144]=02726;
 if(w[0140]==2)w[3]=(uint16_t)(eb*0400u);
 w[0154]=w[address];w[0155]=w[address+1u];w[0156]=0;agree(w);
 memset(&p,0,sizeof p);for(i=0;i<3;++i)p.mpac[i]=w[0154+i];p.mpac[7]=0;sc.mptemp=w[0135];sc.ovfind=w[0121];factor.high=w[0123];factor.low=w[0124];
 (void)p66_scalar_product(&p,&sc,&factor);for(i=0;i<3;++i)w[0154+i]=p.mpac[i];w[0135]=sc.mptemp;w[0116]=0123;a=p.rate.registers.a;w[1]=p.rate.registers.l;
 if(entry==DSKY_DP1OUT){w[0156]=agc_correct(a);a=agc_extend(w[0155]);w[0155]=p.mpac[2];w[0154]=agc_correct(a);}
 else if(entry==DSKY_DP3OUT){
  w[0123]=0;w[0124]=02722;
  for(i=0;i<7;++i){agc_register_pair r={agc_extend(w[0155]),agc_extend(w[0156])};agc_dp pair={w[0155],w[0156]};int overflow;
   agc_double_add(&r,&pair);w[0155]=pair.high;w[0156]=pair.low;a=agc_add_register(r.a,agc_extend(w[0154]));a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);overflow=agc_overflow(a);if(overflow!=0)w[0162]=agc_correct(indicator(overflow>0));w[1]=r.l;
  }
  a=0;
 }
 w[0]=a;w[2]=00566;w[4]=062000;w[5]=02566;w[6]=(uint16_t)(062000|eb);w[7]=0;return P66_BIAS_OK;
}
