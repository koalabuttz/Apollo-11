/* SPDX-License-Identifier: MIT */
/* The readable interpreter (src/interpreter/) as the flight program's
 * interpreter core, under the faithful core's names (c64-port/src/
 * lm_interp_core.c, which cm_interp_core.c builds for the CM):
 *
 *   <m>_interp_core        one pass of the interpreter, from the point Z
 *                          names to the end of an order pair;
 *   <m>_interp_core_basic  the points basic code reaches it at;
 *   <m>_interp_core_mct    its work clock.
 *
 * The readable build links these in their place.
 *
 * Plumbing only, point for point as the faithful core's run(): which point
 * Z names, the states the core takes, where a pass ends, and what each word
 * of interpretive code charges the work clock. Every instruction run is the
 * readable code's. */
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "interp_runtime.h"
#include "interp_word_mct.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"
#include "../interbank/interbank.h"
#include "../agc/names.h"

#if LUMINARY099
#define CORE       lm_interp_core
#define CORE_BASIC lm_interp_core_basic
#define CORE_MCT   lm_interp_core_mct
#define ROPE       0u
#else
#define CORE       cm_interp_core
#define CORE_BASIC cm_interp_core_basic
#define CORE_MCT   cm_interp_core_mct
#define ROPE       1u
#endif

/* RTB's return: BHIZ +1, TCF DANZIG, in bank 01. SWCALL left it in BUF2,
 * and the routine's TC Q reaches it through SWRETURN. */
#define RTB_RETURN ((word)(K_BHIZ + 1u))
#define BANK01     02000u

/* A pass runs at most this many points, as the faithful core's does. */
#define POINTS_PER_PASS 16u

/* ---- The work clock ----
 * Each word of interpretive code NEWOPS fetches adds its fitted charge
 * (c64-port/include/interp_word_mct.h), in units of 4 MCT. The program
 * starts the clock at 0x8000 less the step's budget: once bit 15 sets, the
 * pass fetches no more words, and the program can take a due interrupt. */
uint16_t CORE_MCT;

static void charge(unsigned units)
{
    CORE_MCT = (uint16_t)(CORE_MCT + (units << 2));
}

static bool work_due(void)
{
    return (CORE_MCT & 0x8000u) != 0u;
}

/* ---- Interpretive code in the listing's notation ----
 * While the interpreter runs a table of it (src/interpreter/notation.h,
 * interpret_listing() below), it reads the table's words where the table
 * lies, in place of the rope's. */
static const struct interpretive_code *listing;

/* Whether the pass is interpret_listing_lanec()'s, from the lanec
 * modules' states. */
static bool lanec;

/* Whether the pass is interpret_listing_any_newjob()'s, which takes any
 * NEWJOB. */
static bool any_newjob;

static bool listing_word(unsigned physical, word *out)
{
    unsigned first;

    if (!listing) return false;
    first = listing->bank * 1024u + listing->first - 02000u;
    if (physical < first || physical - first >= listing->count) return false;
    *out = listing->words[physical - first];
    return true;
}

/* ---- Fixed memory: the program's rope ---- */

static word rope_word(unsigned physical)
{
    agc_word value = 0;
    word from_listing;

    if (listing_word(physical, &from_listing)) return from_listing;
    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) return 0u;
    return (word)(value & MINUS_ZERO);
}

/* The word off words after LOC, in the interpretive program's bank (the
 * FB half of BANKSET). False where the rope has none. With the superbank
 * bit, FB 30-33 are banks 40-43: for the faithful core, only with the
 * superbank latch at 0100 itself; for the faithful port's compiled tables
 * (a pass over a table in the listing's notation, or the lanec modules'),
 * by the bit alone. */
static bool code_word(unsigned off, word *out)
{
    unsigned address = (LOC + off) & 077777u, bank, superbank = *agc.superbank & 0160u;
    agc_word value;

    if (address >= 04000u) {
        address = 2048u + (address & 07777u) - 04000u;
    } else {
        if (address < 02000u) return false;
        bank = (BANKSET >> 10) & 037u;
        if (bank >= 030u && (superbank & 0100u)) {
            if (superbank != 0100u && !lanec && !listing) return false;
            bank += 010u;
        }
        address = bank * 1024u + (address & 01777u);
    }
    if (listing_word(address, out)) return true;
    if (!apollo_rope_read(ROPE, (uint16_t)address, &value)) return false;
    *out = (word)(value & MINUS_ZERO);
    return true;
}

/* ---- The states the faithful core takes ---- */

/* The bank registers agree, ZERO is +0, NEWJOB is +0 or a core set, and
 * FIXLOC is one of the VAC areas (0401 + 054 k) or a NOVAC job's work area
 * (FAKEPRET: MPAC -36D). */
static bool as_expected(void)
{
    unsigned area;

    if (agc.memory[ZERO_ADDRESS] != 0u || (NEWJOB > 037777u && !any_newjob) || EBANK > 03400u ||
        (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (lanec) return true; /* (the caller checked the work area) */
    if (FIXLOC == ADDRESS_OF(MPAC) - 044u) return true;
    for (area = 0401u; area <= 0661u; area += 054u)
        if (FIXLOC == area) return true;
    return false;
}

/* The code word's left order code: NEWOPS's CCS leaves the word's
 * magnitude less one, whose low seven bits it is. */
static unsigned left_code(word pair)
{
    return (((~(unsigned)pair & 077777u) - 1u) & 0177u);
}

/* Whether the faithful core runs order code c from here: it refuses an
 * order whose address word is not in the rope, the push-up forms no
 * program assembles (SETPD, CGOTO and SSP with no address), and the two
 * address codes nothing uses (CCALL, and 035). The core checks these as it
 * goes; here they are checked before anything runs. */
static bool order_taken(unsigned c)
{
    word next;
    unsigned index;

    if (c == 0u) return true; /* EXIT */
    if (c & 1u) {             /* an address order */
        index = c >> 2;
        if (!code_word(1u, &next)) return false;
        if (!(c & 2u) && (next & 040000u) && (index == 037u || index == 004u || index == 011u))
            return false;
        if (index == 011u && !code_word(2u, &next)) return false;
        return index != 015u && index != 035u;
    }
    if (c & 2u) return code_word(1u, &next);                  /* a miscellaneous order */
    if (c & 4u) return true;                                  /* a short shift */
    if (((c >> 3) - 1u) == 014u) return code_word(1u, &next); /* STADR */
    return true;
}

/* Whether the pass ends after order code c, as the faithful core's: after
 * EXIT, RTB, CALL, RVQ and CGOTO, and after a branch taken or an alarm
 * (the order did not end at DANZIG). The loads, stores, arithmetic and
 * shifts go on. */
static bool pass_ends(unsigned c)
{
    bool at_danzig = Z == DANZIG || Z == NOIBNKSW;
    unsigned index;

    if (c == 0u) return true;            /* EXIT */
    if (c & 1u) return (c >> 2) == 004u; /* CGOTO */
    if (c & 2u) {
        index = c >> 3;
        if (index == 014u && !(c & 4u)) return true; /* RTB */
        if (index == 015u) return !(c & 4u);         /* CALL; ITA goes on */
        return !at_danzig;
    }
    if (c & 4u) return false; /* a short shift */
    index = (c >> 3) - 1u;
    if (index < 014u) return !at_danzig;
    return index == 015u; /* RVQ; STADR and PUSH go on */
}

/* Q as the flight program's other routines take it. SWITCHES leaves the
 * switch word in Q with TS Q, which on the AGC copies all 16 bits of A (a
 * negative word's sign twice); the faithful core leaves the word's 15
 * bits, and the routines after it take Q so. After a pass that ran a
 * switch order, where the two signs agree, Q is left as the faithful core
 * leaves it. */
static bool switched;

static void note_order(unsigned c)
{
    if ((c & 3u) == 2u && (c >> 3) == 016u) switched = true; /* SW/ */
}

static void q_as_fifteen_bits(void)
{
    if (switched && ((Q >> 15) & 1u) == ((Q >> 14) & 1u)) Q &= 077777u;
}

/* One pass: point after point until the pair's end, an order that ends
 * the pass, or a Z no point names. */
static enum exec_status run(void)
{
    unsigned guard, c, index;
    bool more = true;
    word w;

    for (guard = 0u; guard < POINTS_PER_PASS && more; ++guard) {
        word z = Z;

        if (z == NEWOPS && !work_due()) {
            if (!code_word(0u, &w)) return EXEC_OUT_OF_SCOPE;
            if (!(w & 040000u)) { /* a store code */
                charge(INTERP_WORD_STORE(w));
                newops();
                continue;
            }
            c = left_code(w);
            if (!order_taken(c)) return EXEC_OUT_OF_SCOPE;
            note_order(c);
            newops();
            charge(interp_word_units_of(w));
            more = !pass_ends(c);
        } else if (z == DANZIG || z == NOIBNKSW) {
            if (EDOP == 0u) { /* the pair is done: CHANG2 or the next word */
                if (z == DANZIG)
                    danzig();
                else
                    noibnksw();
                return EXEC_OK;
            }
            c = EDOP - 1u;
            if (!order_taken(c)) return EXEC_OUT_OF_SCOPE;
            note_order(c);
            if (z == DANZIG)
                danzig();
            else
                noibnksw();
            more = !pass_ends(c);
        } else if (z == DODLOAD) {
            dodload();
        } else if (z == DOVLOAD) {
            dovload();
        } else if (z == DIRADRES) {
            /* CYR holds the address order's code cycled right: 20000 + its
             * INDJUMP index. */
            index = agc.memory[CYR_ADDRESS] & 037u;
            if ((agc.memory[CYR_ADDRESS] & 077740u) != 020000u || !code_word(1u, &w))
                return EXEC_OUT_OF_SCOPE;
            if ((w & 040000u) && (index == 037u || index == 004u || index == 011u))
                return EXEC_OUT_OF_SCOPE;
            if (index == 015u || index == 035u) return EXEC_OUT_OF_SCOPE;
            if (index == 011u && !code_word(2u, &w)) return EXEC_OUT_OF_SCOPE;
            diradres();
            more = !pass_ends((index << 2) | 1u);
        } else if (z == DODLOAD_STAR || z == DOVLOAD_STAR) {
            if (!code_word(1u, &w)) return EXEC_OUT_OF_SCOPE;
            if (z == DODLOAD_STAR)
                dodload_star();
            else
                dovload_star();
        } else if (z == FIFTEEN_BITADR) {
            /* CYR holds the miscellaneous order's MISCJUMP index, plus
             * 40000 for its code's bit 3. */
            if ((agc.memory[CYR_ADDRESS] & 017760u) || !code_word(1u, &w)) return EXEC_OUT_OF_SCOPE;
            c = ((agc.memory[CYR_ADDRESS] & 017u) << 3) | 2u |
                (agc.memory[CYR_ADDRESS] & 040000u ? 4u : 0u);
            note_order(c);
            fifteen_bitadr();
            more = !pass_ends(c);
        } else if (z == GOTO) {
            goto_();
            return EXEC_OK;
        } else if (z == SWRETURN) {
            swreturn();
        } else if (z == INTPRET) {
            intpret();
            return EXEC_OK;
        } else if (z == RTB_RETURN && (FBANK & 076000u) == BANK01) {
            bhiz_plus1();
        } else {
            return guard ? EXEC_OK : EXEC_UNMAPPED; /* outside the interpreter */
        }
    }
    return EXEC_OK;
}

static enum exec_status entry(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    /* (The interpreter reads no program tables: callers may pass none.) */
    static const agc_native_program none;
    enum exec_status status;

    if (!bind_machine(program ? program : &none, state, device)) return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = rope_word;
    if (!as_expected()) return EXEC_OUT_OF_SCOPE;
    switched = false;
    status = run();
    if (!lanec) q_as_fifteen_bits();
    return status;
}

const agc_routine CORE = entry;

/* One pass over a table of interpretive code: the interpreter reads the
 * table's words, and the work clock is left as it was (the routines of
 * the program's tables, which this runs as, keep no clock). */
enum exec_status interpret_listing(const agc_native_program *program, apollo_overlay_state *state,
                                   void *device, const struct interpretive_code *code)
{
    uint16_t clock = CORE_MCT;
    enum exec_status status;

    listing = code;
    CORE_MCT = 0u;
    status = entry(program, state, device);
    CORE_MCT = clock;
    listing = NULL;
    return status;
}

enum exec_status interpret_listing_lanec(const agc_native_program *program,
                                         apollo_overlay_state *state, void *device,
                                         const struct interpretive_code *code)
{
    enum exec_status status;

    lanec = true;
    status = interpret_listing(program, state, device, code);
    lanec = false;
    return status;
}

enum exec_status interpret_listing_any_newjob(const agc_native_program *program,
                                              apollo_overlay_state *state, void *device,
                                              const struct interpretive_code *code)
{
    enum exec_status status;

    any_newjob = true;
    status = interpret_listing(program, state, device, code);
    any_newjob = false;
    return status;
}

/* The points basic code reaches the interpreter at, by physical address:
 * RTB's return (bank 01), SWRETURN, INTPRET and DANZIG (fixed-fixed). */
static const agc_routine_entry basic[] = {
    {B1(RTB_RETURN), entry},
    {FF(SWRETURN), entry},
    {FF(INTPRET), entry},
    {FF(DANZIG), entry},
};
const agc_routine_table CORE_BASIC = {basic, COUNT(basic)};
