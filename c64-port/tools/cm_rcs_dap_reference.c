/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle runner: unchanged Comanche055 RCS-CSM_DIGITAL_AUTOPILOT
 * in the pinned yaAGC, from each entry to the first transfer out of the
 * file. NEVER part of the native port. NEEDLER, NEEDLE11, NEEDLES, ZEROJET
 * and STICKCHK also stop at the caller's return, where their TC Q goes.
 * Once NEEDLES has set the IMU CDU drive bits (channel 14 13,14,15),
 * yaAGC's coarse-align model empties CDUXCMD-CDUZCMD (050-052) on the next
 * step; that is the simulator standing in for the CDU hardware, so the
 * runner keeps those three words as the program left them. */
#define main cm_reference_main
#include "cm_executive_reference.c"
#undef main
#include "cm_rcs_dap_cases.h"
#define LIMIT 4000u
static agc_t baseline;
static unsigned caller,cmd,held[3];
static int frontier(unsigned z){static const unsigned f[]={05333u,04740u,05134u,02042u,02054u,03441u,03757u,03250u};unsigned i;if(caller&&z==caller)return 1;for(i=0;i<sizeof f/sizeof f[0];++i)if(z==f[i])return 1;return 0;}
int main(int argc,char **argv){uint32_t n,limit=CM_RCS_DAP_CASES;if(argc<2)return 2;if(argc>2)limit=(uint32_t)strtoul(argv[2],NULL,0);memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;baseline=machine;InhibitAlarms=1;
for(n=0;n<limit;++n){agc_image m;cm_rcs_dap_io io;unsigned i,cycles;uint16_t row[CM_RCS_DAP_ROW_WORDS];cm_rcs_dap_input(&m,&io,n);machine=baseline;for(i=0;i<2048u;++i)*memory(i)=(int16_t)m.words[i];machine.AllowInterrupt=0;machine.InputChannel[014]=io.channel14;caller=(m.words[5]==02420u||m.words[5]==02443u||m.words[5]==02476u||m.words[5]==02632u||m.words[5]==03130u)?m.words[2]:0u;cmd=0;
 machine.InputChannel[012]=io.channel12;machine.InputChannel[013]=io.channel13;machine.InputChannel[031]=io.channel31;
 for(cycles=0;cycles<LIMIT;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&frontier(get(5)))break;if(get(5)==02533u&&!cmd){cmd=1;for(i=0;i<3u;++i)held[i]=get(050+i);}machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
 if(cycles==LIMIT)return 3;
 if(cmd)for(i=0;i<3u;++i)set(050+i,held[i]);
 memset(row,0,sizeof row);row[0]=(uint16_t)n;row[1]=(uint16_t)(n>>16);row[3]=(uint16_t)(machine.InputChannel[012]&077777);row[4]=(uint16_t)(machine.InputChannel[013]&077777);row[5]=(uint16_t)(machine.InputChannel[014]&077777);
 for(i=0;i<2048u;++i)row[i+6u]=(uint16_t)((unsigned)*memory(i)&(i<2u?0177777u:077777u));
 if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}
return 0;}
