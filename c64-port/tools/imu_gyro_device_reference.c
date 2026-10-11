/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only peripheral oracle. Unmodified Virtual AGC engine included solely
 * to reset its file-private peripheral state between isolated observations.
 * No engine code is compiled into the native C64 port. One eligible engine
 * invocation starts the unchanged original IMUBAD CAF ZERO instruction; its
 * two-cycle pending delay prevents any instruction or canonical CPU write.
 * Explicit source engine global device state is compared through native ABI.
 */
#define main cm_reference_main
#define ChannelOutput reference_unused_output
#include "cm_executive_reference.c"
#undef ChannelOutput
#undef main
void ChannelOutput(agc_t *,int,int);
#include "../deps/virtualagc/yaAGC/agc_engine.c"
#include "imu_gyro_device_cases.h"
int DedaMonitor,DebugDsky,InhibitAlarms,ShowAlarms,DebugMode;
uint64_t DedaWhen;
static agc_t baseline;
static agc_word observed[18];static unsigned observed_count;
void ChannelOutput(agc_t *s,int c,int v){if(c==0177){if(observed_count>=18u){fprintf(stderr,"event overflow\n");exit(2);}observed[observed_count++]=(agc_word)v;}else reference_unused_output(s,c,v);}
int main(int argc,char **argv){uint32_t n,limit=IMU_GYRO_DEVICE_CASES;unsigned mission,mode;if(argc<4)return 2;mission=(unsigned)strtoul(argv[2],0,0);mode=(unsigned)strtoul(argv[3],0,0);
#ifdef GYRO_TIMING_SIMULATED
 if(mode!=1u)return 2;
#else
 if(mode!=0u)return 2;
#endif
 if(argc>4)limit=(uint32_t)strtoul(argv[4],0,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;for(n=0;n<limit;++n){apollo_overlay_state m;imu_pulse_io io;imu_gyro_device d;unsigned i;uint16_t row[2078];imu_gyro_device_input(&m,&io,&d,(enum rtb_mission)mission,(enum imu_gyro_clock_mode)mode,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=io.latch.interrupt_enabled;machine.InputChannel[7]=machine.OutputChannel7=io.latch.superbank;machine.InputChannel[011]=io.channel11;machine.InputChannel[012]=io.channel12;machine.InputChannel[014]=io.channel14;GyroTimer=d.phase;GyroCount=d.pending;
#ifdef GYRO_TIMING_SIMULATED
 OldChannel14=d.selection;
#else
 OldChannel14=(unsigned)d.selection<<6;
#endif
 ImuChannel14=0;ImuCduCount=0;observed_count=0;for(i=0;i<18u;++i)observed[i]=(agc_word)(010000u+i);machine.ScalerCounter=0;if(agc_engine(&machine))return 2;if(!machine.PendFlag||get(5)!=m.words[5]){fprintf(stderr,"unexpected CPU instruction effect\n");return 2;}row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[2]=(uint16_t)machine.AllowInterrupt;row[3]=(uint16_t)machine.InputChannel[7];row[4]=(uint16_t)machine.InputChannel[011];row[5]=(uint16_t)machine.InputChannel[012];row[6]=(uint16_t)machine.InputChannel[014];row[7]=(uint16_t)observed_count;row[8]=(uint16_t)GyroTimer;row[9]=(uint16_t)(GyroTimer>>16);row[10]=(uint16_t)GyroCount;
#ifdef GYRO_TIMING_SIMULATED
 row[11]=(uint16_t)OldChannel14;
#else
 row[11]=(uint16_t)(OldChannel14>>6);
#endif
 for(i=0;i<18u;++i)row[i+12u]=observed[i];for(i=0;i<2048u;++i)row[i+30u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
