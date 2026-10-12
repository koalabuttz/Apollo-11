/* SPDX-License-Identifier: MIT */
/* pw-charin-nv: Luminary CHARIN (40,2077) for VERB/NOUN through NVCOM and 2BLANK to
 * ENDOFJOB (src/dsky_charin_nv.c), the LM side of pw-cm-charin-nv. */
#include "lanec_pw_g.h"
#include "dsky_charin_nv.h"
#include "dsky_charin_nv_cases.h"

LANEC_PW_G_CHARIN_RUN(charin_nv_run, dsky_charin_nv(s, &p))
#define CASES 1024u
LANEC_PW_G_MAKE(make, dsky_charin_nv_prepare, DSKY_CHARIN_NV_CASE_COUNT, CASES, charin_nv_run, dsky_charin_nv_channel11(k))
const lanec_suite lanec_suite_pw_charin_nv = { "pw-charin-nv", CASES, make, charin_nv_run };
