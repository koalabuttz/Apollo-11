/* SPDX-License-Identifier: MIT */
/* pw-input-store: PUTCOM/PUTNORM/PUTDPCOM/PUTCOM2 data stores
 * (src/dsky_input_store.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_input_store.h"
#include "dsky_input_store_cases.h"

#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_input_store_input, lanec_pw_lamp_out, lanec_pw_input_store_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_input_store = { "pw-input-store", CASES, make, lanec_pw_input_store_run };
