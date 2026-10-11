/* SPDX-License-Identifier: MIT */
#include "p66_descent_control.h"
#include "p66_descent_control_payload.h"
#include <stdint.h>
typedef char selector_holds_resident_pointer[(sizeof(uintptr_t)==sizeof(uint16_t))?1:-1];
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_descent_control_arguments *args=(p66_descent_control_arguments *)(uintptr_t)selector;
 if(args==0)return 0xffff;
 return p66_descent_control_entry(image,args->io,args->channel14,P66_DESCENT_ONLY_CONTROL);
}
