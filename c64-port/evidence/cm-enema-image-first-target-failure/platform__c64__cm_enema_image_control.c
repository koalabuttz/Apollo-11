/* SPDX-License-Identifier: MIT */
#include "cm_enema_image_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_enema_image_fixture *f=CM_ENEMA_IMAGE_FIXTURE;enum cm_enema_result status;
 if(selector!=0u)return 1;status=cm_enema_image(s,CM_ENEMA_IMAGE_IO);f->status=(uint16_t)status;return status==CM_ENEMA_OUT_OF_SCOPE;
}
