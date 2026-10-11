/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "sband_vector_cases.h"
int main(int argc,char **argv){int j;for(j=1;j<argc;++j){uint32_t id=(uint32_t)strtoul(argv[j],0,0);apollo_overlay_state m;agc_word irq,super;uint16_t row[2050];unsigned i;sband_vector_input(&m,&irq,&super,id&131071u,id>=131072u?APOLLO_FIXED_CM:APOLLO_FIXED_LM);for(i=0;i<2048u;++i)row[i]=m.words[i];row[2048]=irq;row[2049]=super;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
