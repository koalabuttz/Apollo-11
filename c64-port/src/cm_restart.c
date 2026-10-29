/* SPDX-License-Identifier: MIT
 * Comanche 055 cleanup/phase agreement/variable-job source translation. */
#include "cm_restart.h"
void cm_rs_cleanup(cm_restart_state *s,cm_executive *e,cm_waitlist *w) {
 uint8_t i;
 s->channel11&=077603;
 s->channel13&=074777;
 if(s->channel13&010000) s->trap31b=1;
 if(s->channel13&020000) s->trap32=1;
 s->channel13&=043777;
 s->word[CM_RS_FLAGWRD2]=(agc_word)((s->word[CM_RS_FLAGWRD2]&052777)+01000);
 for(i=0;i<8;++i) w->interval[i]=CM_WL_DEFAULT_INTERVAL;
 for(i=0;i<9;++i) { w->task[i].location=CM_WL_END_LOCATION; w->task[i].bank=CM_WL_END_BANK; }
 for(i=0;i<7;++i) e->core[i][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
 s->word[CM_RS_DSRUPTSW]=AGC_NEG_ZERO; e->newjob=AGC_NEG_ZERO;
 for(i=0;i<5;++i) e->vac[i][0]=(agc_word)(CM_EXEC_VAC1USE+i*44u);
 for(i=0;i<11;++i) s->dsptab[i]=073777;
 e->core[0][CM_EXEC_MPAC]=0;
 for(i=CM_RS_DELAYLOC;i<=CM_RS_FLAGWRD4;++i) s->word[i]=0;
 s->word[CM_RS_NOUT]=013; s->word[CM_RS_EXTVBACT]&=020000;
 s->word[CM_RS_SELFRET]=03340; s->word[CM_RS_DSPCOUNT]=077754;
 e->alarm=0; w->alarm=0;
}
void cm_rs_startsub(cm_restart_state *s,cm_executive *e,cm_waitlist *w) {
 s->word[CM_RS_DNTMGOTO]=03530; w->time3=037777;
 s->word[CM_RS_TIME4]=037775; s->word[CM_RS_TIME5]=037774;
 cm_rs_cleanup(s,e,w);
}
enum cm_rs_status cm_rs_validate(cm_executive *e,const cm_restart_phases *p) {
 uint8_t i=6;
 while(i!=0) { --i; e->core[0][5]=i;
  if((agc_word)(p->complement[i]^p->phase[i])!=AGC_NEG_ZERO) return CM_RS_PHASE_BAD;
 }
 e->core[0][6]=0; return CM_RS_OK;
}
static int supported(const cm_restart_phases *p,uint8_t i) {
 agc_word phase=p->phase[i],priority=p->priority[i];
 agc_word corrected=(priority&AGC_SIGN)!=0 ? agc_negate(priority) : priority;
 return phase<=01777 && (phase&01407)==00402 && corrected!=0 &&
 (corrected&0140777)==0 && p->address[i].location>0 && p->address[i].location<=07777 && p->address[i].bank<=AGC_MASK;
}
enum exec_status cm_rs_job(cm_executive *e,const cm_restart_phases *p,uint8_t group) {
 agc_word priority;
 if(group>=6 || !supported(p,group)) return EXEC_OUT_OF_SCOPE;
 if(e->alarm!=0) return EXEC_BAILOUT_PENDING;
 if(e->core[0][CM_EXEC_PRIORITY]==AGC_NEG_ZERO && e->newjob!=AGC_NEG_ZERO) return EXEC_OUT_OF_SCOPE;
 e->core[0][0]=p->phase[group]; e->core[0][1]=(agc_word)(group*2u);
 e->core[0][3]=03755; e->core[0][5]=group;
 e->vac[4][21]=p->address[group].location; e->vac[4][22]=p->address[group].bank; e->vac[4][23]=04675;
 priority=p->priority[group]; e->vac[4][20]=(priority&AGC_SIGN)!=0 ? 05134 : 05147;
 return cm_exec_admit(e,(priority&AGC_SIGN)!=0 ? agc_negate(priority) : priority,p->address[group].location,p->address[group].bank,(priority&AGC_SIGN)==0);
}
