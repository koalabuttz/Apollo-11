/* SPDX-License-Identifier: GPL-2.0-or-later
 * Boundary-only original source probes: manufactured negative post-VSQ input,
 * not a claim that a valid flight vector produces a negative length square. */
#define main original_reference_main
#include "reference_runner.c"
#undef main
static agc_t pristine;
int main(int argc,char **argv){unsigned mode,n;if(argc!=2||agc_load_binfile(&pristine,argv[1]))return 2;InhibitAlarms=1;for(mode=0;mode<2u;++mode)for(n=0;n<256u;++n){unsigned cycles,pc=0;machine=pristine;set(RegFB,0);set(RegEB,03400);set(RegBB,7);set(RegZ,mode?03202:03031);set(RegQ,012345);set(RegL,0);set(0120,0401);set(0166,0401);set(0163,077776);set(0154,077777u^(n+2u));set(0155,(n*19u)&077777u);set(0156,(n*23u)&077777u);set(0121,0);set(0141,012345);set(0164,mode?03416:03374);set(0165,064007);set(023,0);set(0115,040000);machine.AllowInterrupt=(int)(n&1u);
for(cycles=0;cycles<1000;++cycles){pc=get(RegZ);if(!machine.PendFlag&&!machine.ExtraDelay&&pc==03402)break;machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
if(cycles==1000||get(RegFB)||get(RegEB)!=03400||get(RegBB)!=7||get(RegQ)!=(mode?03210u:03043u)||get(0164)!=(mode?03416u:03374u)||get(0165)!=064007||get(023)||get(0115)!=040000||machine.AllowInterrupt!=(int)(n&1u))return 2;
printf("%u %u FB%o EB%o BB%o PC%o Q%o LOC%o BANKSET%o EDOP%o INTBIT15%o IRQ%u\n",mode,n,get(RegFB),get(RegEB),get(RegBB),get(RegZ),get(RegQ),get(0164),get(0165),get(023),get(0115),machine.AllowInterrupt);}
return 0;}
