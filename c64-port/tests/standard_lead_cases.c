/* SPDX-License-Identifier: MIT; input construction only. */
#include "standard_lead_cases.h"
#include "standard_lead_fixed_inputs.h"
void standard_lead_input(apollo_overlay_state *s,enum standard_lead_mission mission,uint16_t n,enum standard_lead_phase *phase,agc_word *fixed){
 static const uint16_t raw[]={0,0177777,1,0177776,037777,0140000,040000,0137777,077777,0100000,077776,0100001};
 uint16_t i,k=(uint16_t)(n%512u),address,ebuf=mission==STANDARD_LEAD_LM?01167:0364;
 agc_word pc[]={mission==STANDARD_LEAD_LM?02576:02036,mission==STANDARD_LEAD_LM?02607:02047,mission==STANDARD_LEAD_LM?02610:02050,mission==STANDARD_LEAD_LM?02611:02051,mission==STANDARD_LEAD_LM?02615:02055,mission==STANDARD_LEAD_LM?02627:02067,mission==STANDARD_LEAD_LM?02632:02607};
 *phase=(enum standard_lead_phase)(n/512u);
 for(i=0;i<2048u;++i)s->words[i]=(uint16_t)(((uint32_t)i*29u+k*251u+0x1234u)&077777u);
 s->words[0]=raw[k%12u];s->words[1]=raw[(k/12u)%12u];s->words[2]=(uint16_t)(04000u+k);
 s->words[3]=(uint16_t)((k%8u)*0400u);s->words[4]=*phase==STANDARD_LEAD_WAKE_RETURN?010000:mission==STANDARD_LEAD_LM?010000:026000;
 s->words[5]=pc[*phase];s->words[6]=(uint16_t)(s->words[4]|(s->words[3]>>8));s->words[7]=0;
 address=(uint16_t)(k%4u==0u?04000u+(k%64u):k%4u==1u?01400u+(k%255u):0300u+(k%512u));
 *fixed=standard_lead_fixed_inputs[mission][k%64u];
 if(*phase==STANDARD_LEAD_BANK_PREPARE)s->words[0133]=address;
 else if(*phase==STANDARD_LEAD_INTERP_PREPARE)s->words[0164]=(agc_word)(address-1u);
 else if(*phase>=STANDARD_LEAD_BANK_RETURN0&&*phase<=STANDARD_LEAD_BANK_RETURN2){s->words[ebuf]=(uint16_t)(0300u+k);s->words[ebuf+1u]=(uint16_t)((k%32u)*02000u+(k/32u)%8u);}
 else if(*phase==STANDARD_LEAD_INTERP_RESUME)s->words[ebuf+1u]=(uint16_t)(0300u+k);
 else if(*phase==STANDARD_LEAD_WAKE_RETURN)s->words[064]=(agc_word)(k%9u==0u?077776u:((k%(mission==STANDARD_LEAD_LM?8u:7u))*12u));
}
