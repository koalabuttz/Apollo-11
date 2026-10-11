/* SPDX-License-Identifier: MIT */
#include "p66_application_split_payload.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t selector){
 p66_application_split_arguments *args=(p66_application_split_arguments *)(uintptr_t)selector;
 if(!args||!args->binding||args->binding->display_job.descent.basic.arithmetic.image!=image)return 0xffff;
 return p66_application_split_service(args->binding,args->service);
}
