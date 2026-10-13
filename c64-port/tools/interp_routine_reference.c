/* SPDX-License-Identifier: GPL-2.0-or-later; host-only oracle for compiled-routine milestones. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#include "interp_case.h"
/* Runs the unmodified ropes (argv[1] Luminary099, argv[2] Comanche055) on
 * each case of argv[3] (written by tools/interp_routine_differential.py)
 * from its image to its stop: NEWOPS with LOC outside the file's ranges,
 * an exit of the interpreter's own bodies, or (reached from the file) one
 * of its outside basic addresses or the case's own stop (a caller's
 * return, or up to four words past it: a skip return, INCR Q and RETURN,
 * or P37's INDEX RTENCKEX, TCF 0 +4), all as physical fixed addresses.
 * Prints the case, the kind (0), Z, the
 * digest and, with argv[4] = "dump", the 2048 words.
 * With IRC_SNAP set, a case's stop word is instead a physical LOC: the run
 * stops at NEWOPS with LOC there and prints "<case> S" and the (address,
 * word) pairs that differ from the case's image, or "<case> N" when it
 * never gets there (tools/interp_routine_differential.py resume_flow).
 * A case flagged in its interrupt-enable byte (0x80) carries a cut, a
 * physical LOC, after its edit count ('c64_cuts'): the run stops at the
 * first NEWOPS with LOC there, and prints kind 8 when it stops anywhere
 * else first (tests/interp_cut.h). */
static agc_t baseline[2];
static unsigned rd16(FILE *f){int a=fgetc(f),b=fgetc(f);if(a<0||b<0){fprintf(stderr,"short case file\n");exit(2);}return (unsigned)a|(unsigned)b<<8;}
static unsigned long rd32(FILE *f){unsigned long lo=rd16(f);return lo|(unsigned long)rd16(f)<<16;}
/* A from_file entry 0xffff followed by INTPRET's address (Lane G's whole-file
 * basic compiles): any transfer from the file to a fixed address outside it
 * other than INTPRET stops too. */
/* Further entries after those two are counter cells yaAGC's device models
 * empty (OPTX/OPTY under channel 14's drive bits): words the file never
 * addresses, so the run leaves them as the case set them. */
struct stops{unsigned newops,nr,first[16],last[16],n,ext[64],nf,from_file[64],any,intpret,nd,device[8];};
static unsigned outside(const struct stops *t,unsigned p){unsigned i;for(i=0;i<t->nr;++i)if(p>=t->first[i]&&p<=t->last[i])return 0;return 1;}
/* Physical fixed address of Z (0xfffe when Z is erasable; 0xffff is no stop). */
static unsigned z_physical(unsigned z){unsigned bank=(get(4)>>10)&037u;if(bank>=030u&&(machine.OutputChannel7&0100))bank+=8u;
 if(z<02000u)return 0xfffeu;
 if(z>=04000u)return 2048u+(z-04000u);
 return bank*1024u+(z&01777u);
}
static unsigned loc_physical(void){unsigned loc=get(0164),bank=(get(0165)>>10)&037u;if(bank>=030u&&(machine.OutputChannel7&0100))bank+=8u;
 if(loc>=04000u)return 2048u+(loc-04000u);
 return bank*1024u+(loc&01777u);
}
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,count,m,dump,trace=getenv("IRC_TRACE")?(unsigned)atoi(getenv("IRC_TRACE")):~0u;unsigned flow=getenv("IRC_FLOW")!=0;struct stops st[2];FILE *f;static irc_edit edits[4096];static agc_word init[2048];unsigned snapmode=getenv("IRC_SNAP")!=0;
 if(argc<4)return 2;
 dump=argc>4&&!strcmp(argv[4],"dump");InhibitAlarms=1;
 for(i=0;i<2;++i){memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+i]))return 2;baseline[i]=machine;}
 f=fopen(argv[3],"rb");if(!f)return 2;
 if(rd32(f)!=0x32435249ul){fprintf(stderr,"bad case file\n");return 2;}
 for(m=0;m<2;++m){st[m].newops=rd16(f);st[m].nr=rd16(f);if(st[m].nr>16u)return 2;for(i=0;i<st[m].nr;++i){st[m].first[i]=rd16(f);st[m].last[i]=rd16(f);}
  st[m].n=rd16(f);if(st[m].n>64u)return 2;for(i=0;i<st[m].n;++i)st[m].ext[i]=rd16(f);
  st[m].nf=rd16(f);if(st[m].nf>64u)return 2;for(i=0;i<st[m].nf;++i)st[m].from_file[i]=rd16(f);
  st[m].any=0;st[m].nd=0;for(i=0;i+1<st[m].nf;++i)if(st[m].from_file[i]==0xffffu){unsigned j;st[m].any=1;st[m].intpret=st[m].from_file[i+1];
   for(j=i+2;j<st[m].nf&&st[m].nd<8u;++j)st[m].device[st[m].nd++]=st[m].from_file[j];break;}}
 count=(unsigned)rd32(f);
 for(n=0;n<count;++n){irc_case c;apollo_overlay_state s;unsigned stopped=0,stop,zp,prev_in=1,was_in=0,cut=0,atcut=0;const struct stops *t;
  c.seed=(uint32_t)rd32(f);c.cm=(uint8_t)fgetc(f);c.interrupt_enabled=(uint8_t)fgetc(f);c.superbank=(agc_word)rd16(f);stop=rd16(f);c.nedits=(uint16_t)rd16(f);
  if(c.nedits>4096u)return 2;
  if(c.interrupt_enabled&0x80u){c.interrupt_enabled&=0x7fu;cut=rd16(f);}
  for(i=0;i<c.nedits;++i){edits[i].addr=(uint16_t)rd16(f);edits[i].word=(agc_word)rd16(f);}
  c.edits=edits;irc_build(&s,&c);t=&st[c.cm];
  {unsigned snap=0xffffu,snapped=0;if(snapmode){snap=stop;stop=0xffffu;for(i=0;i<2048u;++i)init[i]=s.words[i];}
  machine=baseline[c.cm];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  if(irc_ch_on){for(i=0;i<IRC_NCH;++i)machine.InputChannel[irc_ch_num[i]]=(int16_t)irc_ch[i];machine.Trap31A=machine.Trap31B=machine.Trap32=0;}
  for(cycles=0;cycles<4000000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);
    if(pc==t->newops&&snap!=0xffffu&&loc_physical()==snap){printf("%u S",n);
     for(i=0;i<2048u;++i){unsigned w=(unsigned)*memory(i)&(i<2u?0177777u:077777u);if(w!=init[i])printf(" %u %u",i,w);}
     printf("\n");snapped=stopped=1;break;}
    if(pc==t->newops&&cut&&loc_physical()==cut){stopped=atcut=1;break;}
    if(pc==t->newops){unsigned p=loc_physical();if(flow){unsigned v=get(0120);fprintf(stderr,"F %u %u %u %u %u %u %u %u\n",n,p,(get(0166)-v)&077777u,get(0163),get(v+046),get(v+047),get(v+050),get(v+051));}if(trace==n){printf("T %05o",get(0164));for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));printf("\n");}if(outside(t,p)){stopped=1;break;}}
    if(trace==n&&getenv("IRC_ZTRACE"))fprintf(stderr,"Z %05o FB %05o A %06o L %06o Q %05o MPAC %05o %05o %05o BUF %05o %05o %05o BUF2 %05o %05o MPTEMP %05o IE %d\n",pc,get(4),(unsigned)*memory(0)&0177777u,(unsigned)*memory(1)&0177777u,get(2),get(0154),get(0155),get(0156),get(0130),get(0131),get(0132),get(0134),get(0135),get(0136),machine.AllowInterrupt);
    zp=z_physical(pc);
    /* The file's own transfers out (and the case's return) stop only when
     * the file made them; the interpreter's exits stop from anywhere. */
    if(prev_in){if(zp==stop||(stop!=0xffffu&&zp>stop&&zp<=stop+4u&&outside(t,zp))){stopped=1;break;}for(i=0;i<t->nf&&t->from_file[i]!=0xffffu;++i)if(zp==t->from_file[i]){stopped=1;break;}
}
    /* (any: only once Z has been in the file's own words.) */
    if(t->any&&was_in&&zp<0xfffeu&&zp!=t->intpret&&outside(t,zp))stopped=1;
    if(zp<0xfffeu)was_in=!outside(t,zp);
    for(i=0;i<t->n;++i)if(zp==t->ext[i]){stopped=1;break;}
    if(zp!=0xfffeu)prev_in=!outside(t,zp); /* (TC Q runs the word in Q: still from the file) */
    if(stopped)break;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(snapmode){if(!snapped)printf("%u N\n",n);continue;}}
  if(!stopped){fprintf(stderr,"case %u timeout %05o\n",n,pc);printf("%u 9 %u 0\n",n,pc);continue;}
  if(cut&&!atcut){printf("%u 8 %u 0\n",n,pc);continue;}
  {agc_word keep[8];for(i=0;i<t->nd;++i)keep[i]=s.words[t->device[i]];
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
  for(i=0;i<t->nd;++i)s.words[t->device[i]]=keep[i];}
  if(irc_ch_on){for(i=0;i<IRC_NCH;++i)irc_ch[i]=(agc_word)((unsigned)machine.InputChannel[irc_ch_num[i]]&077777u);irc_trap[0]=(agc_word)machine.Trap31A;irc_trap[1]=(agc_word)machine.Trap31B;irc_trap[2]=(agc_word)machine.Trap32;}
  printf("%u 0 %u %lu",n,pc,(unsigned long)irc_digest(0,&s,(uint16_t)machine.AllowInterrupt));
  if(dump){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]);printf(" %u",(unsigned)machine.AllowInterrupt);}
  printf("\n");
 }return 0;
}
