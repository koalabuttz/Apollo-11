/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "cm_ephemeris_velocity.h"
#include "cm_ephemeris_time_cases.h"
int main(int argc,char **argv){uint32_t n,limit=argc>2?(uint32_t)strtoul(argv[2],0,0):65536u;for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2053];unsigned i;cm_ephemeris_time_input(&m,&io,n);m.words[0164]=02120;m.words[m.words[0120]+052u]=023377;row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=(uint16_t)cm_ephemeris_velocity(&m,&io);for(i=0;i<2048u;++i)row[i+5u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
