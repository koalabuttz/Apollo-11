/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cm_rcs_dap_cases.h"
int main(int argc,char **argv){uint32_t n,limit=CM_RCS_DAP_CASES;if(argc>1)limit=(uint32_t)strtoul(argv[1],NULL,0);
for(n=0;n<limit;++n){agc_image m;cm_rcs_dap_io io;uint16_t row[CM_RCS_DAP_ROW_WORDS];unsigned i;cm_rcs_dap_input(&m,&io,n);if(cm_rcs_dap(&m,&io)!=CM_RCS_DAP_OK)return 2;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=io.channel12;row[4]=io.channel13;row[5]=io.channel14;for(i=0;i<2048u;++i)row[i+6u]=m.words[i];
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
