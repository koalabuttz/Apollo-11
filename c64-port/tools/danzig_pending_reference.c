/* SPDX-License-Identifier: GPL-2.0-or-later; host-only yaAGC oracle for the pending-DANZIG transfer point. */
#define main admission_reference_main
#include "cm_executive_reference.c"
#undef main
/* argv: rope, NEWOPS, DANZIG (octal). stdin: one physical pair-word address
 * per line (an RTB-left pair). Each pair runs from NEWOPS with LOC at the
 * pair word, BANKSET/BBANK/FB on its bank and an otherwise zero image, until
 * Z first reaches DANZIG with EDOP nonzero (printed: pair, LOC, BANKSET,
 * EDOP, FB) or 200000 cycles pass (printed as not reached). */
int main(int argc,char **argv){unsigned newops,danzig,phys,bank,off,cycles,bs,ch7;agc_t base;
 if(argc!=4)return 2;memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1]))return 2;base=machine;InhibitAlarms=1;
 newops=(unsigned)strtoul(argv[2],0,8);danzig=(unsigned)strtoul(argv[3],0,8);
 while(scanf("%u",&phys)==1){
  machine=base;bank=phys/1024u;off=phys%1024u;ch7=0;
  if(bank>=040u){ch7=0100u;bs=((bank-010u)<<10)|0100u;}else if(bank>=030u){ch7=0060u;bs=(bank<<10)|0060u;}else bs=bank<<10;
  machine.InputChannel[7]=machine.OutputChannel7=(int16_t)ch7;
  set(0164,(agc_word)(02000u+off));set(0165,(agc_word)bs);set(4,(agc_word)(bs&076000u));set(6,(agc_word)bs);set(5,(agc_word)newops);
  for(cycles=0;cycles<200000u;++cycles){
   if(cycles&&!machine.PendFlag&&!machine.ExtraDelay&&!machine.IndexValue&&!machine.ExtraCode&&get(5)==danzig&&get(023)!=0u){
    printf("%u 1 %u %u %u %u\n",phys,get(0164),get(0165),get(023),get(4));break;}
   machine.ScalerCounter=0;if(agc_engine(&machine))return 2;}
  if(cycles==200000u)printf("%u 0\n",phys);
 }
 return 0;}
