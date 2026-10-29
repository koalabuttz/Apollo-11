/* SPDX-License-Identifier: MIT; isolated epilogue inputs, no native values. */
#include "standard_lead_resume_cases.h"
#include "standard_lead_cases.h"
void standard_lead_resume_input(apollo_overlay_state *s,enum standard_lead_mission mission,uint16_t n){enum standard_lead_phase phase;agc_word fixed;
 standard_lead_input(s,mission,(uint16_t)(2560u+n%512u),&phase,&fixed);s->words[0120]=(agc_word)(0401u+(n%5u)*054u);s->words[067]=0;
}
