/* SPDX-License-Identifier: MIT */
#include "dsky_nv_consumer.h"
enum p66_bias_status dsky_nv_consumer_step(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,enum dsky_nv_mission mission,enum dsky_nv_consumer_stage stage)
{
    enum dsky_nv_route route;enum dsky_nv_continuation_entry continuation;
    if(s==0||io==0||channel11==0||*channel11>AGC_MASK||(unsigned)mission>1u||(unsigned)stage>3u)return P66_BIAS_OUT_OF_SCOPE;
    if(stage==DSKY_NV_CONSUMER_CALL||stage==DSKY_NV_CONSUMER_MONITOR)return dsky_nv_admit(s,io,mission,stage==DSKY_NV_CONSUMER_CALL?DSKY_NV_NVSUB:DSKY_NV_NVMONOPT,&route);
    if(stage==DSKY_NV_CONSUMER_SERVICE){
        switch(s->words[5]){
        case 02337:return dsky_nv_update(s,io,mission,DSKY_NV_VERB_STORE);
        case 02317:return dsky_nv_update(s,io,mission,DSKY_NV_NOUN_STORE);
        case 03516:return dsky_nv_blank(s,io,channel11,mission);
        default:return P66_BIAS_OUT_OF_SCOPE;
        }
    }
    switch(s->words[5]){
    case 03566:continuation=DSKY_NV_ENTSET_CLEAR;break;
    case 03575:continuation=DSKY_NV_NOUN_RETURN;break;
    case 03602:continuation=DSKY_NV_AFTER_VERB;break;
    case 03604:continuation=DSKY_NV_AFTER_NOUN;break;
    default:return P66_BIAS_OUT_OF_SCOPE;
    }
    return dsky_nv_continue(s,io,mission,continuation);
}
