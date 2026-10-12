/* SPDX-License-Identifier: GPL-2.0-or-later; host-only check of lm_p_axis.c's
 * DV against yaAGC executing EXTEND; DV K. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
#define rand lm_pa_rand /* the native RAND helper, not stdlib rand() */
#include "../src/lm_p_axis.c"
#undef rand
static agc_t baseline;
static uint32_t seed=1u;
static uint32_t rnd(void){seed=seed*1664525u+1013904223u;return seed>>8;}
/* A signed value of random width, +0 or -0 one in eight each. */
static agc_word pick(void){unsigned k=rnd()%8u,w=rnd()%15u;agc_word v=(agc_word)(rnd()&((1u<<w)-1u));
 if(k==0u)return 0u;
 if(k==1u)return 077777u;
 return rnd()&1u?(agc_word)(~v&077777u):v;}
/* Runs argv[2] random (A, L, K) through yaAGC's EXTEND; DV 0200 (at 0100 in
 * erasable) and through dv(), and prints the number of differences in A
 * and L. */
int main(int argc,char **argv){unsigned long n,count,bad=0;unsigned i;if(argc!=3)return 2;count=strtoul(argv[2],0,10);
 memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
 for(n=0;n<count;++n){apollo_overlay_state s;agc_word a=pick(),l=pick(),k=pick();
  machine=baseline;machine.AllowInterrupt=0;
  *memory(0100)=6;*memory(0101)=010200;*memory(0200)=(int16_t)k;
  *memory(0)=(int16_t)agc_extend(a);*memory(1)=(int16_t)agc_extend(l);*memory(5)=0100;
  for(i=0;i<20u&&!(get(RegZ)==0102&&!machine.ExtraDelay&&!machine.ExtraCode);++i){machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  memset(&s,0,sizeof s);s.words[0]=agc_extend(a);s.words[1]=agc_extend(l);s.words[0200]=k;dv(&s,0200u);
  if(((unsigned)*memory(0)&0177777u)!=s.words[0]||((unsigned)*memory(1)&0177777u)!=s.words[1])++bad;
 }
 printf("%lu %lu\n",count,bad);return bad!=0;
}
