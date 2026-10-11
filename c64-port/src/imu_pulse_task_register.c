/* SPDX-License-Identifier: MIT */
#include "imu_pulse_task.h"
#include "lm_waitlist_image.h"
#include "cm_waitlist_image.h"
#include <string.h>
enum imu_gyro_task_status imu_gyro_register(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 uint16_t *w=m->words;uint8_t longer;exec_address task;
 if((unsigned)mission>1u||w[5]!=(mission==RTB_LM?05203u:05245u)||w[4]!=016000u)return IMU_GYRO_UNSUPPORTED;
 if(w[2]==(mission==RTB_LM?03554u:03437u))longer=1;
 else if(w[2]==(mission==RTB_LM?03535u:03420u))longer=0;
 else return IMU_GYRO_UNSUPPORTED;
 task.location=(agc_word)(mission==RTB_LM?(longer?03557u:03405u):(longer?03442u:03270u));task.bank=(agc_word)(mission==RTB_LM?016103u:016063u);
 if(mission==RTB_LM){if(lm_waitlist_image(m,&io->latch,LM_WAITLIST_ENTRY,task)!=WL_OK)return IMU_GYRO_UNSUPPORTED;}
 else{cm_native_io cm;memset(&cm,0,sizeof cm);cm.superbank=io->latch.superbank;cm.interrupt_enabled=io->latch.interrupt_enabled;if(cm_waitlist_image(m,&cm,task)!=CM_WL_OK)return IMU_GYRO_UNSUPPORTED;io->latch.superbank=cm.superbank;io->latch.interrupt_enabled=cm.interrupt_enabled;}
 io->channel14=(agc_word)(io->channel14|01000u);w[0]=agc_extend(io->channel14);w[5]=(agc_word)(mission==RTB_LM?03542u:03425u);return IMU_GYRO_TASKOVER;
}
