/* SPDX-License-Identifier: MIT; independent source fixture inputs. */
#include "cm_restart_cases.h"
void cm_rs_prepare(cm_restart_case *c,uint16_t n) {
 uint8_t i,j; agc_word p=(agc_word)((n*173u+05735u)&AGC_MASK);
 for(i=0;i<7;++i) { for(j=0;j<12;++j) c->exec.core[i][j]=(agc_word)((p+i*71u+j*37u)&AGC_MASK); c->exec.core[i][CM_EXEC_PRIORITY]=(agc_word)(02000+CM_EXEC_FAKEPRET); }
 for(i=0;i<5;++i) { for(j=0;j<44;++j) c->exec.vac[i][j]=(agc_word)((p+i*113u+j*29u)&AGC_MASK); c->exec.vac[i][0]=0; }
 c->exec.newjob=0; c->exec.newprio=05000; c->exec.newloc[0]=03210; c->exec.newloc[1]=07065;
 c->exec.locctr=0120; c->exec.fixloc=0110; c->exec.ovfind=AGC_NEG_ZERO; c->exec.alarm=n<512 ? 01202 : 0;
 c->wait.time3=(agc_word)(p&037777); c->wait.ruptagn=AGC_NEG_ZERO; c->wait.alarm=n<512 ? 01203 : 0;
 for(i=0;i<8;++i) c->wait.interval[i]=(agc_word)(057777+i);
 for(i=0;i<9;++i) { c->wait.task[i].location=(agc_word)(03000+i); c->wait.task[i].bank=06065; }
 for(i=0;i<3;++i) c->record.failreg[i]=(agc_word)(01201+i);
 c->record.almcadr[0]=01372; c->record.almcadr[1]=06063;
 for(i=0;i<12;++i) c->record.dsptab[i]=p;
 for(i=0;i<CM_RS_WORD_COUNT;++i) c->record.word[i]=(agc_word)((p+i*313u)&AGC_MASK);
 c->record.channel11=p; c->record.channel12=(agc_word)(p^AGC_MASK); c->record.channel13=(agc_word)((n&7u)*010000u+((n&8u)!=0 ? 03777u : 0)); c->record.channel14=p;
 c->record.trap31a=(agc_word)(n&1u); c->record.trap31b=(agc_word)((n>>1)&1u); c->record.trap32=(agc_word)((n>>2)&1u);
 for(i=0;i<6;++i) { c->phases.phase[i]=(agc_word)((p+i*53u)&AGC_MASK); c->phases.complement[i]=(agc_word)(c->phases.phase[i]^AGC_MASK); c->phases.address[i].location=(agc_word)(03000+i); c->phases.address[i].bank=06003; c->phases.priority[i]=02000; }
 c->group=(uint8_t)((n-768u)%6u);
 if(n>=512 && n<768 && (n&7u)!=0) c->phases.complement[(n-512u)%6u]^=(agc_word)(1u<<((n/6u)%15u));
 if(n>=768) {
  uint8_t v=(uint8_t)((n-768u)/6u%8u);
  c->phases.phase[c->group]=(agc_word)(00402+((n/48u)%4u)*0100u);
  c->phases.complement[c->group]=(agc_word)(c->phases.phase[c->group]^AGC_MASK);
  c->phases.priority[c->group]=(n&1u)!=0 ? (agc_word)(03000^AGC_MASK) : 03000;
  if(v<6) { if(v<2) c->exec.newjob=AGC_NEG_ZERO; c->exec.core[(v/2u)*3u][CM_EXEC_PRIORITY]=AGC_NEG_ZERO; c->exec.vac[(v/2u)*2u][0]=(agc_word)(0400+(v/2u)*88u); }
  if(v==6) c->exec.vac[0][0]=0400;
 }
}
#ifndef CM_RESTART_INPUT_ONLY
int cm_rs_apply(cm_restart_case *c,uint16_t n) {
 if(n<256) { cm_rs_cleanup(&c->record,&c->exec,&c->wait); return CM_RS_OK; }
 if(n<512) { cm_rs_startsub(&c->record,&c->exec,&c->wait); return CM_RS_OK; }
 if(n<768) return cm_rs_validate(&c->exec,&c->phases);
 return cm_rs_job(&c->exec,&c->phases,c->group);
}
#endif
void cm_rs_words(const cm_restart_case *c,void (*emit)(void *,agc_word),void *u) {
 uint8_t i,j;
 for(i=0;i<7;++i) for(j=0;j<12;++j) emit(u,c->exec.core[i][j]);
 for(i=0;i<5;++i) for(j=0;j<44;++j) emit(u,c->exec.vac[i][j]);
 emit(u,c->exec.newjob);emit(u,c->exec.newprio);emit(u,c->exec.newloc[0]);emit(u,c->exec.newloc[1]);emit(u,c->exec.locctr);emit(u,c->exec.fixloc);emit(u,c->exec.ovfind);emit(u,c->exec.alarm);
 emit(u,c->wait.time3);
 for(i=0;i<8;++i) emit(u,c->wait.interval[i]);
 for(i=0;i<9;++i) { emit(u,c->wait.task[i].location);emit(u,c->wait.task[i].bank); }emit(u,c->wait.ruptagn);emit(u,c->wait.alarm);
 for(i=0;i<3;++i) emit(u,c->record.failreg[i]);
 for(i=0;i<2;++i) emit(u,c->record.almcadr[i]);
 for(i=0;i<12;++i) emit(u,c->record.dsptab[i]);
 for(i=0;i<CM_RS_WORD_COUNT;++i) emit(u,c->record.word[i]);
 emit(u,c->record.channel11);emit(u,c->record.channel12);emit(u,c->record.channel13);emit(u,c->record.channel14);emit(u,c->record.trap31a);emit(u,c->record.trap31b);emit(u,c->record.trap32);
 for(i=0;i<6;++i) { emit(u,c->phases.complement[i]);emit(u,c->phases.phase[i]);emit(u,c->phases.address[i].location);emit(u,c->phases.address[i].bank);emit(u,c->phases.priority[i]); }
}
