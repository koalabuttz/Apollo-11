/* SPDX-License-Identifier: MIT */
#include "lm_super_dispatch_cases.h"
#include "lm_admission_image_cases.h"
void lm_super_dispatch_input(apollo_overlay_state *m,p66_bias_io *io,uint16_t n){
 static const agc_word locs[]={0100,01777,02000,03275,05165,05166,06070,07777};
 agc_word bank=(agc_word)(((n/2u)%32u)*02000u|((n/64u)%8u)|((n/512u)%8u)*020u);
 agc_word loc=locs[(n/4096u+n/64u)%8u];
 (void)lm_admission_image_input(m,io,n);
 m->words[5]=(agc_word)(05165u+(n&1u));
 m->words[0]=agc_extend(n&1u?bank:loc);m->words[1]=agc_extend(n&1u?loc:bank);
 m->words[2]=(agc_word)((n*37u)&0177777u);io->interrupt_enabled=(agc_word)((n/8192u)&1u);
}
