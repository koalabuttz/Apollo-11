/* SPDX-License-Identifier: MIT
 * Native original DSPSIGN/+ON/-ON, DSPRND, SHORTMP, TRACE1/DSPIN composition. */
#include "dsky_decimal_raw.h"
#include "dsky_pinball.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t a){agc_dp d;d.high=w[a];d.low=w[a+1u];agc_double_add(r,&d);w[a]=d.high;w[a+1u]=d.low;}
static void shortmp(uint16_t *w){agc_register_pair r;agc_word minor=w[0155],major=w[0154];w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(minor),10);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(major),10);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;}
static uint8_t pin(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_pinball_entry entry)
{
    uint16_t *w=s->words;agc_word relative=entry==DSKY_DSPIN?(agc_word)(w[0143]>>1):w[0];p66_bias_io arithmetic;
    if(w[01023u+relative]==0){
        w[0114]=w[2];w[0141]=relative;
        if(entry==DSKY_DSPIN){w[021]=relative;if(w[0143]&1u){w[0124]=(agc_word)(w[0124]<<5);w[0143]=1;}else w[0143]=0;}
        else w[0143]=2;
        w[0]=0;w[2]=mission==DSKY_NV_LM?03354u:03261u;w[5]=mission==DSKY_NV_LM?05705u:05747u;io->interrupt_enabled=0;return 0;
    }
    /* No SUPER writes occur in DSPIN arithmetic. Independent whole-image
     * source qualification below includes all four selecting SUPER values. */
    arithmetic.superbank=0100;arithmetic.interrupt_enabled=io->interrupt_enabled;
    (void)dsky_pinball_write(s,&arithmetic,entry);io->interrupt_enabled=arithmetic.interrupt_enabled;return 1;
}
enum p66_bias_status dsky_decimal_raw(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,uint8_t rounded,enum dsky_decimal_general_route *route){
 /* SHORTMP major product is at most9 plus two unit DAS carries: [-11,11].
  * TRACE1 INDEX reads RELTAB including its fixed neighbours as DATA only.
  * These mission-specific low5 literals retain even noncanonical raw signs;
  * no instruction word is decoded or executed. */
 static const uint16_t codes[2][23]={{29,3,21,6,3,0,7,0,0,6,2,21,3,25,27,15,30,28,19,29,31,0,0},{23,16,23,16,18,16,6,31,6,11,6,21,3,25,27,15,30,28,19,29,31,0,0}};
 uint16_t *w;agc_word caller,count,minus,plus;uint8_t i,negative;
 if(s==0||io==0||route==0||(unsigned)mission>1u||rounded>1u)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;caller=w[2];count=w[0777];negative=(uint8_t)((w[0154]&040000u)!=0);
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || (io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)) || io->interrupt_enabled>1 || rounded>1 || caller<0100 || caller>07777 || (count!=14 && count!=9 && count!=4))return P66_BIAS_OUT_OF_SCOPE;
 if(w[7]!=0||w[5]!=(mission==DSKY_NV_LM?(rounded?03226u:03262u):(rounded?03133u:03167u)))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 w[0115]=caller;w[0144]=mission==DSKY_NV_LM?(rounded?03231u:03265u):(rounded?03136u:03172u);
 minus=count==14 ? 5 : count==9 ? 3 : 0;plus=(uint16_t)(minus+1u);
 if(negative!=0){w[0154]=agc_negate(w[0154]);}
 /* +ON/-ON derives INREL2/3/4 then toggles both actual sign relays. */
 w[0137]=count==14 ? 2 : count==9 ? 3 : 4;w[1]=mission==DSKY_NV_LM?(negative?03210u:03214u):(negative?03115u:03121u);
 w[0123]=0;w[0122]=negative!=0?minus:plus;w[0124]=0;w[0]=negative!=0 ? plus : minus;w[2]=mission==DSKY_NV_LM?02426u:02327u;if(!pin(s,io,mission,DSKY_11DSPIN)){*route=DSKY_DECIMAL_CCSHOLE;return P66_BIAS_OK;}
 w[0124]=02000;w[0122]=02000;w[0]=negative!=0 ? minus : plus;w[2]=mission==DSKY_NV_LM?02432u:02333u;if(!pin(s,io,mission,DSKY_11DSPIN)){*route=DSKY_DECIMAL_CCSHOLE;return P66_BIAS_OK;}
 if(negative!=0)w[0155]=agc_negate(w[0155]);
 if(rounded!=0){agc_register_pair r={0,02476};accumulate(&r,w,0154);if(r.a!=0 && r.a!=0177777){w[0154]=037777;w[0155]=037777;}w[0]=r.a;w[1]=r.l;}
 for(i=0;i<5;++i){uint16_t code;w[0137]=(uint16_t)(4u-i);shortmp(w);code=codes[(unsigned)mission][(unsigned)(agc_to_int(w[0154])+11)];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;
  w[0124]=code;w[0143]=(uint16_t)(count-i);w[0777]=(uint16_t)(count-i==0 ? 0 : count-i-1u);w[0]=agc_diminished_abs((uint16_t)(count-i));w[2]=mission==DSKY_NV_LM?03253u:03160u;if(!pin(s,io,mission,DSKY_DSPIN)){*route=DSKY_DECIMAL_CCSHOLE;return P66_BIAS_OK;}
 }
 w[0]=0177754;w[1]=0;w[2]=0116;w[5]=caller;w[0777]=077754;w[7]=0;*route=DSKY_DECIMAL_FINISHED;return P66_BIAS_OK;
}
