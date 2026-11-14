/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, AX*SR*T (LM pp. 1263-1265, CM pp.
 * 1369-1371), bank 23.
 *
 * "AX*SR*T COMBINES THE OLD SMNB AND NBSM. FOR THE NB TO SM
 * TRANSFORMATION, ENTER WITH +3 IN A. FOR SM TO NB, ENTER WITH -3. THE
 * VECTOR TO BE TRANSFORMED ARRIVES, AND IS RETURNED, IN VBUF. AX*SR*T
 * EXPECTS TO FIND THE SINES AND COSINES OF THE ANGLES OF ROTATION AT
 * SINCDU AND COSCDU, IN THE ORDER Y Z X."
 *
 * Three rotations, one about each axis: from NB to SM about X, Z, then Y;
 * from SM to NB the other way round, each angle's sine negated. DEXDEX
 * counts them (+3 to +1, or -3 to -1), and picks from INDEXI the two
 * components each turns (DEX1 and DEX2: Z and Y, Y and X, or X and Z, as
 * offsets in VBUF), the angle's sine and cosine at the first one's offset.
 * Each of the two is the other times the sine, plus itself times the
 * cosine; LOOP1 makes one, then LOOP2 the other from the first's old value
 * (BUF holds it, and the index, 1 then 0, that picks which of DEX2 and
 * DEX1 it goes to). "AX*SR*T IS GUARANTEED SAFE ONLY FOR VECTORS OF
 * MAGNITUDE LESS THAN UNITY."
 *
 * The faithful C is c64-port/src/lm_powered_subs.c and cm_powered_subs.c:
 * the LM's runs DMPSUB in line, the CM's leaves the file at it, and the
 * return is a place of its own (loop1_after_sine_product() and
 * loop1_after_cosine_product()). */
#include "powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* ---- The constants -- LM pp. 1264-1265 (CM p. 1371) ---- */

/* SINESLOC ("FOR USE IN SETTING ADDRWD"), and INDEXI, the components'
 * offsets, which the listing's comments, a word to a line, say not to
 * touch. */
static const word constants[] = {
    ADDRESS(SINCDU), /* SINESLOC */
    NUMBER(4),       /* INDEXI */
    NUMBER(2),       /* INDEXI +1 */
    NUMBER(0),       /* INDEXI +2 */
    NUMBER(4),       /* INDEXI +3 */
};

const struct interpretive_code AX_STAR_SR_STAR_T_CONSTANTS = {
    CODE_BANK(I_CDUTRIG), (word)(SINESLOC), COUNT(constants), constants};

/* ---- The rotations -- LM pp. 1263-1264 (CM pp. 1369-1371) ---- */

static void r_star_tl_star_star_p(void);
static void loop1(void);

/* TC DMPSUB at the word at: MPAC times the double word at ADDRWD. In the
 * LM the run goes on through DMPSUB, then into then(); in the CM it leaves
 * the file. */
static void tc_dmpsub(word at, void (*then)(void))
{
    tc_at(at);
#if LUMINARY099
    dmpsub();
    then();
#else
    (void)then;
    continue_at(DMPSUB);
#endif
}

/* AX*SR*T -- LM p. 1263, CM p. 1369: +3 or -3 into DEXDEX ("WHERE IT BECOMES
 * THE INDEX OF INDEXES"), the return into RTNSAVER. */
void ax_star_sr_star_t(void)
{
    ts(ADDRESS_OF(DEXDEX));
    qxch(ADDRESS_OF(RTNSAVER));
    r_star_tl_star_star_p();
}

/* R*TL**P -- LM p. 1263, CM p. 1369: one rotation. Which INDEXI pair (DEXDEX
 * +3, +2, +1 to 0, 1, 2; -3, -2, -1 to 2, 1, 0), into DEX1 and DEX2; BUF the
 * index 1; and the first component, negated ("REALLY BE A SUBTRACT, AND VICE
 * VERSA"). DEXDEX is never zero here; +0 would take the pair at INDEXI +3,
 * and at -0 the DCA INDEXI, with no EXTEND, is CA INDEXI +1. */
static void r_star_tl_star_star_p(void)
{
    switch (ccs_at(ADDRESS_OF(DEXDEX))) {
    case POSITIVE: cs(ADDRESS_OF(DEXDEX));    /* fall through */
    case IS_PLUS_ZERO: /* never */ ad(THREE); /* fall through */
    case NEGATIVE: /* CCS leaves A |DEXDEX| - 1 */ dca(indexed_read(INDEXI, a_word(), 2u)); break;
    case IS_MINUS_ZERO: ca(INDEXI + 1u); break; /* never */
    }
    dxch(ADDRESS_OF(DEXI));
    set_A(ONE);
    ts(BUF_AT(0u));
    dcs(indexed_read(VBUF_AT(0u), DEX1, 2u));
    loop1();
}

/* LOOP2 -- LM p. 1263, CM p. 1370: "LOADING VECTOR COMPONENT, STORING
 * INDEX", then LOOP1. */
static void loop2(void)
{
    dxch(BUF_AT(0u));
    loop1();
}

/* LOOP1 -- LM p. 1264, CM p. 1370: the component times the angle's sine:
 * ADDRWD at SINESLOC (the LM's listing spells it SINSLOC) plus DEX1. */
static void loop1(void)
{
    dxch(ADDRESS_OF(MPAC));
    ca(SINESLOC);
    ad_at(ADDRESS_OF(DEX1));
    ts(ADDRESS_OF(ADDRWD));
    tc_dmpsub(LOOP1 + 4u, loop1_after_sine_product); /* "MULTIPLY AT SIN(CDUANGLE)" */
}

/* INDEX BUF, INDEX DEX1: the offset in VBUF of the component BUF's index
 * (1 or 0) picks, DEX2's or DEX1's. Not modelled: an index that puts the
 * word INDEX DEX1 reads outside erasable memory, or at the editing
 * registers. */
static word picked_offset(void)
{
    unsigned at = indexed(ADDRESS_OF(DEX1), BUF(0));

    if (at < 010u || at > 01777u || (at >= 020u && at <= 023u)) not_modelled();
    return fetch(at);
}

/* LOOP1 +5 -- LM p. 1264, CM p. 1370: After the sine: the product into
 * TERM1TMP ("NBSM CASE"), or negated ("SMNB CASE"); then ADDRWD six on, at
 * the cosine ("SINCDU AND COSCDU (EACH 6 WORDS) MUST BE CONSECUTIVE AND IN
 * THAT ORDER"), and the other component times it. DEXDEX is never zero
 * here; +0 would go on at the DXCH TERM1TMP, and at -0 the DCS MPAC, with
 * no EXTEND, is CS MPAC +1. */
void loop1_after_sine_product(void)
{
    switch (ccs_at(ADDRESS_OF(DEXDEX))) {
    case POSITIVE: dxch(ADDRESS_OF(MPAC)); break;         /* "NBSM CASE"; TCF +3 */
    case IS_PLUS_ZERO: break;                             /* (never) TCF +3 */
    case NEGATIVE: dcs(ADDRESS_OF(MPAC)); break;          /* "SMNB CASE" */
    case IS_MINUS_ZERO: cs(ADDRESS_OF(MPAC) + 1u); break; /* never */
    }
    dxch(ADDRESS_OF(TERM1TMP));
    set_A(SIX);
    ads(ADDRESS_OF(ADDRWD));
    dca(indexed_read(VBUF_AT(0u), picked_offset(), 2u));
    dxch(ADDRESS_OF(MPAC));
    tc_dmpsub(LOOP1 + 022u, loop1_after_cosine_product); /* "MULTIPLY BY COS(CDUANGLE)" */
}

static void tstpoint(void);

/* LOOP1 +023 -- LM p. 1264, CM p. 1370: After the cosine: the sum of the two
 * products, doubled, is the component's new value; its old value goes to BUF
 * ("LOADING INDEX, STORING VECTOR COMPONENT"), and the index to A. Index 1:
 * the other component (LOOP2); 0: the rotation is done ("DECREMENT MAGNITUDE
 * PRESERVING SIGN"). BUF never holds a negative index; one would skip the
 * EXTEND, and the DIM DEXDEX is then ADS DEXDEX; -0 skips that too. */
void loop1_after_cosine_product(void)
{
    unsigned component;

    dxch(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(TERM1TMP));
    dxch(ADDRESS_OF(TERM1TMP));
    ddoubl();
    component = indexed_write(VBUF_AT(0u), picked_offset());
    dxch(component);
    dxch(BUF_AT(0u));
    switch (ccs_at(A_ADDRESS)) { /* "'CAUSE THAT'S WHERE THE INDEX NOW IS" */
    case POSITIVE: loop2(); return;
    case IS_PLUS_ZERO:
        dim(ADDRESS_OF(DEXDEX));
        tstpoint();
        return;
    case NEGATIVE: /* never */
        ads(ADDRESS_OF(DEXDEX));
        tstpoint();
        return;
    case IS_MINUS_ZERO: /* never */ tstpoint(); return;
    }
}

/* TSTPOINT -- LM p. 1264, CM p. 1371: the next rotation, or (DEXDEX -0: DIM
 * of +1 or -1 leaves -0) back by RTNSAVER ("ONLY THE BRANCHING FUNCTION IS
 * USED"). Not modelled: a return that is not an address in the program. */
static void tstpoint(void)
{
    word to;

    switch (ccs_at(ADDRESS_OF(DEXDEX))) {
    case POSITIVE: r_star_tl_star_star_p(); return;
    case IS_PLUS_ZERO: break; /* TC RTNSAVER */
    case NEGATIVE: r_star_tl_star_star_p(); return;
    case IS_MINUS_ZERO: break; /* TC RTNSAVER */
    }
    to = fetch(ADDRESS_OF(RTNSAVER));
    if (to < 010u || to > 07777u) not_modelled();
    tc_through(ADDRESS_OF(RTNSAVER));
}
