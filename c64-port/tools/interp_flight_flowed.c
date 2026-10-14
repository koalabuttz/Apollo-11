/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only flowed-state source for the interp-flight VICE run
 * (tools/interp_flight_test.py --flowed), linked to Virtual AGC. NEVER part
 * of the native port.
 *
 * The flight store stops a FLIGHT_INTERP_STOP module (tools/flight_store_build.py)
 * where its code reaches interpretive code and the compact interpreter takes
 * the job. These are not mission flight states: yaAGC runs the unmodified
 * rope from the first STATES states of each routine of the milestone's cases
 * (its tests/<stem>_cases.h input, image, channels and interrupt state, as
 * its tools/<stem>_reference.c does, scalers frozen) until the run leaves the
 * file, and every first arrival inside the file at an interpreter entry
 * point (distinct entry and rope address of the order LOC names, or at
 * INTPRET of Q; quiet boundary outside an interrupt) is written as a case in
 * tools/interp_flight_capture.c's format: the 2048 erasable words, the
 * superbank, the entry point's index and two spare words.
 *
 *   interp-flight-flowed ROPE.bin STATES N CASES.bin
 *
 * Build with one of -DFLOW_LLD, -DFLOW_LMSV, -DFLOW_LMBB, -DFLOW_AGS,
 * -DFLOW_IPT and the milestone's native sources (its freeze script's CORE). */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#if FLOW_LLD
#include "lm_lunar_landing_cases.h"
typedef lld_case f_case;
#define F_ROUTINES LLD_ROUTINES
#define F_CASES LLD_CASES
#define F_INPUT lld_input
#define F_CHANNELS(c) (machine.InputChannel[012]=(int16_t)(c).chan12,machine.InputChannel[033]=(int16_t)(c).chan33)
#define F_LEAVES(z,fb,sb) lld_leaves(z,fb,sb,(agc_word)get(0164),(agc_word)get(0165))
#elif FLOW_LMSV
#include "lm_servicer_cases.h"
typedef lmsv_case f_case;
#define F_ROUTINES LMSV_ROUTINES
#define F_CASES LMSV_BASE_CASES
#define F_INPUT lmsv_input
#define F_CHANNELS(c) (machine.InputChannel[011]=(int16_t)(c).chan11,machine.InputChannel[012]=(int16_t)(c).chan12,\
 machine.InputChannel[032]=(int16_t)(c).chan32,machine.InputChannel[033]=(int16_t)(c).chan33)
#define F_LEAVES(z,fb,sb) lmsv_leaves(z,fb,sb,(agc_word)get(2),(agc_word)get(0164),(agc_word)get(0165))
#elif FLOW_LMBB
#include "lm_burn_baby_cases.h"
typedef lmbb_case f_case;
#define F_ROUTINES LMBB_ROUTINES
#define F_CASES LMBB_CASES
#define F_INPUT lmbb_input
#define F_CHANNELS(c) (machine.InputChannel[011]=(int16_t)(c).chan11,machine.InputChannel[014]=(int16_t)(c).chan14,\
 machine.InputChannel[030]=(int16_t)(c).chan30)
#define F_LEAVES(z,fb,sb) lmbb_leaves(z,fb,sb,(agc_word)get(0164),(agc_word)get(0165))
#elif FLOW_AGS
#include "lm_ags_init_cases.h"
typedef ags_case f_case;
#define F_ROUTINES AGS_ROUTINES
#define F_CASES AGS_CASES
#define F_INPUT ags_input
#define F_CHANNELS(c) ((void)(c))
#define F_LEAVES(z,fb,sb) ags_leaves(z,fb,sb,(agc_word)get(0164),(agc_word)get(0165))
#elif FLOW_IPT
#include "lm_imu_performance_cases.h"
typedef ipt_case f_case;
#define F_ROUTINES IPT_ROUTINES
#define F_CASES IPT_CASES
#define F_INPUT ipt_input
#define F_CHANNELS(c) (machine.InputChannel[3]=(int16_t)(c).hiscalar,machine.InputChannel[4]=(int16_t)(c).loscalar)
#define F_LEAVES(z,fb,sb) (ipt_leaves(z,fb,sb,(agc_word)get(0164),(agc_word)get(0165))||ipt_idle(z,(agc_word)prev))
#else
#error "build with one of FLOW_LLD, FLOW_LMSV, FLOW_LMBB, FLOW_AGS, FLOW_IPT"
#endif
/* tools/interp_flight_capture.c points[] (LM). */
static const unsigned points[]={06070,06061,06063,06106,06460,06144,06463,06466,06252,06646,06037};
#define POINTS (sizeof points/sizeof points[0])
#define CASE_WORDS 2052u
#define INTPRET 10u
/* The physical rope address of Z under FB and the superbank (as
 * tools/interp_flight_capture.c's physical(); 0xffff in erasable). */
/* The interpreter's banks (INTERPRETER.agc: BANK 00, 01 and BLOCK 03). */
static int interpreter(unsigned ph){return ph!=0xffffu&&(ph/1024u<2u||ph/1024u==3u);}
static unsigned physical(unsigned z,unsigned fb,unsigned sb){unsigned bank;z&=07777u;
 if(z<02000u)return 0xffffu;
 if(z>=06000u)bank=3;else if(z>=04000u)bank=2;else{bank=(fb&076000u)>>10;if(bank>=030u&&(sb&0100u))bank+=8u;}
 return bank*1024u+(z&01777u);}

static unsigned word(unsigned a){return (unsigned)*memory(a)&077777u;}
/* Whether the interpretive run from here stays in a real job's frame, as
 * tools/interp_core_fuzz.c's frame test judges it: at every arrival at an
 * entry point until yaAGC leaves the interpreter for basic code, FIXLOC
 * unchanged, the superbank 0 or 0100 (the rope's; the core reads no other),
 * the push-down pointer in its list (FIXLOC+0 to +45), MODE -1, +0 or +1,
 * LOC in fixed memory. yaAGC runs a copy as tools/interp_flight_capture.c
 * check does (interrupts held off). A first filter, so that a point keeps
 * a state that can pass: tools/interp_flight_verify.c decides which cases
 * keep to the contract (docs/INTERPRETER-RUNTIME.md). */
static int in_frame(void){agc_t keep=machine;unsigned long k;int ok=0;unsigned v=word(0120);
 for(k=0;k<400000ul;++k){
  if(!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&!machine.SubstituteInstruction){
   unsigned z=word(RegZ)&07777u,e,ph=physical(z,word(4),(unsigned)machine.InputChannel[7]);
   for(e=0;e<POINTS;++e)if(z==points[e])break;
   if(e<POINTS){unsigned mode=word(0163),pd=word(0166);
    if(word(0120)!=v||((unsigned)machine.OutputChannel7&0160u)%0100u||pd<v||pd>v+045u||(mode>1u&&mode!=077776u)||
       (z!=points[INTPRET]&&(word(0164)&07777u)<02000u)){
     if(getenv("FLOW_WHY"))fprintf(stderr,"k %lu Z %05o EB %05o FIXLOC %05o/%05o PUSHLOC %05o MODE %05o X1 %05o X2 %05o LOC %05o\n",
      k,z,word(3),word(0120),v,pd,mode,word(v+046u),word(v+047u),word(0164));
     break;}}
   else if(k&&ph!=0xffffu&&!interpreter(ph)){ok=1;break;}}
  machine.ScalerCounter=0;if(agc_engine(&machine))break;
  memset(machine.InterruptRequests,0,sizeof machine.InterruptRequests);}
 machine=keep;return ok;}

int main(int argc,char **argv){int all=argc==6&&!strcmp(argv[5],"all");static struct { unsigned point,at,tries; } seen[8192];
 unsigned n,i,states,want,nseen=0,written=0,cases=0,timeouts=0,outside=0;unsigned long cycles,arrivals=0;agc_t baseline;FILE *f;
 if(argc!=5&&!all)return 2;InhibitAlarms=1;
 states=(unsigned)strtoul(argv[2],0,0);want=(unsigned)strtoul(argv[3],0,0);
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;
 if(!(f=fopen(argv[4],"wb")))return 2;
 for(n=0;n<F_CASES&&n<F_ROUTINES*states;++n){apollo_overlay_state s;f_case c;unsigned pc=0,prev=0,prevph=0xffffu,stopped=0;
  F_INPUT(&s,&c,(uint16_t)n);
  machine=baseline;
  for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)c.superbank;machine.AllowInterrupt=c.interrupt_enabled;
  F_CHANNELS(c);
  ++cases;
  for(cycles=0;cycles<4000000ul;++cycles){
   if(!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode){
    agc_word fb=(agc_word)(get(4)&076000u),sb=(agc_word)machine.InputChannel[7];unsigned e;
    pc=get(RegZ);
    if(cycles&&F_LEAVES((agc_word)pc,fb,sb)){stopped=1;break;}
    for(e=0;e<POINTS&&!machine.InIsr&&!machine.SubstituteInstruction&&(all||!interpreter(prevph));++e)if((pc&07777u)==points[e]){
     /* The order LOC names (at INTPRET, where TC INTPRET returns: Q under FB). */
     unsigned at=e==INTPRET?physical(get(2),fb,sb):physical(get(0164),get(0165),sb);
     ++arrivals;
     for(i=0;i<nseen;++i)if(seen[i].point==e&&seen[i].at==at)break;
     if(i==nseen&&nseen<sizeof seen/sizeof seen[0]){seen[nseen].point=e;seen[nseen].at=at;seen[nseen++].tries=0;}
     /* (up to 8 states at a point before giving it up as outside the frame) */
     if(i<nseen&&!(seen[i].tries&0x100u)){
      if(seen[i].tries>=8u)break;
      ++seen[i].tries;
      if(!in_frame()){++outside;break;}
      seen[i].tries|=0x100u;
      if(written<want){unsigned char b[CASE_WORDS*2u];unsigned w[CASE_WORDS-2048u]={(unsigned)machine.OutputChannel7&0160u,e,0,0},v;
       for(i=0;i<2048u;++i){v=(unsigned)*memory(i)&(i<2u?0177777u:077777u);b[2*i]=(unsigned char)v;b[2*i+1]=(unsigned char)(v>>8);}
       for(i=0;i<CASE_WORDS-2048u;++i){b[4096u+2*i]=(unsigned char)w[i];b[4097u+2*i]=(unsigned char)(w[i]>>8);}
       fwrite(b,1,sizeof b,f);++written;
       printf("case %u: from case %u entry %05o at %02o,%04o LOC %05o BANKSET %05o FB %05o\n",written-1u,n,points[e],
              at/1024u,02000u+at%1024u,get(0164),get(0165),(unsigned)fb);}}
     break;}
    prev=pc;prevph=physical(pc,fb,sb);
   }
   machine.ScalerCounter=0;if(agc_engine(&machine)){fclose(f);return 2;}}
  if(!stopped)++timeouts;
 }
 fclose(f);
 for(i=0,n=0;i<nseen;++i)n+=!(seen[i].tries&0x100u);
 printf("captured %u cases (%u distinct entries, %u only outside the frame; %lu arrivals, %u outside) from %u states, %u timeouts\n",
        written,nseen,n,arrivals,outside,cases,timeouts);
 return written?0:1;}
