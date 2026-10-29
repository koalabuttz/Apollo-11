/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle for the interp-flight VICE run (tools/interp_flight_test.py,
 * docs/INTERPRETER-RUNTIME.md), linked to Virtual AGC. NEVER part of the
 * native port.
 *
 * capture: yaAGC runs a flight scenario from GOJAM as tools/flight_host_run.c
 * runs its reference (same cold start, the same DSKY keys pressed at ADVAN).
 * Every quiet instruction boundary outside an interrupt where Z is one of
 * the interpreter's fixed entry points that the store's resolver sends to
 * $0A01 (all but SWRETURN, BANKCALL's return, which a basic table holds) is
 * a point where the flight resident hands the job to the compact
 * interpreter. The first arrival at each distinct (entry, LOC) pair, up to
 * N, is written as a case: the 2048 erasable words and the superbank.
 *
 *   interp-flight-capture capture ROPE.bin CS KEYS N CASES.bin [CORE]
 *
 * With CORE (a yaAGC core dump, agc_engine_init's format) the scenario
 * starts from that state instead of GOJAM, and the keys are pressed from 2 s
 * after it; a space in KEYS waits another half second.
 *
 * check: each case's C64 result (the exit Z and FB and a digest of the
 * image, from platform/c64/interp_flight_check.c) must be a state yaAGC
 * reaches: yaAGC runs from the case, with interrupts held off as the C64
 * holds them, until a quiet boundary at the exit Z under the exit FB whose
 * digest is the C64's. Prints one line per case and FAIL lines.
 *
 *   interp-flight-capture check ROPE.bin CASES.bin RESULTS.bin
 *
 * Build with -DIF_LM=1 or -DIF_CM=1. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_engine.h"
#if IF_LM
#define ADVAN 03214u
#define IDLE_LO 03206u
#define IDLE_HI 03225u
static const unsigned points[]={06070,06061,06063,06106,06460,06144,06463,06466,06252,06646,06037};
#else
#define ADVAN 03231u
#define IDLE_LO 03223u
#define IDLE_HI 03242u
static const unsigned points[]={06037,06030,06032,06055,06427,06113,06432,06435,06221,06615,06006};
#endif
#define POINTS (sizeof points/sizeof points[0])
#define NEWJOB 067u
#define KEYRUPT1 5u
/* One case in CASES.bin: 2048 little-endian words, then superbank, the entry
 * point's index and two spare words (4104 bytes). */
#define CASE_WORDS 2052u
/* One C64 result in RESULTS.bin (tools/interp_flight_test.py): digest (4),
 * exit Z (2), exit FB (2), resident steps (2), status, flags (12 bytes). */
#define RESULT_BYTES 12u

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
static void cold(void){unsigned i;m=loaded;memset(m.Erasable,0,sizeof m.Erasable);
 for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
 m.InputChannel[030]=037777;m.InputChannel[031]=m.InputChannel[032]=m.InputChannel[033]=077777;
 m.Erasable[0][5]=04000;m.AllowInterrupt=1;}
static unsigned physical(unsigned z,unsigned fb,unsigned sb){unsigned bank;z&=07777u;
 if(z<02000u)return 0xffffu;
 if(z>=06000u)bank=3;else if(z>=04000u)bank=2;else{bank=(fb&076000u)>>10;if(bank>=030u&&(sb&0100u))bank+=8u;}
 return bank*1024u+(z&01777u);}
static unsigned rope_word(unsigned z,unsigned fb,unsigned sb){unsigned p=physical(z,fb,sb);return p==0xffffu?0u:(unsigned)loaded.Fixed[p/1024u][p%1024u];}
static unsigned long now(void){return ((unsigned long)(mword(024)&037777u)<<14)|(mword(025)&037777u);}
/* Scenario time: centiseconds of machine cycles since GOJAM (about 853.3
 * MCT each), whatever the program does to TIME1/TIME2. */
static unsigned long long start;
static unsigned long elapsed(void){return (unsigned long)((m.CycleCounter-start)*3u/2560u);}
static int entry(void){unsigned z=mword(5)&07777u,i;for(i=0;i<POINTS;++i)if(z==points[i])return (int)i;return -1;}

/* The digest platform/c64/interp_flight_check.c takes: two 16-bit running
 * sums over every erasable word but the clock counters 024-031 (yaAGC's
 * keep counting; the C64's are held), then the superbank. */
static unsigned long digest(unsigned sb){unsigned a=0,b=0,i;
 for(i=0;i<2048u;++i){if(i>=024u&&i<=031u)continue;a=(a+mword(i))&0xffffu;b=(b+a)&0xffffu;}
 a=(a+sb)&0xffffu;b=(b+a)&0xffffu;
 return (unsigned long)b<<16|a;}

static unsigned keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(unsigned)(c-'0');}

static int capture(unsigned long cs,const char *keys,unsigned want,const char *out,const char *rope,const char *core){
 static struct { unsigned point,loc,bankset; } seen[16384];
 unsigned nseen=0,written=0,pressed=0,waits=0,i;unsigned long n,arrivals=0;FILE *f=fopen(out,"wb");
 if(!f)return 2;
 if(!core)cold();
 else if(agc_engine_init(&m,rope,core,1)){fprintf(stderr,"cannot load core dump %s\n",core);fclose(f);return 2;}
 start=m.CycleCounter;
 for(n=0;n<2000000000ul&&elapsed()<=cs;++n){
  if(quiet()&&!m.InIsr){
   unsigned z=mword(5)&07777u;int e;
   /* DSKY keys as flight_host_run's reference presses them: at an idle pass
    * through ADVAN, from 2 s, one every half second. */
   while(keys[pressed]==' '){++pressed;++waits;}
   if(z==ADVAN&&(m.InputChannel[4]&020)&&(mword(NEWJOB)==077777u)&&keys[pressed]&&(core?elapsed():now())>=200ul+(pressed+waits)*50ul){
    m.InputChannel[015]=(int16_t)keycode(keys[pressed++]);
    if(m.AllowInterrupt){*cell(015)=(int16_t)(ADVAN+1u);*cell(017)=(int16_t)rope_word(ADVAN,mword(4),(unsigned)m.OutputChannel7);
     *cell(5)=(int16_t)(04000u+4u*KEYRUPT1);m.InIsr=1;}
    else m.InterruptRequests[KEYRUPT1]=1;}
   else if((e=entry())>=0){unsigned loc=mword(0164),bs=mword(0165);
    ++arrivals;
    for(i=0;i<nseen;++i)if(seen[i].point==(unsigned)e&&seen[i].loc==loc&&seen[i].bankset==bs)break;
    if(i==nseen&&nseen<sizeof seen/sizeof seen[0]){seen[nseen].point=(unsigned)e;seen[nseen].loc=loc;seen[nseen++].bankset=bs;
     if(written<want){unsigned char b[CASE_WORDS*2u];unsigned w[CASE_WORDS-2048u]={(unsigned)m.OutputChannel7&0160u,(unsigned)e,0,0};
      for(i=0;i<2048u;++i){b[2*i]=(unsigned char)mword(i);b[2*i+1]=(unsigned char)(mword(i)>>8);}
      for(i=0;i<CASE_WORDS-2048u;++i){b[4096u+2*i]=(unsigned char)w[i];b[4097u+2*i]=(unsigned char)(w[i]>>8);}
      fwrite(b,1,sizeof b,f);++written;
      printf("case %u: %lu cs entry %05o LOC %05o BANKSET %05o FB %05o\n",written-1u,elapsed(),points[e],loc,bs,mword(4));}}}}
  if(agc_engine(&m)){fclose(f);return 2;}}
 fclose(f);
 printf("captured %u cases (%u distinct entries, %lu arrivals) in %lu of %lu cs, %lu instructions\n",written,nseen,arrivals,elapsed(),cs,n);
 if(getenv("IF_SHOW"))printf("MODREG %05o VERBREG %05o NOUNREG %05o, %u keys pressed\n",mword(01011),mword(01001),mword(01002),pressed);
 return written?0:1;}

static int check(const char *cases,const char *results){
 FILE *fc=fopen(cases,"rb"),*fr=fopen(results,"rb");unsigned char b[CASE_WORDS*2u],r[RESULT_BYTES];unsigned k=0,fails=0,i;
 if(!fc||!fr)return 2;
 while(fread(b,1,sizeof b,fc)==sizeof b&&fread(r,1,sizeof r,fr)==sizeof r){
  unsigned long want=(unsigned long)r[0]|(unsigned long)r[1]<<8|(unsigned long)r[2]<<16|(unsigned long)r[3]<<24,n,got=0;
  unsigned z=r[4]|r[5]<<8u,fb=r[6]|r[7]<<8u,steps=r[8]|r[9]<<8u,status=r[10],sb=b[4096]|b[4097]<<8u,arrived=0;int ok=0;
  m=loaded;for(i=0;i<2048u;++i)*cell(i)=(int16_t)(b[2*i]|b[2*i+1]<<8);
  for(i=0;i<NUM_CHANNELS;++i)m.InputChannel[i]=0;
  m.InputChannel[030]=037777;m.InputChannel[031]=m.InputChannel[032]=m.InputChannel[033]=077777;
  m.InputChannel[7]=m.OutputChannel7=(int16_t)sb;m.AllowInterrupt=1;m.InIsr=0;
  memset(m.InterruptRequests,0,sizeof m.InterruptRequests);m.PendFlag=m.PendDelay=m.ExtraDelay=m.IndexValue=m.ExtraCode=m.SubstituteInstruction=0;
  for(n=0;n<4000000ul;++n){
   if(n&&quiet()&&(mword(5)&07777u)==(z&07777u)&&mword(4)==fb){++arrived;got=digest((unsigned)m.OutputChannel7&0160u);if(got==want){ok=1;break;}}
   if(agc_engine(&m))break;
   memset(m.InterruptRequests,0,sizeof m.InterruptRequests);}
  printf("case %u: %s exit Z %05o FB %05o after %u resident steps (status %u), yaAGC %lu instructions, %u arrivals, digest %08lx%s\n",
         k,ok?"ok":"FAIL",z,fb,steps,status,n,arrived,want,ok?"":" (yaAGC never reached it)");
  if(!ok||!steps)++fails;
  ++k;}
 fclose(fc);fclose(fr);
 printf("%u cases, %u failures\n",k,fails);
 return fails||!k?1:0;}

int main(int argc,char **argv){
 InhibitAlarms=1;
 if(argc==7&&!strcmp(argv[1],"capture")&&!agc_load_binfile(&loaded,argv[2]))
  return capture(strtoul(argv[3],0,0),argv[4],(unsigned)strtoul(argv[5],0,0),argv[6],argv[2],0);
 if(argc==8&&!strcmp(argv[1],"capture")&&!agc_load_binfile(&loaded,argv[2]))
  return capture(strtoul(argv[3],0,0),argv[4],(unsigned)strtoul(argv[5],0,0),argv[6],argv[2],argv[7]);
 if(argc==5&&!strcmp(argv[1],"check")&&!agc_load_binfile(&loaded,argv[2]))
  return check(argv[3],argv[4]);
 fprintf(stderr,"usage: interp-flight-capture capture ROPE.bin CS KEYS N CASES.bin [CORE] | check ROPE.bin CASES.bin RESULTS.bin\n");
 return 2;}
