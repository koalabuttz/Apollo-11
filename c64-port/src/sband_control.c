/* SPDX-License-Identifier: MIT */
#include "sband_control.h"
#include "sband_control_source.h"
#include "lm_interbank.h"
#include "cm_interbank_complete.h"
#include "service_routines.h"
static bool context(const apollo_overlay_state *s,const p66_bias_io *io){unsigned i;
 if(!s||!io||s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[4]!=064000u||s->words[6]!=(agc_word)(064000u|(s->words[3]>>8))||(io->superbank&~0160u)||!(io->superbank&0100u)||io->interrupt_enabled>1u)return false;
 for(i=3;i<2048u;++i)if(s->words[i]>AGC_MASK)return false;
 return true;
}
static bool bankcall(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission,const sband_control_bindings *b,agc_word q,agc_word cadr){
 s->words[2]=q;s->words[5]=b->bankcall;
 if(mission==APOLLO_FIXED_LM)return lm_interbank(s,io,LM_IB_BANKCALL,cadr,0);
 else {cm_native_io metadata={0};bool ok;metadata.superbank=io->superbank;metadata.interrupt_enabled=io->interrupt_enabled;ok=cm_interbank_complete(s,&metadata,CM_IC_BANKCALL,cadr,0);if(ok){io->superbank=metadata.superbank;io->interrupt_enabled=metadata.interrupt_enabled;}return ok;}
}
static void interpreter(apollo_overlay_state *s,p66_bias_io *io,const sband_control_bindings *b){
 agc_word oldloc=s->words[0164];s->words[2]=agc_extend(oldloc);s->words[0164]=(agc_word)(b->start+3u);s->words[0165]=s->words[6];s->words[0115]=(agc_word)(s->words[6]&040000u);s->words[0]=agc_extend(s->words[0115]);s->words[023]=0;s->words[5]=b->newops;io->interrupt_enabled=1;
}
bool sband_control(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission,enum sband_control_entry entry){const sband_control_bindings *b;agc_word pc;
 if((mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM)||(unsigned)entry>SBAND_CONTROL_REENTER||!context(s,io))return false;
 b=&sband_control_source[mission];
 switch(entry){case SBAND_CONTROL_START:pc=b->start;break;case SBAND_CONTROL_INTERPRETER:pc=(agc_word)(b->start+2u);break;case SBAND_CONTROL_DISPLAY:pc=b->display;break;case SBAND_CONTROL_AFTER_PRIORITY:if(mission!=APOLLO_FIXED_LM)return false;pc=b->after_priority;break;case SBAND_CONTROL_TERMINATE:pc=b->terminate;break;case SBAND_CONTROL_PROCEED:pc=b->proceed;break;case SBAND_CONTROL_RECYCLE:pc=b->recycle;break;case SBAND_CONTROL_BLANK:pc=b->blank;break;case SBAND_CONTROL_RESTART:pc=b->restart;break;default:pc=b->reenter;break;}
 if(s->words[5]!=pc)return false;
 s->words[1]=agc_extend(agc_correct(s->words[1]));
 if(entry==SBAND_CONTROL_START)return bankcall(s,io,mission,b,(agc_word)(b->start+1u),b->r02cadr);
 if(entry==SBAND_CONTROL_INTERPRETER||entry==SBAND_CONTROL_REENTER){interpreter(s,io,b);return true;}
 if(entry==SBAND_CONTROL_TERMINATE||entry==SBAND_CONTROL_PROCEED){s->words[2]=(agc_word)(pc+1u);s->words[5]=b->b5off;return service_routines_step(s,io,mission==APOLLO_FIXED_LM?SERVICE_ROUTINES_LM:SERVICE_ROUTINES_CM,SERVICE_B5OFF,0,0,0);}
 if(entry==SBAND_CONTROL_RECYCLE){s->words[2]=(agc_word)(pc+1u);s->words[5]=b->endofjob;return true;}
 if(entry==SBAND_CONTROL_BLANK){s->words[0]=4;s->words[2]=(agc_word)(pc+2u);s->words[5]=b->blanket;return true;}
 if(entry==SBAND_CONTROL_RESTART){if(mission==APOLLO_FIXED_LM){s->words[0]=b->prio4;s->words[2]=(agc_word)(pc+2u);s->words[5]=b->priochng;return true;}s->words[0]=1;return bankcall(s,io,mission,b,(agc_word)(pc+2u),b->delaycadr);}
 if(entry==SBAND_CONTROL_DISPLAY){s->words[0]=(agc_word)(s->words[01044]&020u);if(!s->words[0]){s->words[5]=b->endext;return true;}if(mission==APOLLO_FIXED_LM){s->words[0]=b->prio5;s->words[2]=(agc_word)(b->after_priority);s->words[5]=b->priochng;return true;}}
 s->words[0]=b->vn;return bankcall(s,io,mission,b,(agc_word)(b->terminate-1u),b->gomarkcadr);
}
