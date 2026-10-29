/* SPDX-License-Identifier: MIT */
#include "dsky_enter_zero.h"
static enum p66_bias_status enter_zero(apollo_overlay_state *s,p66_bias_io *io,enum dsky_enter_zero_route *route,uint8_t cm){uint16_t *w=s->words,verb=w[01001],noun=w[01002],bank,address;apollo_noun_reader_io reader;
 if(route==0||w[5]!=02035u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u||(verb!=077777u&&verb>99u))return P66_BIAS_OUT_OF_SCOPE;
 if((verb==077777u||verb<28u)&&noun!=077777u&&noun>99u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[01000]=0;w[0777]=077754;w[01041]=agc_negate(verb);w[0]=agc_add_register(agc_extend(w[01041]),034);
 if(verb!=077777u&&verb>=28u){w[5]=02133;*route=DSKY_ZERO_VERB;return P66_BIAS_OK;}
 bank=w[6];w[0]=02051;w[1]=agc_extend(bank);w[3]=0400;w[4]=064000;w[6]=064001;w[5]=cm ? 02062u : 02103u;reader.superbank=io->superbank;reader.interrupt_enabled=io->interrupt_enabled;(void)apollo_noun_reader(s,&reader,cm ? APOLLO_FIXED_CM : APOLLO_FIXED_LM,APOLLO_LODNNTAB);io->superbank=reader.superbank;io->interrupt_enabled=reader.interrupt_enabled;
 if(w[0140]==2u){w[2]=02055;w[5]=02221;*route=DSKY_ZERO_MIXED;return P66_BIAS_OK;}
 address=w[0146];w[0]=agc_diminished_abs(address);
 if(address==0){w[2]=02060;w[5]=02351;*route=DSKY_ZERO_ALARM;return P66_BIAS_OK;}
 if((address&040000u)!=0&&address!=077777u){w[2]=02061;w[5]=02064;*route=DSKY_ZERO_REQUEST_ADDRESS;return P66_BIAS_OK;}
 if(address==077777u){w[01017]=agc_correct(agc_add_register(agc_extend(w[01017]),1));address=w[01017];w[5]=02120;w[2]=02064;*route=DSKY_ZERO_MACHINE_ADDRESS;}else{w[01017]=address;w[5]=02133;w[2]=3;*route=DSKY_ZERO_VERB;}
 w[3]=(uint16_t)(address&03400u);w[6]=(uint16_t)(062000u|(w[3]>>8));w[0]=(uint16_t)(01400u+(address&0377u));w[0145]=w[0];return P66_BIAS_OK;
}
enum p66_bias_status dsky_enter_zero(apollo_overlay_state *s,p66_bias_io *io,enum dsky_enter_zero_route *route){return enter_zero(s,io,route,0);}
enum exec_status cm_enter_zero(apollo_overlay_state *s,cm_native_io *io,enum dsky_enter_zero_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=enter_zero(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
