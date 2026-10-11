/* SPDX-License-Identifier: MIT */
#include "cm_alarm_image.h"
static bool context(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&s->words[2]>=0100u&&s->words[2]<=07775u&&s->words[3]<=03400u&&!(s->words[3]&0377u)&&!(s->words[4]&~076000u)&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;}
bool cm_alarm_fixed_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){agc_word q,bank;if(!context(s,io)||!out||s->words[5]!=05650u)return false;q=s->words[2];if(q<02000u)return false;
 if(q>=06000u)bank=3;else if(q>=04000u)bank=2;else{bank=(agc_word)(s->words[4]>>10);if(bank>=030u&&(io->superbank&0100u))bank=(agc_word)(bank+8u);}*out=(uint16_t)(bank*1024u+(q&01777u));return true;
}
enum exec_status cm_alarm_image(apollo_overlay_state *s,cm_native_io *io,enum cm_alarm_record_entry entry,agc_word fixed_word){agc_word literal=fixed_word,q,physical,incoming_irq;enum exec_status status;
 if(!context(s,io)||fixed_word>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 if(entry==CM_RECORD_ALARM){q=s->words[2];if(q<02000u){physical=q<01400u?q:(agc_word)(s->words[3]+(q&0377u));if(physical<0100u)return EXEC_OUT_OF_SCOPE;literal=physical==01363u?q:s->words[physical];}}
 incoming_irq=io->interrupt_enabled;status=cm_alarm_record(s,io,entry,literal);if(status==EXEC_OK&&entry==CM_RECORD_BORTENT&&s->words[5]==05721u)io->interrupt_enabled=incoming_irq;return status;
}
