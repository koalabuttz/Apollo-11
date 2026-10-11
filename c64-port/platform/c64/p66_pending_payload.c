/* SPDX-License-Identifier: MIT */
#include "p66_pending_payload.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_pending_arguments *arguments=(p66_pending_arguments *)(uintptr_t)selector;
 if(!arguments||!arguments->binding||
  arguments->binding->recurrence.application.display_job.descent.basic.arithmetic.image!=image)return 0xffff;
 return p66_application_pending_entry(arguments->binding,arguments->resume,arguments->service);
}
