/* SPDX-License-Identifier: MIT */
#include "p66_display_job.h"
#include "p66_display_job_payload.h"
#include <stdint.h>
typedef char display_selector_holds_pointer[(sizeof(uintptr_t)==sizeof(uint16_t))?1:-1];
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t argument){p66_display_job_arguments *args=(p66_display_job_arguments *)(uintptr_t)argument;if(args==0)return 0xffff;return p66_display_job_entry(image,args->io,args->channel11,P66_DISPLAY_JOB_ONLY_SELECTOR);}
