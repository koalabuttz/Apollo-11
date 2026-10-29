/* SPDX-License-Identifier: MIT */
#include "cm_enema_image_binding.h"
#include "cm_enema_image_cases.h"
#define CM_ENEMA_IMAGE_INDEX_ONLY 1
#include "cm_enema_image_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_enema_image_fixture *f=CM_ENEMA_IMAGE_FIXTURE;
 if(selector>=CM_ENEMA_IMAGE_TARGET_CASES)return 1;f->run=selector;f->source_case=cm_enema_image_indices[selector];f->status=f->step=f->failures=0;cm_enema_image_input(s,CM_ENEMA_IMAGE_IO,f->source_case);return 0;
}
