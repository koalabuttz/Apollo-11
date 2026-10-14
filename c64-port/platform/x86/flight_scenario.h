/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_SCENARIO_H
#define FLIGHT_SCENARIO_H
/* The flight scenario inputs, on AGC time (docs/X86-FLIGHT.md), shared by
 * the x86 program and its lockstep check: DSKY keys (one every half second
 * from 2 s) and erasable pokes, each at the first idle pass through ADVAN at
 * or after its time, as tools/flight_host_run.c applies them. */
#include <stdio.h>
#include <stdlib.h>
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

typedef struct {
    const char *keys; unsigned pressed;
    struct { unsigned long cs; unsigned addr, value; } pokes[8]; unsigned npokes, poked;
} fs_inputs;

static void fs_parse(fs_inputs *in,const char *keys,const char *pokes){const char *t=pokes;
 in->keys=keys?keys:"";in->pressed=in->npokes=in->poked=0;
 while(t&&*t&&in->npokes<8u){unsigned long cs;unsigned a,v;int n=0;
  if(sscanf(t,"%lu:%o=%o%n",&cs,&a,&v,&n)<3||!n||a>=2048u)break;
  in->pokes[in->npokes].cs=cs;in->pokes[in->npokes].addr=a;in->pokes[in->npokes++].value=v&077777u;t+=n;if(*t==',')++t;}}

static agc_word fs_keycode(char c){return c=='V'?021u:c=='N'?037u:c=='E'?034u:c=='0'?020u:c=='+'?032u:c=='-'?033u:
 c=='C'?036u:c=='P'?022u:c=='K'?031u:c=='R'?022u:(agc_word)(c-'0');}

/* At a sample point: the pokes due, then a key if one is due (channel 15
 * and a KEYRUPT1 request; returns 1). */
static int fs_inputs_due(fs_inputs *in,fd_state *st){unsigned long t=fd_centiseconds(st);
 while(in->poked<in->npokes&&t>=in->pokes[in->poked].cs){st->s.words[in->pokes[in->poked].addr]=(agc_word)in->pokes[in->poked].value;++in->poked;}
 if(in->keys[in->pressed]&&t>=200ul+in->pressed*50ul){st->hw.channel15=fs_keycode(in->keys[in->pressed++]);st->pending|=RK_RUPT_BIT(RK_RUPT_KEY1);return 1;}
 return 0;}

/* FLIGHT_CH30 and FLIGHT_CH31 (octal), defaults as yaAGC's GOJAM. */
static agc_word fs_channel(const char *name,agc_word dflt){const char *v=getenv(name);return v?(agc_word)(strtoul(v,0,8)&077777u):dflt;}
#endif
