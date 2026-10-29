/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdlib.h>
#include "rtb_angles_cases.h"
int main(int argc,char **argv){uint32_t n;unsigned mission,entry;if(argc!=3)return 2;mission=(unsigned)strtoul(argv[1],0,0);entry=(unsigned)strtoul(argv[2],0,0);while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;p66_bias_io io;rtb_angles_input(&m,&io,(enum rtb_mission)mission,(enum rtb_angle_entry)entry,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
