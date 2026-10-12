/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_entry_dap_cases.h"
/* Per case: channels 5 and 6, then the 2048-word input image. */
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){agc_image m;cm_entry_dap_io io;cm_entry_dap_input(&m,&io,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
