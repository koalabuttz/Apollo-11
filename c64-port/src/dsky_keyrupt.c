/* SPDX-License-Identifier: MIT
 * Literal KEYRUPT1 and complete source LODSAMPT composition. */
#include "dsky_keyrupt.h"
enum exec_status dsky_keyrupt(apollo_overlay_state *s,p66_bias_io *io,uint16_t channel15){uint16_t *w=s->words,old_sample=w[014];
 if(channel15>077777 || w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=010000 || w[6]!=(010000u|(w[3]>>8)) || io->superbank>0160 || (io->superbank&017u)!=0 || io->interrupt_enabled>1 || (w[067]!=077777 && (w[067]>84 || w[067]%12u!=0)))return EXEC_OUT_OF_SCOPE;
 /* TS overflow stores the corrected word, leaves +/-1 in A, and skips
  * the next source instruction: XCH Q or the LODSAMPT call respectively. */
 w[016]=agc_correct(w[0]);w[012]=agc_correct(w[2]);if(agc_overflow(w[0])!=0){w[012]=agc_overflow(w[0])>0 ? 1 : 077776;}if(agc_overflow(w[0])!=0 || agc_overflow(w[2])==0){w[013]=w[024];w[014]=w[025];w[1]=agc_extend(old_sample);}w[0]=(uint16_t)(channel15&037u);return dsky_keycom(s,io);
}
