/* SPDX-License-Identifier: MIT */
/* Host target for tools/interp_runtime_differential.py: a compiled-routine
 * milestone's program (its target file built with
 * -Dirc_program=irc_program_compiled) with every interpretive point of the
 * file's ranges run by the runtime interpreter instead of its compiled
 * routines. The file's basic routines stay, with the interpreter's own basic
 * entries after them. IRT_RANGES_LM / IRT_RANGES_CM hold the file's physical
 * ranges ("first-last,..."), IRT_ROPE_LM / IRT_ROPE_CM the yaYUL binaries. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agc_transfer.h"
#include "interp_runtime.h"
#include "rope_image.h"
#include "p66_bias.h"
#include "cm_native_image.h"
const agc_native_program *irc_program_compiled(uint8_t cm);
/* The spec's native RTB routines (tools/interp_runtime_differential.py). */
extern const agc_routine_table irt_rtb_lm,irt_rtb_cm;
const agc_native_program *irc_program(uint8_t cm);
static agc_native_program prog[2];
static agc_routine_table basic[2][8], interp[2][2];
static agc_routine_entry *points[2];
/* The oracle stops at an outside point (IRT_STOPS_LM/CM: the file's body
 * exits and externals) only when a transfer from the file reaches it. Every
 * routine therefore runs through stopper(), which looks the point up in the
 * real program and reports EXEC_IDLE (a stop) when a routine entered inside
 * the file's ranges leaves Z at one of those points. */
static agc_native_program outer[2];
static agc_routine_table otab_b[2][8],otab_i[2][2];
static uint8_t inrange[2][40960/8];
static uint16_t stops[2][128];static unsigned nstops[2];
static enum exec_status stopper(const agc_native_program *p,apollo_overlay_state *s,void *io){
 unsigned cm=p==&outer[1],i;agc_word sb=cm?((cm_native_io *)io)->superbank:((p66_bias_io *)io)->superbank;
 uint16_t pt=agc_transfer_point(&prog[cm],s,sb),z;agc_routine call=agc_transfer_resolve(&prog[cm],s,sb);
 enum exec_status st;unsigned inside=pt<40960u&&((inrange[cm][pt>>3]>>(pt&7u))&1u);
 if(!call)return EXEC_UNMAPPED;
 st=call(&prog[cm],s,io);
 if(st==EXEC_OUT_OF_SCOPE&&getenv("IRT_TRACE"))fprintf(stderr,"refused at point %u Z %05o Q %05o A %06o L %06o FB %05o EB %05o BB %05o NEWJOB %05o SB %o FIXLOC %04o\n",pt,s->words[5],s->words[2],s->words[0],s->words[1],s->words[4],s->words[3],s->words[6],s->words[067],(unsigned)sb,s->words[0120]);
 if(st!=EXEC_OK||!inside)return st;
 z=agc_fixed_physical(s->words[5],s->words[4],sb);
 if(z==AGC_TRANSFER_NONE)return st;
 /* (0xffff in the list marks the whole-file basic rule, never an address: an
  * erasable Z, AGC_TRANSFER_NONE here, is not a stop.) */
 for(i=0;i<nstops[cm];++i)if(stops[cm][i]==z)return EXEC_IDLE;
 return st;
}
static agc_routine_table copy(const agc_routine_table *t){agc_routine_table r;agc_routine_entry *e=calloc(t->count?t->count:1u,sizeof *e);unsigned i;
 for(i=0;i<t->count;++i){e[i]=t->entries[i];e[i].call=stopper;}r.entries=e;r.count=t->count;return r;}
static void wrap(unsigned cm){unsigned i;const char *x=getenv(cm?"IRT_STOPS_CM":"IRT_STOPS_LM");
 outer[cm]=prog[cm];
 for(i=0;i<prog[cm].basic_count;++i)otab_b[cm][i]=copy(&prog[cm].basic[i]);
 for(i=0;i<prog[cm].interpretive_count;++i)otab_i[cm][i]=copy(&prog[cm].interpretive[i]);
#ifdef AGC_TRANSFER_DANZIG_MARKER
 /* (the marker is recognised by identity) */
 if(prog[cm].interpretive_count&&(prog[cm].interpretive[0].entries==agc_lm_danzig_marker.entries||prog[cm].interpretive[0].entries==agc_cm_danzig_marker.entries))
  otab_i[cm][0]=prog[cm].interpretive[0].entries==agc_lm_danzig_marker.entries?agc_lm_danzig_marker:agc_cm_danzig_marker;
#endif
 outer[cm].basic=otab_b[cm];outer[cm].interpretive=otab_i[cm];
 nstops[cm]=0;
 while(x&&*x&&nstops[cm]<128u){char *e;unsigned long v=strtoul(x,&e,10);if(e==x)break;stops[cm][nstops[cm]++]=(uint16_t)v;x=*e==','?e+1:e;}
}
const agc_native_program *irc_program(uint8_t cm){
 static uint8_t ready[2];
 if(cm>1u)return 0;
 if(!ready[cm]){
  const agc_native_program *p=irc_program_compiled(cm);
  const char *r=getenv(cm?"IRT_RANGES_CM":"IRT_RANGES_LM"),*rope=getenv(cm?"IRT_ROPE_CM":"IRT_ROPE_LM");
  static uint8_t in[40960/8];unsigned i,n=0,k=0;
  if(!p||!rope||!apollo_rope_load(cm,rope)){fprintf(stderr,"interp_runtime_target: no program or rope for mission %u\n",cm);exit(2);}
  memset(in,0,sizeof in);
  while(r&&*r){unsigned a,b;char *e;a=(unsigned)strtoul(r,&e,10);if(*e!='-')break;b=(unsigned)strtoul(e+1,&e,10);
   for(i=a;i<=b&&i<40960u;++i){in[i>>3]|=(uint8_t)(1u<<(i&7u));inrange[cm][i>>3]|=(uint8_t)(1u<<(i&7u));}
   r=*e==','?e+1:e;}
  for(i=0;i<40960u;++i)n+=(in[i>>3]>>(i&7u))&1u;
  points[cm]=calloc(n?n:1u,sizeof *points[cm]);
  for(i=0;i<40960u;++i)if((in[i>>3]>>(i&7u))&1u){points[cm][k].physical=(uint16_t)i;points[cm][k].call=cm?cm_interp_runtime:lm_interp_runtime;++k;}
  prog[cm]=*p;
  if(p->basic_count>6u){fprintf(stderr,"too many basic tables\n");exit(2);}
  /* The spec's RTB natives first: the file's compiled basic code may hold
   * a routine at an RTB target that only its inline call ever reached. */
  basic[cm][0]=cm?irt_rtb_cm:irt_rtb_lm;
  for(i=0;i<p->basic_count;++i)basic[cm][i+1u]=p->basic[i];
  basic[cm][i+1u]=cm?cm_interp_runtime_basic:lm_interp_runtime_basic;
  prog[cm].basic=basic[cm];prog[cm].basic_count=(uint8_t)(p->basic_count+2u);
  k=0;
#ifdef AGC_TRANSFER_DANZIG_MARKER
  if(p->interpretive_count&&(p->interpretive==&agc_lm_danzig_marker||p->interpretive==&agc_cm_danzig_marker))interp[cm][k++]=p->interpretive[0];
#endif
  interp[cm][k].entries=points[cm];interp[cm][k].count=(uint16_t)n;++k;
  prog[cm].interpretive=interp[cm];prog[cm].interpretive_count=(uint8_t)k;
  wrap(cm);
  ready[cm]=1;
 }
 return &outer[cm];
}
