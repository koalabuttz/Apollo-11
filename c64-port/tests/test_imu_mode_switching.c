/* SPDX-License-Identifier: MIT */
/* imu-mode-switching: Luminary099 and Comanche055 IMU_MODE_SWITCHING_ROUTINES
 * as native routines, against digests frozen from the original programs;
 * every case stops, every routine and round-trip site is exercised, and each
 * refused input leaves the image, the interrupt state and the channels
 * unchanged. */
#include <stdio.h>
#include <string.h>
#include "imu_mode_switching_cases.h"
#include "imu_mode_switching_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static int sorted(const agc_routine_table *t){uint16_t i;for(i=1;i<t->count;++i)if(t->entries[i-1u].physical>=t->entries[i].physical)return 0;return 1;}
/* Entry case index of the Luminary (cm 0) or Comanche routine at bank, z. */
static uint16_t routine(unsigned cm,unsigned bank,agc_word z){uint16_t r,ph;
 if(cm){z=ims_comanche_address(bank,z);bank=bank==04u?07u:bank==037u?026u:bank;}
 ph=(uint16_t)(bank==02u?2048u+z-04000u:bank*1024u+z-02000u);
 for(r=0;r<IMS_ROUTINES;++r)if((r>=IMS_LM_ROUTINES)==(cm!=0u)&&ims_routine_physical(r)==ph)return r;
 return 0xffffu;
}
typedef void (*patch)(apollo_overlay_state *,ims_case *);
/* Runs entry case n with patch applied; 1 when refused with nothing changed. */
static int refused(uint16_t n,patch p,int no_hardware){apollo_overlay_state s,before;ims_case c;lm_imu_io lio,lio0;cm_native_io cio,cio0;
 agc_hardware hw;agc_native_program program={0,1,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_word sb;uint16_t steps;void *io;unsigned i;
 ims_input(&s,&c,n);p(&s,&c);before=s;
 {uint16_t *w=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *w;++i)w[i]=0;}
 memset(&lio,0,sizeof lio);
 sb=lio.control.superbank=cio.superbank=c.superbank;lio.control.interrupt_enabled=cio.interrupt_enabled=c.interrupt_enabled;
 lio.channel12=cio.channel12=c.channel[1];lio.channel13=cio.channel13=c.channel[2];lio.channel14=cio.channel14=c.channel[3];
 hw.channel11=c.channel[0];hw.in_interrupt=0;program.hardware=no_hardware?0:&hw;lio0=lio;cio0=cio;
 program.basic=c.cm?&cm_imu_mode_switching_routines:&lm_imu_mode_switching_routines;io=c.cm?(void *)&cio:(void *)&lio;
 return agc_transfer_run(&program,&s,io,&sb,0xfffeu,4,&steps)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&
  !memcmp(&lio,&lio0,sizeof lio)&&!memcmp(&cio,&cio0,sizeof cio)&&hw.channel11==c.channel[0];
}
static void q_zero(apollo_overlay_state *s,ims_case *c){(void)c;s->words[2]=0;}
static void q_fixed(apollo_overlay_state *s,ims_case *c){(void)c;s->words[2]=010000u;}
static void a_three(apollo_overlay_state *s,ims_case *c){(void)c;s->words[0]=3u;}
static void locctr(apollo_overlay_state *s,ims_case *c){(void)c;s->words[064]=01614u;}
static void ruptreg2(apollo_overlay_state *s,ims_case *c){(void)c;s->words[071]=3u;}
static void mpac5_low8(apollo_overlay_state *s,ims_case *c){(void)c;s->words[0161]=(agc_word)((s->words[0161]&~0377u)|0373u);}
static void mpac5_e0(apollo_overlay_state *s,ims_case *c){(void)c;s->words[0161]=3u;}
static void mpac4(apollo_overlay_state *s,ims_case *c){(void)c;s->words[0160]=0377u;}
static void mpac4_e0(apollo_overlay_state *s,ims_case *c){(void)c;s->words[3]=0;s->words[6]=s->words[4];s->words[0160]=2u;}
static void lgyro_big(apollo_overlay_state *s,ims_case *c){s->words[c->cm?01304u:01314u]=040000u;}
static void lgyro_e0(apollo_overlay_state *s,ims_case *c){s->words[c->cm?01304u:01314u]=4u;}
static void mpac2(apollo_overlay_state *s,ims_case *c){(void)c;s->words[0156]=7u;}
static void ch13(apollo_overlay_state *s,ims_case *c){(void)s;c->channel[2]|=04000u;}
static void none(apollo_overlay_state *s,ims_case *c){(void)s;(void)c;}
int main(void){apollo_overlay_state s;unsigned n,k,cm;uint16_t ie;agc_word ch[4];static unsigned reached[IMS_ROUTINES],trips[IMS_SITES];
 check(sorted(&lm_imu_mode_switching_routines)&&lm_imu_mode_switching_routines.count==IMS_LM_ROUTINES,"LM table",0);
 check(sorted(&cm_imu_mode_switching_routines)&&cm_imu_mode_switching_routines.count==IMS_CM_ROUTINES,"CM table",0);
 check(IMS_VECTORS==IMS_CASES,"vector count",IMS_VECTORS);
 check(ims_comanche_address(07u,IMS_GOODEND)==03550u&&ims_comanche_address(04u,IMS_R02BOTH)==03653u&&
  ims_comanche_address(037u,IMS_P06)==03660u&&ims_comanche_address(02u,IMS_ZEROICDU)==05521u,"Comanche addresses",0);
 for(n=0;n<IMS_CASES;++n){unsigned kind=ims_run(&s,(uint16_t)n,&ie,ch);
  check(kind==IMS_STOPPED,"stopped",n);
  check(ims_digest(&s,(uint16_t)n)==ims_expected[n],"digest",n);
  if(kind==IMS_STOPPED){if(n<IMS_ENTRY_CASES)++reached[n%IMS_ROUTINES];else ++trips[(n-IMS_TRIP_FIRST)%IMS_SITES];}
 }
 for(k=0;k<IMS_ROUTINES;++k)check(reached[k]==IMS_ENTRY_STATES,"routine",k);
 for(k=0;k<IMS_SITES;++k)check(trips[k]==IMS_SITE_STATES,"round trip site",k);
 /* Each refusal of include/imu_mode_switching.h, both missions. */
 for(cm=0;cm<2u;++cm){
  static const struct {unsigned bank;agc_word z;patch p;int no_hw;const char *what;} r[]={
   {02,IMS_ZEROICDU,q_zero,0,"ZEROICDU Q"},{07,IMS_SETCOARS,q_fixed,0,"SETCOARS Q"},{07,IMS_RNDREFDR,q_zero,0,"RNDREFDR Q"},
   {07,IMS_NOATTOFF,q_fixed,0,"NOATTOFF Q"},{07,IMS_BADEND,a_three,0,"BADEND A"},{07,IMS_GOODEND,a_three,0,"GOODEND A"},
   {07,03702u,locctr,0,"JOBWAKE return LOCCTR"},{07,03736u,ruptreg2,0,"MAKECADR return RUPTREG2"},
   {07,03337u,mpac5_low8,0,"WAITLIST return MPAC +5"},{07,03337u,mpac5_e0,0,"WAITLIST return EBANK 0"},
   {07,03357u,mpac4,0,"TPAGREE return MPAC +4"},{07,03357u,mpac4_e0,0,"TPAGREE return EBANK 0"},
   {07,IMS_STRTGYRO,lgyro_big,0,"STRTGYRO LGYRO"},{07,IMS_STRTGYRO,lgyro_e0,0,"STRTGYRO EBANK 0"},
   {037,03717u,mpac2,0,"FINETIME return MPAC +2"},{037,03676u,ch13,0,"PHASCHNG lead-in channel 13"},
   {037,IMS_POSTAND,ch13,0,"POSTAND channel 13"},{07,IMS_ENDIMU,none,1,"ENDIMU without hardware"},
   {07,IMS_IMUFINED,none,1,"IMUFINED without hardware"},{07,02777u,none,1,"SETISSW return without hardware"}};
  for(k=0;k<sizeof r/sizeof *r;++k){uint16_t i=routine(cm,r[k].bank,r[k].z);unsigned v;
   check(i!=0xffffu,r[k].what,cm);
   for(v=0;v<4u;++v)check(i!=0xffffu&&refused((uint16_t)(i+v*IMS_ROUTINES),r[k].p,r[k].no_hw),r[k].what,cm*10u+v);}
 }
 printf("imu-mode-switching: %u cases (%u entry, %u round trips); %u checks, %u failures\n",
  IMS_CASES,IMS_ENTRY_CASES,IMS_CASES-IMS_ENTRY_CASES,checks,failures);
 return failures!=0u;
}
