/* SPDX-License-Identifier: MIT */
/* pw-nv-key: Luminary VERB (40,2354) and NOUN (40,2370) keys through NVCOM and
 * 2BLANK to ENDOFJOB (src/dsky_nv_key.c); the key is read back from Z. */
#include "lanec_pw_g.h"
#include "dsky_nv_key.h"
#include "dsky_nv_key_cases.h"

LANEC_PW_G_P66_RUN(nv_key_run, dsky_nv_key(s, &p, s->words[LANEC_Z] == 02370u ? DSKY_KEY_NOUN : DSKY_KEY_VERB))
#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_nv_key_prepare, DSKY_NV_KEY_CASE_COUNT, CASES, nv_key_run, io->channel[011])
const lanec_suite lanec_suite_pw_nv_key = { "pw-nv-key", CASES, make, nv_key_run };
