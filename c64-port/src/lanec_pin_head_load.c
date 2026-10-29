/* SPDX-License-Identifier: MIT */
/* Pinball load verbs, Luminary099 and Comanche055 (bank 41): the words of
 * ABCLOAD, ABLOAD, ALOAD, BLOAD and CLOAD that call the component tests,
 * the verb update and the data requests, and LOADLV. Each entry runs to the
 * next call and leaves as the original does there (Z the callee, Q the
 * word after the TC), so the callees run as their own routines. See
 * docs/LANEC-PIN-HEADS.md. */
#include "lanec_pin_heads.h"
#include "lanec_symbols.h"

typedef char same_words[(LANEC_LM_ABCLOAD == LANEC_CM_ABCLOAD && LANEC_LM_ABLOAD == LANEC_CM_ABLOAD &&
    LANEC_LM_ALOAD == LANEC_CM_ALOAD && LANEC_LM_BLOAD == LANEC_CM_BLOAD && LANEC_LM_CLOAD == LANEC_CM_CLOAD &&
    LANEC_LM_COMPTEST == LANEC_CM_COMPTEST && LANEC_LM_NOUNTEST == LANEC_CM_NOUNTEST &&
    LANEC_LM_UPDATVB == LANEC_CM_UPDATVB && LANEC_LM_REQDATX == LANEC_CM_REQDATX &&
    LANEC_LM_REQDATY == LANEC_CM_REQDATY && LANEC_LM_REQDATZ == LANEC_CM_REQDATZ &&
    LANEC_LM_LOADLV == LANEC_CM_LOADLV && LANEC_LM_CLPASS == LANEC_CM_CLPASS &&
    LANEC_LM_DECBRNCH == LANEC_CM_DECBRNCH && LANEC_LM_LOADSTAT == LANEC_CM_LOADSTAT &&
    LANEC_LM_DSPCOUNT == LANEC_CM_DSPCOUNT && LANEC_LM_ABCLOAD_BANK == 041u && LANEC_LM_LOADLV_BANK == 041u &&
    LANEC_LM_ABLOAD == LANEC_LM_ABCLOAD + 065u && LANEC_LM_ALOAD == LANEC_LM_ABLOAD + 027u &&
    LANEC_LM_BLOAD == LANEC_LM_ALOAD + 011u && LANEC_LM_CLOAD == LANEC_LM_BLOAD + 015u) ? 1 : -1];

#define TWO_ 2u
#define VD1 023u        /* VD1: OCT 23, the verb display's first digit */
#define BIT15 040000u
/* The data requests, the component tests and the verb update (UPDATVB -1). */
#define REQX LANEC_LM_REQDATX
#define REQY LANEC_LM_REQDATY
#define REQZ LANEC_LM_REQDATZ
#define COMP LANEC_LM_COMPTEST
#define NOUN LANEC_LM_NOUNTEST
#define UPDT (LANEC_LM_UPDATVB - 1u)

/* One call: the word it starts at, what it puts in A first (CS of a small
 * constant, CAF VBSPnLD or CAF BIT15 then TS CLPASS) and the callee. */
enum load { NONE, CS, CAF, CLPASS };
typedef struct { uint16_t z; uint8_t load; uint16_t k, target; } call;
#define AB(o) (LANEC_LM_ABCLOAD + (o))
static const call calls[] = {
    { AB(0), CS, 2u, COMP },     /* ABCLOAD: CS TWO, TC COMPTEST */
    { AB(2), NONE, 0, NOUN },    /* TC NOUNTEST */
    { AB(3), CAF, 21u, UPDT },   /* CAF VBSP1LD, TC UPDATVB -1 */
    { AB(5), NONE, 0, REQX },    /* TC REQDATX */
    { AB(6), CAF, 22u, UPDT },   /* CAF VBSP2LD, TC UPDATVB -1 */
    { AB(010), NONE, 0, REQY },  /* TC REQDATY */
    { AB(011), CAF, 23u, UPDT }, /* CAF VBSP3LD, TC UPDATVB -1 */
    { AB(013), NONE, 0, REQZ },  /* TC REQDATZ */
    { AB(065), CS, 1u, COMP },   /* ABLOAD: CS ONE, TC COMPTEST */
    { AB(067), NONE, 0, NOUN },  /* TC NOUNTEST */
    { AB(070), CAF, 21u, UPDT }, /* CAF VBSP1LD, TC UPDATVB -1 */
    { AB(072), NONE, 0, REQX },  /* TC REQDATX */
    { AB(073), CAF, 22u, UPDT }, /* CAF VBSP2LD, TC UPDATVB -1 */
    { AB(075), NONE, 0, REQY },  /* TC REQDATY */
    { AB(0114), NONE, 0, REQX }, /* ALOAD: TC REQDATX */
    { AB(0125), CS, 1u, COMP },  /* BLOAD: CS ONE, TC COMPTEST */
    { AB(0127), CLPASS, 0, REQY }, /* CAF BIT15, TS CLPASS, TC REQDATY */
    { AB(0142), CS, 2u, COMP },  /* CLOAD: CS TWO, TC COMPTEST */
    { AB(0144), CLPASS, 0, REQZ }, /* CAF BIT15, TS CLPASS, TC REQDATZ */
};

enum lanec_status lanec_pin_head_load_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, z = w[LANEC_Z], i, len;
    LANEC_ONE_MISSION(m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 041u) return LANEC_OUT_OF_SCOPE;
    if (z == LANEC_LM_LOADLV) {   /* LOADLV: CAF ZERO, TS DECBRNCH, CS ZERO, TS LOADSTAT, TC RELDSP */
        w[LANEC_LM_DECBRNCH] = 0;
        w[LANEC_LM_LOADSTAT] = 077777u;
        w[LANEC_A] = 0177777u;
        lanec_tc(s, m == LANEC_LM ? LANEC_LM_RELDSP : LANEC_CM_RELDSP, (uint16_t)(z + 5u));
        return LANEC_OK;
    }
    if (z == LANEC_LM_LOADLV + 5u) {   /* CS VD1, TS DSPCOUNT, TC POSTJUMP (CADR RECALTST) */
        w[LANEC_A] = (uint16_t)~VD1;
        w[LANEC_LM_DSPCOUNT] = 077777u & (uint16_t)~VD1;
        lanec_tc(s, m == LANEC_LM ? LANEC_LM_POSTJUMP : LANEC_CM_POSTJUMP, (uint16_t)(z + 3u));
        return LANEC_OK;
    }
    for (i = 0; i < sizeof calls / sizeof calls[0] && calls[i].z != z; ++i) {}
    if (i == sizeof calls / sizeof calls[0]) return LANEC_OUT_OF_SCOPE;
    len = 1u;
    switch (calls[i].load) {
    case CS: w[LANEC_A] = (uint16_t)~calls[i].k; ++len; break;           /* 16-bit ~K */
    case CAF: w[LANEC_A] = calls[i].k; ++len; break;
    case CLPASS: w[LANEC_A] = lanec_a16(BIT15); (void)lanec_ts(s, LANEC_LM_CLPASS); len = 3u; break;
    default: break;
    }
    lanec_tc(s, calls[i].target, (uint16_t)(z + len));
    return LANEC_OK;
}
