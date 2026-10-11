/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "rtb_agreement_cases.h"
int main(int argc,char **argv){uint32_t n,limit=RTB_AGREEMENT_CASES;unsigned mission,entry;if(argc<3)return 2;mission=(unsigned)strtoul(argv[1],0,0);entry=(unsigned)strtoul(argv[2],0,0);if(argc>3)limit=(uint32_t)strtoul(argv[3],0,0);for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;uint16_t row[2052];unsigned i;rtb_agreement_input(&m,&io,(enum rtb_mission)mission,(enum rtb_agreement_entry)entry,n);if(rtb_agreement(&m,&io,(enum rtb_mission)mission,(enum rtb_agreement_entry)entry)!=RTB_OK){fprintf(stderr,"rejected%u\n",n);return 2;}row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;for(i=0;i<2048u;++i)row[i+4u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
