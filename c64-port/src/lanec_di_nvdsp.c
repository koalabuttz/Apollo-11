/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES display start-up, Luminary099 and Comanche055
 * (bank 10). Each case below is one basic block of the source, entered by
 * its offset from PINBRNCH; see docs/LANEC-DI-NVDSP.md for the source map. */
#include "lanec_di_nvdsp.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t pinbrnch, eraser, markplay, upflag, keepprio, playjum1, copypacs, cleanend, nvmonopt, rest;
    uint16_t flashoff, copytogo, downflag, blanksub, flashsub, endit, endofjob, markperf;
    uint16_t dspflg, mark2pac, nvword1, priodbit;
} places;

#define PLACES(M, PRIODBIT) { LANEC_##M##_PINBRNCH, LANEC_##M##_ERASER, LANEC_##M##_MARKPLAY, LANEC_##M##_UPFLAG, \
    LANEC_##M##_KEEPPRIO, LANEC_##M##_PLAYJUM1, LANEC_##M##_COPYPACS, LANEC_##M##_CLEANEND, LANEC_##M##_NVMONOPT, \
    LANEC_##M##_REST, LANEC_##M##_FLASHOFF, LANEC_##M##_COPYTOGO, LANEC_##M##_DOWNFLAG, LANEC_##M##_BLANKSUB, \
    LANEC_##M##_FLASHSUB, LANEC_##M##_ENDIT, LANEC_##M##_ENDOFJOB, LANEC_##M##_MARKPERF, LANEC_##M##_EBANKSAV, \
    LANEC_##M##_MARK2PAC, LANEC_##M##_NVWORD1, LANEC_##M##_##PRIODBIT##_WORD }
static const places lm = PLACES(LM, PRIODBIT), cm = PLACES(CM, BIT14);

/* Block offsets from PINBRNCH, the same in both missions. */
enum {
    PINBRNCH = 0, NORMBNCH = 9, NORMBNCH2 = 11, NVDSP = 16, NVDSP_1 = 17, NVMARK = 34, NVMARK_1 = 35,
    NVDSP1 = 39, NV50DSP = 40, NVBUSY = 41, NVDONE = 42, NVCOPY = 43, NVDOWN1 = 44, NVDOWN2 = 46,
    NVDOWN3 = 48, BLANKCHK = 50, BLANKCHK2 = 52, PERFCHEK = 53, GOANIDLE = 57, GOANIDLE4 = 61,
    GOANIDLE8 = 65, ISITN00 = 69, FIRSTOR2ND = 75, FIRSTOR2ND4 = 79, V50PASTE = 91, OUT = 0xffff, REFUSE = 0xfffe
};

#define AT(M, X, N) (LANEC_##M##_##X == LANEC_##M##_PINBRNCH + (N))
#define AT2(X, N) (AT(LM, X, N) && AT(CM, X, N))
#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
typedef char layout[(AT2(NORMBNCH, NORMBNCH) && AT2(NVDSP, NVDSP) && AT2(NVDSP1, NVDSP1) &&
    AT2(NV50DSP, NV50DSP) && AT2(BLANKCHK, BLANKCHK) && AT2(PERFCHEK, PERFCHEK) && AT2(GOANIDLE, GOANIDLE) &&
    AT2(ISITN00, ISITN00) && AT2(1STOR2ND, FIRSTOR2ND) && AT2(V50PASTE, V50PASTE) &&
    LANEC_LM_PINBRNCH_BANK == 010u && LANEC_CM_PINBRNCH_BANK == 010u && LANEC_LM_GOPLAY == LANEC_LM_NVDSP &&
    LANEC_CM_GOPLAY == LANEC_CM_NVDSP && LANEC_LM_PRIODBIT_WORD == LANEC_CM_BIT14_WORD &&
    SAME(MPAC) && SAME(FLAGWRD4) && SAME(LOC) && SAME(CADRFLSH) && SAME(NVWORD) && SAME(MARKNV) &&
    SAME(TEM1) && SAME(BANKSET) && SAME(TEMPOR2) && SAME(PINMASK_WORD) && SAME(SEVEN_WORD) &&
    SAME(BIT13_WORD) && SAME(BIT8_WORD) && SAME(V05N00M1_WORD) && SAME(BIT12_WORD) && SAME(V97N00_WORD) &&
    SAME(V50N00_WORD) && SAME(LOW7_WORD) && SAME(ONE_WORD) && SAME(BIT4_WORD) && SAME(BIT5_WORD) &&
    SAME(BIT6_WORD)) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define TEMPOR2 LANEC_LM_TEMPOR2
#define COPINDEX LANEC_LM_LOC
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define CADRFLSH LANEC_LM_CADRFLSH
#define NVWORD LANEC_LM_NVWORD
#define MARKNV LANEC_LM_MARKNV
#define TEM1 LANEC_LM_TEM1
#define MPAC2SAV LANEC_LM_BANKSET
#define W(X) LANEC_LM_##X##_WORD

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }
static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static uint8_t zero16(uint16_t a) { return (uint8_t)(a == 0 || a == 0177777u); }

static uint16_t call(apollo_overlay_state *s, const places *p, uint16_t target, unsigned next)
{
    lanec_tc(s, target, (uint16_t)(p->pinbrnch + next));
    return OUT;
}

static uint16_t leave(apollo_overlay_state *s, uint16_t target)
{
    s->words[LANEC_Z] = target;
    return OUT;
}

static uint16_t step(apollo_overlay_state *s, lanec_io *io, const places *p, uint16_t at)
{
    uint16_t *w = s->words, i = w[COPINDEX];
    uint8_t k;
    switch (at) {
    case PINBRNCH:                 /* RELINT, CA MARK2PAC, TS MPAC +2, CA FLAGWRD4, MASK PINMASK, CCS A */
        io->interrupt_enabled = 1;
        set_a(s, w[p->mark2pac]);
        w[MPAC + 2u] = w[p->mark2pac];
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(PINMASK));
        k = lanec_ccs_a(s);
        return k == 1u ? leave(s, p->eraser) : k == 2u ? leave(s, p->markplay) : (uint16_t)NORMBNCH;
    case NORMBNCH: return call(s, p, p->upflag, NORMBNCH + 1u);            /* ADRES PINBRFLG follows */
    case NORMBNCH2:                                    /* CAF PRIODBIT, MASK FLAGWRD4, CCS A */
        set_a(s, p->priodbit);
        lanec_mask(s, w[FLAGWRD4]);
        return leave(s, lanec_ccs_a(s) == 0 ? p->keepprio : p->playjum1);
    case NVDSP: return call(s, p, p->copypacs, NVDSP_1);
    case NVDSP_1:                  /* blank bits into L, clear DSPFLG bit 13, save TEM1, MPAC +2, CCS NVWORD */
        if (i > 2u) return REFUSE;
        set_a(s, w[TEMPOR2]);
        lanec_mask(s, W(SEVEN));
        w[LANEC_L] = w[LANEC_A];
        w[LANEC_A] = (uint16_t)~lanec_a16(W(BIT13));
        lanec_mask(s, w[p->dspflg + i]);
        (void)lanec_ts(s, (uint16_t)(p->dspflg + i));
        lanec_mask(s, W(BIT8));
        (void)lanec_ts(s, TEM1);
        set_a(s, w[MPAC + 2u]);
        w[MPAC2SAV] = w[MPAC + 2u];
        w[p->mark2pac] = w[MPAC + 2u];
        k = lanec_ccs(s, w[NVWORD + i]);
        return k == 0 ? (uint16_t)NVDSP1 : k == 1u ? leave(s, p->cleanend) : k == 2u ? (uint16_t)NVMARK : (uint16_t)NVMARK_1;
    case NVMARK: w[LANEC_A] = (uint16_t)~lanec_a16(w[MARKNV]); return NVMARK_1;
    case NVMARK_1:                                     /* TS MARKNV, MASK LOW7, AD V05N00M1, AD TEM1 */
        if (!lanec_ts(s, MARKNV)) lanec_mask(s, W(LOW7));   /* an overflow skips the MASK */
        lanec_ad(s, W(V05N00M1));
        lanec_ad(s, w[TEM1]);
        return NVDSP1;
    case NVDSP1: lanec_ad(s, W(ONE)); return NV50DSP;
    case NV50DSP: return call(s, p, p->nvmonopt, NVBUSY);
    case NVBUSY: return leave(s, p->rest);
    case NVDONE: return call(s, p, p->flashoff, NVCOPY);
    case NVCOPY: return call(s, p, p->copytogo, NVDOWN1);
    case NVDOWN1: return call(s, p, p->downflag, NVDOWN1 + 1u);           /* ADRES MRKNVFLG follows */
    case NVDOWN2: return call(s, p, p->downflag, NVDOWN2 + 1u);           /* ADRES NRMNVFLG follows */
    case NVDOWN3: return call(s, p, p->downflag, NVDOWN3 + 1u);           /* ADRES PRONVFLG follows */
    case BLANKCHK: set_a(s, w[TEMPOR2]); return call(s, p, p->blanksub, BLANKCHK2);
    case BLANKCHK2: return NVDSP;
    case PERFCHEK:                                     /* CAF BIT5, MASK TEMPOR2, CCS A */
        set_a(s, W(BIT5));
        lanec_mask(s, w[TEMPOR2]);
        return lanec_ccs_a(s) == 0 ? (uint16_t)FIRSTOR2ND : (uint16_t)GOANIDLE;
    case GOANIDLE:                                     /* CAF BIT4, MASK TEMPOR2, CCS A */
        set_a(s, W(BIT4));
        lanec_mask(s, w[TEMPOR2]);
        return lanec_ccs_a(s) == 0 ? leave(s, p->flashsub) : (uint16_t)GOANIDLE4;
    case GOANIDLE4:                                    /* CS TEMPOR2, MASK BIT6, CCS A */
        w[LANEC_A] = (uint16_t)~lanec_a16(w[TEMPOR2]);
        lanec_mask(s, W(BIT6));
        return lanec_ccs_a(s) == 0 ? (uint16_t)ISITN00 : (uint16_t)GOANIDLE8;
    case GOANIDLE8:                                    /* INDEX COPINDEX, CA CADRFLSH, TS MPAC +3, TCF ENDIT */
        if (i > 2u) return REFUSE;
        set_a(s, w[CADRFLSH + i]);
        w[MPAC + 3u] = w[CADRFLSH + i];
        return leave(s, p->endit);
    case ISITN00:                                      /* INDEX COPINDEX, CA NVWORD, MASK LOW7, EXTEND, BZF */
        if (i > 2u) return REFUSE;
        set_a(s, w[NVWORD + i]);
        lanec_mask(s, W(LOW7));
        return leave(s, zero16(w[LANEC_A]) ? p->flashsub : p->endofjob);
    case FIRSTOR2ND:                                   /* CA TEMPOR2, MASK BIT13, CCS A */
        set_a(s, w[TEMPOR2]);
        lanec_mask(s, W(BIT13));
        return lanec_ccs_a(s) == 0 ? (uint16_t)GOANIDLE : (uint16_t)FIRSTOR2ND4;
    case FIRSTOR2ND4:              /* CA BIT13, INDEX COPINDEX, ADS DSPFLG, ZL, EXTEND, BZMF MARKPERF */
        if (i > 2u) return REFUSE;
        set_a(s, W(BIT13));
        lanec_ad(s, w[p->dspflg + i]);
        w[p->dspflg + i] = (uint16_t)((w[LANEC_A] & 037777u) | ((w[LANEC_A] >> 1) & 040000u));
        w[LANEC_L] = 0;
        if (zero16(w[LANEC_A]) || (w[LANEC_A] & 0100000u)) return leave(s, p->markperf);
        lanec_mask(s, W(BIT12));                       /* MASK BIT12, EXTEND, BZF V50PASTE */
        if (zero16(w[LANEC_A])) return V50PASTE;
        w[LANEC_A] = (uint16_t)~lanec_a16(w[p->nvword1]);   /* CS NVWORD1, AD V97N00 */
        lanec_ad(s, W(V97N00));
        return NV50DSP;
    case V50PASTE: set_a(s, W(V50N00)); return NV50DSP;
    default: return OUT;
    }
}

static uint8_t is_block(uint16_t at)
{
    switch (at) {
    case PINBRNCH: case NORMBNCH: case NORMBNCH2: case NVDSP: case NVDSP_1: case NVMARK: case NVMARK_1:
    case NVDSP1: case NV50DSP: case NVBUSY: case NVDONE: case NVCOPY: case NVDOWN1: case NVDOWN2:
    case NVDOWN3: case BLANKCHK: case BLANKCHK2: case PERFCHEK: case GOANIDLE: case GOANIDLE4:
    case GOANIDLE8: case ISITN00: case FIRSTOR2ND: case FIRSTOR2ND4: case V50PASTE:
        return 1;
    default:
        return 0;
    }
}

enum lanec_status lanec_di_nvdsp_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z], at, a;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 010u || z < p->pinbrnch) return LANEC_OUT_OF_SCOPE;
    at = (uint16_t)(z - p->pinbrnch);
    if (!is_block(at)) return LANEC_OUT_OF_SCOPE;
    /* COPINDEX selects one of the three PRIO/MARK/NORM display word sets:
     * each step that indexes with it refuses another value. The steps run
     * before one only change A, which is put back. */
    a = w[LANEC_A];
    while (at != OUT && at != REFUSE) at = step(s, io, p, at);
    if (at == REFUSE) { w[LANEC_A] = a; return LANEC_OUT_OF_SCOPE; }
    return LANEC_OK;
}
