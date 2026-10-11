/* SPDX-License-Identifier: MIT */
#include "standard_lead_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;apollo_overlay_state s;enum standard_lead_phase phase;agc_word fixed;for(n=0;n<STANDARD_LEAD_STAGE_CASES;++n){standard_lead_input(&s,STANDARD_LEAD_MISSION,n,&phase,&fixed);if(!standard_lead_stage(&s,STANDARD_LEAD_MISSION,phase,fixed))return 1;standard_lead_dump(n,0,&s);}return 0;}
