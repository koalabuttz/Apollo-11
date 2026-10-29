/* SPDX-License-Identifier: MIT */
#include "sband_control_cases.h"
#include "sband_control_source.h"
void sband_control_input(apollo_overlay_state *s,p66_bias_io *io,uint32_t id,enum apollo_fixed_mission *mission,enum sband_control_entry *entry){
 const sband_control_bindings *b;uint32_t n=id%4096u,group=id/4096u,seed=UINT32_C(0x53424f55)+(n/32u)*UINT32_C(0x9e3779b9);unsigned i,e;
 *mission=group>=10u?APOLLO_FIXED_CM:APOLLO_FIXED_LM;e=(unsigned)(group%10u);*entry=(enum sband_control_entry)e;b=&sband_control_source[*mission];
 for(i=0;i<2048u;++i){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;s->words[i]=(agc_word)(seed&AGC_MASK);}
 s->words[0]=(agc_register)(n*257u);s->words[1]=(agc_register)(n*977u);s->words[2]=(agc_register)(n*983u);s->words[3]=(agc_word)((n%8u)*0400u);s->words[4]=064000;s->words[6]=(agc_word)(064000u|(s->words[3]>>8));s->words[7]=0;s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054;++i)s->words[i]=0;
 io->superbank=(agc_word)(0100u+((n/8u)%4u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
 switch(*entry){case SBAND_CONTROL_START:s->words[5]=b->start;break;case SBAND_CONTROL_INTERPRETER:s->words[5]=(agc_word)(b->start+2u);break;case SBAND_CONTROL_DISPLAY:s->words[5]=b->display;break;case SBAND_CONTROL_AFTER_PRIORITY:s->words[5]=b->after_priority;break;case SBAND_CONTROL_TERMINATE:s->words[5]=b->terminate;break;case SBAND_CONTROL_PROCEED:s->words[5]=b->proceed;break;case SBAND_CONTROL_RECYCLE:s->words[5]=b->recycle;break;case SBAND_CONTROL_BLANK:s->words[5]=b->blank;break;case SBAND_CONTROL_RESTART:s->words[5]=b->restart;break;default:s->words[5]=b->reenter;break;}
 if(n<64u)s->words[01044]=(agc_word)(n&1u?077777:0);
 s->words[0164]=(agc_word)(n*17u&AGC_MASK);
}
