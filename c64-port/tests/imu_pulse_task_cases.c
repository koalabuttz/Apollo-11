/* SPDX-License-Identifier: MIT */
#include "imu_pulse_task_cases.h"
#include "imu_pulse_cases.h"
void imu_pulse_task_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,uint8_t augment,uint32_t n)
{
 unsigned phase=(unsigned)((n>>8)&3u),base=02000u+((n>>11)&3u)*0400u+n%240u,at,i;agc_word lg;
 imu_pulse_service_input(m,io,mission,n);m->words[5]=(agc_word)(mission==RTB_LM?(augment?03557u:03405u):(augment?03442u:03270u));
 lg=(agc_word)(base+phase*010000u+(phase==1u?2u:phase==2u?4u:0u));m->words[mission==RTB_LM?01314u:01304u]=lg;
 m->words[mission==RTB_LM?01302u:01320u]=(agc_word)((n%17u==0)?040u:0u);
 for(i=0;i<3u;++i){agc_word high=(agc_word)((n*(i*2u+1u))&077777u),low=(agc_word)((n*(i*18u+5u)+i*777u)&037777u);if(high&040000u)low=agc_negate(low);m->words[base+i*2u]=high;m->words[base+i*2u+1u]=low;}
 if(n%19u==0)for(i=0;i<6u;++i)m->words[base+i]=0;
 if(augment){at=lg&03777u;m->words[at]=(agc_word)(n%512u);io->channel14&=076777u;io->channel12=(agc_word)((n%23u==0)?010u:0u);}
}
