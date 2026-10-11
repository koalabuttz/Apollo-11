/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_enema_image_cases.h"
#include "cm_enema_cases.h"
void cm_enema_image_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){cm_enema_case c;cm_phase_state p;unsigned i;uint32_t seed=0x551ca55u+n;cm_enema_prepare(&c,(uint16_t)n);for(i=0;i<2048u;++i){seed=seed*1664525u+1013904223u;s->words[i]=(agc_word)((seed>>11)&AGC_MASK);}memset(io,0,sizeof *io);s->words[7]=s->words[035]=s->words[036]=s->words[047]=0;for(i=050;i<=054u;++i)s->words[i]=0;
 cm_native_commit_executive(s,io,&c.previous.exec);cm_native_commit_waitlist(s,io,&c.previous.wait);cm_native_commit_restart(s,io,&c.previous.record);cm_native_gather_phase(s,io,&p);p.records=c.previous.phases;cm_native_commit_phase(s,io,&p);cm_native_commit_enema_io(s,io,&c.io);s->words[5]=02651;
}
