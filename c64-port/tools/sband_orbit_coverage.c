/* SPDX-License-Identifier: MIT */
/* Inspect actual input constructors, not a duplicated guessed domain. */
#include <stdio.h>
#include "sband_orbit_cases.h"
static uint8_t sphere_seen[32768],a_seen[65536],l_seen[65536];
static apollo_overlay_state state;
int main(void){uint32_t n;unsigned mission,i,spheres,accumulators,links,earth,ebs,vacs,super,irq;
 for(mission=0;mission<2u;++mission){p66_bias_io io;enum apollo_fixed_mission got;
  for(i=0;i<32768u;++i)sphere_seen[i]=0;
  for(i=0;i<65536u;++i){a_seen[i]=0;l_seen[i]=0;}
  earth=ebs=vacs=super=irq=0;
  for(n=0;n<65536u;++n){agc_word value;sband_orbit_input(&state,&io,n+mission*65536u,&got);if((unsigned)got!=mission)return 2;
   value=state.words[state.words[0120]+047u];sphere_seen[value]=1;a_seen[state.words[0]]=1;l_seen[state.words[1]]=1;
   earth+=(unsigned)(value==0||value==AGC_MASK);ebs|=1u<<(state.words[3]>>8);vacs|=1u<<((state.words[0120]-0401u)/054u);super|=1u<<((io.superbank-0100u)/020u);irq|=1u<<io.interrupt_enabled;
  }
  spheres=accumulators=links=0;for(i=0;i<32768u;++i)spheres+=sphere_seen[i];for(i=0;i<65536u;++i){accumulators+=a_seen[i];links+=l_seen[i];}
  if(spheres!=32768u||accumulators!=65536u||links!=65536u||ebs!=255u||vacs!=31u||super!=15u||irq!=3u)return 3;
  printf("%s: states65536, distinct X2=%u/rawA=%u/rawL=%u; Earth=%u/Moon=%u; all8EB/all5VAC/all4selectedSUPER/bothIRQ; marginals only\n",mission?"CM":"LM",spheres,accumulators,links,earth,65536u-earth);
 }return 0;
}
