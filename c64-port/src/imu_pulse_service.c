/* SPDX-License-Identifier: MIT */
#include "imu_pulse.h"
#include "lm_waitlist_image.h"
#include "cm_waitlist_image.h"
#include <string.h>
static void exchange(uint16_t *w,unsigned at)
{
 agc_register a=w[0],l=w[1];w[0]=agc_extend(w[at]);w[1]=agc_extend(w[at+1u]);w[at]=agc_correct(a);w[at+1u]=agc_correct(l);
}
/* Source TPAGREE limit/carry chain; no instruction dispatch or return decoder. */
static void agree(uint16_t *w,agc_word returned)
{
 agc_register a=0;agc_word limit;int sign=0,overflow;unsigned i;
 for(i=0;i<3u;++i)if((sign=agc_to_int(w[0154+i]))!=0)break;
 if(!sign)w[0154]=w[0155]=w[0156]=0;
 else{
  limit=(agc_word)(sign>0?037777u:040000u);
  a=agc_add_register(agc_extend(limit),(agc_register)(sign>0?1u:0177776u));
  for(i=2;i>0;--i){a=agc_add_register(a,agc_extend(w[0154+i]));w[0154+i]=agc_correct(a);overflow=agc_overflow(a);a=(agc_register)(overflow?(overflow>0?1u:0177776u):0u);a=agc_add_register(a,agc_extend(limit));}
  a=agc_add_register(a,agc_extend(w[0154]));w[0154]=agc_correct(a);overflow=agc_overflow(a);if(overflow)a=(agc_register)(overflow>0?1u:0177776u);else w[0155]=agc_correct(a);
 }
 w[0]=a;w[1]=returned;w[2]=2;
}
enum imu_pulse_status imu_pulse_register(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 exec_address task;uint16_t *w=m->words;
 if((unsigned)mission>1u||w[5]!=(mission==RTB_LM?05203u:05245u)||w[2]!=(mission==RTB_LM?03335u:03220u)||w[4]!=016000u||(w[0]!=4&&w[0]!=1))return IMU_PULSE_OUT_OF_SCOPE;
 task.location=(agc_word)(mission==RTB_LM?03405u:03270u);task.bank=(agc_word)(mission==RTB_LM?016103u:016063u);
 if(mission==RTB_LM){if(lm_waitlist_image(m,&io->latch,LM_WAITLIST_ENTRY,task)!=WL_OK)return IMU_PULSE_OUT_OF_SCOPE;}
 else{cm_native_io cm;memset(&cm,0,sizeof cm);cm.superbank=io->latch.superbank;cm.interrupt_enabled=io->latch.interrupt_enabled;if(cm_waitlist_image(m,&cm,task)!=CM_WL_OK)return IMU_PULSE_OUT_OF_SCOPE;io->latch.superbank=cm.superbank;io->latch.interrupt_enabled=cm.interrupt_enabled;}
 return IMU_PULSE_WAITLIST;
}
enum imu_pulse_status imu_pulse_finish(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 uint16_t *w=m->words,command=w[0161],oldbank=w[3];unsigned component,base;
 if((unsigned)mission>1u||w[5]!=(mission==RTB_LM?03337u:03222u)||w[4]!=016000u||w[7]||command<02000u||command>03772u||(command&0377u)>0372u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[0133]<02000u||w[0133]>07777u||(w[0134]&~076000u)||io->latch.interrupt_enabled!=0)return IMU_PULSE_OUT_OF_SCOPE;
 w[3]=(agc_word)(command&03400u);w[6]=(agc_word)(w[4]|(w[3]>>8));w[0161]=oldbank;w[mission==RTB_LM?01314u:01304u]=command;w[061]=(agc_word)(command&0377u);base=command;
 for(component=3;component>0;--component){unsigned index=component-1u,at=base+index*2u;w[0157]=(agc_word)index;w[0160]=(agc_word)((command&0377u)+index*2u);w[0]=agc_extend(w[at]);w[1]=agc_extend(w[at+1u]);exchange(w,0154);agree(w,(agc_word)(mission==RTB_LM?03357u:03242u));exchange(w,0154);exchange(w,at);}
 w[0]=oldbank;w[3]=oldbank;w[4]=w[0134];w[6]=(agc_word)(w[4]|(w[3]>>8));w[0134]=016000;w[2]=0134;w[5]=w[0133];w[1]=agc_extend(agc_correct(w[1]));io->latch.interrupt_enabled=1;
 return IMU_PULSE_CAGED_RETURN;
}
