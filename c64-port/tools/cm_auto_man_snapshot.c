/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cm_auto_man_cases.h"
int main(int argc,char **argv){uint32_t n,limit=CM_AUTO_MAN_CASES;if(argc>1)limit=(uint32_t)strtoul(argv[1],NULL,0);
for(n=0;n<limit;++n){agc_image m;cm_auto_man_io io;uint16_t row[CM_AUTO_MAN_ROW_WORDS];unsigned i;cm_auto_man_input(&m,&io,n);if(cm_auto_man(&m,&io)!=CM_AUTO_MAN_OK)return 2;
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);for(i=0;i<2048u;++i)row[i+5u]=m.words[i];
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
