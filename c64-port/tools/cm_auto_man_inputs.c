/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_auto_man_cases.h"
/* Per case: channels 31 and 32, then the 2048-word input image. */
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){agc_image m;cm_auto_man_io io;cm_auto_man_input(&m,&io,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
