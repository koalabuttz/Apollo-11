/* SPDX-License-Identifier: GPL-2.0-or-later
 * FLIGHT_DOWNRUPT=1 for tools/flight_host_run.c (host only, NEVER part of the
 * native port; docs/FLIGHT-DOWNLINK.md): DOWNRUPT every 20 ms of AGC time on
 * both sides, and the downlink word stream (channels 34 and 35) logged and
 * compared. FLIGHT_DOWNLINK=PATH also writes every logged word to PATH.
 *
 * The 20 ms are SCALER1's (channel 4, 3200 counts a second): a DOWNRUPT is
 * requested whenever channel 4 passes a multiple of 64 counts, the same AGC
 * time on both sides under either clock model.
 * - yaAGC's clock (unscaled): each agc_engine call that moves channel 4 over
 *   such a multiple sets InterruptRequests[8] (DOWNRUPT; agc_engine.c
 *   "Request DOWNRUPT"). That covers the free-running reference, the
 *   lockstep spans (whose requests become the native side's) and bridged
 *   code. yaAGC's own DOWNRUPT model (20 ms after both channels are written,
 *   CpuWriteIO's Downlink and DownruptTime) is switched off.
 * - Scaled time (frozen clock): each rk_timer_tick period adds 32 counts
 *   (each rk_timer_phase step 8, 8 or 16), so every second period
 *   requests RK_RUPT_DOWN, natively and in the periods bridged code gets
 *   (ref_tick).
 * They are wrappers, agc_engine(), rk_timer_tick() and rk_timer_phase()
 * below, so the harness
 * keeps its calls; with FLIGHT_DOWNRUPT unset they do nothing more.
 *
 * Streams, each word with its channel, its AGC time (TIME2:TIME1 cs), its
 * 20 ms frame (channels 3:4 over 64) and the word-order bit (channel 13
 * bit 7):
 * - NAT, the native side: the pair DNTMEXIT writes (the words in
 *   channels 34 and 35 when a downtelemetry routine ends at TCF RESUME),
 *   any other change a native routine makes to them, and bridged code's
 *   writes (yaAGC's own).
 * - SPAN, yaAGC in lockstep: the writes of each span that matched a native
 *   routine, and bridged code's. A span that never matched is not logged.
 * - REF (unscaled only), the free-running reference's writes.
 * The writes are seen through CpuWriteIO's Downlink bits: cleared before
 * each instruction, bit 0 (34) or 1 (35) after it says it wrote that
 * channel. */
#ifndef FLIGHT_DOWNLINK_H
#define FLIGHT_DOWNLINK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "flight_io.h"
#include "resident_rupt.h"
#include "flight_gojam.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"
#if RK_LM
#define DL_RESUME LM_DNTM_RESUME
#define DL_TABLE "lm_downtelemetry_routines"
#else
#define DL_RESUME CM_DNTM_RESUME
#define DL_TABLE "cm_downtelemetry_routines"
#endif
#define DL_SYNC 077340u   /* the second word of a list (LOWIDCOD) */
#define DL_WO 0100u       /* channel 13 bit 7, the word-order bit */

typedef struct { unsigned long cs, frame; uint16_t value; uint8_t ch, wo, bridged; } dl_word;
typedef struct { dl_word *w; unsigned long n, cap; } dl_stream;
enum { DL_REF, DL_NAT, DL_SPAN, DL_STREAMS };
static const char *const dl_name[DL_STREAMS]={"REF","NAT","SPAN"};
static dl_stream dl_s[DL_STREAMS], dl_buf;
static int dl_on, dl_native;
static unsigned long dl_ref_requests, dl_unverified, dl_other;
static const char *dl_path;
/* Where each start from 04000 (the cold start, then each GOJAM) falls in
 * the native stream, and its AGC time. */
enum { DL_STARTS=1+GOJAMS };
static unsigned long dl_start_at[DL_STARTS],dl_start_cs[DL_STARTS];
static unsigned dl_starts;
static void dl_mark(const agc_word *w){
 if((w[5]&07777u)==04000u&&dl_starts<DL_STARTS){dl_start_at[dl_starts]=dl_s[DL_NAT].n;
  dl_start_cs[dl_starts++]=((unsigned long)(w[024]&037777u)<<14)|(w[025]&037777u);}}
static void dl_push(dl_stream *t,dl_word w){
 if(t->n==t->cap){t->cap=t->cap?t->cap*2u:4096u;if(!(t->w=realloc(t->w,t->cap*sizeof *t->w))){perror("downlink");exit(2);}}
 t->w[t->n++]=w;}
static unsigned long dl_frame(unsigned c3,unsigned c4){return ((((unsigned long)c3&037777ul)<<14)|(c4&037777ul))>>6;}
static int dl_crossed(unsigned a,unsigned b){return ((a&037777u)>>6)!=((b&037777u)>>6);}

/* The wrappers (see above). */
static void flight_dl_tick(apollo_overlay_state *s,agc_hardware *h,uint16_t *p){agc_word c=h->channel4;
 (rk_timer_tick)(s,h,p);
 if(dl_on&&dl_crossed(c,h->channel4))*p|=RK_RUPT_BIT(RK_RUPT_DOWN);}
static void flight_dl_phase(apollo_overlay_state *s,agc_hardware *h,uint16_t *p,uint8_t phase){agc_word c=h->channel4;
 (rk_timer_phase)(s,h,p,phase);
 if(dl_on&&dl_crossed(c,h->channel4))*p|=RK_RUPT_BIT(RK_RUPT_DOWN);}
static void dl_yaagc(agc_t *s,uint8_t ch){dl_word w;
 w.cs=((unsigned long)(s->Erasable[0][024]&037777)<<14)|(unsigned long)(s->Erasable[0][025]&037777);
 w.frame=dl_frame((unsigned)s->InputChannel[3],(unsigned)s->InputChannel[4]);
 w.value=(uint16_t)(s->InputChannel[ch]&077777);w.ch=ch;w.wo=(s->InputChannel[013]&DL_WO)!=0;w.bridged=0;
 dl_push(dl_native?&dl_buf:&dl_s[DL_REF],w);}
static int flight_dl_engine(agc_t *s){int r;int16_t c;
 if(!dl_on)return (agc_engine)(s);
 c=s->InputChannel[4];s->Downlink=0;s->DownruptTimeValid=0;
 r=(agc_engine)(s);
 if(s->Downlink&1)dl_yaagc(s,034);
 if(s->Downlink&2)dl_yaagc(s,035);
 s->Downlink=0;s->DownruptTimeValid=0;
 if(dl_crossed((unsigned)c,(unsigned)s->InputChannel[4])){s->InterruptRequests[8]=1;if(!dl_native)++dl_ref_requests;}
 return r;}
#define rk_timer_tick(s,h,p) flight_dl_tick(s,h,p)
#define rk_timer_phase(s,h,p,q) flight_dl_phase(s,h,p,q)
#define agc_engine(s) flight_dl_engine(s)

/* Hooks in flight_host_run.c's main. */
static void dl_parse(void){const char *e=getenv("FLIGHT_DOWNRUPT");dl_on=e&&*e&&strcmp(e,"0");dl_path=getenv("FLIGHT_DOWNLINK");}
/* The reference is done: yaAGC's writes from here are spans or bridges. */
static void dl_native_phase(void){dl_native=1;dl_buf.n=0;}
/* Before a span or a bridge: what yaAGC wrote before it is not the run's. */
static void dl_clear(void){dl_buf.n=0;}
static void dl_nat(agc_word ch,agc_word v,const apollo_overlay_state *s,const agc_hardware *hw){dl_word w;
 w.cs=((unsigned long)(s->words[024]&037777u)<<14)|(s->words[025]&037777u);w.frame=dl_frame(hw->channel3,hw->channel4);
 w.value=(uint16_t)(v&077777u);w.ch=(uint8_t)ch;w.wo=(hw->channel13&DL_WO)!=0u;w.bridged=0;dl_push(&dl_s[DL_NAT],w);}
/* After a native routine (from before) and its lockstep (n < 0: yaAGC never
 * matched it). */
static void dl_routine(const char *name,long n,const apollo_overlay_state *before,const apollo_overlay_state *s,const flight_io *io0,
                       const flight_io *io,const agc_hardware *hw){unsigned long i;
 if(!dl_on)return;
 dl_mark(before->words);
 if(!strcmp(name,DL_TABLE)&&s->words[5]==DL_RESUME){dl_nat(034u,io->channel34,s,hw);dl_nat(035u,io->channel35,s,hw);}
 else{if(io->channel34!=io0->channel34){dl_nat(034u,io->channel34,s,hw);++dl_other;}
  if(io->channel35!=io0->channel35){dl_nat(035u,io->channel35,s,hw);++dl_other;}}
 if(n>=0)for(i=0;i<dl_buf.n;++i)dl_push(&dl_s[DL_SPAN],dl_buf.w[i]);
 else dl_unverified+=dl_buf.n;
 dl_buf.n=0;}
/* After a bridge from w: yaAGC's writes are both sides'. */
static void dl_bridged(const agc_word *w){unsigned long i;
 if(!dl_on)return;
 dl_mark(w);
 for(i=0;i<dl_buf.n;++i){dl_word w=dl_buf.w[i];w.bridged=1;dl_push(&dl_s[DL_NAT],w);dl_push(&dl_s[DL_SPAN],w);}
 dl_buf.n=0;}
/* Words a and b (with times when timed) that differ, the first few printed. */
static unsigned long dl_compare(int a,int b){const dl_stream *x=&dl_s[a],*y=&dl_s[b];unsigned long i,n=x->n<y->n?x->n:y->n,d=0;
 for(i=0;i<n;++i){const dl_word *p=&x->w[i],*q=&y->w[i];
  if(p->ch==q->ch&&p->value==q->value&&p->wo==q->wo&&p->frame==q->frame)continue;
  if(++d<=8u)printf("  word %lu: %s ch%02o %05o wo %u frame %lu (%lu cs), %s ch%02o %05o wo %u frame %lu (%lu cs)\n",i,
   dl_name[a],p->ch,p->value,p->wo,p->frame,p->cs,dl_name[b],q->ch,q->value,q->wo,q->frame,q->cs);}
 return d;}
/* A list starts with its ID in channel 34, word order 0, and LOWIDCOD's sync
 * word in channel 35. */
static int dl_list_start(const dl_stream *t,unsigned long i){
 return i+1u<t->n&&t->w[i].ch==034u&&!t->w[i].wo&&t->w[i+1u].ch==035u&&t->w[i+1u].value==DL_SYNC;}
static void dl_report(void){unsigned long i,d,nb=0;int k;FILE *f;
 enum { IDS=16 };struct { uint16_t id; unsigned long n, first; } ids[IDS];unsigned nid=0,j;
 if(!dl_on)return;
 for(i=0;i<dl_s[DL_NAT].n;++i)nb+=dl_s[DL_NAT].w[i].bridged;
 printf("downlink: DOWNRUPT every 20 ms; %lu native words (%lu bridged), %lu yaAGC span words, %lu reference words (%lu DOWNRUPTs requested), %lu other native writes, %lu words of unmatched spans\n",
  dl_s[DL_NAT].n,nb,dl_s[DL_SPAN].n,dl_s[DL_REF].n,dl_ref_requests,dl_other,dl_unverified);
 for(i=0;i<dl_s[DL_NAT].n;++i)if(dl_list_start(&dl_s[DL_NAT],i)){uint16_t id=dl_s[DL_NAT].w[i].value;
  for(j=0;j<nid&&ids[j].id!=id;++j){}
  if(j==nid&&nid<IDS){ids[nid].id=id;ids[nid].n=0;ids[nid++].first=dl_s[DL_NAT].w[i].cs;}
  if(j<nid)++ids[j].n;}
 printf("downlink lists (native):");
 for(j=0;j<nid;++j)printf(" %05o x%lu from %lu cs",ids[j].id,ids[j].n,ids[j].first);
 printf("\n");
 d=dl_compare(DL_NAT,DL_SPAN);
 printf("downlink NAT/SPAN: %lu/%lu words, %lu of the first %lu differ\n",dl_s[DL_NAT].n,dl_s[DL_SPAN].n,d,
  dl_s[DL_NAT].n<dl_s[DL_SPAN].n?dl_s[DL_NAT].n:dl_s[DL_SPAN].n);
 if(dl_s[DL_REF].n){d=dl_compare(DL_NAT,DL_REF);
  printf("downlink NAT/REF: %lu/%lu words, %lu of the first %lu differ\n",dl_s[DL_NAT].n,dl_s[DL_REF].n,d,
   dl_s[DL_NAT].n<dl_s[DL_REF].n?dl_s[DL_NAT].n:dl_s[DL_REF].n);}
 /* After each GOJAM (tools/flight_gojam.h): the first native words. */
 for(j=1;j<dl_starts;++j){i=dl_start_at[j];
  if(i<dl_s[DL_NAT].n)printf("downlink after the GOJAM at %lu cs: first word at %lu cs ch%02o %05o wo %u, %s\n",dl_start_cs[j],
   dl_s[DL_NAT].w[i].cs,dl_s[DL_NAT].w[i].ch,dl_s[DL_NAT].w[i].value,dl_s[DL_NAT].w[i].wo,
   dl_list_start(&dl_s[DL_NAT],i)?"a list's ID word":"not a list's first word");}
 if(!dl_path)return;
 if(!(f=fopen(dl_path,"w"))){perror(dl_path);return;}
 for(j=1;j<dl_starts;++j)fprintf(f,"GOJAM %lu %lu\n",dl_start_at[j],dl_start_cs[j]);
 for(k=0;k<DL_STREAMS;++k)for(i=0;i<dl_s[k].n;++i){const dl_word *w=&dl_s[k].w[i];
  fprintf(f,"%s %lu %lu %lu %02o %05o %u%s\n",dl_name[k],i,w->cs,w->frame,w->ch,w->value,w->wo,w->bridged?" b":"");}
 fclose(f);}
#endif
