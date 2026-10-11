/* SPDX-License-Identifier: MIT; original INDEX ALMCADR/CAF0 control. */
#include "cm_abort_word.h"
static bool context(const apollo_overlay_state *s,const cm_native_io *io){return s&&io&&s->words[7]==0&&(s->words[5]==07762u||s->words[5]==05726u||s->words[5]==05754u)&&s->words[01363]>=0100u&&s->words[01363]<=07775u&&s->words[3]<=03400u&&!(s->words[3]&0377u)&&!(s->words[4]&~076000u)&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&!(io->superbank&~0160u)&&io->interrupt_enabled<=1u;}
bool cm_abort_word_fixed_address(const apollo_overlay_state *s,const cm_native_io *io,uint16_t *out){agc_word q,bank;if(!context(s,io)||!out||s->words[5]==05754u)return false;q=s->words[01363];if(q<02000u)return false;if(q>=06000u)bank=3;else if(q>=04000u)bank=2;else{bank=(agc_word)(s->words[4]>>10);if(bank>=030u&&(io->superbank&0100u))bank=(agc_word)(bank+8u);}*out=(uint16_t)(bank*1024u+(q&01777u));return true;}
enum exec_status cm_abort_word(apollo_overlay_state *s,cm_native_io *io,agc_word fixed_word){agc_word *w,q,physical,code;if(!context(s,io)||fixed_word>AGC_MASK)return EXEC_OUT_OF_SCOPE;w=s->words;q=w[01363];
 if(w[5]==05754u)code=01103;else if(q>=02000u)code=fixed_word;else{physical=q<01400u?q:(agc_word)(w[3]+(q&0377u));if(physical<0100u)return EXEC_OUT_OF_SCOPE;code=w[physical];}
 w[0]=agc_extend(code);w[1]=agc_extend(agc_correct(w[1]));w[2]=w[5]==07762u?07765u:05731u;w[5]=05655;return EXEC_OK;
}
