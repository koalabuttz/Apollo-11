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
 * else first (tests/interp_cut.h).
 * With IRC_SNAP and IRC_SNAPROW set, the run stops at NEWOPS with LOC at the
 * case's stop word and prints the case's usual line there (the C64 segment
 * rows of 'c64_segments'). */
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
/* IRC_QWATCH (#409): after each SWITCH's TS Q (INDEX A, CA STATE, TS Q),
 * find the next instruction that reads or writes Q outside SWITCH's own
 * words, running on past the case's stop when it comes first, and whether
 * Q's bit 16 then equals its bit 15 (and Q's sign). Counts go to stderr
 * at the end; the rows are then not the differential's. */
static unsigned qw_on,qw_watch,qw_tsq,qw_skip=0xffffffffu,qw_frontier;static unsigned long qw_n,qw_bad_a;
static struct{char k[96];unsigned long n;}qw_tab[256];static unsigned qw_nt;
static void qw_count(const char *k){unsigned i;for(i=0;i<qw_nt;++i)if(!strcmp(qw_tab[i].k,k)){qw_tab[i].n++;return;}if(qw_nt<256u){strncpy(qw_tab[qw_nt].k,k,95);qw_tab[qw_nt++].n=1;}}
static unsigned fword(unsigned p){return (unsigned)machine.Fixed[p/1024u][p%1024u]&077777u;}
static unsigned iword(unsigned z){unsigned p=z_physical(z);return p==0xfffeu?get(z):fword(p);}
static int qdec(unsigned z,const char **nm,unsigned *endz){unsigned w=iword(z),ext=0,off=0,op,qc,k10,k12;
 for(;;){if(!ext&&w==6u){ext=1;w=iword(++z);continue;}
  if((w>>12)==5u&&((w>>10)&3u)==0u&&!(w==050017u&&!ext)){unsigned kk=ext?(w&07777u):(w&01777u);off+=(kk<02000u?(unsigned)*memory(kk):iword(kk));w=iword(++z);continue;}
  break;}
 *endz=z;w=(w+off)&077777u;op=w>>12;qc=(w>>10)&3u;k10=w&01777u;k12=w&07777u;*nm="";
 if(!ext){switch(op){
  case 0:if(w==3u||w==4u)return 0;if(k12==2u){*nm="TC Q";return 3;}*nm="TC";return 2;
  case 1:if(qc==0u&&k10==2u){*nm="CCS Q";return 1;}return 0;
  case 2:if(qc==0u&&(k10==2u||k10==3u)){*nm="DAS";return 3;}if(k10!=2u)return 0;*nm=qc==1u?"LXCH Q":qc==2u?"INCR Q":"ADS Q";return 3;
  case 3:if(k12==2u){*nm="CA Q";return 1;}return 0;
  case 4:if(k12==2u){*nm="CS Q";return 1;}return 0;
  case 5:if(qc==1u&&(k10==2u||k10==3u)){*nm="DXCH";return 3;}if(k10!=2u)return 0;if(qc==2u){*nm="TS Q";return 2;}if(qc==3u){*nm="XCH Q";return 3;}return 0;
  case 6:if(k12==2u){*nm="AD Q";return 1;}return 0;
  default:if(k12==2u){*nm="MASK Q";return 1;}return 0;}}
 switch(op){
  case 0:{unsigned io=(w>>9)&7u;static const char *n[8]={"READ Q","WRITE Q","RAND Q","WAND Q","ROR Q","WOR Q","RXOR Q","EDRUPT"};static const int t[8]={1,2,1,3,1,3,1,0};if((w&0777u)!=2u)return 0;*nm=n[io];return t[io];}
  case 1:if(qc==0u&&k10==2u){*nm="DV Q";return 1;}return 0;
  case 2:if(qc==1u){*nm="QXCH";return 3;}if(k10!=2u)return 0;*nm=qc==0u?"MSU Q":qc==2u?"AUG Q":"DIM Q";return qc==0u?1:3;
  case 3:if(k12==2u||k12==3u){*nm="DCA";return 1;}return 0;
  case 4:if(k12==2u||k12==3u){*nm="DCS";return 1;}return 0;
  case 6:if(qc==0u&&k10==2u){*nm="SU Q";return 1;}return 0;
  case 7:if(k12==2u){*nm="MP Q";return 1;}return 0;}
 return 0;}
static void qw_hook(unsigned pc){unsigned zp=z_physical(pc),endz,q,insw;const char *nm;int t;char k[96];
 if(pc==qw_skip){qw_skip=0xffffffffu;return;}qw_skip=0xffffffffu;
 if(iword(pc)==054002u&&iword(pc-1u)==030074u){unsigned a=(unsigned)*memory(0)&0177777u;
  if(qw_watch)qw_count("SWITCH again before Q was read or written");
  qw_watch=1;qw_tsq=zp;qw_frontier=0;qw_n++;if(((a>>15)&1u)!=((a>>14)&1u))qw_bad_a++;return;}
 if(!qw_watch)return;
 t=qdec(pc,&nm,&endz);if(endz!=pc)qw_skip=endz;if(!t)return;
 q=(unsigned)*memory(2)&0177777u;insw=zp!=0xfffeu&&zp+12u>=qw_tsq&&zp<=qw_tsq+40u;
 snprintf(k,sizeof k,"%s %s %+d%s: %s (Q bit 16 %s bit 15; Q %s)",insw?"inside SWITCH":"after SWITCH",t==2?"write":"read",insw?(int)zp-(int)qw_tsq:0,qw_frontier?" past the case's stop":"",nm,((q>>15)&1u)==((q>>14)&1u)?"=":"!=",(q>>14)&1u?"negative":"positive");
 if(!insw&&t!=2&&((q>>14)&1u)&&strcmp(nm,"TC Q")&&strcmp(nm,"QXCH")){char k2[160];snprintf(k2,sizeof k2,"   at %02o,%04o (SWITCH TS Q at %02o,%04o) %s",zp/1024u,(zp%1024u)+((zp/1024u==2u||zp/1024u==3u)?(zp/1024u)*02000u:02000u),qw_tsq/1024u,(qw_tsq%1024u)+02000u,nm);qw_count(k2);}
 qw_count(k);if(!insw)qw_watch=0;}
int main(int argc,char **argv){unsigned n,i,cycles,pc=0,count,m,dump,ret=0xffffu,retloc=0,trace=getenv("IRC_TRACE")?(unsigned)atoi(getenv("IRC_TRACE")):~0u;unsigned flow=getenv("IRC_FLOW")!=0;struct stops st[2];FILE *f;static irc_edit edits[4096];static agc_word init[2048];unsigned snapmode=getenv("IRC_SNAP")!=0;unsigned inputs=getenv("IRC_INPUTS")!=0;unsigned snaprow=getenv("IRC_SNAPROW")!=0;unsigned pendz=0,pendn=0,pendk[32],pende[32],pend;unsigned through=getenv("IRC_SNAP_THROUGH")!=0,maxcycles=getenv("IRC_SNAP_CYCLES")?(unsigned)atoi(getenv("IRC_SNAP_CYCLES")):4000000u;
 if(argc<4)return 2;
 if(getenv("IRC_PENDING_DANZIG")){const char *e=getenv("IRC_PENDING_DANZIG");int k=0;unsigned a,b;
  if(sscanf(e,"%u%n",&pendz,&k)==1)for(e+=k;pendn<32u&&sscanf(e," %u:%u%n",&a,&b,&k)==2;e+=k){pendk[pendn]=a;pende[pendn++]=b;}}
 dump=argc>4&&!strcmp(argv[4],"dump");qw_on=getenv("IRC_QWATCH")!=0;InhibitAlarms=1;
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
  {unsigned j=0;ret=0xffffu;retloc=0;for(i=0;i<c.nedits;++i){if(through&&edits[i].addr>=0xfffeu){ret=edits[i].word;retloc=edits[i].addr==0xfffeu;}else edits[j++]=edits[i];}c.nedits=(uint16_t)j;}
  c.edits=edits;irc_build(&s,&c);t=&st[c.cm];
  {unsigned snap=0xffffu,snapped=0;if(snapmode){snap=stop;stop=0xffffu;for(i=0;i<2048u;++i)init[i]=s.words[i];}
  machine=baseline[c.cm];
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  /* (channel_inputs specs: the differential sets IRC_INPUTS) */
  if(inputs){agc_input_channels in;irc_inputs(&c,&in);machine.InputChannel[3]=(int16_t)in.channel3;machine.InputChannel[4]=(int16_t)in.channel4;
   machine.InputChannel[030]=(int16_t)in.channel30;machine.InputChannel[031]=(int16_t)in.channel31;
   machine.InputChannel[032]=(int16_t)in.channel32;machine.InputChannel[033]=(int16_t)in.channel33;
   machine.InputChannel[016]=(int16_t)in.channel16;}
  /* A basic start counts as in the file, so a first instruction that runs
   * an erasable word (TC INTWAKUQ) still stops at the fixed address it goes
   * to outside the file. */
  {unsigned z0=z_physical(get(RegZ));if(z0<0xfffeu)was_in=!outside(t,z0);}
  if(irc_ch_on){for(i=0;i<IRC_NCH;++i)machine.InputChannel[irc_ch_num[i]]=(int16_t)irc_ch[i];machine.Trap31A=machine.Trap31B=machine.Trap32=0;}
  for(cycles=0;cycles<maxcycles;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){pc=get(RegZ);if(qw_on)qw_hook(pc);
    if(pc==t->newops&&snap!=0xffffu&&loc_physical()==snap&&snaprow){snapped=stopped=1;break;}
    if(snap!=0xffffu&&((pc==t->newops&&(!through||(get(0164)>=02000u&&get(0164)<010000u&&get(0120)==init[0120]))&&loc_physical()==snap)||(through&&pc!=t->newops&&get(0120)==init[0120]&&z_physical(pc)==snap))){printf("%u S",n);
     for(i=0;i<2048u;++i){unsigned w=(unsigned)*memory(i)&(i<2u?0177777u:077777u);if(w!=init[i])printf(" %u %u",i,w);}
     printf("\n");snapped=stopped=1;break;}
    if(pc==t->newops&&cut&&loc_physical()==cut){stopped=atcut=1;break;}
    /* (A snapshot run through outside code ends at its entry's caller
     * return: Z there, or the interpreter's LOC for an interpretive one.) */
    if(through&&ret!=0xffffu&&((!retloc&&z_physical(pc)==ret)||(retloc&&pc==t->newops&&get(0120)==init[0120]&&loc_physical()==ret))){stopped=1;break;}
    /* (IRC_SNAP_THROUGH: a snapshot run goes on through outside code, and
     * takes LOC only as a word of fixed memory, in the job it began in: FIXLOC
     * unchanged.) */
    if(!through){
    if(pc==t->newops){unsigned p=loc_physical();if(flow){unsigned v=get(0120);fprintf(stderr,"F %u %u %u %u %u %u %u %u\n",n,p,(get(0166)-v)&077777u,get(0163),get(v+046),get(v+047),get(v+050),get(v+051));}if(trace==n){printf("T %05o",get(0164));for(i=0;i<2048u;++i)printf(" %u",(unsigned)*memory(i)&(i<2u?0177777u:077777u));printf("\n");}if(outside(t,p)){stopped=1;break;}}
    if(trace==n&&getenv("IRC_ZTRACE"))fprintf(stderr,"Z %05o FB %05o A %06o L %06o Q %05o MPAC %05o %05o %05o BUF %05o %05o %05o BUF2 %05o %05o MPTEMP %05o IE %d\n",pc,get(4),(unsigned)*memory(0)&0177777u,(unsigned)*memory(1)&0177777u,get(2),get(0154),get(0155),get(0156),get(0130),get(0131),get(0132),get(0134),get(0135),get(0136),machine.AllowInterrupt);
    zp=z_physical(pc);
    /* (IRC_PENDING_DANZIG, spec 'pending_danzig': DANZIG with a right-hand
     * order still in EDOP is no stop when LOC is one of the file's RTB
     * operand words and EDOP its pair's order: the run goes on into the
     * order, as the native program does through the mission's DANZIG
     * marker. "danzig key:edop ..." in physical addresses.) */
    pend=0;if(pendz&&zp==pendz){unsigned l=loc_physical(),e=get(023);for(i=0;i<pendn;++i)if(l==pendk[i]&&e==pende[i])pend=1;}
    /* The file's own transfers out (and the case's return) stop only when
     * the file made them; the interpreter's exits stop from anywhere. */
    if(prev_in&&!pend){if(zp==stop||(stop!=0xffffu&&zp>stop&&zp<=stop+4u&&outside(t,zp))){stopped=1;break;}for(i=0;i<t->nf&&t->from_file[i]!=0xffffu;++i)if(zp==t->from_file[i]){stopped=1;break;}
}
    /* (any: only once Z has been in the file's own words.) */
    if(!pend&&t->any&&was_in&&zp<0xfffeu&&zp!=t->intpret&&outside(t,zp))stopped=1;
    if(zp<0xfffeu)was_in=!outside(t,zp);
    if(!pend)for(i=0;i<t->n;++i)if(zp==t->ext[i]||(pc<02000u&&t->ext[i]==(0xc000u|pc))){stopped=1;break;}
    if(zp!=0xfffeu)prev_in=!outside(t,zp); /* (TC Q runs the word in Q: still from the file) */
    }
    if(stopped)break;
   }
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(qw_watch){unsigned k;qw_frontier=1;for(k=0;k<400000u&&qw_watch;++k){machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
   if(!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode)qw_hook(get(RegZ));}
   if(qw_watch){qw_count("unresolved after 400000 more instructions");qw_watch=0;}}
  if(snapmode&&(!snaprow||!snapped)){if(!snapped)printf("%u N\n",n);continue;}}
  if(!stopped){fprintf(stderr,"case %u timeout %05o\n",n,pc);printf("%u 9 %u 0\n",n,pc);continue;}
  if(cut&&!atcut){printf("%u 8 %u 0\n",n,pc);continue;}
  {agc_word keep[8];for(i=0;i<t->nd;++i)keep[i]=s.words[t->device[i]];
  for(i=0;i<2048u;++i)s.words[i]=(agc_word)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
  for(i=0;i<t->nd;++i)s.words[t->device[i]]=keep[i];}
  if(irc_ch_on){for(i=0;i<IRC_NCH;++i)irc_ch[i]=(agc_word)((unsigned)machine.InputChannel[irc_ch_num[i]]&077777u);irc_trap[0]=(agc_word)machine.Trap31A;irc_trap[1]=(agc_word)machine.Trap31B;irc_trap[2]=(agc_word)machine.Trap32;}
  printf("%u 0 %u %lu",n,pc,(unsigned long)irc_digest(0,&s,(uint16_t)machine.AllowInterrupt));
  if(dump){for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]);printf(" %u",(unsigned)machine.AllowInterrupt);}
  printf("\n");
 }
 if(qw_on){unsigned i;fprintf(stderr,"SWITCH TS Q executions: %lu (A bit 16 != bit 15 at TS: %lu)\n",qw_n,qw_bad_a);for(i=0;i<qw_nt;++i)fprintf(stderr,"%8lu  %s\n",qw_tab[i].n,qw_tab[i].k);}
 return 0;
}
