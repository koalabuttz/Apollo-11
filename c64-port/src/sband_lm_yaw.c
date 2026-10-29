/* SPDX-License-Identifier: MIT */
#include "sband_lm_yaw.h"
#include "sband_vector.h"
#include "sband_vector_linear.h"
#include "sband_shift.h"
#include "sband_dot.h"
#include "sband_length.h"
#include "sband_trig.h"
#include "interpreted_sign.h"
#include "inverse_trig.h"
static const uint8_t pos[6]={0,1,3,4,5,6};
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
static enum sband_lm_yaw_status inverse(apollo_overlay_state *s){enum inverse_trig_status t;t=inverse_trig_prepare(s,INVERSE_TRIG_LM,1);if(t==INVERSE_TRIG_OK)t=inverse_trig_root(s,INVERSE_TRIG_LM);if(t==INVERSE_TRIG_OK)t=inverse_trig_polynomial(s,INVERSE_TRIG_LM);if(t==INVERSE_TRIG_OK)t=inverse_trig_finish(s,INVERSE_TRIG_LM);if(t==INVERSE_TRIG_ALARM_1301)return SBAND_LM_YAW_ALARM1301;if(t==INVERSE_TRIG_ALARM_1302)return SBAND_LM_YAW_ALARM1302;return SBAND_LM_YAW_OK;}
enum sband_lm_yaw_status sband_lm_yaw(apollo_overlay_state *s,p66_bias_io *io){static const agc_word uz[6]={0,0,0,0,020000,0},ux[6]={020000,0,0,0,0,0};unsigned i,base,stack;agc_word eb,low,xminor,ymajor,yminor;agc_dp sign_operand;enum sband_lm_yaw_status t;
 if(!s||!io)return SBAND_LM_YAW_UNSUPPORTED;
 base=s->words[0120];eb=s->words[3];
 if(s->words[5]!=06070u||s->words[0164]!=03723u||s->words[023]||s->words[0163]||s->words[4]!=064000u||eb>03400u||(eb&0377u)||s->words[6]!=(agc_word)(064000u|(eb>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=base+014u||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SBAND_LM_YAW_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return SBAND_LM_YAW_UNSUPPORTED;
 low=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+pos[i]]=s->words[base+i];s->words[1]=agc_extend(low);s->words[0163]=077776;
 bank(s,eb,064000);s->words[5]=07460;s->words[0116]=(agc_word)(base+6u);s->words[0164]=03725;s->words[020]=020030;s->words[023]=0;(void)sband_vector_cross(s,APOLLO_FIXED_LM);
 bank(s,eb,0);s->words[5]=02121;s->words[0164]=03726;s->words[020]=020000;s->words[023]=0;(void)sband_shift_left(s,APOLLO_FIXED_LM,1);
 low=s->words[0155];for(i=0;i<6u;++i)s->words[02204u+i]=s->words[0154u+pos[i]];s->words[0154]=s->words[02200];s->words[0155]=s->words[02201];s->words[0156]=0;s->words[0163]=0;s->words[1]=agc_extend(low);s->words[2]=3;
 bank(s,eb,0);s->words[5]=03530;s->words[0116]=01600;s->words[0164]=03731;s->words[020]=1;s->words[023]=016;s->words[0]=1;(void)sband_trig(s,APOLLO_FIXED_LM,0);
 bank(s,eb,046000);s->words[5]=07401;s->words[0116]=02513;s->words[0164]=03732;s->words[020]=020003;s->words[023]=0;(void)sband_vector_linear(s,APOLLO_FIXED_LM,SBAND_VXSC,uz);
 /* Actual vector PDDL places old stackXminor in MPAC.Ylow, old stackY
  * pair in MPAC.Z, and old stackZpair in A/L before scalar COS begins. */
 stack=base+014u;xminor=s->words[stack+1u];ymajor=s->words[stack+2u];yminor=s->words[stack+3u];low=s->words[stack+5u];for(i=0;i<6u;++i)s->words[stack+i]=s->words[0154u+pos[i]];s->words[0154]=s->words[02200];s->words[0155]=s->words[02201];s->words[0156]=0;s->words[0157]=0;s->words[0160]=xminor;s->words[0161]=ymajor;s->words[0162]=yminor;s->words[1]=agc_extend(low);s->words[0163]=0;s->words[0166]=(agc_word)(stack+6u);
 bank(s,eb,0);s->words[5]=03517;s->words[0116]=01600;s->words[0164]=03734;s->words[020]=040001;s->words[023]=0;s->words[0]=2;(void)sband_trig(s,APOLLO_FIXED_LM,1);
 bank(s,eb,046000);s->words[5]=07401;s->words[0116]=02517;s->words[0164]=03736;s->words[020]=020003;s->words[023]=0126;(void)sband_vector_linear(s,APOLLO_FIXED_LM,SBAND_VXSC,ux);
 s->words[0166]=(agc_word)stack;bank(s,eb,064000);s->words[5]=06747;s->words[0116]=(agc_word)stack;s->words[020]=020025;s->words[023]=0;(void)sband_vector_linear(s,APOLLO_FIXED_LM,SBAND_VSU,0);
 bank(s,02000,064000);s->words[5]=07331;s->words[0116]=01604;s->words[0164]=03740;s->words[020]=020027;s->words[023]=062;(void)sband_dot(s,APOLLO_FIXED_LM,0);
 s->words[stack]=s->words[0154];s->words[stack+1u]=s->words[0155];low=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+pos[i]]=s->words[02204u+i];s->words[1]=agc_extend(low);s->words[0163]=077776;s->words[0166]=(agc_word)(stack+2u);
 bank(s,eb,0);s->words[5]=03201;s->words[0164]=03742;s->words[020]=040005;s->words[023]=012;s->words[0116]=01604;if(sband_length(s,io,APOLLO_FIXED_LM)==SBAND_LENGTH_ABORT_1302)return SBAND_LM_YAW_ALARM1302;
 sign_operand.high=s->words[stack];sign_operand.low=s->words[stack+1u];s->words[0166]=(agc_word)stack;bank(s,eb,064000);s->words[5]=07655;s->words[0116]=(agc_word)stack;s->words[020]=020002;s->words[023]=0;(void)interpreted_sign(s,INTERPRETED_SIGN_LM,INTERPRETED_SIGN,&sign_operand);
 bank(s,eb,0);s->words[5]=03610;s->words[0164]=03743;s->words[0]=3;s->words[020]=2;s->words[023]=0;t=inverse(s);if(t!=SBAND_LM_YAW_OK)return t;
 low=s->words[02203];s->words[02202]=s->words[0154];s->words[02203]=s->words[0155];s->words[1]=agc_extend(low);s->words[2]=3;s->words[0]=0;s->words[0116]=01602;s->words[0164]=03745;s->words[5]=06070;bank(s,eb,064000);return SBAND_LM_YAW_OK;
}
