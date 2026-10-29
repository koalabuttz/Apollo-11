/* SPDX-License-Identifier: MIT */
#include "dsky_test_noun.h"
enum p66_bias_status dsky_test_noun(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_enter_zero_route *route)
{uint16_t *w,bank,address,i;apollo_noun_reader_io reader;
 if(!s||!io||!route||(unsigned)mission>1u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;if(w[5]!=02046u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]||io->superbank<0100u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||(w[01002]>99u&&w[01002]!=AGC_NEG_ZERO))return P66_BIAS_OUT_OF_SCOPE;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 /* The original DCA inline table tuple then DXCH Z transfers this exact
  * caller PC/BBCON into A/L. No ENTPAS0 stores precede this direct entry. */
 bank=w[6];w[0]=02051;w[1]=agc_extend(bank);w[3]=0400;w[4]=064000;w[6]=064001;w[5]=mission==DSKY_NV_LM?02103u:02062u;reader.superbank=io->superbank;reader.interrupt_enabled=io->interrupt_enabled;
 if(!apollo_noun_reader(s,&reader,mission==DSKY_NV_LM?APOLLO_FIXED_LM:APOLLO_FIXED_CM,APOLLO_LODNNTAB))return P66_BIAS_OUT_OF_SCOPE;
 io->superbank=reader.superbank;io->interrupt_enabled=reader.interrupt_enabled;
 if(w[0140]==2u){w[2]=02055;w[5]=02221;*route=DSKY_ZERO_MIXED;return P66_BIAS_OK;}
 address=w[0146];w[0]=agc_diminished_abs(address);
 if(address==0u){w[2]=02060;w[5]=02351;*route=DSKY_ZERO_ALARM;return P66_BIAS_OK;}
 if((address&AGC_SIGN)&&address!=AGC_NEG_ZERO){w[2]=02061;w[5]=02064;*route=DSKY_ZERO_REQUEST_ADDRESS;return P66_BIAS_OK;}
 if(address==AGC_NEG_ZERO){w[01017]=agc_correct(agc_add_register(agc_extend(w[01017]),1));address=w[01017];w[5]=02120;w[2]=02064;*route=DSKY_ZERO_MACHINE_ADDRESS;}
 else{w[01017]=address;w[5]=02133;w[2]=3;*route=DSKY_ZERO_VERB;}
 w[3]=(agc_word)(address&03400u);w[6]=(agc_word)(062000u|(w[3]>>8));w[0]=(agc_word)(01400u+(address&0377u));w[0145]=w[0];return P66_BIAS_OK;
}
