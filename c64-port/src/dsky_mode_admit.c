/* SPDX-License-Identifier: MIT */
#include "dsky_mode_admit.h"
static int domain(const apollo_overlay_state *s,const p66_bias_io *io,uint8_t cm){uint8_t i,cores=cm ? 7u : 8u;const uint16_t *w;
 if(!s||!io)return 0;
 w=s->words;
 if(w[5]!=(cm ? 02566u : 02636u)||w[7]||w[2]<0100u||w[2]>07777u||w[3]>03400u||(w[3]&0377u)||w[4]!=010000u||w[6]!=(uint16_t)(010000u|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||w[0167]==0u||w[0167]>=040000u||(w[067]!=077777u&&(w[067]>(cm ? 72u : 84u)||w[067]%12u)))return 0;
 for(i=1;i<cores;++i)if(w[0167u+12u*i]==0)return 0;
 for(i=0;i<5u;++i)if(w[0400u+i*44u]&&w[0400u+i*44u]!=(uint16_t)(0400u+i*44u))return 0;
 return 1;
}
static enum exec_status body(apollo_overlay_state *s,p66_bias_io *io,cm_native_io *cmio,uint8_t cm,enum dsky_mode_admit_route *route){uint16_t *w=s->words;exec_address job={cm ? 03441u : 03534u,060101};enum exec_status status;
 w[1]=agc_extend(agc_correct(w[1]));w[0154]=w[2];w[0]=030000;w[2]=cm ? 02573u : 02643u;w[5]=cm ? 05134u : 05072u;io->interrupt_enabled=0;
 if(cm){cmio->interrupt_enabled=0;status=cm_admission_complete(s,cmio,CM_ADMIT_NOVAC,job);io->superbank=cmio->superbank;io->interrupt_enabled=cmio->interrupt_enabled;}
 else status=lm_admission_image(s,io,LM_ADMIT_NOVAC,job);
 if(status==EXEC_NO_CORE){*route=DSKY_MODE_ADMIT_ALARM;return EXEC_OK;}
 if(status!=EXEC_OK)return status;
 io->interrupt_enabled=1;if(cm)cmio->interrupt_enabled=1;w[2]=0155;w[5]=w[0154];*route=DSKY_MODE_ADMIT_CALLER;return EXEC_OK;
}
enum p66_bias_status dsky_mode_admit(apollo_overlay_state *s,p66_bias_io *io,enum dsky_mode_admit_route *route){if(!route||!domain(s,io,0))return P66_BIAS_OUT_OF_SCOPE;return body(s,io,0,0,route)==EXEC_OK ? P66_BIAS_OK : P66_BIAS_OUT_OF_SCOPE;}
enum exec_status cm_mode_admit(apollo_overlay_state *s,cm_native_io *io,enum dsky_mode_admit_route *route){p66_bias_io p;if(!io||!route||io->executive_alarm)return EXEC_OUT_OF_SCOPE;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;if(!domain(s,&p,1))return EXEC_OUT_OF_SCOPE;return body(s,&p,io,1,route);}
