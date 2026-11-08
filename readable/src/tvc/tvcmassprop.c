/* SPDX-License-Identifier: MIT */
/* TVCMASSPROP.agc (Comanche055 pp. 951-955, bank 5): MASSPROP, the CSM's
 * mass properties for the TVC DAP's gains. "OUTPUTS": IXX, IAVG and
 * IAVG/TLX, "STORED IN CONSECUTIVE REGISTERS IXX0, IXX1, IXX2", each a
 * straight line in the CSM's mass from a breakpoint at NEGBPW's mass,
 * with one slope below and another above; and WEIGHT/G, the whole
 * vehicle's mass. MASSPROP works out the breakpoint values and slopes for
 * the LM docked or not, "IF LEM MASS OR CONFIGURATION HAS BEEN UPDATED",
 * and FIXCW the outputs from them ("RETURNS VIA Q"). The faithful C is
 * c64-port/src/cm_tvc_massprop.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- Words read and written under INDEX ---- */

/* The words in fixed memory MASSPROP reads under INDEX: the tables
 * NOLEMVAL, INTVALUE and SLOPEVAL (NOLEMVAL to SLOPEVAL +9). */
static bool a_constant(unsigned address)
{
    return address >= ADDRESS_OF(NOLEMVAL) && address <= ADDRESS_OF(SLOPEVAL) + 9u;
}

/* INDEX, then an order that reads the word at field: the field plus the
 * index, a 12-bit address. Not modelled (the faithful port refuses them
 * too): a sum outside 0-07777 or at Z, or a word of fixed memory outside
 * the tables. */
static unsigned read_field(unsigned field, unsigned index_address)
{
    int sum = (int)field + value_of(fetch(index_address));

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !a_constant((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* INDEX, then TS field: the field plus the index, an erasable address.
 * Not modelled (the faithful port refuses them too): a sum outside
 * 0-01777, or FB, Z or BB. */
static unsigned store_field(unsigned field, unsigned index_address)
{
    int sum = (int)field + value_of(fetch(index_address));

    if (sum < 0 || sum > 01777 || (sum >= (int)FBANK_ADDRESS && sum <= (int)BBANK_ADDRESS))
        not_modelled();
    return (unsigned)sum;
}

/* ---- MASSPROP -- pp. 952-953 ---- */

/* DXTEST, for DAPDATR1 negative ("IF NEG, BIT15 IS 1, LEM DSCNT STAGE
 * OFF"): VARST0 +8 and +9 added into VARST0 +1 and +2 (by DXCH, the
 * pair's old words to A,L, then DAS), and DXITFIX into VARST0 +7. With
 * DAPDATR1 -0, the CCS leaves +0 in A for the DAS (and L as it was);
 * positive or +0, on to FIXCW as it is. */
static void dxtest(void)
{
    enum sign configuration = ccs_at(ADDRESS_OF(DAPDATR1));

    if (configuration != NEGATIVE && configuration != IS_MINUS_ZERO) return;
    if (configuration == NEGATIVE) dxch(ADDRESS_OF(VARST0) + 8u);
    das(ADDRESS_OF(VARST0) + 1u);
    set_A(DXITFIX);
    ads(ADDRESS_OF(VARST0) + 7u);
}

/* MASSPROP: the breakpoint values and slopes, VARST0 to VARST0 +9, in
 * "TEN PASSES" from PHI333 9 down. "LEM NOT ATTACHED" (bit 13 of
 * DAPDATR1 set), each is NOLEMVAL's; "LEM IS ATTACHED", INTVALUE's plus
 * SLOPEVAL's times the LM's mass (LEMMASS, doubled; DDOUBL the product).
 * An overflow at the TS VARST0 skips the CCS PHI333, so the TCF MASSPROP
 * +1 stores the +1 or -1 it leaves in A as PHI333. "ARE ALL TEN PASSES
 * COMPLETED": PHI333 +0, DXTEST, then FIXCW; negative or -0, FIXCW (the
 * TCF FIXCW's at DXTEST +1 and +2). */
void massprop(void)
{
    enum sign passes_left;

    set_A(NINE); /* "SET UP TEN PASSES" */
    do {
        ts(ADDRESS_OF(PHI333)); /* MASSPROP +1 */
        ca(ADDRESS_OF(DAPDATR1));
        mask(BIT13);
        if (!A_is_zero()) { /* EXTEND; BZF LEMYES */
            /* LEMNO */ ca(read_field(ADDRESS_OF(NOLEMVAL), ADDRESS_OF(PHI333)));
            ts(store_field(ADDRESS_OF(VARST0), ADDRESS_OF(PHI333))); /* STOINST */
            passes_left = ccs_at(ADDRESS_OF(PHI333));
            continue;
        }
        /* LEMYES */ ca(ADDRESS_OF(LEMMASS));
        double_A();
        mp_at(read_field(ADDRESS_OF(SLOPEVAL), ADDRESS_OF(PHI333)));
        ddoubl();
        ad_at(read_field(ADDRESS_OF(INTVALUE), ADDRESS_OF(PHI333)));
        if (ts(store_field(ADDRESS_OF(VARST0), ADDRESS_OF(PHI333)))) /* STOINST */
            passes_left = POSITIVE;
        else
            passes_left = ccs_at(ADDRESS_OF(PHI333));
    } while (passes_left == POSITIVE); /* TCF MASSPROP +1 */
    if (passes_left == IS_PLUS_ZERO) dxtest();
    fixcw();
}

/* ---- FIXCW -- p. 953 ---- */

/* The bookkeeping after an output, CCS PSI333: positive, "NO - GO TAKE
 * CARE OF INDEXING REGISTERS" (false). Otherwise WEIGHT/G, the CSM's
 * mass plus (bit 14 of DAPDATR1 set, LEM on) the LM's: true, unless an
 * overflow at the TS WEIGHT/G skips the TC Q, on to BOKKEP2 with +1 or
 * -1 in A. (PSI333 negative takes its count less one to the MASK, -0 the
 * CCS's +0 to the CCS A.) */
static bool massprop_finished(void)
{
    switch (ccs_at(ADDRESS_OF(PSI333))) {
    case POSITIVE: return false;
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(DAPDATR1)); /* "UPDATE WEIGHT/G" */
        mask(BIT14);
        break;
    case NEGATIVE: mask(BIT14); break;
    case IS_MINUS_ZERO: break;
    }
    if (ccs_register(A_ADDRESS) == POSITIVE) ca(ADDRESS_OF(LEMMASS)); /* (A is BIT14 or +0) */
    ad_at(ADDRESS_OF(CSMMASS));
    return !ts(ADDRESS_OF(WEIGHT_G)); /* "SCALED AT B+16 KILOGRAMS" */
}

/* FIXCW, "COMPUTATION PHASE BEGINS HERE": "THREE PASSES" from PHI333 and
 * PSI333 2. TEMP333 the "DELTA CSM WEIGHT" (CSMMASS plus NEGBPW, doubled),
 * whose sign "DETERMINES" the "SLOPE LOCATIONS": PHI333 -2, the slopes
 * above the breakpoint, for a positive one. (An overflow at the TS TEMP333
 * skips the EXTEND, so the BZMF's word runs as AD PEGGY, whose sum the CAF
 * NEG2 replaces: the slopes above.) PEGGY, each pass: the output IXX +
 * PSI333, its breakpoint value plus the slope (VARST5 + PHI333) times
 * TEMP333, doubled. An overflow at the TS skips the CCS PSI333, on to
 * BOKKEP2 with +1 or -1 in A. BOKKEP2: PSI333 one less ("REDUCE PSI BY
 * ONE"), PHI333 one nearer +0 (DIM). Then ENDMASSP, TC Q. */
void fixcw(void)
{
    set_A(BIT2);
    ts(ADDRESS_OF(PHI333));
    ts(ADDRESS_OF(PSI333));
    ca(ADDRESS_OF(CSMMASS));
    ad(NEGBPW);
    double_A();
    if (ts(ADDRESS_OF(TEMP333)) || !A_is_zero_or_minus()) { /* EXTEND; BZMF PEGGY */
        set_A(NEG2);
        ts(ADDRESS_OF(PHI333));
    }
    for (;;) {
        /* PEGGY, "ALL IS READY - CALCULATE OUTPUTS NOW" */
        ca(read_field(ADDRESS_OF(VARST5), ADDRESS_OF(PHI333))); /* "GET SLOPE" */
        mp_at(ADDRESS_OF(TEMP333));
        double_A();
        ad_at(read_field(ADDRESS_OF(VARST0), ADDRESS_OF(PSI333))); /* "ADD BREAKPOINT VALUE" */
        if (!ts(store_field(ADDRESS_OF(IXX), ADDRESS_OF(PSI333))) && massprop_finished()) break;
        ts(ADDRESS_OF(PSI333)); /* BOKKEP2 */
        dim(ADDRESS_OF(PHI333));
    }
    tc_q(); /* ENDMASSP */
}

#endif
