/* SPDX-License-Identifier: MIT */
#include "sband_frame.h"
#include "rtb_unit.h"
#include "lm_powered_interface.h"
#include "lm_powered_trig.h"
#include "matrix_native.h"
#include "sband_shift.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
static uint8_t common(const apollo_overlay_state *s,const p66_bias_io *io){unsigned i,b=s->words[0120];
 if(s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||s->words[0115]!=040000u||s->words[067]||s->words[0163]!=077776u||b<0401u||b>0661u||(b-0401u)%054u||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return 0;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return 0;
 return 1;
}
enum sband_frame_status sband_frame(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission,enum sband_frame_stage stage){unsigned i,base;uint8_t cm;agc_word eb;
#ifdef SBAND_FRAME_ONLY_STAGE
 if(stage!=SBAND_FRAME_ONLY_STAGE)return SBAND_FRAME_UNSUPPORTED;
#endif
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||(unsigned)stage>SBAND_FRAME_SHIFT||!common(s,io))return SBAND_FRAME_UNSUPPORTED;
 cm=(uint8_t)(mission==APOLLO_FIXED_CM);base=s->words[0120];eb=(agc_word)((s->words[0165]&7u)<<8);
 if((s->words[0165]&077770u)!=064000u)return SBAND_FRAME_UNSUPPORTED;
 if(stage==SBAND_FRAME_UNIT){enum rtb_unit_status status;
  if(cm||s->words[5]!=03023u||s->words[4]||s->words[0164]!=03641u||s->words[023])return SBAND_FRAME_UNSUPPORTED;
  status=rtb_unit_begin(s,io,RTB_LM);if(status!=RTB_UNIT_OK)return SBAND_FRAME_UNSUPPORTED;
  for(i=0;i<3u;++i){status=rtb_unit_phase(s,io,RTB_LM,(uint8_t)i);if(status==RTB_UNIT_ABORT_1302)return SBAND_FRAME_ABORT_1302;if(status==RTB_UNIT_OVERFLOW)return SBAND_FRAME_OK;}
  return SBAND_FRAME_OK;
 }
 if(stage==SBAND_FRAME_AFTER_UNIT){static const uint8_t positions[6]={0,1,3,4,5,6};
  if(cm||s->words[5]!=06061u||s->words[4]||s->words[0164]!=03641u||s->words[023])return SBAND_FRAME_UNSUPPORTED;
  for(i=0;i<6u;++i)s->words[0154u+positions[i]]=agc_negate(s->words[0154u+positions[i]]);
  s->words[base+052u]=065644;s->words[0117]=047537;s->words[0164]=03537;s->words[020]=050006;s->words[023]=0;s->words[0]=agc_extend(040000);
  bank(s,eb,046000);s->words[0165]=s->words[6];s->words[5]=06070;
  (void)lm_powered_interface_start(s,0);(void)lm_powered_cdu_start(s,1);
  for(i=0;i<3u;++i){(void)lm_powered_cdu_cosine(s);(void)lm_powered_cdu_sine(s);(void)lm_powered_cdu_next(s);}
  (void)lm_powered_interface_reentry(s,&io->interrupt_enabled);(void)lm_powered_interface_return(s);
  s->words[0116]=01733;s->words[0164]=03645;s->words[023]=5;s->words[020]=020013;s->words[0]=01733;s->words[5]=07334;bank(s,01400,064000);
  return SBAND_FRAME_OK;
 }
 if(stage==SBAND_FRAME_MATRIX){agc_word matrix[18];unsigned at=cm?01735u:01733u;
  if(s->words[5]!=(cm?07303u:07334u)||s->words[4]!=064000u||s->words[3]!=01400u||s->words[0164]!=(cm?03703u:03645u)||s->words[023]!=(cm?0u:5u)||s->words[0116]!=at)return SBAND_FRAME_UNSUPPORTED;
  for(i=0;i<18u;++i)matrix[i]=s->words[01400u+(at&0377u)+i];
  for(i=0;i<4u;++i)(void)matrix_native_phase(s,mission,0,matrix,(uint8_t)i);
  return SBAND_FRAME_OK;
 }
 if(s->words[5]!=(cm?06030u:06061u)||s->words[4]!=064000u||s->words[3]!=01400u||s->words[0164]!=(cm?03703u:03645u)||s->words[023]!=(cm?0u:5u))return SBAND_FRAME_UNSUPPORTED;
 s->words[023]=cm?052u:0;s->words[0164]=cm?03704u:03645u;s->words[020]=020000;bank(s,eb,0);s->words[5]=02121;
 (void)sband_shift_left(s,mission,1);return SBAND_FRAME_OK;
}
