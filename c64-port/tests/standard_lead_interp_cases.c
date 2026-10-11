/* SPDX-License-Identifier: MIT; input-only source caller construction. */
#include "standard_lead_interp_cases.h"
void standard_lead_interp_input(apollo_overlay_state *s,enum standard_lead_mission mission,uint16_t n){
 enum standard_lead_phase phase;agc_word fixed;uint16_t address=(uint16_t)(0300u+(n%48u));
 standard_lead_input(s,mission,(uint16_t)(2048u+n%512u),&phase,&fixed);
 s->words[0164]=(agc_word)(address-1u);s->words[address]=mission==STANDARD_LEAD_LM?047542:047502;
 s->words[0120]=(agc_word)(0401u+(n%5u)*054u);s->words[0166]=s->words[0120];s->words[067]=0;
}
