/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_consumer.h"
#include "dsky_decimal_load.h"
#include "dsky_decimal_select.h"
#include "dsky_scalar_output.h"
#include "dsky_decimal_row.h"
#include "dsky_scale.h"
#include "cm_scale.h"
#include "dsky_decimal_raw.h"
enum p66_bias_status dsky_decimal_consumer(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_decimal_consumer_stage stage,agc_word channel33,enum dsky_decimal_consumer_route *route)
{
 enum p66_bias_status status=P66_BIAS_OUT_OF_SCOPE;enum dsky_decimal_consumer_route result=DSKY_DECIMAL_NEXT;agc_word ignored=0;unsigned i=0;
 (void)ignored;(void)i;
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==2
 static const agc_word scalar_pc[2][6]={{02615,02677,02704,02623,02635,0},{02516,02603,02610,02524,0,02653}};
#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==3
 static const agc_word dp_pc[2][3]={{02710,02715,02717},{02614,02621,02623}};
#endif
 if(!s||!io||!route||(unsigned)mission>1u||(unsigned)stage>6u||channel33>AGC_MASK||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 switch(stage){

#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==0
 case DSKY_DECIMAL_LOAD:status=dsky_decimal_load(s,io,mission);break;

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==1
 case DSKY_DECIMAL_SELECT:status=dsky_decimal_select(s,io,mission,&ignored);break;

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==2
 case DSKY_DECIMAL_SCALAR:
  for(i=0;i<6u;++i)if(s->words[5]==scalar_pc[(unsigned)mission][i]&&scalar_pc[(unsigned)mission][i]!=0){status=dsky_scalar_output(s,io,mission,(enum dsky_scalar_output_entry)i,channel33);break;}
  break;

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==3
 case DSKY_DECIMAL_DP:
  for(i=0;i<3u;++i)if(s->words[5]==dp_pc[(unsigned)mission][i]){
   /* Selected SUPER does not change erasable READLO or native scale math.
    * Full original composition proof qualifies the retained actual values. */
   p66_bias_io math;math.superbank=0100;math.interrupt_enabled=io->interrupt_enabled;
   if(mission==DSKY_NV_LM)status=dsky_scale_output(s,&math,(enum dsky_scale_entry)i);
   else{cm_native_io cm={0};cm.superbank=0100;cm.interrupt_enabled=io->interrupt_enabled;status=(unsigned)cm_scale_output(s,&cm,(enum dsky_scale_entry)i)==(unsigned)EXEC_OK?P66_BIAS_OK:P66_BIAS_OUT_OF_SCOPE;math.interrupt_enabled=cm.interrupt_enabled;}
   if(status==P66_BIAS_OK)io->interrupt_enabled=math.interrupt_enabled;
   break;
  }
  break;

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==4
 case DSKY_DECIMAL_CALL:status=dsky_decimal_row(s,io,mission,DSKY_DECIMAL_ROW_CALL);break;

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==5
 case DSKY_DECIMAL_FORMAT:{enum dsky_decimal_general_route out;status=dsky_decimal_raw(s,io,mission,1,&out);if(status==P66_BIAS_OK&&out==DSKY_DECIMAL_CCSHOLE)result=DSKY_DECIMAL_RECOVERY;break;}

#endif
#if !defined(DSKY_DECIMAL_CONSUMER_ONLY_STAGE) || DSKY_DECIMAL_CONSUMER_ONLY_STAGE==6
 case DSKY_DECIMAL_RETURN:status=dsky_decimal_row(s,io,mission,DSKY_DECIMAL_ROW_RETURN);if(status==P66_BIAS_OK&&s->words[5]==0136u)result=DSKY_DECIMAL_ENTRET;break;
#endif
 default:break;
 }
 if(status==P66_BIAS_OK)*route=result;
 return status;
}
