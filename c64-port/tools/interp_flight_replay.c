/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only diagnostic for tools/interp_flight_test.py, linked to Virtual
 * AGC. NEVER part of the native port. Replays one interp-flight case slot
 * by slot with the host build of the compact interpreter
 * (src/lm_interp_core.c, src/cm_interp_core.c) and checks each slot in
 * lockstep against yaAGC run from the slot's start (interrupts held off):
 * yaAGC must reach the slot's end Z under its FB with the same erasable
 * memory (clock counters 024-031 aside). The first slot that yaAGC does
 * not match is shown word by word against yaAGC's first arrival there.
 *
 *   interp-flight-replay ROPE.bin YAAGC-ROPE.bin CASES.bin INDEX
 *
 * Build with -DIF_LM=1 or -DIF_CM=1. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#include "interp_runtime.h"
#include "rope_image.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "interp_word_mct.h"
#include "agc_transfer.h"
#if IF_LM
#define MISSION 0u
#define CORE lm_interp_core
#define CORE_MCT lm_interp_core_mct
#define FETCH 06070u
static const unsigned points[]={06070,06061,06063,06106,06460,06144,06463,06466,06252,06646,06037};
#else
#define MISSION 1u
#define CORE cm_interp_core
#define CORE_MCT cm_interp_core_mct
#define FETCH 06037u
static const unsigned points[]={06037,06030,06032,06055,06427,06113,06432,06435,06221,06615,06006};
#endif
#define POINTS (sizeof points/sizeof points[0])
#define CASE_BYTES 4104u

void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void RequestRadarData(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int data){(void)s;(void)data;}
void UnblockSocket(int n){(void)n;}
static agc_t loaded,m;
static int16_t *cell(unsigned a){return &m.Erasable[a/0400u][a%0400u];}
static unsigned mword(unsigned i){return (unsigned)*cell(i)&(i<2u?0177777u:077777u);}
static int quiet(void){return !m.PendFlag&&!m.ExtraDelay&&!m.IndexValue&&!m.ExtraCode&&!m.SubstituteInstruction;}
static int at_point(const apollo_overlay_state *s){unsigned i;for(i=0;i<POINTS;++i)if((s->words[5]&07777u)==points[i])return 1;return 0;}
static void load(const apollo_overlay_state *s,unsigned sb){unsigned i;
 m=loaded;for(i=0;i<2048u;++i)*cell(i)=(int16_t)s->words[i];
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
 m.InputChannel[030]=037777;m.InputChannel[031]=m.InputChannel[032]=m.InputChannel[033]=077777;
 m.InputChannel[7]=m.OutputChannel7=(int16_t)sb;m.AllowInterrupt=1;m.InIsr=0;
 memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.PendFlag=m.PendDelay=m.ExtraDelay=m.IndexValue=m.ExtraCode=m.SubstituteInstruction=0;}
static int same(const apollo_overlay_state *s){unsigned i;for(i=0;i<2048u;++i)if(!(i>=024u&&i<=031u)&&mword(i)!=s->words[i])return 0;return 1;}

int main(int argc,char **argv){static apollo_overlay_state s,before,first;unsigned char b[CASE_BYTES];FILE *f;unsigned i,slot,sb;long idx;
#if IF_LM
 p66_bias_io io={0,1};
#else
 cm_native_io io;memset(&io,0,sizeof io);io.interrupt_enabled=1;
#endif
 InhibitAlarms=1;
 if(argc!=5||!apollo_rope_load(MISSION,argv[1])||agc_load_binfile(&loaded,argv[2])||!(f=fopen(argv[3],"rb"))){
  fprintf(stderr,"usage: interp-flight-replay ROPE.bin YAAGC-ROPE.bin CASES.bin INDEX\n");return 2;}
 idx=strtol(argv[4],0,0);
 if(fseek(f,idx*(long)CASE_BYTES,SEEK_SET)||fread(b,1,sizeof b,f)!=sizeof b)return 2;
 fclose(f);
 for(i=0;i<2048u;++i)s.words[i]=(agc_word)(b[2*i]|b[2*i+1]<<8);
 sb=b[4096]|b[4097]<<8u;io.superbank=(agc_word)sb;
 printf("case %ld: Z %05o FB %05o LOC %05o BANKSET %05o EDOP %05o superbank %03o\n",idx,s.words[5],s.words[4],s.words[0164],s.words[0165],s.words[063],sb);
 for(slot=0;slot<4000u;++slot){enum exec_status st;unsigned long n;int arrived=0,ok=0;unsigned zsb=io.superbank;
  unsigned long words=0;
  before=s;CORE_MCT=0;st=CORE(0,&s,&io);
  load(&before,zsb);
  for(n=0;n<2000000ul;++n){
   if(n&&quiet()&&(mword(5)&07777u)==(s.words[5]&07777u)&&mword(4)==s.words[4]){
    if(same(&s)&&(unsigned)(m.OutputChannel7&0160)==io.superbank){ok=1;break;}
    if(!arrived++)for(i=0;i<2048u;++i)first.words[i]=(agc_word)mword(i);}
   /* The work clock's words, as tools/flight_host_run.c word_fetch counts
    * them: each quiet arrival at the fetch point (NEWOPS), the word at LOC. */
   if(quiet()&&(mword(5)&07777u)==FETCH){agc_word fb=(agc_word)mword(4),sbk=(agc_word)(m.OutputChannel7&0160);
    uint16_t p=agc_fixed_physical((agc_word)mword(0164),fb,sbk);
    words+=4ul*interp_word_units_of(p==AGC_TRANSFER_NONE?0u:(agc_word)loaded.Fixed[p/1024u][p%1024u]);}
   if(agc_engine(&m))break;
   memset(m.InterruptRequests,0,sizeof m.InterruptRequests);}
  printf("slot %u: status %d Z %05o FB %05o LOC %05o -> %s (%lu yaAGC instructions), charge %u MCT, yaAGC's words %lu%s\n",slot,(int)st,s.words[5],s.words[4],s.words[0164],ok?"ok":"MISMATCH",n,
         (unsigned)CORE_MCT,words,st==EXEC_OK&&CORE_MCT!=words?" CHARGE MISMATCH":"");
  if(!ok){
   if(!arrived)printf("  yaAGC never arrived at Z %05o FB %05o\n",s.words[5],s.words[4]);
   else for(i=0;i<2048u;++i)if(!(i>=024u&&i<=031u)&&first.words[i]!=s.words[i])printf("  %04o: native %06o yaAGC %06o (before %06o)\n",i,s.words[i],first.words[i],before.words[i]);
   return 1;}
  if(st!=EXEC_OK||!at_point(&s))break;}
 printf("left the interpreter: Z %05o FB %05o\n",s.words[5],s.words[4]);
 return 0;}
