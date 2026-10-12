/* SPDX-License-Identifier: MIT */
/* Entry images of the named sampled LSPOS/LUNVEL states: 2048 words, IRQ, SUPER. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_ephemeris_cases.h"
int main(int argc,char **argv){int j;for(j=1;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint16_t row[2050];unsigned i;lm_ephemeris_input(&m,&io,(uint32_t)strtoul(argv[j],0,0));for(i=0;i<2048u;++i)row[i]=(uint16_t)m.words[i];row[2048]=io.interrupt_enabled;row[2049]=io.superbank;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
