/* SPDX-License-Identifier: MIT */
#include "sband_plane.h"
#include "sband_vector.h"
#include "sband_shift.h"
#include "rtb_unit.h"
#include "agc_multiword.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
static void add_pair(apollo_overlay_state *m,unsigned at){agc_register_pair r={m->words[0],m->words[1]};agc_dp p={m->words[at],m->words[at+1u]};agc_double_add(&r,&p);m->words[at]=p.high;m->words[at+1u]=p.low;m->words[0]=r.a;m->words[1]=r.l;}
static void component_add(apollo_overlay_state *m,unsigned at,uint8_t offset){add_pair(m,at);if(m->words[0]){agc_word limit=m->words[0]==1u?037777u:040000u;agc_register a;int overflow;m->words[0130]=limit;m->words[1]=offset;a=agc_add_register(agc_extend(limit),m->words[0]==1u?1u:0177776u);a=agc_add_register(a,agc_extend(m->words[at+1u]));m->words[at+1u]=agc_correct(a);overflow=agc_overflow(a);a=overflow?(overflow>0?1u:0177776u):0u;a=agc_add_register(a,agc_extend(limit));a=agc_add_register(a,agc_extend(m->words[at]));m->words[at]=agc_correct(a);overflow=agc_overflow(a);if(overflow){a=overflow>0?1u:0177776u;m->words[0121]=agc_correct(a);}m->words[0]=a;m->words[2]=3;}}
static void reverse_component(apollo_overlay_state *s,unsigned operand,unsigned at,uint8_t offset){agc_word h=s->words[at],l=s->words[at+1u];s->words[at]=s->words[operand];s->words[at+1u]=s->words[operand+1u];s->words[0]=agc_extend(agc_negate(h));s->words[1]=agc_extend(agc_negate(l));component_add(s,at,offset);}
static uint8_t overflow_branch(apollo_overlay_state *s){agc_word flag=s->words[0121];if(flag==0)return 0;if(flag!=AGC_NEG_ZERO){s->words[0121]=agc_diminished_abs(flag);}return 1;}
enum sband_plane_status sband_plane(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission){
 static const uint8_t positions[6]={0,1,3,4,5,6};static const agc_word unity[6]={0,0,020000,0,0,0},unitz[6]={0,0,0,0,020000,0};
 unsigned i,base,stack,r;agc_word eb,low;uint8_t cm,branch;enum rtb_unit_status status;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return SBAND_PLANE_UNSUPPORTED;
 cm=(uint8_t)(mission==APOLLO_FIXED_CM);base=s->words[0120];eb=s->words[3];stack=base+(cm?010u:0u);r=cm?base+2u:02204u;
 if(s->words[5]!=(cm?06030u:06061u)||s->words[0164]!=(cm?03713u:03670u)||s->words[023]!=062u||s->words[0163]!=077776u||s->words[4]||eb>03400u||(eb&0377u)||s->words[6]!=(eb>>8)||s->words[0165]!=(agc_word)(064000u|(eb>>8))||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=stack||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SBAND_PLANE_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return SBAND_PLANE_UNSUPPORTED;
 /* Source PDVL reads each original R pair, exchanges MPAC into the stack,
  * and ends with the old destination stack-Z pair in A/L. R and stack are
  * distinct for both exact caller layouts. MPAC+2 is left unchanged. */
 low=s->words[stack+5u];for(i=0;i<6u;++i){s->words[stack+i]=s->words[0154u+positions[i]];s->words[0154u+positions[i]]=s->words[r+i];}
 s->words[1]=agc_extend(low);s->words[0166]=(agc_word)(stack+6u);
 bank(s,eb,cm?054000u:046000u);s->words[5]=cm?07374u:07425u;s->words[0116]=cm?03326u:02515u;s->words[0164]=cm?03716u:03673u;s->words[020]=020031;s->words[023]=025;
 (void)sband_vector_projection_fixed(s,mission,cm?unitz:unity);
 bank(s,eb,0);s->words[5]=02121;s->words[020]=020001;s->words[023]=0;(void)sband_shift_left(s,mission,2);
 bank(s,cm?eb:02000u,064000);s->words[0116]=cm?(agc_word)r:01604u;s->words[0164]=cm?03720u:03675u;s->words[020]=020026;s->words[023]=0177;
 reverse_component(s,r+2u,0157,3);reverse_component(s,r+4u,0161,5);reverse_component(s,r,0154,0);
 /* Both outcomes of this first genuine BOV reach COVCNV. Its CCS still
  * updates a nonzero ordinary flag to diminished magnitude; -0 skips TS. */
 s->words[0117]=cm?065722u:065677u;(void)overflow_branch(s);
 bank(s,eb,0);s->words[0]=011;s->words[5]=03023;s->words[020]=5;s->words[023]=0177;s->words[0164]=cm?03722u:03677u;
 status=rtb_unit_begin(s,io,cm?RTB_CM:RTB_LM);if(status==RTB_UNIT_OUT_OF_SCOPE)return SBAND_PLANE_UNSUPPORTED;
 for(i=0;i<3u;++i){status=rtb_unit_phase(s,io,cm?RTB_CM:RTB_LM,(uint8_t)i);if(status==RTB_UNIT_ABORT_1302)return SBAND_PLANE_ABORT_1302;if(status==RTB_UNIT_OVERFLOW)break;}
 s->words[0117]=cm?065737u:065745u;branch=overflow_branch(s);bank(s,eb,064000);s->words[023]=0;s->words[020]=branch?070007u:060017u;s->words[0]=branch?agc_extend(040000u):0u;s->words[5]=cm?06037u:06070u;s->words[0164]=branch?(cm?03737u:03745u):(cm?03724u:03701u);
 return branch?SBAND_PLANE_NULL_EXIT:SBAND_PLANE_OK;
}
