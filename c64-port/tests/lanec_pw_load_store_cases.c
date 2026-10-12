/* SPDX-License-Identifier: MIT */
/* pw-load-store: the load callers, PUTCOM returns/stores and noun-7 bit
 * controls (src/dsky_load_store.c). Generator numbers 0-4095 are LM,
 * 4096-8191 CM. */
#include "lanec_pw_c.h"
#include "dsky_load_store.h"
#include "dsky_load_store_cases.h"

LANEC_PW_LAMP_RUN(load_store_run, dsky_load_store, cm_load_store, enum dsky_load_store_route, DSKY_LOAD_STORE_COMPONENT)
#define CASES 1024u
LANEC_PW_MAKE(make, dsky_lamp_io, dsky_load_store_input, lanec_pw_lamp_out, load_store_run, 8192u, CASES)
const lanec_suite lanec_suite_pw_load_store = { "pw-load-store", CASES, make, load_store_run };
