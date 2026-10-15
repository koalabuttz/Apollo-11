/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_SCENARIO_H
#define FLIGHT_SCENARIO_H
/* The flight scenario inputs, on AGC time (docs/X86-FLIGHT.md), shared by
 * the x86 program and its lockstep check, with tools/flight_host_run.c's
 * meanings:
 *   KEYS            DSKY keys, one every half second from 2 s; @CS (decimal)
 *                   moves the next key and the ones after it to CS or later
 *                   (V37E63E@9000P presses PRO at 90 s)
 *   FLIGHT_CH30-33  input channels at start (octal)
 *   FLIGHT_POKE     CS:ADDR=VALUE,... erasable words (octal)
 *   FLIGHT_LOAD     CS:FILE,...: the same from files of octal "ADDR VALUE"
 *                   lines ('#' starts a comment; tools/flight_scenarios/)
 *   FLIGHT_GOJAM    CS[,CS...]: a hardware restart
 *   FLIGHT_DOWNRUPT 1: a DOWNRUPT every 20 ms (fd_state.downrupt)
 * Pokes, restarts and keys are applied at the first idle pass through ADVAN
 * at or after their time, in that order. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flight_dispatch.h"
#if RK_LM
#define FS_ADVAN 03214u
#else
#define FS_ADVAN 03231u
#endif
#define FS_NEWJOB 067u

/* The sample point: Z at ADVAN in bank 01, no interrupt in progress or
 * pending, NEWJOB -0. (The timers count whole periods at boundaries, so
 * there is no scaler phase to wait for, as under tools/flight_host_run.c's
 * scaled time.) */
static int fs_at_idle_advan(const fd_state *st){const agc_word *w=st->s.words;
 return !st->pending&&!st->hw.in_interrupt&&(w[FS_NEWJOB]&077777u)==077777u
        &&(w[5]&07777u)==FS_ADVAN&&agc_fixed_physical(FS_ADVAN,w[4],st->io.superbank)==1024u+(FS_ADVAN-02000u);}

enum { FS_KEYS = 256, FS_POKES = 4096, FS_GOJAMS = 16 };
typedef struct {
    char keys[FS_KEYS + 1]; unsigned long key_at[FS_KEYS]; unsigned pressed;
    struct { unsigned long cs; uint16_t addr, value; } pokes[FS_POKES]; unsigned npokes, poked;
    unsigned long gojam_at[FS_GOJAMS]; unsigned ngojams, gojams;
} fs_inputs;

static int fs_poke(fs_inputs *in,unsigned long cs,unsigned a,unsigned v){
 if(a>=2048u||in->npokes>=FS_POKES)return 0;
 in->pokes[in->npokes].cs=cs;in->pokes[in->npokes].addr=(uint16_t)a;in->pokes[in->npokes++].value=(uint16_t)(v&077777u);return 1;}

/* The keys argument and the environment above; 0 (with a message) on a bad
 * value. */
static int fs_parse(fs_inputs *in,const char *keys){const char *t;unsigned long at=200ul;unsigned n=0,i,j;
 memset(in,0,sizeof *in);
 for(t=keys?keys:"";*t&&n<FS_KEYS;){
  if(*t=='@'){char *e;unsigned long v=strtoul(t+1,&e,10);if(e==t+1){fprintf(stderr,"bad key time in %s\n",keys);return 0;}if(v>at)at=v;t=e;continue;}
  in->keys[n]=*t++;in->key_at[n++]=at;at+=50ul;}
 for(t=getenv("FLIGHT_POKE");t&&*t;){unsigned long cs;unsigned a,v;int k=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&a,&v,&k)<3||!k||!fs_poke(in,cs,a,v)){fprintf(stderr,"FLIGHT_POKE=CS:ADDR=VALUE,...\n");return 0;}
  t+=k;if(*t==',')++t;}
 for(t=getenv("FLIGHT_LOAD");t&&*t;){unsigned long cs;char path[512],line[256];int k=0;FILE *f;
  if(sscanf(t,"%lu:%511[^,]%n",&cs,path,&k)<2||!k){fprintf(stderr,"FLIGHT_LOAD=CS:FILE,...\n");return 0;}
  t+=k;if(*t==',')++t;
  if(!(f=fopen(path,"r"))){fprintf(stderr,"FLIGHT_LOAD: cannot open %s\n",path);return 0;}
  while(fgets(line,sizeof line,f)){unsigned a,v;char *c=strchr(line,'#');if(c)*c=0;
   if(sscanf(line,"%o %o",&a,&v)!=2)continue;
   if(!fs_poke(in,cs,a,v)){fclose(f);fprintf(stderr,"FLIGHT_LOAD: bad address or too many words in %s\n",path);return 0;}}
  fclose(f);}
 /* In time order, keeping each time's words in the order given. */
 for(i=1;i<in->npokes;++i)for(j=i;j>0&&in->pokes[j-1].cs>in->pokes[j].cs;--j){unsigned long c=in->pokes[j].cs;uint16_t a=in->pokes[j].addr,v=in->pokes[j].value;
  in->pokes[j]=in->pokes[j-1];in->pokes[j-1].cs=c;in->pokes[j-1].addr=a;in->pokes[j-1].value=v;}
 for(t=getenv("FLIGHT_GOJAM");t&&*t&&in->ngojams<FS_GOJAMS;){char *e;in->gojam_at[in->ngojams++]=strtoul(t,&e,10);
  if(e==t){fprintf(stderr,"FLIGHT_GOJAM=CS[,CS...]\n");return 0;}t=*e==','?e+1:e;}
 return 1;}

static agc_word fs_keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}

/* A hardware restart, as yaAGC's GOJAM and tools/flight_gojam.h do it:
 * Q = Z, Z = 04000, interrupts enabled and none in progress or pending,
 * HANDRUPT traps cleared, output channels 5, 6, 10-14, 34 and 35 zeroed,
 * channel 33 bit 11 set. The superbank and the rest of erasable stay. */
static void fs_gojam(fd_state *st){agc_word *w=st->s.words;
 w[2]=w[5];w[5]=04000u;
 st->io.interrupt_enabled=1u;st->hw.in_interrupt=0u;st->pending=0u;
 st->io.trap31a=st->io.trap31b=st->io.trap32=0u;
 st->hw.channel5=st->hw.channel6=st->hw.channel10=st->hw.channel11=st->hw.channel12=st->hw.channel13=st->hw.channel14=0u;
 st->io.channel34=st->io.channel35=0u;
 st->hw.channel33|=02000u;}

enum { FS_NONE, FS_KEY, FS_GOJAM };
/* At a sample point: the pokes due, then a restart if one is due
 * (FS_GOJAM), else a key if one is due (channel 15 and a KEYRUPT1 request;
 * FS_KEY). */
static int fs_inputs_due(fs_inputs *in,fd_state *st){unsigned long t=fd_centiseconds(st);
 while(in->poked<in->npokes&&t>=in->pokes[in->poked].cs){st->s.words[in->pokes[in->poked].addr]=(agc_word)in->pokes[in->poked].value;++in->poked;}
 if(in->gojams<in->ngojams&&t>=in->gojam_at[in->gojams]){
  while(in->gojams<in->ngojams&&t>=in->gojam_at[in->gojams])++in->gojams;
  fs_gojam(st);return FS_GOJAM;}
 if(in->keys[in->pressed]&&t>=in->key_at[in->pressed]){st->hw.channel15=fs_keycode(in->keys[in->pressed++]);st->pending|=RK_RUPT_BIT(RK_RUPT_KEY1);return FS_KEY;}
 return FS_NONE;}

/* FLIGHT_CH30-33 (octal), defaults as yaAGC's GOJAM. */
static agc_word fs_channel(const char *name,agc_word dflt){const char *v=getenv(name);return v?(agc_word)(strtoul(v,0,8)&077777u):dflt;}
/* Cold start with the scenario's input channels and DOWNRUPT. */
static void fs_start(fd_state *st,uint8_t mode){const char *d=getenv("FLIGHT_DOWNRUPT");
 fd_init(st,mode,fs_channel("FLIGHT_CH30",037777u),fs_channel("FLIGHT_CH31",077777u));
 st->hw.channel32=fs_channel("FLIGHT_CH32",077777u);st->hw.channel33=fs_channel("FLIGHT_CH33",077777u);
 st->downrupt=d&&*d&&strcmp(d,"0")!=0;}
#endif
