/* SPDX-License-Identifier: MIT */
#include "imu_gyro_device.h"
uint8_t imu_gyro_device_step(apollo_overlay_state *m,imu_pulse_io *io,imu_gyro_device *d,enum imu_gyro_clock_mode mode,agc_word events[IMU_GYRO_OUTPUT_EVENTS],uint8_t *count)
{
 uint8_t n=0;uint32_t boundary;agc_word command=m->words[047];
 if((unsigned)mode>1u||command>037777u||io->channel14>077777u||d->pending>037777u||(d->selection&~(mode==IMU_GYRO_CLOCK_TIMED?01740u:0740u)))return 0;
 boundary=mode==IMU_GYRO_CLOCK_TIMED?160UL:128000UL;
 if(d->phase>=boundary||(mode==IMU_GYRO_CLOCK_TIMED&&d->pending>=800u))return 0;
 if(mode==IMU_GYRO_CLOCK_BURST&&command&&(io->channel14&01000u)){
  while(d->pending){agc_word batch=d->pending>03777u?03777u:d->pending;events[n++]=(agc_word)((d->selection<<6)|batch);d->pending=(agc_word)(d->pending-batch);}
  d->pending=command;m->words[047]=0;d->selection=(agc_word)(io->channel14&0740u);d->phase=127994UL;
 }
 d->phase+=6u;
 if(d->phase>=boundary){
  d->phase-=boundary;
  if(mode==IMU_GYRO_CLOCK_TIMED){if(command&&(io->channel14&01000u)){++d->pending;m->words[047]=(agc_word)(command-1u);if(!m->words[047])io->channel14=(agc_word)(io->channel14&076777u);}}
  else if(d->pending){agc_word batch=d->pending>1024u?1024u:d->pending;events[n++]=(agc_word)((d->selection<<6)|batch);d->pending=(agc_word)(d->pending-batch);}
 }
 if(mode==IMU_GYRO_CLOCK_TIMED){agc_word selected=(agc_word)(io->channel14&01740u);if(selected!=d->selection||d->pending>=800u){events[n++]=(agc_word)(((d->selection&0740u)<<6)|d->pending);d->selection=selected;d->pending=0;}}
 *count=n;return 1;
}
