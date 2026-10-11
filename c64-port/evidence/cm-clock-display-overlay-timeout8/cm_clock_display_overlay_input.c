/* SPDX-License-Identifier: MIT */
#include "cm_clock_display_overlay_binding.h"
#include "cm_clock_display_mission_cases.h"
#include "cm_clock_display_overlay_vectors.h"
uint16_t apollo_overlay_main(apollo_overlay_state *s,uint16_t selector){cm_mission_fixture *f=CM_CLOCK_DISPLAY_FIXTURE;
 if(selector>=8u)return 1;f->run=selector;f->source_case=cm_clock_display_overlay_indices[selector];f->step=0;f->failures=0;
 cm_clock_display_input(s,CM_CLOCK_DISPLAY_IO,f->source_case);return 0;
}
