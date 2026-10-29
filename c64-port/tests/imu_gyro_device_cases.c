/* SPDX-License-Identifier: MIT */
#include "imu_gyro_device_cases.h"
#include "imu_pulse_cases.h"
void imu_gyro_device_input(apollo_overlay_state *m,imu_pulse_io *io,imu_gyro_device *d,enum rtb_mission mission,enum imu_gyro_clock_mode mode,uint32_t n)
{
 static const agc_word edges[]={0,1,2,15,16,31,799,800,801,1023,1024,1025,2047,2048,8192,037777};
 imu_pulse_service_input(m,io,mission,n);m->words[5]=(agc_word)(mission==RTB_LM?03637u:03522u);m->words[047]=(agc_word)(n&037777u);if(n<1024u)m->words[047]=edges[n&15u];
 io->channel14=(agc_word)((n&0740u)|((n&1u)?01000u:0u));
 d->pending=mode==IMU_GYRO_CLOCK_TIMED?(agc_word)((n*17u)%800u):(agc_word)((n*29u)&037777u);
 d->selection=(agc_word)(((n>>4)&0740u)|(mode==IMU_GYRO_CLOCK_TIMED&&(n&2u)?01000u:0u));
 d->phase=mode==IMU_GYRO_CLOCK_TIMED?(uint32_t)(n%160u):(n*101u)%128000UL;
 if(n%8u==0)d->phase=mode==IMU_GYRO_CLOCK_TIMED?159u:127999UL;
}
