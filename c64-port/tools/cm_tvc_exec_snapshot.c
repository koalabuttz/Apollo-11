/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cm_tvc_exec_cases.h"
int main(int argc,char **argv){uint32_t n,limit=CM_TVC_EXEC_CASES;if(argc>1)limit=(uint32_t)strtoul(argv[1],NULL,0);
for(n=0;n<limit;++n){agc_image m;agc_word irq,ch11;uint16_t row[CM_TVC_EXEC_ROW_WORDS];unsigned i;enum cm_tvc_exec_status s;cm_tvc_exec_input(&m,&irq,&ch11,n);s=cm_tvc_exec(&m,ch11);if(s==CM_TVC_EXEC_OUT_OF_SCOPE||s==CM_TVC_EXEC_FAULT)return 2;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)s;row[3]=irq;if(s==CM_TVC_EXEC_OK)for(i=0;i<2048u;++i)row[i+4u]=m.words[i];
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
