/* SPDX-License-Identifier: MIT */
#include "imu_standby_cases.h"
#include "imu_mode_control_cases.h"
void imu_standby_input(apollo_overlay_state *m,imu_standby_io *io,enum rtb_mission mission,enum imu_standby_stage stage,uint32_t n){
 static const agc_word entry[2][21]={{03655,03657,03664,03665,03672,03674,03676,03706,03711,03712,03713,03714,03717,03734,03744,03745,03752,03755,03763,03767,03773},{03660,03662,03667,03670,03675,03677,03701,03711,03714,03715,03716,03717,03722,03737,03747,03750,03755,03760,03766,03772,03776}};
 imu_mode_control_input(m,&io->pulse,mission,IMU_MODE_ZERO_CDU,n);m->words[5]=entry[mission][stage];m->words[3]=(agc_word)(((n/8u)%8u)*0400u);m->words[4]=mission==RTB_LM?076000u:054000u;m->words[6]=(agc_word)(m->words[4]|(m->words[3]>>8));io->channel13=(agc_word)((n*117u)&043777u);if(mission==RTB_LM)io->pulse.latch.superbank&=060u;
 if(stage==IMU_STANDBY_SCAL_BEGIN){m->words[2]=entry[mission][n&1u?IMU_STANDBY_PRE_RETRY:IMU_STANDBY_POST_RETRY];io->pulse.latch.interrupt_enabled=0;}
 if(stage==IMU_STANDBY_SCAL_FINISH){m->words[2]=3;m->words[0156]=entry[mission][n&1u?IMU_STANDBY_PRE_RETRY:IMU_STANDBY_POST_RETRY];io->pulse.latch.interrupt_enabled=0;m->words[0]=(agc_word)(n&037777u);m->words[1]=(agc_word)((n*39u)%037777u);}
 if(stage==IMU_STANDBY_PRE_READY||stage==IMU_STANDBY_PRE_RETRY||stage==IMU_STANDBY_POST_READY||stage==IMU_STANDBY_POST_RETRY){m->words[2]=0157;m->words[1]=0;m->words[0]=stage==IMU_STANDBY_PRE_RETRY||stage==IMU_STANDBY_POST_RETRY?0:(agc_word)(n%31u);m->words[0156]=entry[mission][stage];}
 if(stage==IMU_STANDBY_POST_SHORT_RETURN){m->words[2]=3;m->words[1]=0;}
 if(stage==IMU_STANDBY_POST_AGREE_RETURN||stage==IMU_STANDBY_POST_TIME_RETURN){m->words[2]=mission==RTB_LM?07304u:07273u;m->words[1]=entry[mission][stage];m->words[0]=(agc_register)((agc_register[]){0,1,0177776}[n%3u]);}
}
