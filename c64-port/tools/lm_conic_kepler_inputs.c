/* SPDX-License-Identifier: MIT */
/* Entry images of the named sampled KEPLERN entries (or, after
 * --debris, their orbit-free debris images): 2048 words, IRQ, SUPER. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lm_conic_kepler_cases.h"
int main(int argc,char **argv){int j,debris=0;for(j=1;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint16_t row[2050];unsigned i;uint32_t n;
 if(!strcmp(argv[j],"--debris")){debris=1;continue;}
 n=(uint32_t)strtoul(argv[j],0,0);if(debris)lm_conic_kepler_debris(&m,&io,n);else lm_conic_kepler_input(&m,&io,n);
 for(i=0;i<2048u;++i)row[i]=(uint16_t)m.words[i];
 row[2048]=io.interrupt_enabled;row[2049]=io.superbank;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
