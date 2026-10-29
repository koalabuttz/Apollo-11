/* SPDX-License-Identifier: MIT */
#include "imu_mode_transfer_cases.h"
#include "imu_mode_control_cases.h"
void imu_mode_transfer_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_transfer_stage stage,uint32_t n)
{
 static const agc_word pc[2][11]={{03133,03140,03115,03712,03714,03716,03717,03726,03731,03735,03741},{03016,03023,03000,03573,03575,03577,03600,03607,03612,03616,03622}};
 unsigned mode=mission==RTB_LM?01302u:01320u;
 imu_mode_control_input(m,io,mission,IMU_MODE_ZERO_CDU,n);m->words[5]=pc[mission][stage];io->latch.interrupt_enabled=(agc_word)(n&1u);io->channel14&=006777u;
 m->words[mode]=(agc_word)((n*43u)&AGC_MASK);m->words[mode+1u]=(agc_word)((n*133u)&AGC_MASK);
 if(stage==IMU_TRANSFER_SEND)m->words[050]=m->words[051]=m->words[052]=0;
 m->words[0133]=02770;m->words[0134]=024000;m->words[071]=(agc_word)(n&3u);
 if(stage==IMU_TRANSFER_STALL)m->words[0]=(agc_word)(n&3u);
 if(stage==IMU_TRANSFER_FAILED)m->words[0]=0;
 if(stage==IMU_TRANSFER_GOOD)m->words[0]=(agc_word)(n&037777u);
 m->words[mode+2u+(n&3u)]=(agc_word)(n&AGC_MASK);
 if(n<512u){m->words[01036]=(agc_word)((n*29u)&AGC_MASK);m->words[mode+2u+(n&3u)]=(agc_word)((agc_word[]){0,077777,077776,077775,1,037777,040000,040001}[n&7u]);}
}
