/* SPDX-License-Identifier: MIT */
/* Host runner for compiled-routine milestones: each case of argv[1] through
 * the milestone's routines (irc_program, from the milestone's target file)
 * to its stop. Prints what tools/interp_routine_reference.c prints. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp_case.h"
#include "cm_native_image.h"
#ifdef IRC_CUTS
/* 'c64_cuts' specs: a case flagged in its interrupt-enable byte (0x80)
 * carries a cut after its edit count (tests/interp_cut.h). */
#include "interp_cut.h"
static uint16_t cut;
#endif
const agc_native_program *irc_program(uint8_t cm);
#ifdef IRC_EDGES
void irc_edges_reset(void);void irc_edges_print(void);void irc_edges_map(void);
#endif
/* Routines a case may run (a spec's 'max_steps' raises it for loops that
 * span several routines, as P37's GAMDV iteration). */
#ifndef IRC_MAX_STEPS
#define IRC_MAX_STEPS 200
#endif
#ifdef IRC_WRAP_EXIT
/* Linked with -Wl,--wrap=exit: a routine that calls exit(3) during a case
 * (lmi_goto_any on a cycle of erasable CADRs, which the AGC loops on until
 * the night watchman restarts it) ends that case as a timeout row, as the
 * oracle's step limit does, instead of ending the whole run (#224). */
#include <setjmp.h>
static jmp_buf irc_case_jmp;
static int irc_in_case;
void __real_exit(int);
void __wrap_exit(int);
void __wrap_exit(int code){
 if(irc_in_case&&code==3){irc_in_case=0;longjmp(irc_case_jmp,1);}
 __real_exit(code);
}
#endif
enum { IRC_TIMEOUT = 9 };
static unsigned rd16(FILE *f){int a=fgetc(f),b=fgetc(f);if(a<0||b<0){fprintf(stderr,"short case file\n");exit(2);}return (unsigned)a|(unsigned)b<<8;}
static unsigned long rd32(FILE *f){unsigned long lo=rd16(f);return lo|(unsigned long)rd16(f)<<16;}
/* visits: the points the run enters, in agc_transfer_run's order, so the
 * differential can pick C64 cases that stay within one group of routines. */
static uint16_t visit[IRC_MAX_STEPS];static unsigned nvisit;
static void irc_visits(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t max_steps){
 /* (the io records and input latches irc_run gives the routines) */
 lm_channel_io lio;cm_channel_io cio;agc_native_program q;void *io;agc_word superbank=c->superbank;unsigned i;agc_routine call;
 lio.io.superbank=c->superbank;lio.io.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cio.io;for(i=0;i<sizeof cio.io/sizeof *w;++i)w[i]=0;}
 cio.io.superbank=c->superbank;cio.io.interrupt_enabled=c->interrupt_enabled;
 irc_inputs(c,&lio.in);cio.in=lio.in;
 if(c->cm)p=irc_cm_begin(p,&q,&cio.io);
 io=c->cm?(void *)&cio:(void *)&lio;nvisit=0;
 while(nvisit<max_steps&&nvisit<IRC_MAX_STEPS){
  uint16_t pt=agc_transfer_point(p,s,superbank);
  call=agc_transfer_resolve(p,s,superbank);
  if(!call)break;
#ifdef IRC_CUTS
  if(cut&&pt==cut)break;
#endif
  visit[nvisit++]=pt;
  if(call(p,s,io)!=EXEC_OK)break;
 }
}
/* cover: the labels of the compiled basic code a case reaches (a build with
 * -DIRC_COVER_ON). */
static uint8_t cover[1024];
void irc_cover(unsigned k);
void irc_cover(unsigned k){if(k<8192u)cover[k>>3]=(uint8_t)(cover[k>>3]|1u<<(k&7u));}
/* With -DIRC_EDGE_COVER (tools/irc_edge_cover.c linked, the native sources
 * built with edge and comparison coverage), cover also prints g and the
 * number of each edge or comparison outcome the case takes. */
#ifdef IRC_EDGE_COVER
unsigned irc_edge_count(void);void irc_edges_reset(void);int irc_edge_hit(unsigned k);unsigned irc_edge_byte(unsigned k);
#endif
int main(int argc,char **argv){static unsigned n,i,m,count,dump,visits,covers,edges,fbs;static FILE *f;static irc_edit edits[4096];
 /* (static: a case may longjmp back here, IRC_WRAP_EXIT) */
 if(argc<2)return 2;
 dump=argc>2&&!strcmp(argv[2],"dump");fbs=argc>2&&!strcmp(argv[2],"fb");visits=argc>2&&!strcmp(argv[2],"visits");covers=argc>2&&!strcmp(argv[2],"cover");
 edges=argc>2&&!strcmp(argv[2],"edges");
 f=fopen(argv[1],"rb");if(!f)return 2;
 if(rd32(f)!=0x32435249ul)return 2;
 for(m=0;m<2;++m){unsigned k,j;rd16(f);k=rd16(f);for(i=0;i<2u*k;++i)rd16(f);for(j=0;j<2u;++j){k=rd16(f);for(i=0;i<k;++i)rd16(f);}}
 count=(unsigned)rd32(f);
 for(n=0;n<count;++n){irc_case c;apollo_overlay_state s;unsigned kind;uint16_t ie=0;
  c.seed=(uint32_t)rd32(f);c.cm=(uint8_t)fgetc(f);c.interrupt_enabled=(uint8_t)fgetc(f);c.superbank=(agc_word)rd16(f);(void)rd16(f);c.nedits=(uint16_t)rd16(f);
  if(c.nedits>4096u)return 2;
#ifdef IRC_CUTS
  cut=0;if(c.interrupt_enabled&0x80u){c.interrupt_enabled&=0x7fu;cut=(uint16_t)rd16(f);}
#endif
  for(i=0;i<c.nedits;++i){edits[i].addr=(uint16_t)rd16(f);edits[i].word=(agc_word)rd16(f);}
  c.edits=edits;irc_build(&s,&c);
#ifdef IRC_WRAP_EXIT
  if(setjmp(irc_case_jmp)){printf("%u %u 0 0\n",n,(unsigned)IRC_TIMEOUT);continue;}
  irc_in_case=1;
#endif
  if(visits){irc_build(&s,&c);irc_visits(irc_program(c.cm),&s,&c,IRC_MAX_STEPS);irc_build(&s,&c);}
  memset(cover,0,sizeof cover);
#ifdef IRC_EDGE_COVER
  if(covers){irc_edges_reset();irc_build(&s,&c);}
#endif
#ifdef IRC_EDGES
  if(edges)irc_edges_reset();
#endif
#ifdef IRC_CUTS
  kind=irc_run_cut(irc_program(c.cm),&s,&c,cut,IRC_MAX_STEPS,&ie);
#else
  kind=irc_run(irc_program(c.cm),&s,&c,IRC_MAX_STEPS,&ie);
#endif
#ifdef IRC_WRAP_EXIT
  irc_in_case=0;
#endif
  printf("%u %u %u %lu",n,kind,(unsigned)(s.words[5]&077777u),(unsigned long)irc_digest(0,&s,ie));
  /* IRC_Q16: Q at 16 bits, which the digest leaves at 15 (the differential compares it). */
  if(getenv("IRC_Q16"))printf(" q%u",(unsigned)(s.words[2]&0177777u));
  /* fb: the FB the run stops under, LOC and BANKSET, to place a stop in a bank (#325). */
  if(fbs)printf(" %u %u %u",(unsigned)(s.words[4]&077777u),(unsigned)(s.words[0164]&077777u),(unsigned)(s.words[0165]&077777u));
  if(dump){for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<2u?0177777u:077777u)));printf(" %u",(unsigned)ie);}
  if(visits)for(i=0;i<nvisit;++i)printf(" v%u",(unsigned)visit[i]);
  if(covers)for(i=0;i<8192u;++i)if(cover[i>>3]&(1u<<(i&7u)))printf(" c%u",i);
#ifdef IRC_EDGE_COVER
  if(covers)for(i=0;i<irc_edge_count();i+=8u)if(irc_edge_byte(i>>3))for(m=0;m<8u;++m)if(irc_edge_hit(i+m))printf(" g%u",i+m);
#endif
#ifdef IRC_EDGES
  if(edges)irc_edges_print();
#endif
  printf("\n");
 }
#ifdef IRC_EDGES
 if(edges)irc_edges_map();
#endif
 (void)edges;return 0;
}
