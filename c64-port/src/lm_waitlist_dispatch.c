/* SPDX-License-Identifier: MIT */
#include "lm_waitlist_dispatch.h"
enum wl_status lm_waitlist_dispatch(apollo_overlay_state *image,p66_bias_io *io,uint16_t in_interrupt,exec_address *next){uint16_t *w,old_bank;agc_word saved_q;waitlist s;exec_address target;enum wl_status status;uint8_t i,vector;int time_overflow;
 if(!image||!io||!next)return WL_OUT_OF_SCOPE;
 w=image->words;vector=(uint8_t)(w[5]==04014u);
 if((!vector&&(w[5]!=05261u||w[0734]!=1u))||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[015]==0||w[015]>010000u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled!=0||in_interrupt!=1)return WL_OUT_OF_SCOPE;
 s.time3=w[026];s.ruptagn=w[0734];s.alarm=0;for(i=0;i<8u;++i)s.interval[i]=w[01400u+i];for(i=0;i<9u;++i){s.task[i].location=w[01410u+2u*i];s.task[i].bank=w[01411u+2u*i];}
 time_overflow=agc_overflow(agc_add_register(agc_add_register(agc_extend(s.interval[0]),037777u),agc_extend(s.time3)));
 status=wl_begin(&s,&target);if(status!=WL_OK)return status;
 /* Original TS RUPTAGN skips CS ZERO on either sign of TIME3 overflow.
  * The typed timing primitive models the ordinary positive-overflow route;
  * canonical source metadata retains negative-overflow -1 as well. */
 if(time_overflow<0)s.ruptagn=077776u;
 if(vector){old_bank=w[6];w[010]=agc_correct(w[0]);w[011]=agc_correct(w[1]);w[016]=(agc_word)(old_bank|io->superbank);saved_q=w[012];w[012]=agc_correct(w[2]);w[2]=agc_extend(saved_q);}
 w[026]=s.time3;w[0734]=s.ruptagn;for(i=0;i<8u;++i)w[01400u+i]=s.interval[i];for(i=0;i<9u;++i){w[01410u+2u*i]=s.task[i].location;w[01411u+2u*i]=s.task[i].bank;}
 w[0]=03452;w[1]=02003;w[5]=target.location;w[6]=(agc_word)(target.bank&076007u);w[3]=(agc_word)((target.bank&7u)*0400u);w[4]=(agc_word)(target.bank&076000u);w[7]=0;io->superbank=(agc_word)(target.bank&0160u);*next=target;return WL_OK;
}
