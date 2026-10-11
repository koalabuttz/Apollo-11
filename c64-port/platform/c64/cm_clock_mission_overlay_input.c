/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission_overlay_binding.h"
#include "cm_clock_mission_cases.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_CLOCK_MISSION_FIXTURE;
 if(selector>=7u)return 1;f->run=selector;f->source_case=(uint16_t)(selector*64u);f->step=0;f->failures=0;
 cm_clock_mission_input(s,CM_CLOCK_MISSION_IO,f->source_case);return 0;
}
