/* SPDX-License-Identifier: GPL-2.0-or-later
 * Lockstep check of the native x86_64 flight program (docs/X86-FLIGHT.md)
 * against Virtual AGC. Host only, NEVER part of the port: it links yaAGC.
 *
 * The x86 program's own step (platform/x86/flight_dispatch.c) runs the
 * mission from GOJAM on its virtual AGC clock, with the scenario inputs of
 * platform/x86/flight_scenario.h. Every routine it runs (and every
 * interpreter core slot) is checked: yaAGC, with its own clock frozen
 * (AgcFrozenClock, as tools/flight_host_run.c's scaled time), runs from the
 * state the routine started in until it reaches the routine's continuation
 * with the same 2048 erasable words (Z aside), the I/O channels and the
 * interrupt state. The clock periods and interrupts are the native step's
 * (rk_timer_phase and rk_rupt_take, both checked against yaAGC by
 * make test-resident-rupt), so a run with no miss is one yaAGC run whose
 * clock ran on the x86 program's schedule. A basic point no routine holds
 * fails the run (the x86 program has no bridge); interpretive points the
 * core ran are listed.
 *   x86-flight-lockstep-<m> ROPE.bin [CS [KEYS]]   (FLIGHT_CH30, FLIGHT_CH31,
 *   FLIGHT_POKE as the x86 program) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "flight_scenario.h"
#ifndef X86_INTERP_MODE
#define X86_INTERP_MODE FD_INTERP_COMPILED
#endif
#if RK_LM
#define MISSION 0u
#else
#define MISSION 1u
#endif
#include "rope_image.h"

/* Virtual AGC adapter: no peripherals; channel 7 as SocketAPI.c stores it. */
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
extern int AgcFrozenClock;
static agc_t loaded,m;
static int16_t *cell(unsigned a){return &m.Erasable[a/0400u][a%0400u];}
static agc_word mword(unsigned i){return (agc_word)(*cell(i)&(i<2u?0177777u:077777u));}
static int quiet(void){return !m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode&&!m.SubstituteInstruction;}

static const unsigned short ch[]={3,4,5,6,010,011,012,013,014,015,030,031,032,033};
#define CHANNELS (sizeof ch/sizeof ch[0])
static agc_word *hwc(agc_hardware *hw,unsigned i){agc_word *c[]={&hw->channel3,&hw->channel4,&hw->channel5,&hw->channel6,&hw->channel10,
 &hw->channel11,&hw->channel12,&hw->channel13,&hw->channel14,&hw->channel15,&hw->channel30,&hw->channel31,&hw->channel32,&hw->channel33};return c[i];}
/* After a native RESUME yaAGC runs BRUPT in place of the word at Z. */
static int substitute;
static void load(const fd_state *st){unsigned i;agc_hardware hw=st->hw;
 for(i=0;i<2048u;++i)*cell(i)=(int16_t)st->s.words[i];
 for(i=0;i<CHANNELS;++i)m.InputChannel[ch[i]]=(int16_t)*hwc(&hw,i);
 m.InputChannel[016]=(int16_t)st->io.channel16;m.InputChannel[034]=(int16_t)st->io.channel34;m.InputChannel[035]=(int16_t)st->io.channel35;
 m.InputChannel[7]=m.OutputChannel7=(int16_t)st->io.superbank;m.AllowInterrupt=st->io.interrupt_enabled!=0u;m.InIsr=st->hw.in_interrupt!=0u;
 m.Trap31A=st->io.trap31a;m.Trap31B=st->io.trap31b;m.Trap32=st->io.trap32;
 memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.DownruptTimeValid=0;
 m.SubstituteInstruction=(unsigned)substitute;m.PendFlag=0;m.PendDelay=0;m.ExtraDelay=0;m.IndexValue=0;m.ExtraCode=0;}
/* yaAGC's state as an fd_state view, for fd_point. */
static void view(fd_state *v){unsigned i;
 for(i=0;i<2048u;++i)v->s.words[i]=mword(i);
 v->io.superbank=(agc_word)(m.OutputChannel7&0160);}
/* Where the two differ, at most n words: erasable (Z aside), channels,
 * interrupt state. */
static unsigned differs(const fd_state *st,char *why,size_t len){unsigned i,d=0;size_t o=0;agc_hardware hw=st->hw;
 for(i=0;i<2048u;++i)if(i!=5u&&mword(i)!=st->s.words[i]){if(d++<6u&&o<len)o+=(size_t)snprintf(why+o,len-o," %04o %05o/%05o",i,(unsigned)st->s.words[i],(unsigned)mword(i));}
 for(i=0;i<CHANNELS;++i)if(ch[i]!=3u&&ch[i]!=4u&&((agc_word)(m.InputChannel[ch[i]]&077777))!=*hwc(&hw,i)){if(d++<6u&&o<len)o+=(size_t)snprintf(why+o,len-o," ch%02o %05o/%05o",ch[i],(unsigned)*hwc(&hw,i),(unsigned)(m.InputChannel[ch[i]]&077777));}
 /* The downlink words (FLIGHT_DOWNRUPT). */
 if((agc_word)(m.InputChannel[034]&077777)!=st->io.channel34||(agc_word)(m.InputChannel[035]&077777)!=st->io.channel35){
  if(d++<6u&&o<len)o+=(size_t)snprintf(why+o,len-o," ch34/35 %05o %05o/%05o %05o",(unsigned)st->io.channel34,(unsigned)st->io.channel35,
   (unsigned)(m.InputChannel[034]&077777),(unsigned)(m.InputChannel[035]&077777));}
 if((m.OutputChannel7&0160)!=st->io.superbank||m.AllowInterrupt!=(st->io.interrupt_enabled!=0u)||m.InIsr!=(st->hw.in_interrupt!=0u)){++d;if(o<len)snprintf(why+o,len-o," superbank/inhibit/isr");}
 return d;}

/* Routines yaAGC did not match, by table and point. */
enum { MISSES=256 };
static struct { const char *name; uint16_t key; uint8_t interp; unsigned long count; char why[160]; } misses[MISSES];
static unsigned nmisses;
static void miss(const fd_ran *r,const char *why){unsigned i;const char *name=r->table?r->table->name:"interpreter core";
 for(i=0;i<nmisses;++i)if(misses[i].name==name&&misses[i].key==r->key&&misses[i].interp==r->interp){++misses[i].count;return;}
 if(nmisses<MISSES){misses[nmisses].name=name;misses[nmisses].key=r->key;misses[nmisses].interp=r->interp;misses[nmisses].count=1;
  snprintf(misses[nmisses].why,sizeof misses[nmisses].why,"%s",why);++nmisses;}}

/* yaAGC from before until the native continuation with equal state;
 * the instructions it took, or -1 (why says where it was closest). */
static long lockstep(const fd_state *before,const fd_state *after,char *why,size_t len){static fd_state v;long n;int in,tin;
 uint16_t target=fd_point(after,&tin);unsigned best=~0u;
 load(before);substitute=0;why[0]=0;
 for(n=0;n<200000l;++n){
  if(n&&(quiet()||(m.SubstituteInstruction&&!m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode))){view(&v);
   if(fd_point(&v,&in)==target&&in==tin){char w[160];unsigned d=differs(after,w,sizeof w);
    if(!d){substitute=m.SubstituteInstruction;return n;}
    if(d<best){best=d;snprintf(why,len,"%s",w);}}}
  /* yaAGC's own DOWNRUPT timer stays off: the x86 side's clock requests them. */
  m.Downlink=0;m.DownruptTimeValid=0;
  if(agc_engine(&m))break;}
 if(best==~0u)snprintf(why,len," yaAGC never reached the native continuation (Z %05o, native Z %05o FB %05o)",(unsigned)mword(5),(unsigned)after->s.words[5],(unsigned)after->s.words[4]);
 if(getenv("X86_LOCKSTEP_TRACE")){static int shown;if(!shown++){unsigned long k;load(before);
  printf("trace: before Z %05o FB %05o EB %05o isr %u ie %u pending %04o; target Z %05o FB %05o\n",(unsigned)before->s.words[5],(unsigned)before->s.words[4],
   (unsigned)before->s.words[3],(unsigned)before->hw.in_interrupt,(unsigned)before->io.interrupt_enabled,(unsigned)before->pending,(unsigned)after->s.words[5],(unsigned)after->s.words[4]);
  for(k=0;k<24u;++k){static fd_state vv;int in2;view(&vv);printf("  yaAGC Z %05o FB %05o A %06o isr %d quiet %d pend %d xd %d iv %d xc %d sub %d point %u/%u\n",(unsigned)mword(5),(unsigned)mword(4),(unsigned)mword(0),m.InIsr,quiet(),m.PendFlag,m.ExtraDelay,m.IndexValue,m.ExtraCode,m.SubstituteInstruction,fd_point(&vv,&in2),fd_point(after,&in2));if(agc_engine(&m))break;}}}
 return -1;}

int main(int argc,char **argv){static fd_state st,before;static fs_inputs in;fd_ran ran;unsigned long cs=argc>2?strtoul(argv[2],0,0):1000ul,checked=0,instructions=0;
 char why[160];unsigned i;
 if(argc<2||!apollo_rope_load(MISSION,argv[1])||agc_load_binfile(&loaded,argv[1])){fprintf(stderr,"usage: x86-flight-lockstep ROPE.bin [CS [KEYS]]\n");return 2;}
 AgcFrozenClock=1;InhibitAlarms=1;m=loaded;
 if(!fs_parse(&in,argc>3?argv[3]:""))return 2;
 fs_start(&st,X86_INTERP_MODE);
 while(fd_centiseconds(&st)<cs){long n;
  /* An interrupt taken at a RESUME point saves the word at Z as BRUPT:
   * yaAGC then runs the vector, not BRUPT. */
  if(fd_boundary(&st))substitute=0;
  if(fs_at_idle_advan(&st)){int k=fs_inputs_due(&in,&st);
   if(k==FS_GOJAM){printf("GOJAM at %lu cs\n",fd_centiseconds(&st));substitute=0;}
   if(k)continue;}
  before=st;
  if(fd_run(&st,&ran)==EXEC_UNMAPPED){printf("no routine at basic %02o,%04o (Z %05o FB %05o) at %lu cs\n",ran.key/1024u,ran.key%1024u+02000u,
   (unsigned)st.s.words[5],(unsigned)st.s.words[4],fd_centiseconds(&st));break;}
  if(ran.status!=EXEC_OK){printf("%s refused at %02o,%04o at %lu cs\n",ran.table?ran.table->name:"interpreter core",ran.key/1024u,ran.key%1024u+02000u,fd_centiseconds(&st));break;}
  ++checked;
  if((n=lockstep(&before,&st,why,sizeof why))<0)miss(&ran,why);else instructions+=(unsigned long)n;
  fs_after_routine(&in,&st);}
 printf("%s x86 lockstep (%s): %lu cs, %lu routines and core slots checked (%lu yaAGC instructions), %u points yaAGC did not match, "
  "%lu clock periods; major mode %02o, FAILREG %05o\n",MISSION?"CM":"LM",X86_INTERP_MODE==FD_INTERP_CORE?"interpreter core":"compiled",
  fd_centiseconds(&st),checked,instructions,nmisses,st.periods,(unsigned)st.s.words[01011],(unsigned)st.s.words[0375]);
 for(i=0;i<nmisses;++i)printf("  miss %s %02o,%04o %s: %lu times;%s (native/yaAGC)\n",misses[i].interp?"interp":"basic ",misses[i].key/1024u,
  misses[i].key%1024u+02000u,misses[i].name,misses[i].count,misses[i].why);
 printf("points the interpreter core ran (%u):\n",st.ngaps);
 for(i=0;i<st.ngaps;++i)printf("  %s %02o,%04o: %lu arrivals, %lu slots%s%s\n",st.gaps[i].interp?"interp":"basic ",st.gaps[i].key/1024u,
  st.gaps[i].key%1024u+02000u,st.gaps[i].arrivals,st.gaps[i].slots,st.gaps[i].by?", refused by ":"",st.gaps[i].by?st.gaps[i].by:"");
 return nmisses||st.unmapped||fd_centiseconds(&st)<cs?1:0;}
