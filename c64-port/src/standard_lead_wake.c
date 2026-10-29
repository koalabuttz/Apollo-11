/* SPDX-License-Identifier: MIT */
#include "standard_lead_wake.h"
static int entry(const apollo_overlay_state *s,uint16_t pc){return s&&s->words[5]==pc&&s->words[4]==010000u&&s->words[072]>=0100u&&s->words[072]<=07777u;}
static void finish(apollo_overlay_state *s){s->words[5]=s->words[072];s->words[2]=073;}
enum exec_status standard_lead_wake_lm(apollo_overlay_state *s,p66_bias_io *io){agc_word oldpc,oldq;enum exec_status status;
 if(!io||io->interrupt_enabled!=0||!entry(s,02631))return EXEC_OUT_OF_SCOPE;
 oldpc=s->words[5];oldq=s->words[2];s->words[5]=05137;s->words[2]=02632;status=lm_basic_wake(s,io);
 if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND){s->words[5]=oldpc;s->words[2]=oldq;return status;}
 if(!standard_lead_stage(s,STANDARD_LEAD_LM,STANDARD_LEAD_WAKE_RETURN,0))return EXEC_OUT_OF_SCOPE;
 finish(s);return status;
}
enum exec_status standard_lead_wake_cm(apollo_overlay_state *s,cm_native_io *io){agc_word oldpc,oldq;enum exec_status status;
 if(!io||io->interrupt_enabled!=0||!entry(s,02606))return EXEC_OUT_OF_SCOPE;
 oldpc=s->words[5];oldq=s->words[2];s->words[5]=05201;s->words[2]=02607;status=cm_basic_wake(s,io);
 if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND){s->words[5]=oldpc;s->words[2]=oldq;return status;}
 if(!standard_lead_stage(s,STANDARD_LEAD_CM,STANDARD_LEAD_WAKE_RETURN,0))return EXEC_OUT_OF_SCOPE;
 finish(s);return status;
}
