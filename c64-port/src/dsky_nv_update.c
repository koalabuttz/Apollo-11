/* SPDX-License-Identifier: MIT */
#include "dsky_nv_update.h"
#include "dsky_two_digit.h"
#include "cm_formatter.h"
#include "noun_reader.h"
enum p66_bias_status dsky_nv_update(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_nv_update_entry entry)
{
    static const agc_word pc[4]={02337,02340,02317,02320};
    uint16_t *w;agc_word value,caller,oldbank;uint8_t noun;apollo_noun_reader_io table_io;cm_native_io cmio={0};
    if(s==0||io==0||(unsigned)mission>1u||(unsigned)entry>3u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;noun=(uint8_t)(entry>=DSKY_NV_NOUN_STORE);value=entry==DSKY_NV_VERB_STORE||entry==DSKY_NV_NOUN_STORE?w[0]:w[noun?01002u:01001u];
    if(w[5]!=pc[entry]||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]!=0||io->superbank!=0100u||io->interrupt_enabled>1u||value>99u||w[2]<02000u||w[2]>07777u||w[noun?01033u:01034u]==0)return P66_BIAS_OUT_OF_SCOPE;
    caller=w[2];oldbank=w[6];w[1]=agc_extend(agc_correct(w[1]));w[noun?01002u:01001u]=value;w[0117]=caller;
    if(noun){w[0]=02325;w[1]=agc_extend(oldbank);w[3]=0400;w[4]=064000;w[6]=064001;w[5]=mission==DSKY_NV_LM?02103u:02062u;table_io.superbank=io->superbank;table_io.interrupt_enabled=io->interrupt_enabled;if(!apollo_noun_reader(s,&table_io,mission==DSKY_NV_LM?APOLLO_FIXED_LM:APOLLO_FIXED_CM,APOLLO_LODNNTAB))return P66_BIAS_OUT_OF_SCOPE;if(w[0146]<AGC_SIGN){w[01017]=w[0146];w[3]=(agc_word)(w[0146]&03400u);w[0145]=(agc_word)(01400u+(w[0146]&0377u));}}
    w[0777]=noun?021u:023u;w[0]=value;w[4]=060000;w[6]=(agc_word)(060000u|(w[3]>>8));w[5]=mission==DSKY_NV_LM?03306u:03213u;w[2]=mission==DSKY_NV_LM?03320u:03225u;
    if(mission==DSKY_NV_LM){if(dsky_two_digit(s,io)!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;}
    else{cmio.superbank=io->superbank;cmio.interrupt_enabled=io->interrupt_enabled;if((unsigned)cm_two_digit(s,&cmio)!=(unsigned)CM_FORMATTER_OK)return P66_BIAS_OUT_OF_SCOPE;io->superbank=cmio.superbank;io->interrupt_enabled=cmio.interrupt_enabled;}
    w[4]=062000;w[6]=(agc_word)(062000u|(w[3]>>8));w[2]=0120;w[5]=caller;return P66_BIAS_OK;
}
