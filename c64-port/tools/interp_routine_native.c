/* SPDX-License-Identifier: MIT */
/* Host runner for compiled-routine milestones: each case of argv[1] through
 * the milestone's routines (irc_program, from the milestone's target file)
 * to its stop. Prints what tools/interp_routine_reference.c prints. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp_case.h"
#include "cm_native_image.h"
const agc_native_program *irc_program(uint8_t cm);
static unsigned rd16(FILE *f){int a=fgetc(f),b=fgetc(f);if(a<0||b<0){fprintf(stderr,"short case file\n");exit(2);}return (unsigned)a|(unsigned)b<<8;}
static unsigned long rd32(FILE *f){unsigned long lo=rd16(f);return lo|(unsigned long)rd16(f)<<16;}
/* visits: the points the run enters, in agc_transfer_run's order, so the
 * differential can pick C64 cases that stay within one group of routines. */
static uint16_t visit[256];static unsigned nvisit;
static void irc_visits(const agc_native_program *p,apollo_overlay_state *s,const irc_case *c,uint16_t max_steps){
 p66_bias_io lio;cm_native_io cio;void *io;agc_word superbank=c->superbank|irc_superbank;unsigned i;agc_routine call;
 lio.superbank=superbank;lio.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *w;++i)w[i]=0;}
 cio.superbank=superbank;cio.interrupt_enabled=c->interrupt_enabled;
 {uint16_t *w=(uint16_t *)&cm_channels;for(i=0;i<sizeof cm_channels/sizeof *w;++i)w[i]=0;}
 if(irc_ch_on)irc_cm_io(&cio,0);
 io=c->cm?(void *)&cio:(void *)&lio;nvisit=0;
 while(nvisit<max_steps&&nvisit<256u){
  uint16_t pt=agc_transfer_point(p,s,superbank);
  call=agc_transfer_resolve(p,s,superbank);
  if(!call)break;
  visit[nvisit++]=pt;
  if(call(p,s,io)!=EXEC_OK)break;
 }
}
/* cover: the labels of the compiled basic code a case reaches (a build with
 * -DIRC_COVER_ON). */
static uint8_t cover[1024];
void irc_cover(unsigned k);
void irc_cover(unsigned k){if(k<8192u)cover[k>>3]=(uint8_t)(cover[k>>3]|1u<<(k&7u));}
/* With -DIRC_EDGES (tools/irc_edge_cover.c linked, the native sources built
 * with edge and comparison coverage), cover also prints g and the number
 * of each edge or comparison outcome the case takes. */
#ifdef IRC_EDGES
unsigned irc_edge_count(void);void irc_edges_reset(void);int irc_edge_hit(unsigned k);unsigned irc_edge_byte(unsigned k);
#endif
int main(int argc,char **argv){unsigned n,i,m,count,dump,visits,covers;FILE *f;static irc_edit edits[4096];
 if(argc<2)return 2;
 dump=argc>2&&!strcmp(argv[2],"dump");visits=argc>2&&!strcmp(argv[2],"visits");covers=argc>2&&!strcmp(argv[2],"cover");
 f=fopen(argv[1],"rb");if(!f)return 2;
 if(rd32(f)!=0x32435249ul)return 2;
 for(m=0;m<2;++m){unsigned k,j;rd16(f);k=rd16(f);for(i=0;i<2u*k;++i)rd16(f);for(j=0;j<2u;++j){k=rd16(f);for(i=0;i<k;++i)rd16(f);}}
 count=(unsigned)rd32(f);
 for(n=0;n<count;++n){irc_case c;apollo_overlay_state s;unsigned kind;uint16_t ie=0;
  c.seed=(uint32_t)rd32(f);c.cm=(uint8_t)fgetc(f);c.interrupt_enabled=(uint8_t)fgetc(f);c.superbank=0;(void)rd16(f);c.nedits=(uint16_t)rd16(f);
  if(c.nedits>4096u)return 2;
  for(i=0;i<c.nedits;++i){edits[i].addr=(uint16_t)rd16(f);edits[i].word=(agc_word)rd16(f);}
  c.edits=edits;irc_build(&s,&c);
  if(visits){irc_build(&s,&c);irc_visits(irc_program(c.cm),&s,&c,200);irc_build(&s,&c);}
  memset(cover,0,sizeof cover);
#ifdef IRC_EDGES
  if(covers){irc_edges_reset();irc_build(&s,&c);}
#endif
  kind=irc_run(irc_program(c.cm),&s,&c,200,&ie);
  printf("%u %u %u %lu",n,kind,(unsigned)(s.words[5]&077777u),(unsigned long)irc_digest(0,&s,ie));
  if(dump){for(i=0;i<2048u;++i)printf(" %u",(unsigned)(s.words[i]&(i<2u?0177777u:077777u)));printf(" %u",(unsigned)ie);}
  if(visits)for(i=0;i<nvisit;++i)printf(" v%u",(unsigned)visit[i]);
  if(covers)for(i=0;i<8192u;++i)if(cover[i>>3]&(1u<<(i&7u)))printf(" c%u",i);
#ifdef IRC_EDGES
  if(covers)for(i=0;i<irc_edge_count();i+=8u)if(irc_edge_byte(i>>3))for(m=0;m<8u;++m)if(irc_edge_hit(i+m))printf(" g%u",i+m);
#endif
  printf("\n");
 }return 0;
}
