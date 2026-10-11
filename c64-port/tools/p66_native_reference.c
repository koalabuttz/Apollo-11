/* SPDX-License-Identifier: GPL-2.0-or-later
 * Genuine unchanged updated-velocity group and RODCOMP; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "p66_native_cases.h"
static agc_t baseline;static int mixed;
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
static void emit(velocity_case *c,uint16_t n,uint8_t step,uint8_t stage) {
    uint16_t w[VL_WORD_COUNT]; unsigned i;
    for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i]) & (i==4 || i==5 ? 0177777u : 077777u));
    w[41]=(uint16_t)get(0120); w[42]=(uint16_t)get(0166);
    for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(c->previous.previous.previous.previous.push.base+i);
    w[86]=(uint16_t)get(01234); w[87]=(uint16_t)get(01235); w[88]=(uint16_t)get(0130);
    w[89]=(uint16_t)get(0131); w[90]=(uint16_t)get(0136); w[91]=(uint16_t)get(0137); w[92]=(uint16_t)get(0140);
    for(i=0;i<6;++i) w[93+i]=(uint16_t)get(01236+i);
    for(i=0;i<6;++i) { w[99+i]=(uint16_t)get(02630+i); w[105+i]=(uint16_t)get(03526+i); w[111+i]=(uint16_t)get(03520+i); }
    vl_from_words(c,w); (void)step;printf("%u %u",n,stage);
    for(i=0;i<VL_WORD_COUNT;++i) printf(" %u",w[i]);
    if(mixed){for(i=0;i<96;++i)printf(" %u",get(0154+i));for(i=0;i<220;++i)printf(" %u",get(0400+i));printf(" %u %u",get(0120),get(0121));printf(" %u %u %u %u %u %u %u",get(0116),get(023),get(0115),get(7),get(3),get(4),get(6));}puts("");
}
int main(int argc,char **argv) {
 uint16_t n;mixed=argc==3;if(argc!=2 && argc!=3)return 2;memset(&machine,0,sizeof machine);
 if(agc_load_binfile(&machine,argv[1])!=0)return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<(mixed?16u:PN_CASES);++n){
  velocity_case c;uint16_t w[VL_WORD_COUNT];unsigned i,cycles,stage=0,base;
  static const unsigned normal[5]={03353,03356,03361,03364,03373};
  static const unsigned interleaved[7]={03353,03356,03353,03356,03361,03364,03373};
  const unsigned *boundaries=mixed?interleaved:normal;
  vl_prepare(&c,pn_number(n));vl_next(&c,pn_number(n),0);vl_words(&c,w);base=w[41];machine=baseline;
  for(i=0;i<41;++i)*memory(addresses[i])=(int16_t)w[i];
  set(0120,base);set(0166,w[42]);set(base-1u,0);set(0167,01000u+base);
  for(i=0;i<43;++i)set(base+i,w[43+i]);
  set(01234,w[86]);set(01235,w[87]);set(0130,w[88]);set(0131,w[89]);set(0136,w[90]);set(0137,w[91]);set(0140,w[92]);
  for(i=0;i<6;++i){set(01236+i,w[93+i]);set(02630+i,w[99+i]);set(03526+i,w[105+i]);set(03520+i,w[111+i]);}
  if(mixed){unsigned j,high=base==0401?0455:0401;
   for(i=0;i<8;++i){for(j=0;j<12;++j)if(i!=0 || j>=8)set(0154+i*12+j,0);if(i!=0)set(0154+i*12+11,077777);}
   set(0167,01000u+base);set(0166,w[42]);
   for(i=0;i<5;++i)if(0401u+i*054u!=base){for(j=0;j<44;++j)set(0400+i*44+j,0);set(0400+i*44,0400+i*44);}
   set(high-1,0);for(i=0;i<43;++i)set(high+i,(i*17u+n*31u)&077777u);
   set(0154+12+8,03275);set(0154+12+9,064007);set(0154+12+10,high);set(0154+12+11,04000u+high);
  }
  machine.InputChannel[7]=machine.OutputChannel7=(n&1u)?060:0;
  set(RegEB,03400);set(RegFB,064000);set(RegBB,064007);set(RegQ,01030);set(RegZ,03275);set(067,0);machine.AllowInterrupt=1;
  for(cycles=0;cycles<5000;++cycles){
   if(!machine.PendFlag && !machine.ExtraDelay &&
      ((stage==0 || (mixed && stage==2))?get(RegZ)==03353:get(RegZ)==06061 && get(0164)==boundaries[stage])) {
    c.previous.previous.previous.previous.push.base=(agc_word)get(0120);
    emit(&c,n,0,(uint8_t)stage);if(++stage==(mixed?7u:5u))break;
    if(mixed && stage==1)set(0067,12);
   }
   machine.ScalerCounter=0;if(agc_engine(&machine)!=0)return 2;
  }
  if(stage!=(mixed?7u:5u)){fprintf(stderr,"timeout %u stage%u PC%o LOC%o\n",n,stage,get(RegZ),get(0164));return 2;}
 }
 return 0;
}
