/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "p66_application_recurrence.h"
static uint8_t enter(void *u){(void)u;return 0;}static void leave(void *u,uint8_t p){(void)u;(void)p;}
static enum exec_action callback(executive*s,void*u,exec_address*next){(void)s;(void)u;(void)next;return EXEC_YIELD;}
static exec_continuation resolve(void*u,exec_address a,enum exec_context_kind kind){(void)u;(void)a;(void)kind;return callback;}
static const exec_exclusion guard={enter,leave,0};
static void input(p66_application_recurrence*b,apollo_overlay_state*m,exec_native*r,executive*s){p66_native_binding*a;memset(b,0,sizeof *b);memset(m,0,sizeof *m);memset(r,0,sizeof *r);memset(s,0,sizeof *s);a=&b->application.display_job.descent.basic.arithmetic;a->image=m;a->runtime=r;a->producers=&guard;r->state=s;r->resolve=resolve;m->words[5]=03214;m->words[4]=02000;m->words[3]=03400;m->words[6]=02007;m->words[067]=m->words[0167]=AGC_NEG_ZERO;m->words[01410]=03263;m->words[01411]=064067;m->words[026]=037634;b->application.display_job.descent.basic.control.interrupt_enabled=1;}
int main(void){p66_application_recurrence b,old;apollo_overlay_state m,before;exec_native r,oldr;executive s;uint16_t n,consumed;
 for(n=0;n<20u;++n){input(&b,&m,&r,&s);switch(n){case 0:r.fault=EXEC_UNMAPPED;break;case 1:r.running=1;break;case 2:r.in_callback=1;break;case 3:b.in_interrupt=1;break;case 4:b.pending_t3=2;break;case 5:m.words[5]++;break;case 6:m.words[4]=0;break;case 7:m.words[7]=1;break;case 8:m.words[0]=040000;break;case 9:b.application.display_job.descent.basic.control.interrupt_enabled=0;break;case 10:m.words[067]=0;break;case 11:m.words[0167]=01000;break;case 12:m.words[01410]=02000;break;case 13:m.words[01411]=060067;break;case 14:m.words[024]=040000;break;case 15:m.words[025]=040000;break;case 16:m.words[026]=040000;break;case 17:r.resolve=0;break;case 18:r.state=0;break;case 19:b.application.display_job.descent.basic.arithmetic.image=0;break;}
  before=m;old=b;oldr=r;consumed=123;if(p66_application_recur_prepare(&b,100,&consumed)!=EXEC_OUT_OF_SCOPE||consumed!=123||memcmp(&m,&before,sizeof m)||memcmp(&b,&old,sizeof b)||memcmp(&r,&oldr,sizeof r)){fprintf(stderr,"guard%u failed\n",n);return 1;}}
 input(&b,&m,&r,&s);before=m;consumed=123;if(p66_application_recur_prepare(&b,0,&consumed)!=EXEC_IDLE||consumed||memcmp(&m,&before,sizeof m))return 2;
 if(p66_application_recur_prepare(&b,99,&consumed)!=EXEC_IDLE||consumed!=99||m.words[026]!=037777||m.words[5]!=03214||b.in_interrupt)return 3;
 if(p66_application_recur_prepare(&b,1,&consumed)!=EXEC_OK||consumed!=1||m.words[026]||m.words[015]!=03215||m.words[017]!=010067||m.words[5]!=04014||b.in_interrupt!=1||b.pending_t3||b.application.display_job.descent.basic.control.interrupt_enabled!=1)return 4;
 if(p66_application_recur_prepare(0,1,&consumed)!=EXEC_OUT_OF_SCOPE||p66_application_recur_prepare(&b,1,0)!=EXEC_OUT_OF_SCOPE)return 5;
 puts("PASS:20 recurrence preflight rejects preserve full source/native state;2 null interfaces;zero/99/1 tick request prefix and exact architectural capture");return 0;
}
