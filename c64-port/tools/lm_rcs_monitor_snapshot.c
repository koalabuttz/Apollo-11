/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "lm_rcs_monitor_cases.h"
int main(int argc,char **argv){uint32_t n,limit=LM_RCS_MONITOR_CASES;if(argc>1)limit=(uint32_t)strtoul(argv[1],NULL,0);for(n=0;n<limit;++n){apollo_overlay_state m;p66_bias_io io;agc_word ch;uint16_t row[2053];unsigned i;enum exec_status s;lm_rcs_monitor_input(&m,&io,&ch,n);s=lm_rcs_monitor(&m,&io,ch);if(s!=EXEC_OK&&s!=EXEC_NO_CORE){fprintf(stderr,"rejected%u status%u\n",n,(unsigned)s);return 2;}row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=io.interrupt_enabled;row[3]=io.superbank;row[4]=ch;for(i=0;i<2048u;++i)row[i+5u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
