/* SPDX-License-Identifier: MIT */
#include "p66_elapsed_return_control.h"
agc_word p66_elapsed_return_q(agc_word q,agc_dp mpac,agc_dp piptime){
 agc_register_pair registers;
 registers.a=agc_extend(agc_negate(piptime.high));
 registers.l=agc_extend(agc_negate(piptime.low));
 agc_double_add(&registers,&mpac);
 return registers.a!=0?3u:q;
}
uint16_t p66_elapsed_return_control(apollo_overlay_state *image,agc_word q,
 agc_dp before,agc_dp piptime){uint16_t *w;
 if(!image||q>AGC_MASK||before.high>AGC_MASK||before.low>AGC_MASK||
    piptime.high>AGC_MASK||piptime.low>AGC_MASK)return 2u;
 w=image->words;
 if(w[5]!=06061u||w[0164]!=03360u||w[023]!=0u||w[0163]!=0u||
    w[4]!=064000u||w[3]!=01000u||w[6]!=064002u)return 2u;
 w[2]=p66_elapsed_return_q(q,before,piptime);return 0;
}
