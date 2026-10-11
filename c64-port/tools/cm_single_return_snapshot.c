/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_single_return_cases.h"
int main(void){uint16_t row[2052];while(fread(row,sizeof row,1,stdin)==1){apollo_overlay_state m;unsigned i;if(row[2]!=0)return 2;for(i=0;i<2048u;++i)m.words[i]=row[i+4u];if(cm_single_return(&m)!=CM_TRIG_OK)return 2;row[2]=1;for(i=0;i<2048u;++i)row[i+4u]=m.words[i];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;if(fread(row,sizeof row,1,stdin)!=1||row[2]!=1)return 2;}return 0;}
