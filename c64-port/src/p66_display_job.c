/* SPDX-License-Identifier: MIT */
#include "p66_display_job.h"
#ifndef P66_DISPLAY_JOB_DISPATCH_ONLY
#include "p66_makeplay.h"
#include "p66_makeplay_sleep.h"
#include "lm_basic_priority.h"
#include "lm_basic_wake.h"
#include "lm_basic_sleep.h"
#include "lm_pinball_busy.h"
#include "lm_basic_finish.h"
#include "p66_goplay.h"
#include "dsky_scale.h"
uint16_t p66_display_job_entry(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel,uint16_t selector){exec_address next;
#ifdef P66_DISPLAY_JOB_ONLY_SELECTOR
 if(selector!=P66_DISPLAY_JOB_ONLY_SELECTOR)return 0xffff;
 selector=P66_DISPLAY_JOB_ONLY_SELECTOR;
#endif
 switch(selector){
 case P66_DISPLAY_PRIORITY_ENTER:return (uint16_t)p66_makeplay_priority(s,io);
 case P66_DISPLAY_PRIORITY_CHANGE:return (uint16_t)lm_basic_priority_change(s,io,&next);
 case P66_DISPLAY_SELECT:return (uint16_t)p66_makeplay_select(s,io,channel);
 case P66_DISPLAY_WAKE:return (uint16_t)lm_basic_wake(s,io);
 case P66_DISPLAY_WAKE_RETURN:return (uint16_t)p66_makeplay_wake_return(s,io);
 case P66_DISPLAY_SLEEP_ENTER:return (uint16_t)p66_makeplay_sleep_enter(s,io);
 case P66_DISPLAY_SLEEP_RETURN:return (uint16_t)p66_makeplay_sleep_return(s,io);
 case P66_DISPLAY_SLEEP:return (uint16_t)lm_basic_sleep(s,io,channel,&next);
 case P66_DISPLAY_RESUME:return (uint16_t)p66_makeplay_sleep_resume(s,io);
 case P66_DISPLAY_GOPLAY:return (uint16_t)p66_goplay_enter(s,io);
 case P66_DISPLAY_NV_ENTER:return (uint16_t)p66_nv_enter(s,io);
 case P66_DISPLAY_NV_VERB:return (uint16_t)p66_nv_verb(s,io);
 case P66_DISPLAY_NV_NOUN:return (uint16_t)p66_nv_noun(s,io);
 case P66_DISPLAY_NV_PREPARE:return (uint16_t)p66_nv_data_prepare(s,io);
 case P66_DISPLAY_NV_FACTOR:return (uint16_t)p66_nv_scale_select(s,io);
 case P66_DISPLAY_NV_SCALE:return (uint16_t)dsky_scale_output(s,io,s->words[5]==02710u?DSKY_DP1OUT:DSKY_DP3OUT);
 case P66_DISPLAY_NV_DECIMAL:return (uint16_t)p66_nv_decimal_row(s,io);
 case P66_DISPLAY_NV_NEXT:return (uint16_t)p66_nv_next_row(s,io);
 case P66_DISPLAY_NV_FINISH:return (uint16_t)p66_goplay_finish(s,io,channel);
 case P66_DISPLAY_REST:return (uint16_t)p66_makeplay_busy(s,io);
 case P66_DISPLAY_BUSY:return (uint16_t)lm_pinball_busy(s,io,channel);
 case P66_DISPLAY_END:{exec_transfer transfer;return(uint16_t)lm_basic_finish(s,io,channel,&transfer);}
 default:return 0xffff;
 }
}
#endif
#ifndef P66_DISPLAY_JOB_KERNEL_ONLY
static uint16_t selector(agc_word pc){
 switch(pc){case 02546:return P66_DISPLAY_PRIORITY_ENTER;case 05146:return P66_DISPLAY_PRIORITY_CHANGE;case 02553:return P66_DISPLAY_SELECT;case 05137:return P66_DISPLAY_WAKE;case 02614:return P66_DISPLAY_WAKE_RETURN;case 02754:return P66_DISPLAY_SLEEP_ENTER;case 02770:return P66_DISPLAY_SLEEP_RETURN;case 05133:return P66_DISPLAY_SLEEP;case 02615:return P66_DISPLAY_RESUME;case 03070:return P66_DISPLAY_GOPLAY;case 04155:return P66_DISPLAY_NV_ENTER;case 02337:return P66_DISPLAY_NV_VERB;case 03602:return P66_DISPLAY_NV_NOUN;case 03604:return P66_DISPLAY_NV_PREPARE;case 02537:return P66_DISPLAY_NV_FACTOR;case 02710:case 02717:return P66_DISPLAY_NV_SCALE;case 02566:return P66_DISPLAY_NV_DECIMAL;case 02570:return P66_DISPLAY_NV_NEXT;case 03122:return P66_DISPLAY_NV_FINISH;case 03121:return P66_DISPLAY_REST;case 04442:return P66_DISPLAY_BUSY;case 05155:return P66_DISPLAY_END;default:return 0;}
}
static uint8_t accepted(uint16_t op,uint16_t status){
 switch(op){case P66_DISPLAY_END:return(uint8_t)(status==EXEC_OK||status==EXEC_IDLE);case P66_DISPLAY_SELECT:return(uint8_t)(status>=1u&&status<=3u);case P66_DISPLAY_WAKE:return(uint8_t)(status==0||status==EXEC_NOT_FOUND);case P66_DISPLAY_WAKE_RETURN:return(uint8_t)(status==4u);case P66_DISPLAY_SLEEP_RETURN:return(uint8_t)(status==1u);case P66_DISPLAY_SLEEP:return(uint8_t)(status==0||status==EXEC_IDLE);case P66_DISPLAY_RESUME:return(uint8_t)(status==2u);case P66_DISPLAY_NV_ENTER:return(uint8_t)(status<=1u);case P66_DISPLAY_REST:return(uint8_t)(status==3u||status==4u);default:return(uint8_t)(status==0);}
}
static enum exec_action execute(executive *state,void *user,exec_address *resume,uint16_t op,uint8_t source_control){
 p66_display_job_binding *b=user;p66_basic_binding *basic=&b->descent.basic;p66_native_binding *a=&basic->arithmetic;
 uint16_t bank;uint16_t *w=a->image->words;uint8_t token=a->producers->enter(a->producers->user);
 p66_native_export_executive(a->image,state);if(!source_control){bank=(agc_word)(a->runtime->current.bank&076007u);w[3]=(agc_word)((bank&7u)*0400u);w[4]=(agc_word)(bank&076000u);w[6]=bank;w[5]=a->runtime->current.location;basic->control.superbank=(agc_word)(a->runtime->current.bank&0160u);}
 if(b->invoke_display)b->status=b->invoke_display(b->display_modules,a->image,&basic->control,&b->channel11,op);
#ifndef P66_DISPLAY_JOB_DISPATCH_ONLY
 else b->status=p66_display_job_entry(a->image,&basic->control,&b->channel11,op);
#else
 else b->status=0xffff;
#endif
 p66_native_import_executive(state,a->image);a->status=b->status;
 resume->location=w[5];resume->bank=(agc_word)(w[6]|basic->control.superbank);
 if((op==P66_DISPLAY_SLEEP||op==P66_DISPLAY_END)&&b->status==EXEC_IDLE){resume->location=0;resume->bank=0;}
 if(!accepted(op,b->status))a->runtime->fault=b->status==01206u?EXEC_BAILOUT_PENDING:EXEC_OUT_OF_SCOPE;
 else if(a->observe)a->observe(a->observer,a->image,w[5]);
 if(op==P66_DISPLAY_END&&b->status==EXEC_OK&&w[5]==06070u){resume->location=w[0164];a->runtime->interpreter_control.intbit15=w[0115];a->runtime->interpreter_control.edop=w[023];a->producers->leave(a->producers->user,token);return EXEC_DISPATCHED_INTERPRETER;}
 a->producers->leave(a->producers->user,token);return EXEC_DISPATCHED_BASIC;
}
static enum exec_action continuation(executive *state,void *user,exec_address *resume){p66_display_job_binding *b=user;return execute(state,user,resume,selector(b->descent.basic.arithmetic.runtime->current.location),0);}
enum exec_action p66_display_job_finish(executive *state,void *user,exec_address *resume){return execute(state,user,resume,P66_DISPLAY_END,1);}
const exec_context_mapping p66_display_job_mappings[P66_DISPLAY_JOB_MAPPING_COUNT]={
 {{002546,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{005146,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{002553,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{005137,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{002614,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{002754,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{002770,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{005133,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{002615,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{003070,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{004155,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{003121,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{004442,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{003122,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{005155,0020067},EXEC_CONTEXT_BASIC,continuation},
 {{005133,0010067},EXEC_CONTEXT_BASIC,continuation},
 {{002337,0062101},EXEC_CONTEXT_BASIC,continuation},
 {{003602,0062101},EXEC_CONTEXT_BASIC,continuation},
 {{003604,0062100},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062100},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062101},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062102},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062103},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062104},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062105},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062106},EXEC_CONTEXT_BASIC,continuation},
 {{002537,0062107},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062100},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062101},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062102},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062103},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062104},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062105},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062106},EXEC_CONTEXT_BASIC,continuation},
 {{002566,0062107},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062100},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062101},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062102},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062103},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062104},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062105},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062106},EXEC_CONTEXT_BASIC,continuation},
 {{002570,0062107},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060100},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060101},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060102},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060103},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060104},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060105},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060106},EXEC_CONTEXT_BASIC,continuation},
 {{002710,0060107},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060100},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060101},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060102},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060103},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060104},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060105},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060106},EXEC_CONTEXT_BASIC,continuation},
 {{002717,0060107},EXEC_CONTEXT_BASIC,continuation},
 {{005155,0010067},EXEC_CONTEXT_BASIC,continuation},
};

#endif
