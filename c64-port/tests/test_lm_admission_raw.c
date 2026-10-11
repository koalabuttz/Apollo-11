/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_admission_raw_cases.h"
#include "lm_admission_image_cases.h"
static int reject(unsigned n){apollo_overlay_state image,before;p66_bias_io io,oldio;exec_address job=lm_admission_raw_input(&image,&io,0);enum lm_admission_entry entry=LM_ADMIT_NOVAC;uint16_t *w=image.words;
 switch(n){
 case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[2]=07776;break;
 case 3:w[3]=04000;break;case 4:w[3]|=1;break;case 5:w[4]|=1;break;case 6:w[6]^=1;break;
 case 7:io.superbank=1;break;case 8:io.interrupt_enabled=2;break;case 9:w[0]=040000;break;
 case 10:w[0]=037001;break;case 11:w[2]=077;break;case 12:w[2]=02000;job.location=0100000;break;case 13:w[2]=02000;job.bank=0100000;break;
 case 14:w[067]=1;break;case 15:w[067]=96;break;case 16:w[067]=0;break;
 case 17:w[0167]=0;break;case 18:w[0400]=1;break;case 19:entry=(enum lm_admission_entry)4;break;
 case 20:entry=LM_ADMIT_SPVAC;w[5]=05116;io.interrupt_enabled=1;break;
 case 21:entry=LM_ADMIT_SPVAC;w[5]=05116;io.interrupt_enabled=0;w[063]=01000;w[0]=03275;w[1]=064067;w[2]=1;break;
 default:return 0;
 }
 before=image;oldio=io;return lm_admission_raw(&image,&io,entry,job)==EXEC_OUT_OF_SCOPE&&!memcmp(&image,&before,sizeof image)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){unsigned n;apollo_overlay_state image;p66_bias_io io;exec_address job=lm_admission_raw_input(&image,&io,0);
 for(n=0;n<22u;++n)if(!reject(n)){fprintf(stderr,"guard %u failed\n",n);return 1;}
 if(lm_admission_raw(0,&io,LM_ADMIT_NOVAC,job)!=EXEC_OUT_OF_SCOPE||lm_admission_raw(&image,0,LM_ADMIT_NOVAC,job)!=EXEC_OUT_OF_SCOPE)return 2;
 for(n=0;n<LM_ADMISSION_IMAGE_CASES;++n){apollo_overlay_state before; p66_bias_io oldio;enum exec_status oldstatus,newstatus;
 job=lm_admission_image_input(&image,&io,(uint16_t)n);before=image;oldio=io;
 oldstatus=lm_admission_image(&before,&oldio,(enum lm_admission_entry)(n%4u),job);
 newstatus=lm_admission_raw(&image,&io,(enum lm_admission_entry)(n%4u),job);
 if(oldstatus!=newstatus||memcmp(&image,&before,sizeof image)||memcmp(&io,&oldio,sizeof io)){fprintf(stderr,"compatibility%u failed\n",n);return 3;}}
 puts("PASS:16384 immutable old admission comparisons;22 invalid admission domains and2 null interfaces preserve complete canonical state and external I/O");return 0;
}
