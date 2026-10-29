/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_consumer.h"
#include "dsky_test_noun.h"
#include "dsky_nv_update_general.h"
#include "dsky_mixed_noun.h"
#include "dsky_machine_cadr.h"
#include "dsky_verb_dispatch.h"
#include "dsky_octal_consumer.h"
#include "dsky_decimal_consumer.h"
#include "dsky_monitor_paste.h"
#include "dsky_blanksub.h"
enum dsky_monitor_consumer_stage dsky_monitor_consumer_entry(const apollo_overlay_state *s,enum dsky_nv_mission m)
{agc_word pc,fb;unsigned i;static const agc_word scalar[2][6]={{02615,02677,02704,02623,02635,0},{02516,02603,02610,02524,0,02653}},dp[2][3]={{02710,02715,02717},{02614,02621,02623}};if(!s||(unsigned)m>1u)return DSKY_MONC_FRONTIER;pc=s->words[5];fb=s->words[4];
 if(fb==062000u){switch(pc){case 03321:return DSKY_MONC_DISPLAY;case 02317:return DSKY_MONC_NOUN;case 03333:return DSKY_MONC_AFTER;case 02046:return DSKY_MONC_TEST;case 02221:return DSKY_MONC_MIXED;case 02120:return DSKY_MONC_MACHINE;case 02133:return DSKY_MONC_DISPATCH;case 02365:case 02373:case 02400:case 02360:case 02353:return DSKY_MONC_OCTAL_BEGIN;case 02504:return DSKY_MONC_CHANNEL;case 03363:return DSKY_MONC_OCTAL_FORMAT;case 02422:return DSKY_MONC_OCTAL_RETURN;case 02523:return DSKY_MONC_DECIMAL_LOAD;case 02537:return DSKY_MONC_DECIMAL_SELECT;case 02566:return DSKY_MONC_DECIMAL_CALL;case 0136:return DSKY_MONC_PASTE_BEGIN;case 02337:return DSKY_MONC_VERB;case 03351:return DSKY_MONC_BUSY;default:break;}
  if(pc==(m==DSKY_NV_CM?04675u:04631u))return DSKY_MONC_PASTE_UPDATED;
  if(pc==(m==DSKY_NV_CM?04271u:04255u))return DSKY_MONC_BLANK;
  if(pc==(m==DSKY_NV_CM?04156u:04142u)||pc==(m==DSKY_NV_CM?04157u:04143u))return DSKY_MONC_PASTE_BLANKED;
 }
 if(fb==060000u){if(pc==(m==DSKY_NV_CM?03133u:03226u))return DSKY_MONC_DECIMAL_FORMAT;if(pc==(m==DSKY_NV_CM?04675u:04631u))return DSKY_MONC_DECIMAL_RETURN;for(i=0;i<6u;++i)if(scalar[(unsigned)m][i]&&pc==scalar[(unsigned)m][i])return DSKY_MONC_SCALAR;for(i=0;i<3u;++i)if(pc==dp[(unsigned)m][i])return DSKY_MONC_DP;}
 return DSKY_MONC_FRONTIER;
}
enum p66_bias_status dsky_monitor_consumer(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_nv_mission m,enum dsky_monitor_consumer_stage stage,agc_word channel,enum dsky_monitor_consumer_stage *next)
{enum p66_bias_status status=P66_BIAS_OUT_OF_SCOPE;enum dsky_monitor_route mon;enum dsky_enter_zero_route noun;enum dsky_nv_update_route update;enum dsky_mixed_route mixed;enum dsky_machine_route machine;enum dsky_verb_route verb;enum dsky_octal_consumer_route octal;enum dsky_decimal_consumer_route decimal;cm_native_io c={0};
 if(!s||!io||!next||(unsigned)m>1u||(unsigned)stage>=24u||channel>AGC_MASK||io->channel11>AGC_MASK||io->wrote_channel11>1u||dsky_monitor_consumer_entry(s,m)!=stage)return P66_BIAS_OUT_OF_SCOPE;
#if defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE)
 if(stage!=DSKY_MONITOR_CONSUMER_ONLY_STAGE)return P66_BIAS_OUT_OF_SCOPE;
#endif
 switch(stage){
 case DSKY_MONC_DISPLAY:status=dsky_monitor(s,io,m,DSKY_MONITOR_DISPLAY,&mon);break;
 case DSKY_MONC_NOUN:status=dsky_nv_update_general(s,&io->control,m,DSKY_NV_NOUN_STORE,&update);break;
 case DSKY_MONC_AFTER:status=dsky_monitor(s,io,m,DSKY_MONITOR_DISPLAY_AFTER,&mon);break;
 case DSKY_MONC_TEST:status=dsky_test_noun(s,&io->control,m,&noun);break;
 case DSKY_MONC_MIXED:if(m==DSKY_NV_LM)status=dsky_mixed_noun(s,&io->control,&mixed);else{c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;status=(unsigned)cm_mixed_noun(s,&c,&mixed)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;if(status==P66_BIAS_OK){io->control.superbank=c.superbank;io->control.interrupt_enabled=c.interrupt_enabled;}}break;
 case DSKY_MONC_MACHINE:if(m==DSKY_NV_LM)status=dsky_machine_cadr(s,&io->control,&machine);else{c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;status=(unsigned)cm_machine_cadr(s,&c,&machine)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;if(status==P66_BIAS_OK){io->control.superbank=c.superbank;io->control.interrupt_enabled=c.interrupt_enabled;}}break;
 case DSKY_MONC_DISPATCH:if(m==DSKY_NV_LM)status=dsky_verb_dispatch(s,&io->control,&verb);else{c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;status=(unsigned)cm_verb_dispatch(s,&c,&verb)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;if(status==P66_BIAS_OK){io->control.superbank=c.superbank;io->control.interrupt_enabled=c.interrupt_enabled;}}break;
 case DSKY_MONC_OCTAL_BEGIN:case DSKY_MONC_CHANNEL:case DSKY_MONC_OCTAL_FORMAT:case DSKY_MONC_OCTAL_RETURN:status=dsky_octal_consumer(s,&io->control,m,(enum dsky_octal_consumer_stage)(stage-DSKY_MONC_OCTAL_BEGIN),channel,&octal);break;
 case DSKY_MONC_DECIMAL_LOAD:case DSKY_MONC_DECIMAL_SELECT:case DSKY_MONC_SCALAR:case DSKY_MONC_DP:case DSKY_MONC_DECIMAL_CALL:case DSKY_MONC_DECIMAL_FORMAT:case DSKY_MONC_DECIMAL_RETURN:status=dsky_decimal_consumer(s,&io->control,m,(enum dsky_decimal_consumer_stage)(stage-DSKY_MONC_DECIMAL_LOAD),channel,&decimal);break;
 case DSKY_MONC_PASTE_BEGIN:status=dsky_monitor_paste(s,&io->control,m,DSKY_MONITOR_PASTE_BEGIN);break;
 case DSKY_MONC_VERB:status=dsky_nv_update_general(s,&io->control,m,DSKY_NV_VERB_STORE,&update);break;
 case DSKY_MONC_PASTE_UPDATED:status=dsky_monitor_paste(s,&io->control,m,DSKY_MONITOR_PASTE_UPDATED);break;
 case DSKY_MONC_BLANK:if(m==DSKY_NV_LM)status=dsky_blanksub(s,&io->control);else{c.superbank=io->control.superbank;c.interrupt_enabled=io->control.interrupt_enabled;status=(unsigned)cm_blanksub(s,&c)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;if(status==P66_BIAS_OK){io->control.superbank=c.superbank;io->control.interrupt_enabled=c.interrupt_enabled;}}break;
 case DSKY_MONC_PASTE_BLANKED:status=dsky_monitor_paste(s,&io->control,m,DSKY_MONITOR_PASTE_BLANKED);break;
 case DSKY_MONC_BUSY:status=dsky_monitor(s,io,m,DSKY_MONITOR_BUSY,&mon);break;
 default:break;
 }
 if(status==P66_BIAS_OK)*next=dsky_monitor_consumer_entry(s,m);
 return status;
}
