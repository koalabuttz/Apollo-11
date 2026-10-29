/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "sband_frame_cases.h"
#include "sband_frame_target.h"
static apollo_overlay_state state,before;
static int reject(uint32_t id,unsigned at,agc_word value){p66_bias_io io,saved;enum apollo_fixed_mission mission;enum sband_frame_stage stage;
 sband_frame_input(&state,&io,id,&mission,&stage);state.words[at]=value;before=state;saved=io;
 return sband_frame(&state,&io,mission,stage)==SBAND_FRAME_UNSUPPORTED&&!memcmp(&state,&before,sizeof state)&&!memcmp(&io,&saved,sizeof io);
}
int main(void){unsigned n,g;int status=sband_frame_target_check(&n);
 if(status){fprintf(stderr,"frozen frame failure %d row %u\n",status,n);return 1;}
 for(g=0;g<6u;++g){uint32_t id=g*8192u;
  if(!reject(id,5,0)||!reject(id,0163,0)||!reject(id,3,1)||!reject(id,6,1)||!reject(id,7,1)||!reject(id,01000,0100000)||!reject(id,0165,0)||!reject(id,067,1)||!reject(id,0120,0402)||!reject(id,0115,0))return 2;
 }
 puts("PASS exact original antenna frame fixtures and unchanged unsupported images");return 0;
}
