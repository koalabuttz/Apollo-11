/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_time_consumer.h"
#include "dsky_decimal_load.h"
#include "dsky_decimal_select.h"
#include "dsky_hours_output.h"
#include "dsky_minutes_output.h"
#include "dsky_two_integer.h"
enum p66_bias_status dsky_decimal_time_consumer(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,uint8_t stage,enum dsky_decimal_consumer_route *route)
{
 enum p66_bias_status status=P66_BIAS_OUT_OF_SCOPE;enum dsky_decimal_consumer_route result=DSKY_DECIMAL_NEXT;unsigned i;
 if(!s||!io||!route||(unsigned)mission>1u||stage>21u)return status;
#if defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE)
 if(stage!=DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE)return status;
#endif
 for(i=3;i<2048u;++i)if(s->words[i]>AGC_MASK)return status;
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE==0
 if(stage==0u)status=dsky_decimal_load(s,io,mission);
#endif
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE==1
 if(stage==1u){agc_word ignored;status=dsky_decimal_select(s,io,mission,&ignored);}
#endif
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || (DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE>=2 && DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE<=9)
 if(stage>=2u&&stage<=9u){status=dsky_hours_output(s,io,mission,(enum dsky_hours_stage)(stage-2u));if(status==P66_BIAS_OK&&stage==9u)result=DSKY_DECIMAL_ENTRET;}
#endif
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || (DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE>=10 && DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE<=15)
 if(stage>=10u&&stage<=15u)status=dsky_minutes_output(s,io,mission,(enum dsky_minutes_stage)(stage-10u));
#endif
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || (DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE>=16 && DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE<=20)
 if(stage>=16u&&stage<=20u)status=dsky_two_integer(s,io,mission,(enum dsky_two_integer_stage)(stage-16u));
#endif
#if !defined(DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_TIME_CONSUMER_ONLY_STAGE==21
 if(stage==21u){uint16_t *w=s->words;agc_word count=w[0117];
  if(w[5]!=02570u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]||count>2u||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u)return status;
  /* This source CCS/TS/TC does not execute SWRETURN or touch BUF2. */
  w[0]=count?(agc_word)(count-1u):0u;if(count){w[0117]=(agc_word)(count-1u);w[2]=02575;w[5]=02537;}else{w[2]=02573;w[5]=0136;result=DSKY_DECIMAL_ENTRET;}status=P66_BIAS_OK;
 }
#endif
 if(status==P66_BIAS_OK)*route=result;
 return status;
}
