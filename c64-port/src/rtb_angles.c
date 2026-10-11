/* SPDX-License-Identifier: MIT */
#include "rtb_angles.h"
static void exchange_pair(uint16_t *w,unsigned at,agc_register_pair *r)
{
    agc_word high=w[at],low=w[at+1u];
    w[at]=agc_correct(r->a);w[at+1u]=agc_correct(r->l);
    r->a=agc_extend(high);r->l=agc_extend(low);
}
/* Raw accumulator sign includes its overflow bit. The source CCS/AD/COM
 * sequence collapses either zero and restores positive magnitude or the
 * complemented diminished negative magnitude; no instruction dispatch. */
static agc_register twos_word(agc_register value)
{
    uint32_t magnitude=(value&0100000u)?(value^0177777u):value;
    if(magnitude==0)return 0;
    return (value&0100000u)?(agc_register)((magnitude-1u)^0177777u):value;
}
static agc_register overflow_limit(agc_register value)
{
    return agc_extend((agc_word)(agc_overflow(value)>0?040000u:037777u));
}
static void one_to_two(uint16_t *w,agc_register_pair *r)
{
    agc_register low,high,converted;int carry;
    exchange_pair(w,0154,r);
    low=agc_add_register(r->l,r->l);high=agc_add_register(r->a,r->a);
    carry=agc_overflow(low);
    if(carry)high=agc_add_register(high,(agc_register)(carry>0?1u:0177776u));
    r->l=agc_extend(agc_correct(low));converted=twos_word(high);
    w[0154]=agc_correct(converted);
    if(agc_overflow(converted)){
        r->a=agc_add_register(overflow_limit(converted),agc_extend(w[0154]));
        w[0154]=agc_correct(r->a);
    }else r->a=converted;
}
static unsigned target_address(const uint16_t *w)
{
    unsigned address=w[0130];
    return address>=01400u?w[3]+address-01400u:address;
}
static void increment(uint16_t *w,agc_register_pair *r)
{
    agc_word value;agc_register magnitude,converted;
    w[0142]=agc_correct(r->a);value=w[target_address(w)];
    if(value&040000u){magnitude=agc_extend(agc_negate(value));r->a=(agc_register)(agc_add_register(magnitude,1)^0177777u);}
    else r->a=agc_extend(value);
    r->a=agc_add_register(r->a,agc_extend(w[0142]));converted=twos_word(r->a);
    w[0142]=agc_correct(converted);
    if(agc_overflow(converted))r->a=agc_add_register(overflow_limit(converted),agc_extend(w[0142]));else r->a=converted;
    w[target_address(w)]=agc_correct(r->a);r->l=agc_extend(agc_correct(r->l));
}
enum rtb_status rtb_angles(apollo_overlay_state *image,p66_bias_io *io,enum rtb_mission mission,enum rtb_angle_entry entry)
{
    static const uint16_t start[2][9]={{03644,03614,03620,03635,03576,03660,0,03573,0},{03576,03546,03552,03567,03513,03626,03612,03510,03527}};
    uint16_t *w=image->words;agc_register_pair r;unsigned k;uint16_t fb;
    if((unsigned)mission>1u||(unsigned)entry>8u)return RTB_OUT_OF_SCOPE;
    fb=(uint16_t)(mission==RTB_LM?020000u:044000u);
    if(!start[mission][entry]||w[5]!=start[mission][entry]||w[4]!=fb||w[3]>03400u||
       (w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||io->interrupt_enabled>1u||
       io->superbank>0160u||(io->superbank&017u))return RTB_OUT_OF_SCOPE;
    if((entry==RTB_ONE_TO_TWO_SUB||entry==RTB_CDU_INCREMENT)&&(w[2]<02000u||w[2]>07777u))return RTB_OUT_OF_SCOPE;
    if(entry==RTB_CDU_INCREMENT&&(w[0]!=agc_extend(agc_correct(w[0]))||w[0130]<040u||w[0130]>01777u||target_address(w)<040u))return RTB_OUT_OF_SCOPE;
    r.a=w[0];r.l=w[1];
    if(entry==RTB_ONE_TO_TWO_SUB){uint16_t returned=w[2];one_to_two(w,&r);w[5]=returned;w[2]=3;}
    else if(entry==RTB_CDU_INCREMENT){uint16_t returned=w[2];increment(w,&r);w[5]=returned;w[2]=3;}
    else{
        if(entry==RTB_ONE_TO_TWO){one_to_two(w,&r);w[2]=3;w[0155]=0;w[0163]=0;r.a=0;}
        else if(entry==RTB_VECTOR_ONE_TO_TWO){
            one_to_two(w,&r);exchange_pair(w,0161,&r);exchange_pair(w,0154,&r);
            one_to_two(w,&r);w[0156]=agc_correct(r.a);
            exchange_pair(w,0157,&r);exchange_pair(w,0154,&r);
            one_to_two(w,&r);w[0155]=agc_correct(r.a);w[0154]=w[0161];w[0163]=1;r.a=1;w[2]=3;
        }else if(entry==RTB_TWO_VECTOR_ONE_TO_TWO){
            one_to_two(w,&r);exchange_pair(w,0157,&r);exchange_pair(w,0154,&r);
            one_to_two(w,&r);r.l=agc_extend(agc_correct(r.a));r.a=agc_extend(w[0157]);
            exchange_pair(w,0154,&r);w[0156]=0;w[0163]=0;r.a=0;w[2]=3;
        }else if(entry==RTB_CDU_LOGIC){
            agc_dp p={0,(agc_word)((w[0154]&040000u)?057777u:0)};
            r=agc_multiply(agc_extend(w[0154]),020000);agc_double_add(&r,&p);w[0154]=p.high;w[0155]=p.low;
        }else if(entry==RTB_INCREMENT_CDUS){
            w[0130]=01155;
            for(k=0;k<3u;++k){r.a=agc_extend(w[0154+(k==0u?0u:(k==1u?3u:5u))]);increment(w,&r);if(k<2u)w[0130]=agc_add_word(w[0130],1);}
            w[0163]=077776;r.a=0177776;w[2]=3;
        }else if(entry==RTB_LOAD_TIME){
            r.a=agc_extend(w[024]);r.l=agc_extend(w[025]);exchange_pair(w,0154,&r);w[0156]=0;w[0163]=0;r.a=0;
        }else if(entry==RTB_READ_PIPS){
            r.l=agc_extend(agc_correct(r.l));w[0154]=w[037];w[0157]=w[040];w[0161]=w[041];w[0155]=w[0160]=w[0162]=0;w[0163]=077776;r.a=0177776;io->interrupt_enabled=1;
        }
        w[5]=(uint16_t)(mission==RTB_LM?06061u:06030u);
    }
    w[0]=r.a;w[1]=r.l;return RTB_OK;
}

uint8_t rtb_angles_constant_read(enum rtb_mission mission,uint16_t address,uint16_t *out)
{
    if((unsigned)mission!=1u||address!=19349u||out==0)return 0;
    *out=01155;return 1;
}
