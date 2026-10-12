/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_tvc_daps_cases.h"
/* Per case: channel 14, a zero word, then the 2048-word input image. */
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){agc_image m;cm_tvc_daps_io io;agc_word pad=0;cm_tvc_daps_input(&m,&io,n);if(fwrite(&io.channel14,sizeof io.channel14,1,stdout)!=1||fwrite(&pad,sizeof pad,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
