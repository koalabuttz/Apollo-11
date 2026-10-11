/* SPDX-License-Identifier: GPL-2.0-or-later */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "imu_coarse_cases.h"
static agc_t pristine;
int main(int argc,char **argv){uint32_t n,limit=65536;unsigned mission,stage;if(argc<4||agc_load_binfile(&pristine,argv[1]))return 2;mission=(unsigned)strtoul(argv[2],NULL,0);stage=(unsigned)strtoul(argv[3],NULL,0);if(argc>4)limit=(uint32_t)strtoul(argv[4],NULL,0);InhibitAlarms=1;
 for(n=0;n<limit;++n){apollo_overlay_state m;imu_pulse_io io;unsigned i,cycles;uint16_t row[2056];enum imu_coarse_status status=IMU_COARSE_UNSUPPORTED;imu_coarse_input(&m,&io,(enum rtb_mission)mission,(enum imu_coarse_stage)stage,n);machine=pristine;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.latch.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.latch.superbank;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[014]=io.channel14;
  for(cycles=0;cycles<3000u;++cycles){if(cycles&&!machine.PendFlag&&!machine.ExtraDelay){unsigned pc=get(5);if(stage==0u&&pc==07770u&&get(2)==3u)status=IMU_COARSE_RETURN;else if(stage==1u&&pc==(mission?05272u:05224u))status=IMU_COARSE_DELAY;else if(stage==2u&&pc==(mission?03016u:03133u))status=IMU_COARSE_SEND;else if(stage==2u&&pc==(mission?05267u:05221u))status=IMU_COARSE_SETTLE;else if(stage==3u&&pc==(mission?03514u:03631u))status=IMU_COARSE_END;else if(stage==3u&&pc==(mission?05650u:05567u))status=IMU_COARSE_ALARM;else if(pc==(mission?03522u:03637u))status=IMU_COARSE_BAD;if(status!=IMU_COARSE_UNSUPPORTED)break;}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==3000u){fprintf(stderr,"coarse %u/%u/%u timeoutPC%o\n",mission,stage,n,get(5));return 2;}
  row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.OutputChannel7;row[4]=(uint16_t)machine.InputChannel[011];row[5]=(uint16_t)machine.InputChannel[012];row[6]=(uint16_t)machine.InputChannel[014];row[7]=(uint16_t)status;for(i=0;i<2048u;++i)row[i+8u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;
 }
 return 0;
}
