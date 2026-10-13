/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PW_H
#define LANEC_PW_H
/* Shared pieces of the pw-* suites (tests/lanec_pw_*.c): the earlier Pinball
 * modules (src/dsky_*.c and their CM twins) run through the Lane C harness,
 * so the Virtual AGC oracle compares their complete results and records
 * which original words they cover. Each suite draws its states from the
 * module's own input generator, spread over the generator's whole domain.
 * The oracle runs the original program from the entry until Z first reaches
 * the point (bank and address) where the native module stopped with the
 * same Q; the native module claims nothing past it. Test-only. */
#include "lanec_suites.h"
#include "lanec_pw_run.h"
#include "p66_bias.h"
#include "cm_native_image.h"

void lanec_pw_exit_stop(lanec_meta *, const apollo_overlay_state *, const lanec_io *, lanec_run);
/* A generator's image in the forms the oracle can hold (L sign-extended,
 * OPTY/OPTX and GYROCMD..OPTXCMD idle, as lanec_fill leaves them). */
void lanec_pw_settle(apollo_overlay_state *);
/* Generator number of case n of `cases` spread evenly over 0..domain-1;
 * n = cases + i gives domain + i, a suite's own targeted case i. */
uint32_t lanec_pw_spread(uint32_t n, uint32_t cases, uint32_t domain);
/* CHARIN image patches for DECTOBIN's fifth-digit carry (LM and CM). */
#define LANEC_PW_FIFTH_CHAR 8u
void lanec_pw_fifth_char(apollo_overlay_state *, unsigned);

#endif
