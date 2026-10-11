/* SPDX-License-Identifier: MIT */
#include "interpreted_square.h"
static void twice(agc_register_pair *r)
{
    agc_register low=agc_add_register(r->l,r->l);
    agc_register high=agc_add_register(r->a,r->a);
    int overflow=agc_overflow(low);
    if(overflow)high=agc_add_register(high,overflow>0?1u:0177776u);
    r->a=high;r->l=agc_extend(agc_correct(low));
}
enum interpreted_square_status interpreted_square(apollo_overlay_state *m,
 enum interpreted_square_mission mission,enum interpreted_square_kind kind)
{
    agc_register_pair r;agc_dp pair;agc_word major,minor,saved;
    if((mission!=INTERPRETED_SQUARE_LM&&mission!=INTERPRETED_SQUARE_CM)||
       (kind!=INTERPRETED_DSQ&&kind!=INTERPRETED_DSQSUB))
        return INTERPRETED_SQUARE_OUT_OF_SCOPE;
    if(m->words[5]!=(kind==INTERPRETED_DSQ?03174u:03300u)||m->words[4]!=0||
       m->words[7]!=0||m->words[3]>03400u||(m->words[3]&0377u)!=0||
       m->words[6]!=(m->words[3]>>8)||
       (kind==INTERPRETED_DSQSUB&&(m->words[2]<02000u||m->words[2]>07777u)))
        return INTERPRETED_SQUARE_OUT_OF_SCOPE;
    saved=m->words[2];major=m->words[0154];minor=m->words[0155];
    /* Retain high(minor^2), then the unrounded doubled cross term. */
    r=agc_multiply(agc_extend(minor),minor);
    m->words[0156]=agc_correct(r.a);m->words[0155]=0;
    r=agc_multiply(agc_extend(minor),major);twice(&r);
    pair.high=0;pair.low=m->words[0156];agc_double_add(&r,&pair);
    m->words[0155]=pair.high;m->words[0156]=pair.low;
    /* Source XCH saves the cross-term DAS overflow before squaring major. */
    m->words[0154]=agc_correct(r.a);
    r=agc_multiply(agc_extend(major),major);
    pair.high=m->words[0154];pair.low=m->words[0155];agc_double_add(&r,&pair);
    m->words[0154]=pair.high;m->words[0155]=pair.low;
    m->words[0]=r.a;m->words[1]=r.l;m->words[2]=3;
    m->words[5]=kind==INTERPRETED_DSQSUB?saved:
          (agc_word)(mission==INTERPRETED_SQUARE_LM?06061u:06030u);
    return INTERPRETED_SQUARE_OK;
}
