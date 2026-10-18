/* SPDX-License-Identifier: MIT */
/* The flight resident's timers and interrupt entry (src/resident_rupt.c)
 * against yaAGC: every case of tests/resident_rupt_cases.c must give the
 * digest the reference (tools/resident_rupt_reference.c, frozen in
 * tests/resident_rupt_vectors.h by tools/resident_rupt_vectors.py, one
 * digest per block of RR_BLOCK cases) recorded.
 * Entry: which request agc_engine takes before the instruction at Z and,
 * when it takes one, Z, ZRUPT, BRUPT, the requests left and InIsr. Timers:
 * one 10 ms period of TIME1-TIME6 in its three steps, the requests raised
 * after each and channel 13.
 * The word at Z comes from the unmodified rope
 * (evidence/fixed-data-<m>-original.rope.bin) through program->fixed_data.
 * Other vectors (tests/resident_vector_cases.h): the run through the
 * resident and the wrapped interrupt bodies (tests/resident_vector_run.h)
 * against yaAGC run to the same point.
 *   test-resident-rupt [ROPE.bin]   (build with -DRK_LM=1 or -DRK_CM=1)
 * RR_SHOW=n prints T3RUPT case n's final state (as the
 * reference does), RR_SHOW_V=n other-vector case n's. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resident_rupt.h"
#include "resident_rupt_cases.h"
#include "resident_rupt_vectors.h"
#include "resident_kernel.h"
#include "resident_vector_run.h"
#if RK_LM
#include "p66_bias.h"
typedef p66_bias_io rk_io;
#define TABLES {lm_kernel_routines,lm_kernel_job_routines,lm_kernel_task_routines,lm_kernel_idle_routines}
#define MISSION 0u
#define NAME "lm"
#define VECTORS rr_lm_vectors
#else
#include "cm_native_image.h"
typedef cm_native_io rk_io;
#define TABLES {cm_kernel_routines,cm_kernel_job_routines,cm_kernel_idle_routines}
#define MISSION 1u
#define NAME "cm"
#define VECTORS rr_cm_vectors
#endif
static agc_word rope_words[36u*1024u];
static agc_word rope(unsigned mission,uint16_t physical){(void)mission;return physical<36u*1024u?rope_words[physical]:0u;}
static bool fixed(const agc_native_program *p,uint16_t physical,agc_word *word){(void)p;
 if(physical>=36u*1024u)return false;
 *word=rope_words[physical];return true;}
static int load(const char *path){FILE *f=fopen(path,"rb");unsigned i,bank;unsigned char b[2];
 if(!f)return 0;
 for(i=0;i<36u*1024u;++i){if(fread(b,1,2,f)!=2u){fclose(f);return 0;}
  bank=i/1024u;bank=bank<2u?bank+2u:bank<4u?bank-2u:bank;    /* yaYUL order 2, 3, 0, 1, 4, ... */
  rope_words[bank*1024u+i%1024u]=(agc_word)((((unsigned)b[0]<<8)|b[1])>>1);}
 fclose(f);return 1;}
int main(int argc,char **argv){const char *path=argc>1?argv[1]:"evidence/fixed-data-" NAME "-original.rope.bin";
 const char *show=getenv("RR_SHOW");uint32_t t3_steps=0,n,h,block=RR_BASIS,failures=0,checks=0;unsigned i,t3_done=0,taken=0,why[5]={0,0,0,0,0},raised[11]={0},vkind[5]={0};
 uint32_t v_routines=0;const char *show_v=getenv("RR_SHOW_V");
 if(!load(path)){fprintf(stderr,"FAIL: resident-rupt %s: cannot read %s\n",NAME,path);return 1;}
 for(n=0;n<RR_ENTRY_CASES;++n){static rr_case c;static apollo_overlay_state image;agc_hardware hw={0};agc_native_program p={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
  uint16_t pending;uint8_t got;
  c.s=&image;rr_entry_input(MISSION,(uint16_t)n,rope,&c);hw.in_interrupt=c.in_interrupt;p.hardware=&hw;p.fixed_data=fixed;pending=c.pending;
  got=rk_rupt_take(&p,c.s,c.superbank,c.interrupt_enabled,&pending);
  h=rr_mix(RR_BASIS,got);
  if(got){++taken;h=rr_mix(h,c.s->words[5]);h=rr_mix(h,c.s->words[015]);h=rr_mix(h,c.s->words[017]);
   for(i=1;i<=10u;++i)h=rr_mix(h,(uint16_t)(pending>>i&1u));
   h=rr_mix(h,hw.in_interrupt);}
  else{++checks;if(pending!=c.pending||hw.in_interrupt!=c.in_interrupt){++failures;fprintf(stderr,"entry %u: changed state without taking\n",(unsigned)n);}
   why[!(c.pending&03776u)?0:c.in_interrupt?1:!c.interrupt_enabled?2:agc_overflow(c.s->words[0])?3:4]++;}
  block=rr_fold(block,h);
  if(n%RR_BLOCK==RR_BLOCK-1u){++checks;if(block!=VECTORS.entry[n/RR_BLOCK]){++failures;fprintf(stderr,"entry cases %u-%u differ from yaAGC\n",(unsigned)(n+1u-RR_BLOCK),(unsigned)n);}block=RR_BASIS;}}
 /* T3RUPT through the kernel: the entry, then the trampoline to RESUME. */
 for(n=0;n<RR_T3_CASES;++n){static rr_case c;static apollo_overlay_state image;const agc_routine_table tables[]=TABLES;rk_io io;agc_hardware hw={0};
  agc_native_program p={tables,sizeof tables/sizeof tables[0],0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
  uint16_t pending,stop,steps=0;uint8_t done=0;enum exec_status status=EXEC_OK;
  c.s=&image;rr_t3_input(MISSION,(uint16_t)n,&c);memset(&io,0,sizeof io);io.superbank=c.superbank;io.interrupt_enabled=c.interrupt_enabled;
  p.hardware=&hw;p.fixed_data=fixed;pending=c.pending;
  stop=agc_transfer_point(&p,c.s,io.superbank);
  if(rk_rupt_take(&p,c.s,io.superbank,io.interrupt_enabled,&pending)==RK_RUPT_T3){
   status=agc_transfer_run(&p,c.s,&io,&io.superbank,stop,2000u,&steps);
   done=(uint8_t)(status==EXEC_OK&&!hw.in_interrupt);}
  if(show&&(unsigned)atoi(show)==n){fprintf(stderr,"native T3RUPT %u: %s status %d steps %u ch7 %o inten %o isr %u\n",(unsigned)n,done?"RESUME":"no RESUME",(int)status,steps,
    (unsigned)io.superbank,(unsigned)io.interrupt_enabled,(unsigned)hw.in_interrupt);
   for(i=0;i<2048u;++i)fprintf(stderr,"%04o %06o\n",i,(unsigned)(c.s->words[i]&(i<3u?0177777u:077777u)));}
  h=rr_mix(RR_BASIS,done);
  if(done){++t3_done;t3_steps+=steps;for(i=0;i<2048u;++i)h=rr_mix(h,(uint16_t)(c.s->words[i]&(i<3u?0177777u:077777u)));
   h=rr_mix(h,io.superbank);h=rr_mix(h,io.interrupt_enabled);h=rr_mix(h,hw.in_interrupt);}
  if(show&&(unsigned)atoi(show)==n)fprintf(stderr,"digest %08x\n",(unsigned)h);
  block=rr_fold(block,h);
  if(n%RR_BLOCK==RR_BLOCK-1u){++checks;if(block!=VECTORS.t3[n/RR_BLOCK]){++failures;fprintf(stderr,"T3RUPT cases %u-%u differ from yaAGC\n",(unsigned)(n+1u-RR_BLOCK),(unsigned)n);}block=RR_BASIS;}}
 /* The other vectors through the resident and the wrapped interrupt
  * bodies, to RESUME or the first point no table holds. */
 for(n=0;n<RR_VECTOR_CASES;++n){static rr_vcase c;static apollo_overlay_state image;uint16_t point,routines;uint8_t kind;
  c.s=&image;rr_vector_input(MISSION,(uint16_t)n,&c);
  kind=rr_vector_run(MISSION,&c,fixed,&point,&routines);
  ++vkind[kind];v_routines+=routines;
  if(show_v&&(unsigned)atoi(show_v)==n){fprintf(stderr,"native vector %u (request %u): kind %u point %u routines %u ch7 %o inten %o isr %u ch5 %o ch6 %o ch10 %o ch11 %o ch13 %o\n",(unsigned)n,rr_vector((uint16_t)n),kind,point,routines,
    (unsigned)(MISSION?c.cm.superbank:c.lm.superbank),(unsigned)(MISSION?c.cm.interrupt_enabled:c.lm.interrupt_enabled),(unsigned)c.hw.in_interrupt,(unsigned)c.hw.channel5,(unsigned)c.hw.channel6,
    (unsigned)c.hw.channel10,(unsigned)c.hw.channel11,(unsigned)c.hw.channel13);
   for(i=0;i<2048u;++i)fprintf(stderr,"%04o %06o\n",i,(unsigned)(c.s->words[i]&(i<3u?0177777u:077777u)));}
  block=rr_fold(block,rr_vector_digest(MISSION,&c,kind));
  if(n%RR_BLOCK==RR_BLOCK-1u){++checks;if(block!=VECTORS.vector[n/RR_BLOCK]){++failures;fprintf(stderr,"vector cases %u-%u (request %u) differ from yaAGC\n",(unsigned)(n+1u-RR_BLOCK),(unsigned)n,rr_vector((uint16_t)n));}block=RR_BASIS;}}
 /* No key in two tables: the C64 resident runs a wrapper's table alone. */
 {uint8_t count,resident,a,b;const agc_routine_table *t=rr_vector_tables(MISSION,&count,&resident);uint16_t x,y;
  for(a=0;a<count;++a)for(b=(uint8_t)(a+1u);b<count;++b)for(x=0;x<t[a].count;++x)for(y=0;y<t[b].count;++y)
   if(t[a].entries[x].physical==t[b].entries[y].physical){++failures;fprintf(stderr,"key %u in tables %u and %u\n",t[a].entries[x].physical,a,b);}
  ++checks;}
 for(n=0;n<RR_TIMER_CASES;++n){static apollo_overlay_state s;agc_word t[6];agc_hardware hw={0};uint16_t pending=0;
  rr_timer_input(n,t,&hw.channel13);for(i=0;i<6u;++i)s.words[024u+i]=t[i];
  h=RR_BASIS;
  /* The period's three steps, with the requests after the first two. */
  rk_timer_phase(&s,&hw,&pending,RK_PHASE_T4);h=rr_mix(h,pending);
  rk_timer_phase(&s,&hw,&pending,RK_PHASE_T3);h=rr_mix(h,pending);
  rk_timer_phase(&s,&hw,&pending,RK_PHASE_T5);
  for(i=1;i<=10u;++i)if(pending>>i&1u)++raised[i];
  for(i=0;i<6u;++i)h=rr_mix(h,s.words[024u+i]);
  h=rr_mix(h,pending);h=rr_mix(h,hw.channel13);
  block=rr_fold(block,h);
  if(n%RR_BLOCK==RR_BLOCK-1u){++checks;if(block!=VECTORS.timer[n/RR_BLOCK]){++failures;fprintf(stderr,"timer cases %u-%u differ from yaAGC\n",(unsigned)(n+1u-RR_BLOCK),(unsigned)n);}block=RR_BASIS;}}
 /* The radar cycle (yaAGC agc_engine.c RadarGateCounter): with channel 13
  * bit 4 set, the gate counts TIME5's steps; RADARUPT and bit 4's reset
  * come at the TIME3 step after the ninth count (yaAGC: SCALER1 036, the
  * next step before TIME5), so in the tenth period; a reset bit 4 holds the
  * count. */
 {static apollo_overlay_state s;agc_hardware hw={0};uint16_t pending;unsigned period,p,at=0;
  hw.channel13=040010u;
  for(period=1;period<=12u;++period){
   if(period==4u)hw.channel13&=(agc_word)~010u;   /* held for periods 4-5 */
   if(period==6u)hw.channel13|=010u;
   for(p=RK_PHASE_T4;p<=RK_PHASE_T5;++p){pending=0;rk_timer_phase(&s,&hw,&pending,(uint8_t)p);
    if(pending&RK_RUPT_BIT(RK_RUPT_RADAR)){if(at||p!=RK_PHASE_T3)at=99u;else at=period;}}}
  ++checks;
  if(at!=12u||(hw.channel13&010u)||hw.radar_gate){++failures;fprintf(stderr,"radar cycle: RADARUPT in period %u, channel 13 %o, gate %u\n",at,(unsigned)hw.channel13,(unsigned)hw.radar_gate);}}
 /* Resampling (P20-P25 NOEND/RESAMPLE: RADARUPT sets bit 4 again before
  * it resumes): each cycle takes nine TIME5 steps, as on yaAGC, so the
  * requests come every nine periods, at periods 10, 19, 28, 37 and 46. */
 {static apollo_overlay_state s;agc_hardware hw={0};uint16_t pending;unsigned period,p,k=0,bad=0;
  static const unsigned want[5]={10u,19u,28u,37u,46u};
  hw.channel13=010u;
  for(period=1;period<=50u;++period)
   for(p=RK_PHASE_T4;p<=RK_PHASE_T5;++p){pending=0;rk_timer_phase(&s,&hw,&pending,(uint8_t)p);
    if(pending&RK_RUPT_BIT(RK_RUPT_RADAR)){if(k>=5u||want[k]!=period||p!=RK_PHASE_T3)++bad;++k;hw.channel13|=010u;}}
  ++checks;
  if(bad||k!=5u){++failures;fprintf(stderr,"radar resample: %u requests, %u off the yaAGC periods\n",k,bad);}}
 printf("%s: resident-rupt %s: %u entry cases (%u taken; not taken: %u nothing pending, %u in an interrupt, %u inhibited, %u overflow in A, %u RELINT/INHINT/EXTEND at Z), "
        "%u T3RUPT cases (%u to RESUME, %u kernel routines), %u other-vector cases (%u to RESUME, %u to a point no table holds, %u refused, %u not taken, %u runaway; %u routines), %u timer periods (T6 %u, T5 %u, T3 %u, T4 %u raised); %u checks, %u failures\n",
        failures?"FAIL":"PASS",NAME,(unsigned)RR_ENTRY_CASES,taken,why[0],why[1],why[2],why[3],why[4],(unsigned)RR_T3_CASES,t3_done,(unsigned)t3_steps,
        (unsigned)RR_VECTOR_CASES,vkind[0],vkind[1],vkind[2],vkind[3],vkind[4],(unsigned)v_routines,(unsigned)RR_TIMER_CASES,
        raised[1],raised[2],raised[3],raised[4],(unsigned)checks,(unsigned)failures);
 return failures?1:0;
}
