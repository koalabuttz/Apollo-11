/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES ENDIDLE handling and display returns,
 * Luminary099 and Comanche055 (bank 10). Each case below is one basic block
 * of the source, entered by its offset from CLEANEND; see
 * docs/LANEC-DI-IDLE.md for the source map. */
#include "lanec_di_idle.h"
#include "lanec_symbols.h"

typedef struct {
    uint16_t cleanend, findvac, upent2, downent2, nvsubusy, flashon, endidle, nvsub, priochng, supersw;
    uint16_t novac, bankjump, endofjob, priobort, keepprio, markwake, wakeplay, gosleeps, normbnch, pinbrnch;
    uint16_t r1save, ebanktem, priotime, nvcadr[3];
} places;

#define PLACES(M) { LANEC_##M##_CLEANEND, LANEC_##M##_FINDVAC, LANEC_##M##_UPENT2, LANEC_##M##_DOWNENT2, \
    LANEC_##M##_NVSUBUSY, LANEC_##M##_FLASHON, LANEC_##M##_ENDIDLE, LANEC_##M##_NVSUB, LANEC_##M##_PRIOCHNG, \
    LANEC_##M##_SUPERSW, LANEC_##M##_NOVAC, LANEC_##M##_BANKJUMP, LANEC_##M##_ENDOFJOB, LANEC_##M##_PRIOBORT, \
    LANEC_##M##_KEEPPRIO, LANEC_##M##_MARKWAKE, LANEC_##M##_WAKEPLAY, LANEC_##M##_GOSLEEPS, \
    LANEC_##M##_NORMBNCH, LANEC_##M##_PINBRNCH, LANEC_##M##_R1SAVE, LANEC_##M##_EBANKTEM, LANEC_##M##_PRIOTIME, \
    { LANEC_##M##_NVCADR_WORD, LANEC_##M##_NVCADR_P_1_WORD, LANEC_##M##_NVCADR_P_2_WORD } }
static const places lm = PLACES(LM), cm = PLACES(CM);

/* Block offsets from CLEANEND, the same in both missions. */
enum {
    CLEANEND = 0, CLEANEND4 = 4, ISITPRIO = 5, REST = 10, RESTSLEP = 14, RESTSLEP4 = 18, FLASHSUB = 21,
    FLASHSUB1 = 22, ITISMASK = 27, CHKREPT = 28, GOIDLE = 37, IDLERET1 = 38, IDLERET2 = 39, IDLERET3 = 40,
    OKTOENT = 46, ENDOUT = 47, TIMECHEK = 54, NORMWAKE = 64, ENDRET = 66, ENDRET4 = 70, PINIDMSK = 76,
    BLANKALL = 77, NVSUBRET = 79, ENDIT = 80, ENDIT3 = 83, UNSETR1 = 85, UNSETR16 = 91, IMMEDRET = 92,
    TERMATE = 95, LINUSCHR = 97, PROCEED = 110, MARKRET = 112, MARKOVER = 118, NORMRET = 124,
    NORMRET4 = 128, NORMRET8 = 132, NORMRET12 = 136, NOVACRET = 144, MARSLEEP = 145, LOADITIS = 150,
    ERASER = 158, ERASER2 = 160, ERASER3 = 161, OUT = 0xffff, REFUSE = 0xfffe
};

#define AT(M, X) (LANEC_##M##_##X == LANEC_##M##_CLEANEND + (X))
#define AT2(X) (AT(LM, X) && AT(CM, X))
#define SAME(X) (LANEC_LM_##X == LANEC_CM_##X)
typedef char layout[(AT2(ISITPRIO) && AT2(REST) && AT2(RESTSLEP) && AT2(FLASHSUB) && AT2(ITISMASK) &&
    AT2(IDLERET1) && AT2(OKTOENT) && AT2(ENDOUT) && AT2(TIMECHEK) && AT2(NORMWAKE) && AT2(ENDRET) &&
    AT2(PINIDMSK) && AT2(ENDIT) && AT2(UNSETR1) && AT2(IMMEDRET) && AT2(TERMATE) && AT2(LINUSCHR) &&
    AT2(PROCEED) && AT2(MARKRET) && AT2(MARKOVER) && AT2(NORMRET) && AT2(MARSLEEP) && AT2(LOADITIS) &&
    AT2(ERASER) && LANEC_LM_CLEANEND_BANK == 010u && LANEC_CM_CLEANEND_BANK == 010u &&
    SAME(MPAC) && SAME(FLAGWRD4) && SAME(LOC) && SAME(CADRSTOR) && SAME(CADRFLSH) && SAME(NVWORD) &&
    SAME(NVSAVE) && SAME(MODE) && SAME(TIME1) &&
    SAME(PRIO32_WORD) && SAME(ITISMASK_WORD) && SAME(ASTROMSK_WORD) && SAME(IDLEMASK_WORD) &&
    SAME(BIT3_WORD) && SAME(BIT3_P_1_WORD) && SAME(BIT3_P_2_WORD) && SAME(LOWLOAD_WORD) && SAME(TWO_WORD) &&
    SAME(OCT60000_WORD) && SAME(OCT37776_WORD) && SAME(ONE_WORD) && SAME(_M_2SEC_WORD) &&
    SAME(PINIDMSK_WORD) && SAME(THREE_WORD) && SAME(PRIO37_WORD) && SAME(SIX_WORD) && SAME(MINUS1_WORD) &&
    SAME(PRIO30_WORD) && SAME(BITS5_P_11_WORD) && SAME(BITS4_P_10_WORD) && SAME(OCT50_WORD) &&
    SAME(PRIO15_WORD) && SAME(LOW7_WORD)) ? 1 : -1];

#define MPAC LANEC_LM_MPAC
#define COPMPAC (MPAC + 3u)
#define OUTHERE (MPAC + 5u)
#define GENMASK (MPAC + 6u)
#define COPINDEX LANEC_LM_LOC
#define USERPRIO LANEC_LM_MODE
#define FLAGWRD4 LANEC_LM_FLAGWRD4
#define CADRSTOR LANEC_LM_CADRSTOR
#define CADRFLSH LANEC_LM_CADRFLSH
#define NVWORD LANEC_LM_NVWORD
#define NVSAVE LANEC_LM_NVSAVE
#define TIME1 LANEC_LM_TIME1
#define W(X) LANEC_LM_##X##_WORD

static const uint16_t bit3[3] = { W(BIT3), W(BIT3_P_1), W(BIT3_P_2) };

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }
static void set_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
static void cs_a(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = (uint16_t)~lanec_a16(word); }
static uint8_t zero16(uint16_t a) { return (uint8_t)(a == 0 || a == 0177777u); }

/* TC to a callee outside this code: Q is the next source word. */
static uint16_t call(apollo_overlay_state *s, const places *p, uint16_t target, unsigned next)
{
    lanec_tc(s, target, (uint16_t)(p->cleanend + next));
    return OUT;
}

static uint16_t leave(apollo_overlay_state *s, uint16_t target)
{
    s->words[LANEC_Z] = target;
    return OUT;
}

static uint16_t step(apollo_overlay_state *s, lanec_io *io, const places *p, uint16_t at)
{
    uint16_t *w = s->words;
    uint8_t k;
    switch (at) {
    case CLEANEND: set_a(s, W(PRIO32)); return call(s, p, p->findvac, CLEANEND + 2u);   /* 2CADR JAMTERM follows */
    case CLEANEND4: return FLASHSUB1;
    case ISITPRIO:                                     /* CA FLAGWRD4, MASK ITISMASK, EXTEND, BZF PRIOBORT */
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(ITISMASK));
        return leave(s, zero16(w[LANEC_A]) ? p->priobort : p->endofjob);
    case REST:                                         /* CCS CADRSTOR */
        k = lanec_ccs(s, w[CADRSTOR]);
        return (k == 1u || k == 3u) ? (uint16_t)RESTSLEP : leave(s, p->endofjob);
    case RESTSLEP: set_a(s, w[GENMASK]); lanec_mask(s, W(ASTROMSK)); return call(s, p, p->upent2, RESTSLEP + 3u);
    case RESTSLEP4: if (w[COPINDEX] > 2u) return REFUSE; set_a(s, p->nvcadr[w[COPINDEX]]); return call(s, p, p->nvsubusy, FLASHSUB);
    case FLASHSUB: return call(s, p, p->flashon, FLASHSUB1);
    case FLASHSUB1:                                    /* CA COPINDEX, TS COPMPAC, CA GENMASK, MASK IDLEMASK */
        set_a(s, w[COPINDEX]);
        w[COPMPAC] = w[COPINDEX];
        set_a(s, w[GENMASK]);
        lanec_mask(s, W(IDLEMASK));
        return call(s, p, p->upent2, ITISMASK);
    case CHKREPT:                                      /* CA R1SAVE, INDEX COPINDEX, MASK BIT3, CCS A */
        if (w[COPINDEX] > 2u) return REFUSE;
        set_a(s, w[p->r1save]);
        lanec_mask(s, bit3[w[COPINDEX]]);
        if (lanec_ccs_a(s) == 0) return UNSETR1;
        k = lanec_ccs(s, w[CADRSTOR]);                 /* someone already in ENDIDLE? */
        return (k == 0 || k == 2u) ? (uint16_t)ISITPRIO : (uint16_t)GOIDLE;
    case GOIDLE: return call(s, p, p->endidle, IDLERET1);
    case IDLERET1: return TERMATE;
    case IDLERET2: return PROCEED;
    case IDLERET3:                                     /* CS LOWLOAD, AD MPAC, EXTEND, DIM A, EXTEND, BZF */
        cs_a(s, W(LOWLOAD));
        lanec_ad(s, w[MPAC]);
        if (!zero16(w[LANEC_A])) lanec_ad(s, (w[LANEC_A] & 0100000u) ? 1u : 077776u);
        return zero16(w[LANEC_A]) ? (uint16_t)LOADITIS : (uint16_t)OKTOENT;
    case OKTOENT: set_a(s, W(TWO)); return ENDOUT;
    case ENDOUT:                                       /* TS OUTHERE, CA FLAGWRD4, MASK OCT60000, CCS A */
        if (!lanec_ts(s, OUTHERE)) set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(OCT60000));
        k = lanec_ccs_a(s);
        return k == 1u ? (uint16_t)NORMRET : k == 2u ? (uint16_t)MARKRET : (uint16_t)TIMECHEK;
    case TIMECHEK:                                     /* CS TIME1, AD PRIOTIME, CCS A, COM, AD..., BZMF */
        cs_a(s, w[TIME1]);
        lanec_ad(s, w[p->priotime]);
        k = lanec_ccs_a(s);
        if (k == 0) w[LANEC_A] = (uint16_t)~w[LANEC_A];
        if (k <= 1u) lanec_ad(s, W(OCT37776));
        if (k <= 2u) lanec_ad(s, W(ONE));
        lanec_ad(s, W(_M_2SEC));
        if (zero16(w[LANEC_A]) || (w[LANEC_A] & 0100000u)) return leave(s, p->keepprio);
        return NORMRET;
    case NORMWAKE: set_a(s, W(ONE)); return leave(s, p->wakeplay);
    case ENDRET:                                       /* CCS OUTHERE, AD ONE, TCF +2, TCF ENDOFJOB */
        k = lanec_ccs(s, w[OUTHERE]);
        if (k == 2u) return leave(s, p->endofjob);
        if (k == 0) lanec_ad(s, W(ONE));
        return ENDRET4;
    case ENDRET4:                                      /* INDEX COPMPAC, AD CADRFLSH, TS MPAC +3 */
        if (w[COPMPAC] > 2u) return REFUSE;
        lanec_ad(s, w[CADRFLSH + w[COPMPAC]]);
        if (!lanec_ts(s, COPMPAC)) set_a(s, w[GENMASK]);   /* an overflow skips CA GENMASK */
        lanec_mask(s, W(PINIDMSK));
        return call(s, p, p->downent2, PINIDMSK);
    case BLANKALL: cs_a(s, W(THREE)); return call(s, p, p->nvsub, NVSUBRET);
    case NVSUBRET: return ENDIT;
    case ENDIT: set_a(s, w[USERPRIO]); lanec_mask(s, W(PRIO37)); return call(s, p, p->priochng, ENDIT3);
    case ENDIT3: set_a(s, w[COPMPAC]); return leave(s, p->bankjump);
    case UNSETR1:                                      /* INDEX COPINDEX, CS BIT3, MASK R1SAVE, TS R1SAVE */
        if (w[COPINDEX] > 2u) return REFUSE;
        cs_a(s, bit3[w[COPINDEX]]);
        lanec_mask(s, w[p->r1save]);
        (void)lanec_ts(s, p->r1save);
        w[LANEC_A] = 0;                                /* CAF ZERO, TC SUPERSW */
        return call(s, p, p->supersw, UNSETR16);
    case UNSETR16: set_a(s, W(THREE)); return IMMEDRET;
    case IMMEDRET: if (w[COPINDEX] > 2u) return REFUSE; lanec_ad(s, w[CADRFLSH + w[COPINDEX]]); return leave(s, p->bankjump);
    case TERMATE: w[LANEC_A] = 0; return ENDOUT;
    case PROCEED: set_a(s, W(ONE)); return ENDOUT;
    case MARKRET:                                      /* CS SIX, MASK FLAGWRD4, INHINT, TS FLAGWRD4, RELINT */
        cs_a(s, W(SIX));
        lanec_mask(s, w[FLAGWRD4]);
        (void)lanec_ts(s, FLAGWRD4);
        io->interrupt_enabled = 1;
        return ENDRET;
    case MARKOVER:                                     /* CAF MINUS1, TS OUTHERE, CA FLAGWRD4, MASK PRIO30, CCS A */
        set_a(s, W(MINUS1));
        (void)lanec_ts(s, OUTHERE);
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(PRIO30));
        return lanec_ccs_a(s) == 0 ? leave(s, p->normbnch) : (uint16_t)NORMRET;
    case NORMRET:                                      /* is MARK sleeping or waiting? */
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(BITS5_P_11));
        return lanec_ccs_a(s) == 0 ? leave(s, p->markwake) : (uint16_t)NORMRET4;
    case NORMRET4:                                     /* is NORMAL interrupted or waiting? */
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(BITS4_P_10));
        return lanec_ccs_a(s) == 0 ? (uint16_t)NORMWAKE : (uint16_t)NORMRET8;
    case NORMRET8:                                     /* a flash request or a GODSPRET? */
        set_a(s, w[p->ebanktem]);
        lanec_mask(s, W(OCT50));
        return lanec_ccs_a(s) == 0 ? (uint16_t)ENDRET : (uint16_t)NORMRET12;
    case NORMRET12:                                    /* CA NVSAVE, EXTEND, BZF ENDRET, CAF PRIO15, INHINT, TC NOVAC */
        set_a(s, w[NVSAVE]);
        if (zero16(w[LANEC_A])) return ENDRET;
        set_a(s, W(PRIO15));
        io->interrupt_enabled = 0;
        return call(s, p, p->novac, NOVACRET - 2u);    /* 2CADR PLAYJUM1 follows */
    case NOVACRET: return ENDRET;
    case MARSLEEP:
        set_a(s, w[FLAGWRD4]);
        lanec_mask(s, W(BITS5_P_11));
        return leave(s, lanec_ccs_a(s) == 0 ? p->endofjob : p->gosleeps);
    case LOADITIS:                                     /* INDEX COPMPAC, CA NVWORD, MASK LOW7, COM, AD MPAC +1 */
        if (w[COPMPAC] > 2u) return REFUSE;
        set_a(s, w[NVWORD + w[COPMPAC]]);
        lanec_mask(s, W(LOW7));
        w[LANEC_A] = (uint16_t)~w[LANEC_A];
        lanec_ad(s, w[MPAC + 1u]);
        return zero16(w[LANEC_A]) ? (uint16_t)OKTOENT : leave(s, p->pinbrnch);
    case ERASER: cs_a(s, W(THREE)); return call(s, p, p->nvsub, ERASER2);
    case ERASER2: case ERASER3: return leave(s, p->endofjob);
    default: return OUT;
    }
}

static uint8_t is_block(uint16_t at)
{
    switch (at) {
    case CLEANEND: case CLEANEND4: case ISITPRIO: case REST: case RESTSLEP: case RESTSLEP4: case FLASHSUB:
    case FLASHSUB1: case CHKREPT: case GOIDLE: case IDLERET1: case IDLERET2: case IDLERET3: case OKTOENT:
    case ENDOUT: case TIMECHEK: case NORMWAKE: case ENDRET: case ENDRET4: case BLANKALL: case NVSUBRET:
    case ENDIT: case ENDIT3: case UNSETR1: case UNSETR16: case IMMEDRET: case TERMATE: case PROCEED:
    case MARKRET: case MARKOVER: case NORMRET: case NORMRET4: case NORMRET8: case NORMRET12: case NOVACRET:
    case MARSLEEP: case LOADITIS: case ERASER: case ERASER2: case ERASER3:
        return 1;
    default:
        return 0;
    }
}

enum lanec_status lanec_di_idle_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p = at(m);
    uint16_t *w = s->words, z = w[LANEC_Z], at, i;
    /* the words the blocks write, kept to undo a refusal */
    uint16_t keep[7], where[7];
    uint16_t ie = io->interrupt_enabled;
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 010u) return LANEC_OUT_OF_SCOPE;
    if (z < p->cleanend) return LANEC_OUT_OF_SCOPE;
    at = (uint16_t)(z - p->cleanend);
    if (!is_block(at)) return LANEC_OUT_OF_SCOPE;
    where[0] = LANEC_A; where[1] = LANEC_Q; where[2] = LANEC_Z; where[3] = OUTHERE; where[4] = COPMPAC;
    where[5] = FLAGWRD4; where[6] = p->r1save;
    for (i = 0; i < 7u; ++i) keep[i] = w[where[i]];
    /* COPINDEX and COPMPAC index three-word tables (PRIOOCT order) and the
     * CADRFLSH and NVWORD cells: a block that indexes with a larger value
     * reads words this module does not hold, so the case is refused there,
     * with the image and I/O as they were on entry. */
    while (at != OUT && at != REFUSE) at = step(s, io, p, at);
    if (at == REFUSE) {
        for (i = 0; i < 7u; ++i) w[where[i]] = keep[i];
        io->interrupt_enabled = ie;
        return LANEC_OUT_OF_SCOPE;
    }
    return LANEC_OK;
}
