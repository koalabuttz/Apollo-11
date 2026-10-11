/* SPDX-License-Identifier: MIT */
#include "imu_attitude_admission_cases.h"
#include "imu_mode_control_cases.h"
void imu_attitude_admission_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_attitude_stage stage,uint32_t n){
 static const agc_word pc[2][5]={{03750,03254,03270,03272,03267},{03627,03653,03667,03671,03666}};unsigned at=mission==RTB_LM?01307u:01325u;
 imu_mode_control_input(m,io,mission,IMU_MODE_ZERO_CDU,n);m->words[5]=pc[mission][stage];m->words[4]=mission==RTB_LM&&stage!=IMU_ATTITUDE_MANEUVER?010000u:016000u;m->words[6]=(agc_word)(m->words[4]|3u);
 m->words[0133]=(agc_word)(02000u+(n&01777u));m->words[0134]=(agc_word)(((n/16u)%32u)*02000u);m->words[at]=(agc_word)(n&AGC_MASK);m->words[077]=(agc_word)((n*17u)&AGC_MASK);
 if(n<512u)m->words[at]=(agc_word)((agc_word[]){0,077777,1,077776,037777,040000,2,077775}[n&7u]);
}
