/* SPDX-License-Identifier: MIT; constructors have no native result links. */
#include "standard_lead_bank_cases.h"
#include "standard_lead_cases.h"
void standard_lead_bank_input(apollo_overlay_state *s,enum standard_lead_mission mission,uint16_t n){
 enum standard_lead_phase phase;agc_word fixed;uint16_t address,physical,ebuf=mission==STANDARD_LEAD_LM?01167:0364;
 standard_lead_input(s,mission,(uint16_t)(n%512u),&phase,&fixed);
 address=(uint16_t)(n%2u?01400u+n%255u:0300u+n%256u);
 physical=address<01400u?address:(uint16_t)(s->words[3]+(address&0377u));
 while(physical==ebuf||physical==ebuf+1u||physical==0133u||physical==0134u||physical<0100u){++address;physical=address<01400u?address:(uint16_t)(s->words[3]+(address&0377u));}
 /* Byte-original phase-table TCQ is a genuine TC Q in common fixed bank3;
  * SWCALL enters its identical banked alias. The callee executes one native
  * TC-Q continuation; no fabricated instruction is placed in the rope. */
 s->words[0133]=address;s->words[physical]=mission==STANDARD_LEAD_LM?06742:06711;
 s->words[0134]=(agc_word)((n%32u)*02000u);
}
