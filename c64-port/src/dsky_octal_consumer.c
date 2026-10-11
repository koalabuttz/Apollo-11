/* SPDX-License-Identifier: MIT */
#include "dsky_octal_consumer.h"
enum p66_bias_status dsky_octal_consumer(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_octal_consumer_stage stage,agc_word channel_value,enum dsky_octal_consumer_route *route)
{
 enum p66_bias_status status;enum dsky_octal_verb_route v;
 if(!s||!io||!route||(unsigned)stage>3u||(unsigned)mission>1u)return P66_BIAS_OUT_OF_SCOPE;
 (void)channel_value;
 switch(stage){
#if !defined(DSKY_OCTAL_CONSUMER_ONLY_STAGE)||DSKY_OCTAL_CONSUMER_ONLY_STAGE==0
 case DSKY_OCTAL_BEGIN:status=dsky_octal_verb_begin(s,io,mission,&v);break;
#endif
#if !defined(DSKY_OCTAL_CONSUMER_ONLY_STAGE)||DSKY_OCTAL_CONSUMER_ONLY_STAGE==1
 case DSKY_OCTAL_CHANNEL:status=dsky_octal_verb_channel(s,io,mission,channel_value,&v);break;
#endif
#if !defined(DSKY_OCTAL_CONSUMER_ONLY_STAGE)||DSKY_OCTAL_CONSUMER_ONLY_STAGE==2
 case DSKY_OCTAL_FORMAT:{enum dsky_octal_route o;cm_native_io cm={0};
  if(s->words[5]!=03363u||s->words[2]!=02422u)return P66_BIAS_OUT_OF_SCOPE;
  if(mission==DSKY_NV_LM)status=dsky_octal_display(s,io,&o);
  else{cm.superbank=io->superbank;cm.interrupt_enabled=io->interrupt_enabled;status=(unsigned)cm_octal_display(s,&cm,&o)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;if(status==P66_BIAS_OK){io->superbank=cm.superbank;io->interrupt_enabled=cm.interrupt_enabled;}}
  if(status!=P66_BIAS_OK)return status;
  *route=o==DSKY_OCTAL_CCS_HOLE?DSKY_OCTAL_RECOVERY:DSKY_OCTAL_NEXT;return P66_BIAS_OK;
 }
#endif
#if !defined(DSKY_OCTAL_CONSUMER_ONLY_STAGE)||DSKY_OCTAL_CONSUMER_ONLY_STAGE==3
 case DSKY_OCTAL_RETURN:status=dsky_octal_verb_return(s,io,mission,&v);break;
#endif
 default:return P66_BIAS_OUT_OF_SCOPE;
 }
#if !defined(DSKY_OCTAL_CONSUMER_ONLY_STAGE)||DSKY_OCTAL_CONSUMER_ONLY_STAGE!=2
 if(status!=P66_BIAS_OK)return status;
 *route=v==DSKY_VERB_ENTRET?DSKY_OCTAL_ENTRET:v==DSKY_VERB_ALARM?DSKY_OCTAL_ALARM:DSKY_OCTAL_NEXT;return P66_BIAS_OK;
#else
 (void)v;return P66_BIAS_OUT_OF_SCOPE;
#endif
}
