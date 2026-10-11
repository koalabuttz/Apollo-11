/* SPDX-License-Identifier: MIT */
#include "cm_native_image.h"
typedef char cm_image_is_4096[(sizeof(apollo_overlay_state)==4096)?1:-1];
typedef char cm_exec_cache_fits[(sizeof(cm_executive)+sizeof(cm_restart_state)<=768)?1:-1];
typedef char cm_scheduler_views_fit[(sizeof(cm_waitlist)+sizeof(cm_phase_state)+sizeof(cm_native_io)+sizeof(cm_enema_io)<=1024)?1:-1];
static const uint16_t restart_addresses[CM_RS_WORD_COUNT]={
 027,030,0335,076,01302,01141,01142,01143,01144,01073,045,0776,
 01042,01013,01015,01012,01020,01021,01001,01002,01043,01330,
 01322,01323,01324,01325,01304,0100,01016,01044,01361,0777,0102,0106};
void cm_native_gather_executive(const apollo_overlay_state *s,const cm_native_io *io,cm_executive *e) {
 uint8_t i,j;
 for(i=0;i<7;++i) for(j=0;j<12;++j) e->core[i][j]=s->words[0154u+i*12u+j];
 for(i=0;i<5;++i) for(j=0;j<44;++j) e->vac[i][j]=s->words[0400u+i*44u+j];
 e->newjob=s->words[0067];e->newprio=s->words[0063];
 e->newloc[0]=s->words[0065];e->newloc[1]=s->words[0066];
 e->locctr=s->words[0064];e->fixloc=s->words[0120];e->ovfind=s->words[0121];e->alarm=io->executive_alarm;
}
void cm_native_commit_executive(apollo_overlay_state *s,cm_native_io *io,const cm_executive *e) {
 uint8_t i,j;
 for(i=0;i<7;++i) for(j=0;j<12;++j) s->words[0154u+i*12u+j]=e->core[i][j];
 for(i=0;i<5;++i) for(j=0;j<44;++j) s->words[0400u+i*44u+j]=e->vac[i][j];
 s->words[0067]=e->newjob;s->words[0063]=e->newprio;
 s->words[0065]=e->newloc[0];s->words[0066]=e->newloc[1];
 s->words[0064]=e->locctr;s->words[0120]=e->fixloc;s->words[0121]=e->ovfind;io->executive_alarm=e->alarm;
}
void cm_native_gather_waitlist(const apollo_overlay_state *s,const cm_native_io *io,cm_waitlist *w) {
 uint8_t i;w->time3=s->words[026];w->ruptagn=s->words[0734];w->alarm=io->waitlist_alarm;
 for(i=0;i<8;++i) w->interval[i]=s->words[01400u+i];
 for(i=0;i<9;++i) { w->task[i].location=s->words[01410u+i*2u];w->task[i].bank=s->words[01411u+i*2u]; }
}
void cm_native_commit_waitlist(apollo_overlay_state *s,cm_native_io *io,const cm_waitlist *w) {
 uint8_t i;s->words[026]=w->time3;s->words[0734]=w->ruptagn;io->waitlist_alarm=w->alarm;
 for(i=0;i<8;++i) s->words[01400u+i]=w->interval[i];
 for(i=0;i<9;++i) { s->words[01410u+i*2u]=w->task[i].location;s->words[01411u+i*2u]=w->task[i].bank; }
}
void cm_native_gather_restart(const apollo_overlay_state *s,const cm_native_io *io,cm_restart_state *r) {
 uint8_t i;for(i=0;i<3;++i) r->failreg[i]=s->words[0375u+i];
 for(i=0;i<2;++i) r->almcadr[i]=s->words[01363u+i];
 for(i=0;i<12;++i) r->dsptab[i]=s->words[01023u+i];
 for(i=0;i<CM_RS_WORD_COUNT;++i) r->word[i]=s->words[restart_addresses[i]];
 r->channel11=io->channel11;r->channel12=io->channel12;r->channel13=io->channel13;r->channel14=io->channel14;
 r->trap31a=io->trap31a;r->trap31b=io->trap31b;r->trap32=io->trap32;
}
void cm_native_commit_restart(apollo_overlay_state *s,cm_native_io *io,const cm_restart_state *r) {
 uint8_t i;for(i=0;i<3;++i) s->words[0375u+i]=r->failreg[i];
 for(i=0;i<2;++i) s->words[01363u+i]=r->almcadr[i];
 for(i=0;i<12;++i) s->words[01023u+i]=r->dsptab[i];
 for(i=0;i<CM_RS_WORD_COUNT;++i) s->words[restart_addresses[i]]=r->word[i];
 io->channel11=r->channel11;io->channel12=r->channel12;io->channel13=r->channel13;io->channel14=r->channel14;
 io->trap31a=r->trap31a;io->trap31b=r->trap31b;io->trap32=r->trap32;
}
static void gather_phases(const apollo_overlay_state *s,cm_restart_phases *p) {
 uint8_t i;for(i=0;i<6;++i) {
  p->complement[i]=s->words[0752u+i*2u];p->phase[i]=s->words[0753u+i*2u];
  p->address[i].location=s->words[01436u+i*2u];p->address[i].bank=s->words[01437u+i*2u];p->priority[i]=s->words[01054u+i*2u];
 }
}
static void commit_phases(apollo_overlay_state *s,const cm_restart_phases *p) {
 uint8_t i;for(i=0;i<6;++i) {
  s->words[0752u+i*2u]=p->complement[i];s->words[0753u+i*2u]=p->phase[i];
  s->words[01436u+i*2u]=p->address[i].location;s->words[01437u+i*2u]=p->address[i].bank;s->words[01054u+i*2u]=p->priority[i];
 }
}
void cm_native_gather_phase(const apollo_overlay_state *s,const cm_native_io *io,cm_phase_state *p) {
 uint8_t i;gather_phases(s,&p->records);
 for(i=0;i<6;++i) p->tbase[i]=s->words[01053u+i*2u];
 for(i=0;i<2;++i) { p->longbase[i]=s->words[01135u+i];p->time[i]=s->words[024u+i]; }
 for(i=0;i<CM_PH_SCRATCH_COUNT;++i) p->scratch[i]=s->words[i<6 ? 061u+i : 070u+i-6u];
 p->a=s->words[0];p->l=s->words[1];p->q=s->words[2];p->bbank=s->words[6];p->superbank=io->superbank;p->interrupt_enabled=io->interrupt_enabled;
}
static void commit_registers(apollo_overlay_state *s,cm_native_io *io,
 agc_register a,agc_register l,agc_word q,agc_word bb,agc_word sb,agc_word irq) {
 s->words[0]=a;s->words[1]=l;s->words[2]=q;s->words[6]=bb;
 s->words[3]=(agc_word)((bb&7u)<<8);s->words[4]=(agc_word)(bb&076000);
 io->superbank=sb;io->interrupt_enabled=irq;
}
void cm_native_commit_phase(apollo_overlay_state *s,cm_native_io *io,const cm_phase_state *p) {
 uint8_t i;commit_phases(s,&p->records);
 for(i=0;i<6;++i) s->words[01053u+i*2u]=p->tbase[i];
 for(i=0;i<2;++i) { s->words[01135u+i]=p->longbase[i];s->words[024u+i]=p->time[i]; }
 for(i=0;i<CM_PH_SCRATCH_COUNT;++i) s->words[i<6 ? 061u+i : 070u+i-6u]=p->scratch[i];
 commit_registers(s,io,p->a,p->l,p->q,p->bbank,p->superbank,p->interrupt_enabled);
}
void cm_native_gather_enema_io(const apollo_overlay_state *s,const cm_native_io *io,cm_enema_io *p) {
 p->main_keys=io->main_keys;p->navigation_keys=io->navigation_keys;
 p->a=s->words[0];p->l=s->words[1];p->q=s->words[2];p->bbank=s->words[6];p->superbank=io->superbank;p->interrupt_enabled=io->interrupt_enabled;
 p->waitexit=s->words[061];p->waitbank=s->words[062];
}
void cm_native_commit_enema_io(apollo_overlay_state *s,cm_native_io *io,const cm_enema_io *p) {
 io->main_keys=p->main_keys;io->navigation_keys=p->navigation_keys;
 s->words[061]=p->waitexit;s->words[062]=p->waitbank;
 commit_registers(s,io,p->a,p->l,p->q,p->bbank,p->superbank,p->interrupt_enabled);
}
