/* SPDX-License-Identifier: MIT */
#include "cm_mission_overlay_binding.h"
#include "cm_mission_loop_cases.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_MISSION_FIXTURE;
 if(selector>=6u)return 1;f->run=selector;f->source_case=(uint16_t)(selector*64u);f->step=0;f->failures=0;
 cm_mission_loop_input(s,CM_MISSION_IO,f->source_case);return 0;
}
