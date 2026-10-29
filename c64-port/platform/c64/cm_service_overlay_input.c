/* SPDX-License-Identifier: MIT */
#include "cm_service_overlay_binding.h"
#include "cm_service_loop_cases.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_service_fixture *f=CM_SERVICE_FIXTURE;
 if(selector>=6u)return 1;f->run=selector;f->source_case=(uint16_t)(selector*64u);f->step=0;f->failures=0;
 cm_service_loop_input(s,CM_SERVICE_IO,f->source_case);return 0;
}
