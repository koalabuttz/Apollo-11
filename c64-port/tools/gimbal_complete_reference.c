/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only source oracle: input-only constructors, no native math linked. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
#include "gimbal_complete_cases.h"
static void emit(unsigned w){putchar((int)(w&255u));putchar((int)(w>>8));}
int main(int argc,char **argv){static agc_t pristine;static apollo_overlay_state s;static unsigned char seen[1024];
 gimbal_io io;unsigned n,i,cycles,first,stop,entry,trace;FILE *log;
 if(argc!=2 || agc_load_binfile(&pristine,argv[1]))return 2;
 first=strstr(argv[1],"Comanche055")!=0?5120u:0u;stop=first?06037:06070;entry=first?03032:03010;
 log=fopen(first?"build/gimbal-complete-cm-trace.txt":"build/gimbal-complete-lm-trace.txt","w");if(!log)return 2;
 for(n=first;n<first+5120u;++n){memcpy(&machine,&pristine,sizeof machine);gimbal_complete_prepare(&s,&io,(uint16_t)n);InhibitAlarms=1;
 for(i=0;i<2048u;++i)*memory(i)=(int16_t)s.words[i];
 machine.OutputChannel7=machine.InputChannel[7]=io.superbank;machine.AllowInterrupt=io.interrupt_enabled;
 for(cycles=0;cycles<30000u;++cycles){if(!machine.PendFlag && !machine.ExtraDelay && get(RegZ)==stop && get(0164)==entry)break;
 if((get(0165)&076000u)==044000u && get(0164)>=02000u && get(0164)<04000u)seen[get(0164)-02000u]=1;
 trace=get(RegZ);machine.ScalerCounter=0;if(agc_engine(&machine))return 2;
 if(n==first)for(i=0;i<2048u;++i){uint16_t actual=(uint16_t)((unsigned)*memory(i)&(i<=2?0177777u:077777u));
 if(actual!=s.words[i]){fprintf(log,"%05o word%04o %06o -> %06o\n",trace,i,s.words[i],actual);s.words[i]=actual;}}}
 if(cycles==30000){fprintf(stderr,"gimbal%u timeout pc%05o LOC%05o\n",n,get(RegZ),get(0164));return 2;}
 emit(n);emit(1);for(i=0;i<2048u;++i)emit((unsigned)*memory(i)&(i<=2?0177777u:077777u));
 emit((unsigned)machine.OutputChannel7);emit((unsigned)machine.AllowInterrupt);}
 fclose(log);log=fopen(first?"build/gimbal-complete-cm-loc.txt":"build/gimbal-complete-lm-loc.txt","w");if(!log)return 2;
 for(i=0;i<1024u;++i)if(seen[i])fprintf(log,"%04o\n",i+02000u);
 fclose(log);return ferror(stdout)?2:0;}
