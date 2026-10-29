/* SPDX-License-Identifier: MIT */
#include "imu_mode_control_cases.h"
#include "imu_coarse_cases.h"
void imu_mode_control_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_mode_stage stage,uint32_t n)
{
 static const agc_word pc[2][20]={{05457,03266,02757,02766,03234,03254,03273,03310,03210,02714,02747,03225,03231,02777,03253,03307,03321,03000,03007,02723},{05521,03151,02642,02651,03117,03137,03156,03173,03073,02577,02632,03110,03114,02662,03136,03172,03204,02663,02672,02606}};
 unsigned mode=mission==RTB_LM?01302u:01320u;
 imu_coarse_input(m,io,mission,IMU_COARSE_SET,n);m->words[5]=pc[mission][stage];io->latch.interrupt_enabled=(agc_word)(n&1u);io->channel14&=006777u;
 m->words[mode]=(agc_word)((n*43u)&AGC_MASK);m->words[mode+1u]=(agc_word)((n*133u)&AGC_MASK);
 if(n<512u){m->words[01036]=(agc_word)((n*29u)&AGC_MASK);m->words[mode]=(agc_word)(n&077u);}
}
