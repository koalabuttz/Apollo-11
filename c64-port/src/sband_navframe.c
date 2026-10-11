/* SPDX-License-Identifier: MIT */
#include "sband_navframe.h"
#include "sband_scalar.h"
#include "rtb_unit.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
static void scalar_entry(apollo_overlay_state *s,agc_word eb,agc_word pc,agc_word addr,agc_word loc,agc_word cyr,agc_word edop){bank(s,eb,064000);s->words[5]=pc;s->words[0116]=addr;s->words[0164]=loc;s->words[020]=cyr;s->words[023]=edop;}
static void store_vector(apollo_overlay_state *s,unsigned at){static const uint8_t positions[6]={0,1,3,4,5,6};unsigned i;for(i=0;i<6u;++i)s->words[at+i]=s->words[0154u+positions[i]];}
enum sband_navframe_status sband_navframe(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission){
 static const agc_word inverse_sqrt_two[2]={026501,007463};static const uint8_t positions[6]={0,1,3,4,5,6};
 unsigned i,base,at;agc_word eb,low;uint8_t cm;enum rtb_unit_status status;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return SBAND_NAVFRAME_UNSUPPORTED;
 cm=(uint8_t)(mission==APOLLO_FIXED_CM);base=s->words[0120];eb=s->words[3];
 if(s->words[5]!=(cm?06037u:06070u)||s->words[0164]!=(cm?03712u:03654u)||s->words[023]||s->words[0163]!=077776u||s->words[4]!=064000u||eb>03400u||(eb&0377u)||s->words[6]!=(agc_word)(064000u|(eb>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=base+(cm?010u:0u)||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SBAND_NAVFRAME_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return SBAND_NAVFRAME_UNSUPPORTED;
 if(cm){at=base+2u;low=s->words[at+5u];store_vector(s,at);s->words[1]=agc_extend(low);s->words[0116]=(agc_word)at;}
 else{
  low=s->words[0155];store_vector(s,02204);s->words[0154]=s->words[02206];s->words[0155]=s->words[02207];s->words[0156]=0;s->words[0163]=0;s->words[1]=agc_extend(low);
  /* PUSH DP leaves MPAC loaded. The following DCS discards its old-stack
   * A/L debris, while the stored original Y pair is later popped by DLOAD. */
  s->words[base]=s->words[0154];s->words[base+1u]=s->words[0155];s->words[0166]=(agc_word)(base+2u);
  scalar_entry(s,02000,07005,01604,03657,020032,0);(void)sband_scalar(s,mission,SBAND_DSU,0);
  scalar_entry(s,eb,07572,03770,03661,020036,0);(void)sband_scalar(s,mission,SBAND_DMP,inverse_sqrt_two);
  low=s->words[0155];s->words[02206]=s->words[0154];s->words[02207]=low;
  s->words[0154]=s->words[base];s->words[0155]=s->words[base+1u];s->words[0156]=0;s->words[1]=agc_extend(low);s->words[0166]=(agc_word)base;
  scalar_entry(s,02000,06775,01604,03664,020034,0172);(void)sband_scalar(s,mission,SBAND_DAD,0);
  scalar_entry(s,eb,07572,03770,03665,020036,0);(void)sband_scalar(s,mission,SBAND_DMP,inverse_sqrt_two);
  s->words[02204]=s->words[0154];s->words[02205]=s->words[0155];low=s->words[0162];
  for(i=0;i<6u;++i){s->words[0154u+positions[i]]=s->words[02204u+i];}s->words[1]=agc_extend(low);s->words[0163]=077776;s->words[0116]=01604;
 }
 bank(s,eb,0);s->words[0]=011;s->words[2]=3;s->words[5]=03023;s->words[020]=5;s->words[023]=062;s->words[0164]=cm?03713u:03670u;
 status=rtb_unit_begin(s,io,cm?RTB_CM:RTB_LM);if(status==RTB_UNIT_OUT_OF_SCOPE)return SBAND_NAVFRAME_UNSUPPORTED;
 for(i=0;i<3u;++i){status=rtb_unit_phase(s,io,cm?RTB_CM:RTB_LM,(uint8_t)i);if(status==RTB_UNIT_ABORT_1302)return SBAND_NAVFRAME_ABORT_1302;if(status==RTB_UNIT_OVERFLOW)return SBAND_NAVFRAME_OK;}
 return SBAND_NAVFRAME_OK;
}
