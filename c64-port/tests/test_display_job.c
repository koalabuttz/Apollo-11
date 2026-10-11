/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "display_job_cases.h"
static apollo_overlay_state image,before;static executive state;static exec_native runtime;
int main(void){p66_bias_io io,old;agc_word channel,oldchannel;uint16_t selector,failures=0;display_job_input(&image,&io,&channel,0);if(exec_native_context_init(&runtime,&state,p66_display_job_mappings,P66_DISPLAY_JOB_MAPPING_COUNT,0)!=EXEC_OK)++failures;for(selector=P66_DISPLAY_PRIORITY_ENTER;selector<=P66_DISPLAY_BUSY;++selector){display_job_input(&image,&io,&channel,0);image.words[5]=0;before=image;old=io;oldchannel=channel;if(p66_display_job_entry(&image,&io,&channel,selector)==0||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||channel!=oldchannel)++failures;}display_job_input(&image,&io,&channel,0);before=image;old=io;oldchannel=channel;if(p66_display_job_entry(&image,&io,&channel,0xffff)!=0xffff||memcmp(&image,&before,sizeof image)||memcmp(&io,&old,sizeof io)||channel!=oldchannel)++failures;printf("60 real display mappings and22 unchanged invalid source entries: %s\n",failures?"FAIL":"PASS");return failures?1:0;}
