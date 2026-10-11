/* SPDX-License-Identifier: MIT */
#include "dsky_read_pair.h"
enum p66_bias_status dsky_read_pair(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission)
{
 uint16_t *w;agc_word caller,address,major,minor,oldminor;unsigned i;
 if(!s||!io||(unsigned)mission>1u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;caller=w[2];
 if(w[5]!=(mission==DSKY_NV_LM?02747u:02655u)||w[4]!=060000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(060000u|(w[3]>>8))||w[7]||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||caller<0100u||caller>07777u||w[0117]>2u||(w[0140]!=1u&&w[0140]!=2u))return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 if(w[0140]==2u)address=(agc_word)(w[0150u+w[0117]]&03777u);
 else{if(w[0145]>01777u)return P66_BIAS_OUT_OF_SCOPE;address=w[0145]<01400u?w[0145]:(agc_word)(w[3]+(w[0145]&0377u));}
 if(address<0100u||address>03776u||(address&0377u)==0377u)return P66_BIAS_OUT_OF_SCOPE;
 w[0144]=caller;
 if(w[0140]==2u){w[3]=(agc_word)(address&03400u);w[6]=(agc_word)(060000u|(w[3]>>8));}
 major=w[address];minor=w[address+1u];oldminor=w[0155];w[0154]=major;w[0155]=minor;w[0156]=0;w[0]=0;w[1]=agc_extend(oldminor);w[2]=0145;w[5]=caller;return P66_BIAS_OK;
}
