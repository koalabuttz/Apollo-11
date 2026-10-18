/* SPDX-License-Identifier: MIT */
/* Luminary DMP and DMPSUB run in place (include/lm_dmpsub.h). Each
 * instruction below is the original's, on the canonical image. */
#include "lm_dmpsub.h"
#include "agc_image_ops.h"
#include "rtb_angles.h"
#include "agc_image_w.h"
#define MPAC   0154u
#define MPAC1  0155u
#define MPAC2  0156u
#define ADDRWD 0116u
#define MPTEMP 0135u
/* An erasable operand pair at a (12-bit, EB for 01400-01777). */
static int erasable_pair(agc_word a)
{
    a &= 07777u;
    return a >= 010u && a < 01777u;
}
static void dmpsub_body(apollo_overlay_state *s, agc_word a)
{
    /* DMPSUB (07107): INDEX ADDRWD; CA 1; TS MPAC +2; CAF ZERO; XCH MPAC +1;
     * TS MPTEMP; EXTEND; MP MPAC +2; XCH MPAC +2; EXTEND; MP MPAC;
     * DAS MPAC +1; INDEX ADDRWD; CA 0; XCH MPTEMP. */
    ai_ca(s, (unsigned)a + 1u);
    (void)ai_ts(s, MPAC2);
    ai_caf(s, 0u);
    ai_xch(s, MPAC1);
    (void)ai_ts(s, MPTEMP);
    ai_mp(s, MPAC2);
    ai_xch(s, MPAC2);
    ai_mp(s, MPAC);
    ai_das(s, MPAC1);
    ai_ca(s, a);
    ai_xch(s, MPTEMP);
    /* DMPSUB2 (07126): EXTEND; MP MPTEMP; DAS MPAC +1; XCH MPAC; EXTEND;
     * MP MPTEMP; DAS MPAC; TC Q (07135: runs C(Q) as a TC, Q = 3). */
    ai_mp(s, MPTEMP);
    ai_das(s, MPAC1);
    ai_xch(s, MPAC);
    ai_mp(s, MPTEMP);
    ai_das(s, MPAC);
    ai_tc(s, 3u);
}
uint8_t lm_dmpsub(apollo_overlay_state *s)
{
    agc_word a = (agc_word)(W[ADDRWD] & 07777u);
    if (!erasable_pair(a)) return RTB_OUT_OF_SCOPE;
    dmpsub_body(s, a);
    return RTB_OK;
}
uint8_t lm_dmp(apollo_overlay_state *s, uint16_t adres)
{
    /* DMP (07103): INDEX Q; CAF 0 (the ADRES word); INCR Q; TS ADDRWD. */
    if (!erasable_pair(adres)) return RTB_OUT_OF_SCOPE;
    ai_caf(s, adres);
    ai_incr(s, 2u);
    (void)ai_ts(s, ADDRWD);
    dmpsub_body(s, (agc_word)(adres & 07777u));
    return RTB_OK;
}
