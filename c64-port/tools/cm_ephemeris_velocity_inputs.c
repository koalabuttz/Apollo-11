/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "cm_ephemeris_time_cases.h"
int main(int argc,char **argv){int j;if(argc<3)return 2;for(j=2;j<argc;++j){apollo_overlay_state m;p66_bias_io io;uint16_t row[2050];unsigned i;cm_ephemeris_time_input(&m,&io,(uint32_t)strtoul(argv[j],0,0));m.words[0164]=02120;m.words[m.words[0120]+052u]=023377;for(i=0;i<2048u;++i)row[i]=m.words[i];row[2048]=io.interrupt_enabled;row[2049]=io.superbank;if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
