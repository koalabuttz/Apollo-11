/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_enema_image_cases.h"
#include "cm_enema_image_dump.h"
int main(void){uint32_t n;for(n=0;n<CM_ENEMA_IMAGE_CASES;++n){apollo_overlay_state s;cm_native_io io;enum cm_enema_result status;cm_enema_image_input(&s,&io,n);status=cm_enema_image(&s,&io);if(status==CM_ENEMA_OUT_OF_SCOPE)return 2;cm_enema_image_dump(n,(uint16_t)status,&s,&io);}return 0;}
