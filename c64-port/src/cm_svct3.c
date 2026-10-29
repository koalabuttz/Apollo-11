/* SPDX-License-Identifier: MIT */
#include "cm_svct3.h"
enum cm_svct3_exit cm_svct3_phase(apollo_overlay_state *s,cm_native_io *io,uint8_t phase){enum agc_ccs_class flag,imu;
 if(s==0||io==0||phase>2u||s->words[6]!=004003u||s->words[3]!=01400u||s->words[4]!=004000u||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return CM_SVCT3_INVALID;
 if(phase==1u){s->words[5]=05324;return CM_SVCT3_TASKOVER;}
 if(phase==2u)s->words[2]=07722;
 flag=agc_classify(s->words[0076]);s->words[0]=agc_diminished_abs(s->words[0076]);
 if(flag==AGC_POSITIVE||flag==AGC_PLUS_ZERO){s->words[5]=05324;return CM_SVCT3_TASKOVER;}
 imu=agc_classify(s->words[01322]);s->words[0]=agc_diminished_abs(s->words[01322]);
 if(imu==AGC_PLUS_ZERO){s->words[0]=035000;s->words[2]=05321;s->words[5]=05134;return CM_SVCT3_NOVAC;}
 s->words[2]=07720;s->words[5]=05267;return CM_SVCT3_FIXDELAY;
}
