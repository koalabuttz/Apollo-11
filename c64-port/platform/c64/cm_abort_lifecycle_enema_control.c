/* SPDX-License-Identifier: MIT */
#include "cm_abort_lifecycle_binding.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_lifecycle_fixture *f=CM_LIFECYCLE_FIXTURE;enum cm_enema_result status;if(selector!=13u)return 1;status=cm_enema_image(s,CM_LIFECYCLE_IO);f->status=(uint16_t)status;return status==CM_ENEMA_OUT_OF_SCOPE;}
