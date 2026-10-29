/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "service_routines_cases.h"
#include "service_routines_target.h"
static apollo_overlay_state s,before,whole;
int main(void){unsigned completed;uint16_t iterations,i;uint32_t address;agc_word literal;unsigned hits;p66_bias_io io,prior,io2;enum service_routines_mission mission;enum service_routines_entry entry;agc_word fixed[3];
 for(i=0;i<2u;++i){hits=0;for(address=0;address<65536u;++address){literal=012345;if(service_routines_literal((enum service_routines_mission)i,(uint16_t)address,&literal))++hits;else assert(literal==012345);}assert(hits==3u);}
 assert(service_routines_target_check(&completed)==0&&completed==416u);
 for(i=0;i<2u;++i){service_routines_input(&s,&io,(uint32_t)(i?11u:0u)*4096u,&mission,&entry,fixed);s.words[3]=1;before=s;prior=io;assert(!service_routines_step(&s,&io,mission,entry,0,0,0));assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&prior,sizeof io));}
 service_routines_input(&s,&io,24u*4096u+63u,&mission,&entry,fixed);whole=s;io2=io;iterations=0;assert(service_routines_block(&whole,&io2,mission,0,0,0,0,1024,&iterations)==SERVICE_BLOCK_DONE&&iterations==64u);
 for(i=0;i<64u;++i){enum service_routines_block_status status;iterations=0;status=service_routines_block(&s,&io,mission,0,0,0,0,1,&iterations);assert(iterations==1&&status==(i==63u?SERVICE_BLOCK_DONE:SERVICE_BLOCK_LOOP));}
 assert(!memcmp(&s,&whole,sizeof s)&&!memcmp(&io,&io2,sizeof io));
 service_routines_input(&s,&io,8u*4096u+7u,&mission,&entry,fixed);before=s;prior=io;iterations=17;assert(service_routines_block(&s,&io,mission,0,0,0,0,1024,&iterations)==SERVICE_BLOCK_BAD);assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&prior,sizeof io)&&iterations==17u);
 service_routines_input(&s,&io,8u*4096u,&mission,&entry,fixed);s.words[0]=040000;before=s;prior=io;assert(!service_routines_step(&s,&io,mission,entry,0,0,0));assert(!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&prior,sizeof io));
 puts("PASS416 exact original states, unchanged invalid guards, full/chunked native transfer continuation and missing passive resolver preflight");return 0;
}
