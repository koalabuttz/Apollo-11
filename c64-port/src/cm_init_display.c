/* SPDX-License-Identifier: MIT */
#include "cm_init_display.h"
static int valid_bank(const apollo_overlay_state *s,const cm_native_io *io){const uint16_t *w=s->words;return w[3]<=03400u&&(w[3]&0377u)==0&&w[4]==020000u&&w[6]==(uint16_t)(020000u|(w[3]>>8))&&w[7]==0&&io->superbank<=0160u&&(io->superbank&017u)==0&&io->interrupt_enabled<=1u;}
enum exec_status cm_init_display_return(apollo_overlay_state *s,cm_native_io *io,exec_address *next){uint16_t *w=s->words,target;
 if(next==0||w[5]!=03267u||!valid_bank(s,io)||w[0374]<4u||w[0374]>077777u)return EXEC_OUT_OF_SCOPE;
 /* CS THREE / AD TEMPFLSH; TS FBANK / MASK LOW10 / XCH Q /
  * INDEX Q / TCF10000. TCF preserves the exchanged Q. */
 /* Source hardware normalizes L at every instruction boundary. */
 w[1]=agc_extend(agc_correct(w[1]));
 target=agc_correct(agc_add_register(agc_extend(077774),agc_extend(w[0374])));
 w[4]=(uint16_t)(target&076000u);w[6]=(uint16_t)(w[4]|(w[3]>>8));
 w[0]=w[2];w[2]=(uint16_t)(target&01777u);w[5]=(uint16_t)(02000u+w[2]);
 next->location=w[5];next->bank=(uint16_t)(w[6]|io->superbank);return EXEC_OK;
}
enum exec_status cm_init_display(apollo_overlay_state *s,cm_native_io *io,exec_address *next){uint16_t *w=s->words;enum exec_status status;
 if(next==0||w[5]!=03261u||!valid_bank(s,io)||(w[0366]&037000u)==0||w[0374]<4u||w[0374]>077777u)return EXEC_OUT_OF_SCOPE;
 /* Validate unsupported priority domains before source-visible stores. */
 {uint8_t i;for(i=1;i<7u;++i)if(w[0167u+i*12u]==0)return EXEC_OUT_OF_SCOPE;if(w[0167]==0||w[0167]>=040000u||io->executive_alarm!=0)return EXEC_OUT_OF_SCOPE;}
 w[3]=(uint16_t)(w[01071]&03400u);w[6]=(uint16_t)(w[4]|(w[3]>>8));
 io->superbank=(uint16_t)(w[0366]&0160u);w[0]=(uint16_t)(w[0366]&037000u);w[2]=03267;w[5]=05210;
 status=cm_basic_priority_change(s,io,next);if(status!=EXEC_OK)return status;
 if(next->location==03267u&&(next->bank&076000u)==020000u)return cm_init_display_return(s,io,next);
 return EXEC_OK;
}
