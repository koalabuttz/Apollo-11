/* SPDX-License-Identifier: MIT */
#include "imu_mode_control.h"
#include "agc_multiword.h"
static void bits(uint16_t *w,unsigned at,agc_word value){w[0]=agc_add_register(agc_extend((agc_word)(agc_negate(w[at])&value)),agc_extend(w[at]));w[at]=agc_correct(w[0]);}
static void zero_cdu(uint16_t *w){w[0]=0;w[032]=w[033]=w[034]=0;w[2]=3;}
static void no_att(uint16_t *w){w[0]=agc_add_register(agc_extend((agc_word)(w[01036]&037767u)),agc_extend(040000));w[01036]=agc_correct(w[0]);w[2]=3;}
enum imu_mode_status imu_mode_control(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_mode_stage stage)
{
 static const agc_word entry[2][20]={{05457,03266,02757,02766,03234,03254,03273,03310,03210,02714,02747,03225,03231,02777,03253,03307,03321,03000,03007,02723},{05521,03151,02642,02651,03117,03137,03156,03173,03073,02577,02632,03110,03114,02662,03136,03172,03204,02663,02672,02606}};
 uint16_t *w;unsigned i,mode;agc_word q;enum imu_mode_status result;
 if(!m||!io||(unsigned)mission>1u||(unsigned)stage>19u)return IMU_MODE_UNSUPPORTED;
 w=m->words;mode=mission==RTB_LM?01302u:01320u;
 if(w[5]!=entry[mission][stage]||w[4]!=016000u||w[3]!=01400u||w[6]!=016003u||w[7]||w[2]<02000u||w[2]>07777u||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>AGC_MASK||io->channel12>AGC_MASK||io->channel14>AGC_MASK||w[035]||w[036]||(w[047]&&w[047]!=077777u)||(w[050]&&w[050]!=077777u)||(w[051]&&w[051]!=077777u)||(w[052]&&w[052]!=077777u)||(io->channel14&071000u))return IMU_MODE_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return IMU_MODE_UNSUPPORTED;
 q=w[2];w[1]=agc_extend(agc_correct(w[1]));
 if(stage==IMU_MODE_ZERO_CDU){zero_cdu(w);w[5]=q;return IMU_MODE_RETURN;}
 if(stage==IMU_MODE_NO_ATT){no_att(w);w[5]=q;return IMU_MODE_RETURN;}
 if(stage>=IMU_MODE_ZERO_REGISTERED){
  if(stage==IMU_MODE_ZERO_REGISTERED){w[0]=(w[mode]&0400u)?0:0377;if(w[mode]&0400u){w[2]=(agc_word)(mission==RTB_LM?02754u:02637u);w[5]=(agc_word)(mission==RTB_LM?05567u:05650u);return IMU_MODE_ALARM;}w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;}
  if(stage==IMU_MODE_FINE_REGISTERED){w[0]=0310;w[2]=(agc_word)(mission==RTB_LM?03227u:03112u);w[5]=(agc_word)(mission==RTB_LM?05203u:05245u);return IMU_MODE_WAITLIST;}
  if(stage==IMU_MODE_WARNING_ZERO_DONE){w[5]=(agc_word)(mission==RTB_LM?03631u:03514u);return IMU_MODE_END;}
  if(stage==IMU_MODE_WARNING_FAIL_DONE){w[5]=(agc_word)(mission==RTB_LM?05261u:05324u);return IMU_MODE_TASKOVER;}
  if(stage==IMU_MODE_PIPA_ALARM_DONE){io->latch.interrupt_enabled=0;w[2]=(agc_word)(mission==RTB_LM?03306u:03171u);w[5]=(agc_word)(mission==RTB_LM?04674u:04740u);return IMU_MODE_WARNING_CALL;}
  if(stage==IMU_MODE_ZERO_ALARM_DONE){w[0]=0177777;w[mode+2u]=077777;w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;}
  if(stage==IMU_MODE_COARSE_START){enum imu_coarse_status r;io->latch.interrupt_enabled=0;if(w[mode]&040u){w[0]=0177777;w[mode+2u]=077777;w[2]=(agc_word)(entry[mission][stage]+2u);w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;}w[5]=(agc_word)(mission==RTB_LM?03144u:03027u);w[2]=(agc_word)(entry[mission][stage]+3u);r=imu_coarse(m,io,mission,IMU_COARSE_SET);if(r!=IMU_COARSE_RETURN)return IMU_MODE_UNSUPPORTED;w[0]=6;w[2]=(agc_word)(mission==RTB_LM?03005u:02670u);w[5]=(agc_word)(mission==RTB_LM?05203u:05245u);return IMU_MODE_WAITLIST;}
  w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;
 }
 if(stage==IMU_MODE_PIPA_USE){w[0]=0177777;w[037]=w[040]=w[041]=077777;}
 if(stage==IMU_MODE_FINE||stage==IMU_MODE_ZERO)io->latch.interrupt_enabled=0;
 if(stage==IMU_MODE_ZERO&&!(agc_negate(w[01036])&050u)){w[0]=0;w[2]=(agc_word)(mission==RTB_LM?02722u:02605u);w[5]=(agc_word)(mission==RTB_LM?05567u:05650u);return IMU_MODE_ALARM;}
 if(stage==IMU_MODE_ZERO2||stage==IMU_MODE_ZERO3){
  w[2]=(agc_word)(entry[mission][stage]+1u);
  if(w[mode]&040u){w[0]=037;w[5]=(agc_word)(mission==RTB_LM?03637u:03522u);return IMU_MODE_BAD;}
  w[0]=0;w[2]=3;
  if(stage==IMU_MODE_ZERO2){zero_cdu(w);io->channel12&=077757u;w[0]=02000;w[2]=entry[mission][IMU_MODE_ZERO3];w[5]=(agc_word)(mission==RTB_LM?05224u:05272u);return IMU_MODE_DELAY;}
  w[mode]&=077763u;w[mode+1u]&=077717u;w[0]=agc_extend(w[mode+1u]);w[2]=(agc_word)(entry[mission][stage]+010u);result=IMU_MODE_WARNING_CALL;
 }else if(stage==IMU_MODE_FAIL_OK||stage==IMU_MODE_PIPA_OK||stage==IMU_MODE_PIPA_USE){
  w[2]=3;w[0]=(w[mode]&040u)?0:037;
  if(w[mode]&040u){w[5]=(agc_word)(stage==IMU_MODE_PIPA_USE?(mission==RTB_LM?04631u:04675u):(mission==RTB_LM?05261u:05324u));return stage==IMU_MODE_PIPA_USE?IMU_MODE_BANK_RETURN:IMU_MODE_TASKOVER;}
  if(stage==IMU_MODE_FAIL_OK){if(io->channel12&010u){w[0]=7;w[5]=(agc_word)(mission==RTB_LM?05261u:05324u);return IMU_MODE_TASKOVER;}bits(w,mode,010000);w[mode]&=077767u;w[0]=agc_extend(w[mode]);}
  else if(stage==IMU_MODE_PIPA_OK){bits(w,mode,01000);bits(w,mode+1u,010000);w[mode]&=077757u;w[0]=agc_extend(w[mode]);}
  else{io->latch.interrupt_enabled=0;w[mode]&=077776u;w[0]=agc_extend(w[mode]);}
  w[2]=(agc_word)(stage==IMU_MODE_PIPA_USE?(mission==RTB_LM?03306u:03171u):(mission==RTB_LM?03252u:03135u));result=IMU_MODE_WARNING_CALL;
 }else if(stage==IMU_MODE_PIPA_FREE){
  io->latch.interrupt_enabled=0;bits(w,mode,1);
  if(w[mode]&01000u){w[0]=0777;w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;}
  w[0]=0;w[2]=(agc_word)(mission==RTB_LM?03320u:03203u);w[5]=(agc_word)(mission==RTB_LM?05567u:05650u);return IMU_MODE_ALARM;
 }else{
  if(w[mode]&040u){w[0]=0177777;w[mode+2u]=077777;w[2]=(agc_word)(stage==IMU_MODE_FINE?entry[mission][stage]+2u:(mission==RTB_LM?02725u:02610u));w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_MODE_EXIT;}
  if(stage==IMU_MODE_FINE){io->channel12&=077747u;w[mode+1u]&=077737u;no_att(w);w[0]=01000;w[2]=(agc_word)(mission==RTB_LM?03223u:03106u);}
  else{bits(w,mode+1u,060);bits(w,mode,014);io->channel12&=077727u;no_att(w);io->channel12|=020u;zero_cdu(w);w[0]=040;w[2]=(agc_word)(mission==RTB_LM?02745u:02630u);}
  w[5]=(agc_word)(mission==RTB_LM?05203u:05245u);return IMU_MODE_WAITLIST;
 }
 w[5]=(agc_word)(mission==RTB_LM?04674u:04740u);return result;
}

#include "imu_mode_control_literals.h"
uint8_t imu_mode_literal(enum rtb_mission mission,uint16_t at,agc_word *out){unsigned i;if((unsigned)mission>1u||!out)return 0;for(i=0;i<(mission==RTB_LM?15u:14u);++i)if(mode_literals[mission][i].address==at){*out=mode_literals[mission][i].value;return 1;}return 0;}
