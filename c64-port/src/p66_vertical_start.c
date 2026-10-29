/* SPDX-License-Identifier: MIT
 * Source-defined VRTSTART/STOPRATE/DOWNFLAG/P66VERT/P66VERTA composition.
 * Luminary099 guidance174..185,897..920,941..947 and called basic services.
 * Fixed native control path; no AGC opcode execution or memory-bank emulator. */
#include "p66_vertical_start.h"
#include "p66_initialization.h"
#include "p66_startup.h"
static const uint16_t controls[]={03243,03244,03245,03240,03241,03242,01277,01300,01301};
static void downflag(uint16_t *w,uint8_t index,agc_word mask,agc_word return_address) {
 agc_word previous=w[074+index];
 w[061]=index;w[062]=(agc_word)(index==13 ? 6 : 9);
 w[063]=previous;w[074+index]=(agc_word)(previous&agc_negate(mask));
 w[0]=agc_extend(w[074+index]);w[1]=return_address;w[2]=2;
}
static void phases(phase_state *p,uint16_t *w,uint8_t read) {
 uint8_t i;
 for(i=0;i<6;++i){
  if(read){p->records.complement[i]=w[0752+2u*i];p->records.phase[i]=w[0753+2u*i];p->records.address[i].location=w[01436+2u*i];p->records.address[i].bank=w[01437+2u*i];p->records.priority[i]=w[01054+2u*i];p->tbase[i]=w[01053+2u*i];}
  else{w[0752+2u*i]=p->records.complement[i];w[0753+2u*i]=p->records.phase[i];w[01436+2u*i]=p->records.address[i].location;w[01437+2u*i]=p->records.address[i].bank;w[01054+2u*i]=p->records.priority[i];w[01053+2u*i]=p->tbase[i];}
 }
 for(i=0;i<2;++i){if(read){p->longbase[i]=w[01151+i];p->time[i]=w[024+i];}else{w[01151+i]=p->longbase[i];w[024+i]=p->time[i];}}
 for(i=0;i<PH_SCRATCH_COUNT;++i){uint16_t a=(uint16_t)(i<6 ? 061+i : 070+i-6);if(read)p->scratch[i]=w[a];else w[a]=p->scratch[i];}
}
enum wl_status p66_vertical_start(apollo_overlay_state *image,p66_bias_io *io) {
 p66_basic_initialization basic={0};phase_state p;waitlist wait;uint16_t *w=image->words;uint8_t i;enum wl_status status;
 if(w[0]!=0 || w[3]!=03400 || w[4]!=062000 || w[6]!=062007 || w[026]>037777 ||
    io->superbank>060 || (io->superbank&017u)!=0 || io->interrupt_enabled>1)return WL_OUT_OF_SCOPE;
 w[03647]=0;w[03621]=2;w[01351]=2;
 /* BANKCALL preserves A/L via BUF2, invokes actual STOPRATE and restores FB.
  * SWRETURN retains the called bank in BUF2+1, with caller return in BUF2. */
 for(i=0;i<9;++i)basic.control[i]=w[controls[i]];
 basic.bbank=040007;basic.superbank=io->superbank;basic.interrupt_enabled=io->interrupt_enabled;basic.q=04631;
 (void)p66_stoprate(&basic);
 for(i=0;i<9;++i)w[controls[i]]=basic.control[i];
 w[0133]=02544;w[0134]=040000;w[0]=basic.a;w[1]=basic.l;w[2]=0134;
 downflag(w,13,0400,02546);downflag(w,6,040,02550);io->interrupt_enabled=1;
 /* CCS WCHVERT +0 selects P66. POSTJUMP transfers to source bank32. */
 w[0]=0;w[2]=01270;w[4]=064000;w[6]=064007;
 phases(&p,w,1);p.a=w[0];p.l=w[1];p.q=w[2];p.bbank=w[6];p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;
 wait.time3=w[026];wait.ruptagn=w[0734];wait.alarm=0;
 for(i=0;i<8;++i)wait.interval[i]=w[01400+i];
 for(i=0;i<9;++i){wait.task[i].location=w[01410+2u*i];wait.task[i].bank=w[01411+2u*i];}
 status=p66_verta(&p,&wait);
 phases(&p,w,0);w[0]=p.a;w[1]=p.l;w[2]=p.q;w[6]=p.bbank;w[3]=(uint16_t)((p.bbank&7u)<<8);w[4]=(uint16_t)(p.bbank&076000);
 w[5]=status==WL_OK ? 03275 : 05716;w[7]=0;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;
 w[026]=wait.time3;w[0734]=wait.ruptagn;
 for(i=0;i<8;++i)w[01400+i]=wait.interval[i];
 for(i=0;i<9;++i){w[01410+2u*i]=wait.task[i].location;w[01411+2u*i]=wait.task[i].bank;}
 return status;
}
