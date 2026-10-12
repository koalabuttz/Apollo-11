/* SPDX-License-Identifier: MIT */
/* pw-input-store: PUTCOM/PUTNORM/PUTDPCOM/PUTCOM2 data stores
 * (src/dsky_input_store.c). Generator numbers 0-4095 are LM, 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_input_store.h"
#include "dsky_input_store_cases.h"

LANEC_PW_LAMP_RUN(input_store_run, dsky_input_store, cm_input_store, enum dsky_input_store_route, DSKY_INPUT_STORE_CALLER)
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_input_store_input, lanec_pw_lamp_out, input_store_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_input_store = { "pw-input-store", CASES, make, input_store_run };
