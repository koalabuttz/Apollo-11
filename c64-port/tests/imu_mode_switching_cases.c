/* SPDX-License-Identifier: MIT */
#include "imu_mode_switching_cases.h"
/* The C64 diagnostics build subsets: -DIMS_ENTRY_ONLY (entry cases, this
 * file's routines only), -DIMS_TRIP_LM or -DIMS_TRIP_CM (one mission's
 * round trips with its INTER-BANK routines, no entry-case shaping). Host
 * builds have everything. */
#if !defined IMS_TRIP_LM && !defined IMS_TRIP_CM
#define IMS_ENTRY_CASES_BUILT 1
#endif
#ifndef IMS_ENTRY_ONLY
#define AGC_SITES_CALLEES 1
#ifndef IMS_TRIP_CM
#define IMS_LM_TRIPS 1
#include "lm_interbank_routines.h"
#include "../src/lm_interbank_sites.inc"
#endif
#ifndef IMS_TRIP_LM
#define IMS_CM_TRIPS 1
#include "cm_interbank_routines.h"
#include "../src/cm_interbank_sites.inc"
#endif
#endif
#define ANY IMS_ANY_FB
/* Externals of the file (both missions' addresses as each program
 * assembles them), and the subroutine-return stops 04000/04001. */
static const ims_stop lm_ext[]={{05567u,ANY},{05203u,ANY},{05224u,ANY},{05221u,ANY},{05261u,ANY},{04674u,ANY},
 {04631u,ANY},{05133u,ANY},{05137u,ANY},{04645u,ANY},{05716u,ANY},{05652u,ANY},{04635u,ANY},{07257u,ANY},
 {07307u,ANY},{04103u,ANY},{05504u,ANY},{05516u,ANY},{05353u,ANY},{04616u,ANY},{06001u,ANY},{05744u,ANY},
 {04000u,ANY},{04001u,ANY},{0,0}};
static const ims_stop cm_ext[]={{05650u,ANY},{05245u,ANY},{05272u,ANY},{05267u,ANY},{05324u,ANY},{04740u,ANY},
 {04675u,ANY},{05175u,ANY},{05201u,ANY},{04711u,ANY},{05721u,ANY},{04701u,ANY},{07226u,ANY},{07256u,ANY},
 {04554u,ANY},{05546u,ANY},{05560u,ANY},{05412u,ANY},{04662u,ANY},{04106u,ANY},{05764u,ANY},
 {04000u,ANY},{04001u,ANY},{0,0}};
/* Round trips: the INTER-BANK entries (BANKCALL, SWRETURN, IBNKCALL,
 * MAKECADR, POSTJUMP) run natively; IBNKCALL's SETISSW (06,2703 /
 * 06,2665) is where native code ends. */
static const ims_stop lm_trip_ext[]={{05567u,ANY},{05203u,ANY},{05224u,ANY},{05221u,ANY},{05261u,ANY},
 {05133u,ANY},{05137u,ANY},{05716u,ANY},{05652u,ANY},{07257u,ANY},{07307u,ANY},{04103u,ANY},{05504u,ANY},
 {05516u,ANY},{05353u,ANY},{06001u,ANY},{05744u,ANY},{04000u,ANY},{04001u,ANY},{02703u,014000u},{0,0}};
static const ims_stop cm_trip_ext[]={{05650u,ANY},{05245u,ANY},{05272u,ANY},{05267u,ANY},{05324u,ANY},
 {05175u,ANY},{05201u,ANY},{05721u,ANY},{07226u,ANY},{07256u,ANY},{04554u,ANY},{05546u,ANY},{05560u,ANY},
 {05412u,ANY},{04106u,ANY},{05764u,ANY},{04000u,ANY},{04001u,ANY},{02665u,014000u},{0,0}};
const ims_stop *ims_externals(const ims_case *c){
 if(c->stop_z)return c->cm?cm_trip_ext:lm_trip_ext;
 return c->cm?cm_ext:lm_ext;
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word r15(uint32_t *s){return (agc_word)((rnd(s)>>12)&AGC_MASK);}
uint16_t ims_routine_physical(uint16_t r){
 return ims_routine_keys[r];
}
/* Physical key of a Luminary label (bank, address) in mission cm. */
static uint16_t key(uint8_t cm,unsigned bank,agc_word a){
 if(bank==02u)return (uint16_t)(2048u+(cm?a+042u:a)-04000u);
 return (uint16_t)((cm?(bank==04u?07u:bank==037u?026u:bank):bank)*1024u+(cm?ims_comanche_address(bank,a):a)-02000u);
}
static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){m->words[4]=fb;m->words[3]=eb;m->words[6]=(agc_word)(fb|(eb>>8));}
/* A word of one of four CCS classes. */
static agc_word ccs_class(uint32_t *seed,unsigned k){agc_word v=(agc_word)((rnd(seed)>>17)%037776u+1u);
 return k==0u?v:k==1u?0u:k==2u?(agc_word)(~v&077777u):077777u;
}
/* A small signed word (|x| < n) or a zero of either sign. */
static agc_word small(uint32_t *seed,unsigned n){agc_word v=(agc_word)((rnd(seed)>>17)%n);return (rnd(seed)>>16)&1u?(agc_word)(~v&077777u):v;}
/* GSELECT's skip (major +-0, minor +-0 or under GYROMIN pulses: CCS
 * leaves |x|-1, AD -GYROMIN, BZMF) for the pair each step from LGYRO g
 * reads: X and Y add 2, Z subtracts 4 and adds 0, each with BIT13, as
 * strtgyro_domain walks it. Every pair skips when all is set, else three
 * in four do; the minor is +-0, |x| 1..0177, or 0177/0200 at the edge. */
static void shape_skips(uint16_t *w,agc_word g,agc_word lgyro,int all,uint32_t *seed){unsigned a,ad;agc_word l=g,mn;
 for(;;){a=l>>11;
  if((a&1u)||a>=6u)return;
  if(a==4u)l=(agc_word)(l-4u);
  l=(agc_word)(l+(a==4u?0u:2u)+010000u);
  ad=((l>>8)&7u)*0400u+(l&0377u);
  if(ad<010u||ad==lgyro||ad+1u==lgyro||(!all&&(rnd(seed)>>17)%4u==0u))continue;
  switch((rnd(seed)>>17)%4u){
   case 0u:mn=0u;break;
   case 1u:mn=(agc_word)(1u+(rnd(seed)>>17)%0177u);break;
   case 2u:mn=0177u;break;
   default:mn=0200u;break;
  }
  if((rnd(seed)>>16)&1u)mn=(agc_word)(~mn&077777u);
  w[ad]=(rnd(seed)>>16)&1u?077777u:0u;w[ad+1u]=mn;
 }
}
#define IS(b,a) (ph==key(c->cm,(b),(a)))
#define CDUX_OF(i) (032u+(i))
#ifdef IMS_ENTRY_CASES_BUILT
static void shape_entry(apollo_overlay_state *m,ims_case *c,uint16_t ph,unsigned v,uint32_t *seed){
 uint16_t *w=m->words;unsigned i;
 agc_word imodes30=c->cm?01320u:01302u,lgyro=c->cm?01304u:01314u,modecadr=c->cm?01322u:01304u;
 agc_word thetad=c->cm?01155u:0321u,attcadr=c->cm?01325u:01307u;
 /* Subroutine entries return through TC Q to the stop at 04000. */
 if(IS(02,IMS_ZEROICDU)||IS(07,IMS_SETCOARS)||IS(07,IMS_RNDREFDR)||IS(07,IMS_NOATTOFF))w[2]=04000u;
 if(IS(07,IMS_BADEND)||IS(07,IMS_GOODEND))w[0]=(agc_word)(v%3u);
 if(IS(07,03702u))w[064]=(agc_word)((v%9u)*014u);
 if(IS(07,03736u))w[071]=(agc_word)(v%3u);
 if(IS(07,03337u))w[0161]=(agc_word)((w[0161]&~0377u)|(010u+(rnd(seed)>>17)%0363u));
 if(IS(07,03357u)){agc_word n=v%5u<3u?(agc_word)(v%5u):ccs_class(seed,2u+(v/5u)%2u);
  w[0157]=n;w[0160]=(agc_word)(010u+(rnd(seed)>>17)%0367u);
  if(n>=1u&&n<=2u)w[061]=(agc_word)(010u+(rnd(seed)>>17)%(0367u-2u*(n-1u)));
 }
 /* LGYRO for STRTGYRO: step count 0..3 in bits 13-14, an odd step in a
  * quarter of the states, EBANK bits, a LOW8 index 020..0355; the six
  * gyro command words small in a quarter of the states, and in another
  * quarter the pairs the steps read set to GSELECT's skip. */
 if(IS(07,IMS_STRTGYRO)){agc_word g=(agc_word)(((v%4u)<<12)|(((v/4u)%4u)==3u?04000u:0u)|(r15(seed)&03400u)|(020u+(rnd(seed)>>17)%0337u));
  w[lgyro]=g;
  if((v/16u)%4u==3u)shape_skips(w,g,lgyro,(v%4u)==0u&&(v/64u)%2u==0u,seed);
  if((v/16u)%4u==1u){unsigned base=((g>>8)&7u)*0400u+(g&0377u);
   for(i=0;i<8u;++i)if(base+i!=lgyro)w[base+i]=i%2u?small(seed,0400u):small(seed,4u);}
  /* A quarter of the states: gyro pairs of zero commands (both words +-0)
   * or with a high word of 0100-0177, which GMERGE's AD MINUS1 leaves at
   * -0 (the CAF BIT14; ADS ITEMP2 fix before LASTSEG). */
  if((v/16u)%4u==2u){unsigned base=((g>>8)&7u)*0400u+(g&0377u);
   for(i=0;i<8u;i+=2u)if(base+i!=lgyro&&base+i+1u!=lgyro){
    if((rnd(seed)>>17)&1u){w[base+i]=(rnd(seed)>>17)&1u?077777u:0u;w[base+i+1u]=(rnd(seed)>>17)&1u?077777u:0u;}
    else{w[base+i]=(agc_word)(0100u+(rnd(seed)>>17)%0100u);w[base+i+1u]=small(seed,0400u);}}}
 }
 /* 8192AUG: the remaining gyro word in each CCS class in a quarter of
  * the states. */
 if(IS(07,IMS_8192AUG)&&(v/8u)%4u==3u){agc_word g=w[lgyro];unsigned a=((g>>8)&7u)*0400u+(g&0377u);
  if(a!=lgyro&&(a>=010u||(g&03400u)))w[a]=ccs_class(seed,v%4u);}
 /* CCS classes: LGYRO (IMUPULSE, GWAKE), MODECADR (stalls and mode
  * ends, with -1 for MODEGOOD's +0), MPAC (POSTAND's TPAGREE return),
  * ATTCADR zero for GOMANUR. */
 if(IS(07,IMS_IMUPULSE)||IS(07,IMS_GWAKE))w[lgyro]=ccs_class(seed,v%4u);
 if(IS(07,IMS_AOTSTALL)||IS(07,IMS_RADSTALL)||IS(07,IMS_IMUSTALL)||IS(07,IMS_BADEND)||IS(07,IMS_GOODEND)||
    IS(07,IMS_IMUBAD)||IS(07,IMS_ENDIMU)){
  for(i=0;i<3u;++i)w[modecadr+i]=(v/4u)%5u==4u?077776u:ccs_class(seed,(v/4u+i)%4u);
 }
 if(IS(037,03755u))w[0154]=ccs_class(seed,v%4u);
 if(IS(07,IMS_GOMANUR)&&v%2u)w[attcadr]=0;
 /* COARS2: commands near the per-pass limit in half the states. EB is
  * bank 3, where COMMAND lives, in half the states. */
 if(IS(07,IMS_COARS2)&&v%2u){unsigned eb=(w[3]>>8)&7u;for(i=0;i<3u;++i)w[eb*0400u+071u+i]=small(seed,0500u);}
 /* COARS2 with no axis to command (every COMMAND +-0, the end of a
  * coarse align) in an eighth of the states. */
 if(IS(07,IMS_COARS2)&&v%8u==2u){unsigned eb=(w[3]>>8)&7u;for(i=0;i<3u;++i)w[eb*0400u+071u+i]=(rnd(seed)>>17)&1u?077777u:0u;}
 /* CHKCORS: CDUs within a few bits of THETAD in half the states. */
 if(IS(07,03072u)&&v%2u)for(i=0;i<3u;++i)w[CDUX_OF(i)]=(agc_word)((w[thetad+i]+((rnd(seed)>>17)%041u)-020u)&077777u);
 /* SCALPREP's return: the genuine retry sites, or the stop. */
 if(IS(037,03717u)){static const agc_word lm[3]={03664u,03744u,04000u},cmr[3]={03667u,03747u,04000u};w[0156]=c->cm?cmr[v%3u]:lm[v%3u];}
 (void)imodes30;
}
#endif
#ifndef IMS_ENTRY_ONLY
/* Round-trip site k: its index in the mission's interbank table. */
static int mine(uint8_t cm,agc_word cadr){uint16_t ph=(uint16_t)((cadr>>10)*1024u+(cadr&01777u)),i;
 for(i=0;i<IMS_LM_ROUTINES;++i)if(ims_routine_keys[cm?IMS_LM_ROUTINES+i:i]==ph)return 1;
 return 0;
}
static int site(uint16_t k,uint8_t *cm,uint16_t *ph,agc_word *callee,agc_word *cadr){uint16_t i,seen=0;
#ifdef IMS_LM_TRIPS
 for(i=0;i<LM_INTERBANK_SITES_COUNT;++i)if((lm_interbank_sites_callee[i]==04616u||lm_interbank_sites_callee[i]==04674u)&&mine(0,lm_interbank_sites[i].word)){
  if(seen==k){*cm=0;*ph=lm_interbank_sites[i].physical;*callee=lm_interbank_sites_callee[i];*cadr=lm_interbank_sites[i].word;return 1;}++seen;}
#else
 seen=IMS_LM_SITES;
#endif
#ifdef IMS_CM_TRIPS
 for(i=0;i<CM_INTERBANK_SITES_COUNT;++i)if((cm_interbank_sites_callee[i]==04662u||cm_interbank_sites_callee[i]==04740u)&&mine(1,cm_interbank_sites[i].word)){
  if(seen==k){*cm=1;*ph=cm_interbank_sites[i].physical;*callee=cm_interbank_sites_callee[i];*cadr=cm_interbank_sites[i].word;return 1;}++seen;}
#endif
 (void)k;(void)seen;return 0;
}
#endif
uint16_t ims_target(uint16_t n,uint8_t *cm){
 if(n<IMS_ENTRY_CASES){uint16_t r=(uint16_t)(n%IMS_ROUTINES);*cm=r>=IMS_LM_ROUTINES;return ims_routine_physical(r);}
#ifdef IMS_ENTRY_ONLY
 *cm=2;return 0xffffu;
#else
 {uint16_t ph;agc_word callee,cadr;
  if(!site((uint16_t)((n-IMS_TRIP_FIRST)%IMS_SITES),cm,&ph,&callee,&cadr)){*cm=2;return 0xffffu;}
  return (uint16_t)((cadr>>10)*1024u+(cadr&01777u));}
#endif
}
void ims_input(apollo_overlay_state *m,ims_case *c,uint16_t n){uint32_t seed=0x1d1aul+(uint32_t)n*11u;unsigned i,v;agc_word z,fb;
 for(i=0;i<2048u;++i)m->words[i]=r15(&seed);
 m->words[7]=0;
 m->words[025]=(agc_word)(r15(&seed)&037777u);m->words[024]=(agc_word)(r15(&seed)&037777u);
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 for(i=0;i<4u;++i)c->channel[i]=r15(&seed);
 c->channel[2]&=043777u;
 c->stop_z=c->stop_fb=0;c->interrupt_enabled=(uint16_t)(n&1u);c->cm=0;
 if(n<IMS_ENTRY_CASES){uint16_t r=(uint16_t)(n%IMS_ROUTINES),ph=ims_routine_physical(r);unsigned bank=ph/1024u;
  v=n/IMS_ROUTINES;c->cm=r>=IMS_LM_ROUTINES;
  if(bank==02u){z=(agc_word)(04000u+ph%1024u);fb=(agc_word)(((rnd(&seed)>>17)%030u)<<10);}
  else{z=(agc_word)(02000u+ph%1024u);fb=(agc_word)(bank<<10);}
  c->superbank=bank==037u?(agc_word)((v%4u)<<4):(agc_word)(((rnd(&seed)>>17)%8u)<<4);
  set_banks(m,fb,(agc_word)((v%8u)<<8));m->words[5]=z;
#ifdef IMS_ENTRY_CASES_BUILT
  shape_entry(m,c,ph,v,&seed);
#endif
  return;
 }
#ifdef IMS_ENTRY_ONLY
 c->stop_z=1;(void)z;(void)fb;
#else
 {uint8_t cm;uint16_t ph;agc_word callee,cadr;unsigned bank;
  v=(n-IMS_TRIP_FIRST)/IMS_SITES;
  if(!site((uint16_t)((n-IMS_TRIP_FIRST)%IMS_SITES),&cm,&ph,&callee,&cadr)){c->stop_z=1;c->cm=2;return;}
  c->cm=cm;bank=ph/1024u;
  /* Banks 040-043 are FB 030-033 with the SUPERBNK bit. */
  if(bank>=040u){fb=(agc_word)((030u|(bank&7u))<<10);c->superbank=0100u;}
  else{fb=(agc_word)(bank<<10);c->superbank=(agc_word)(((rnd(&seed)>>17)%4u)<<4);}
  set_banks(m,fb,(agc_word)((v%8u)<<8));
  m->words[2]=(agc_word)(02000u+ph%1024u);m->words[5]=callee;
  c->stop_z=(agc_word)(m->words[2]+1u);c->stop_fb=fb;c->interrupt_enabled=1;
  /* BADEND and GOODEND callers hand over the MODECADR index in A. */
  {agc_word g=cm?ims_comanche_address(07u,IMS_GOODEND):IMS_GOODEND,b=cm?ims_comanche_address(07u,IMS_BADEND):IMS_BADEND;
   if(cadr==(agc_word)(016000u+g-02000u)||cadr==(agc_word)(016000u+b-02000u))m->words[0]=(agc_word)(v%3u);}
 }
#endif
}
int ims_executed_erasable(agc_word z){return z==2u||z==0156u||z==0133u||z==072u;}
static int external(const ims_case *c,const apollo_overlay_state *s){const ims_stop *e=ims_externals(c);
 if(s->words[5]<02000u)return 1;
 for(;e->z;++e)if(e->z==s->words[5]&&(e->fb==ANY||e->fb==s->words[4]))return 1;
 return 0;
}
unsigned ims_run(apollo_overlay_state *s,uint16_t n,uint16_t *ie,agc_word channel[4]){ims_case c;agc_routine_table tables[2];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};lm_imu_io lio;cm_native_io cio;agc_hardware hw;void *io;enum exec_status st;uint16_t steps,stop;unsigned i;
 ims_input(s,&c,n);
 if(c.cm>1u)return 3u;
 {uint16_t *p=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *p;++i)p[i]=0;}
 lio.control.superbank=cio.superbank=c.superbank;lio.control.interrupt_enabled=cio.interrupt_enabled=c.interrupt_enabled;
 hw.channel11=c.channel[0];hw.in_interrupt=0;program.hardware=&hw;lio.channel12=cio.channel12=c.channel[1];
 lio.channel13=cio.channel13=c.channel[2];lio.channel14=cio.channel14=c.channel[3];
 io=c.cm?(void *)&cio:(void *)&lio;
 tables[0]=c.cm?cm_imu_mode_switching_routines:lm_imu_mode_switching_routines;program.basic=tables;program.basic_count=1;
 if(c.stop_z){
#ifdef IMS_ENTRY_ONLY
  return 3u;
#else
#ifdef IMS_CM_TRIPS
  if(c.cm){tables[1]=cm_interbank_routines;program.basic_count=2;program.operands=cm_interbank_site_table(&program.operand_count);}
#endif
#ifdef IMS_LM_TRIPS
  if(!c.cm){tables[1]=lm_interbank_routines;program.basic_count=2;program.operands=lm_interbank_site_table(&program.operand_count);}
#endif
  if(program.basic_count!=2u)return 3u;
#endif
 }
 stop=c.stop_z?agc_fixed_physical(c.stop_z,c.stop_fb,c.superbank):(uint16_t)0xfffeu;
 st=agc_transfer_run(&program,s,io,&c.superbank,stop,32,&steps);
 if(ie)*ie=c.cm?cio.interrupt_enabled:lio.control.interrupt_enabled;
 if(channel){channel[0]=hw.channel11;channel[1]=c.cm?cio.channel12:lio.channel12;
  channel[2]=c.cm?cio.channel13:lio.channel13;channel[3]=c.cm?cio.channel14:lio.channel14;}
 if(st==EXEC_OK)return IMS_STOPPED;
 if(st==EXEC_UNMAPPED&&external(&c,s))return IMS_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return IMS_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t ims_digest(apollo_overlay_state *s,uint16_t n){uint16_t ie;agc_word ch[4];unsigned i,kind=ims_run(s,n,&ie,ch);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=IMS_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));mix(ie);for(i=0;i<4u;++i)mix(ch[i]);}
 return (uint32_t)sum_b<<16|sum_a;
}
