/* SPDX-License-Identifier: MIT */
#include "imu_attitude_admission.h"
enum imu_attitude_status imu_attitude_admission(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_attitude_stage stage){
 static const agc_word entry[2][5]={{03750,03254,03270,03272,03267},{03627,03653,03667,03671,03666}};
 uint16_t *w;agc_word fb,at,mode;unsigned i;
 if(!m||!io||(unsigned)mission>1u||(unsigned)stage>4u)return IMU_ATTITUDE_UNSUPPORTED;
 w=m->words;fb=mission==RTB_LM&&stage!=IMU_ATTITUDE_MANEUVER?010000u:016000u;at=mission==RTB_LM?01307u:01325u;mode=mission==RTB_LM?01302u:01320u;
 if(w[5]!=entry[mission][stage]||w[4]!=fb||w[3]!=01400u||w[6]!=(fb|3u)||w[7]||w[2]<02000u||w[2]>07777u||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>AGC_MASK||io->channel12>AGC_MASK||io->channel14>AGC_MASK||(io->channel14&071000u))return IMU_ATTITUDE_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return IMU_ATTITUDE_UNSUPPORTED;
 if(stage==IMU_ATTITUDE_MANEUVER&&(w[0133]<02000u||w[0133]>07777u||(w[0134]&01777u)||w[0134]>076000u))return IMU_ATTITUDE_UNSUPPORTED;
 w[1]=agc_extend(agc_correct(w[1]));
 if(stage==IMU_ATTITUDE_MANEUVER){
  w[0]=agc_extend(w[at]);
  if(w[at]&&w[at]!=AGC_MASK){w[2]=(agc_word)(mission==RTB_LM?03754u:03633u);w[5]=(agc_word)(mission==RTB_LM?05652u:05721u);return IMU_ATTITUDE_POODOO;}
  w[0]=agc_extend(w[at]);w[1]=agc_extend(w[at+1u]);w[at]=w[0133];w[at+1u]=w[0134];w[0]=agc_add_register(agc_extend((agc_word)(w[6]&7u)),agc_extend(w[at+1u]));w[at+1u]=agc_correct(w[0]);
  w[at+2u]=(agc_word)(w[0167]&037000u);w[0]=mission==RTB_LM?03276u:03261u;w[3]=03000;w[6]=(agc_word)(w[4]|6u);w[2]=(agc_word)(mission==RTB_LM?03771u:03650u);w[5]=(agc_word)(mission==RTB_LM?04635u:04701u);return IMU_ATTITUDE_POSTJUMP;
 }
 if(stage==IMU_ATTITUDE_FLAG_RETURN){w[5]=(agc_word)(mission==RTB_LM?04631u:04675u);return IMU_ATTITUDE_BANK_RETURN;}
 if(stage==IMU_ATTITUDE_ALARM_RETURN){w[2]=(agc_word)(mission==RTB_LM?03270u:03667u);w[5]=(agc_word)(mission==RTB_LM?06001u:04106u);return IMU_ATTITUDE_GOTOPOOH;}
 if(stage==IMU_ATTITUDE_STATUS){
  w[0]=(agc_word)(w[077]&010000u);
  if(w[0])w[0]=(agc_register)(w[0]-1u);
  else{w[0]=(w[mode]&0400u)?agc_add_register(agc_extend(077767u),agc_extend(0220u)):0220u;w[2]=(agc_word)(mission==RTB_LM?03267u:03666u);w[5]=(agc_word)(mission==RTB_LM?05744u:05764u);return IMU_ATTITUDE_VARALARM;}
 }
 w[2]=(agc_word)(mission==RTB_LM?03271u:03670u);w[5]=(agc_word)(mission==RTB_LM?05504u:05546u);return IMU_ATTITUDE_UPFLAG;
}
uint8_t imu_attitude_literal(enum rtb_mission mission,uint16_t at,agc_word *out){
 static const uint16_t address[2][6]={{017754,017771,017772,011253,011271,011273},{017633,017650,017651,017652,017670,017672}};
 static const agc_word value[2][6]={{01210,044004,03276,063,7,0220},{01210,044000,03261,063,7,0220}};unsigned i;
 if((unsigned)mission>1u||!out)return 0;
 for(i=0;i<6u;++i)if(at==address[mission][i]){*out=value[mission][i];return 1;}
 return 0;
}
