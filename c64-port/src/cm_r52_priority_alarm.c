/* SPDX-License-Identifier: MIT; explicit actual P51 priority-alarm caller. */
#include "cm_r52_priority_alarm.h"
#include "cm_interbank_complete.h"
#include "cm_priolarm.h"
#include "cm_priority_display.h"
#include "cm_basic_finish.h"
enum exec_status cm_r52_priority_alarm_step(apollo_overlay_state *s,cm_native_io *io,unsigned group,exec_transfer *next){agc_word *w;
#if !defined(CM_R52_ADMIT_ONLY) && !defined(CM_R52_FINISH_ONLY)
 enum cm_priolarm_result alarm;
#endif
 if(!s||!io||group>6u||!next)return EXEC_OUT_OF_SCOPE;
 w=s->words;(void)w;
#if !defined(CM_R52_ADMIT_ONLY) && !defined(CM_R52_FINISH_ONLY)
 if(group==0u){if(w[5]!=02174u||w[4]!=030000u||w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(030000u|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
  w[0]=0404;w[2]=02176;w[5]=04662;if(!cm_interbank_complete(s,io,CM_IC_BANKCALL,021766,0))return EXEC_OUT_OF_SCOPE;return EXEC_OK;
 }
 if(group==1u){alarm=cm_priolarm(s,io);return alarm==CM_PRIOLARM_OUT_OF_SCOPE?EXEC_OUT_OF_SCOPE:EXEC_OK;}
 if(group==2u)return cm_priority_display_step(s,io,0);
 if(group==4u)return cm_priority_display_step(s,io,2);
 if(group==5u){if(w[5]!=02202u||w[4]!=030000u||w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(030000u|(w[3]>>8)))return EXEC_OUT_OF_SCOPE;w[5]=05217;w[1]=agc_extend(agc_correct(w[1]));return EXEC_OK;}
#endif
#if !defined(CM_R52_CALLER_ONLY) && !defined(CM_R52_FINISH_ONLY)
 if(group==3u)return cm_priority_display_step(s,io,1);
#endif
#if !defined(CM_R52_CALLER_ONLY) && !defined(CM_R52_ADMIT_ONLY)
 if(group==6u)return cm_basic_finish(s,io,next);
#endif
 return EXEC_OUT_OF_SCOPE;
}
