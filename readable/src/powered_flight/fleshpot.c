/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc, CALCSMSC and FLESHPOT (LM pp.
 * 1265-1267), bank 10: the LM's alone.
 *
 * "ROUTINE FLESHPOT COMPUTES THE BODY-STABLE MEMBER TRANSFORMATION MATRIX
 * (COMMONLY CALLED XNB) AND STORES IT IN THE LOCATIONS SPECIFIED BY THE
 * ECADR ENTERING IN A." The matrix's rows are the navigation base's axes
 * in stable member coordinates, from the CDU angles' sines and cosines
 * at SINCDU and COSCDU (Y, Z, X): the X axis (words 0, 2 and 4) and the
 * Z axis (14, 16 and 20) by DMP and DMPSUB, the Y axis (6, 10 and 12) as
 * their cross product, by the interpreter's VXV, which FLESHPOT enters
 * from basic code and leaves by an EXIT it puts in EDOP. CALCSMSC, called
 * in interpretive, makes the matrix at XNB.
 *
 * The run leaves the file at BANKCALL, at INTPRET, at TCF VXV and at TCF
 * SWRETURN; the LM runs DMP and DMPSUB in line, as the faithful C does
 * (c64-port/src/lm_powered_subs.c). */
#include "powered_flight.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- CALCSMSC -- LM p. 1265 ---- */

/* CALCSMSC: out of the interpreter to FLESHPOT -1 by BANKCALL, and back
 * in to return (RVQ). */
static const word calcsmsc_words[] = {/* CALCSMSC */ ORDER(EXIT)};
static const word calcsmsc_rvq_words[] = {ORDER(RVQ)};

const struct interpretive_code CALCSMSC_CODE = {CODE_BANK(I_CALCSMSC), CODE_Z(I_CALCSMSC),
                                                COUNT(calcsmsc_words), calcsmsc_words};
const struct interpretive_code CALCSMSC_RVQ_CODE = {CODE_BANK(I_CALCSMSC),
                                                    (word)(CODE_Z(I_CALCSMSC) + 4u),
                                                    COUNT(calcsmsc_rvq_words), calcsmsc_rvq_words};

/* After CALCSMSC's EXIT: TC BANKCALL, CADR FLESHPOT -1. */
void calcsmsc_plus1(void)
{
    tc_at(CALCSMSC + 1u);
    continue_at(BANKCALL);
}

/* FLESHPOT's return: back into the interpreter, to the RVQ. */
void calcsmsc_plus3(void)
{
    tc_at(CALCSMSC + 3u);
    intpret();
}

/* ---- FLESHPOT -- LM pp. 1265-1267 ---- */

/* INDEX TEM1, DXCH k: the matrix's word k (and k +1). */
static void store_matrix(word k)
{
    dxch(indexed_write(k, TEM1));
}

/* TC DMP at the word at, with the operand's address in the word after it: MPAC
 * times it, run in line. */
static void tc_dmp(word at)
{
    tc_at(at);
    dmp();
}

/* TC DMPSUB at the word at: MPAC times the double word at ADDRWD, which the last
 * DMP left there, run in line. */
static void tc_dmpsub(word at)
{
    tc_at(at);
    dmpsub();
}

/* FLESHPOT -1: CALCSMSC's ECADR, XNB's. */
void fleshpot_minus1(void)
{
    set_A(XNBECADR);
    fleshpot();
}

/* FLESHPOT: the ECADR into EBANK and the user's EBANK into TEM2 (a TS of
 * an overflowing A skips the XCH EBANK); the matrix's address in the
 * E-bank's window into TEM1. Then the X axis, "= COSY COSZ", "= SINZ" and
 * "= - SINY COSZ", and the Z axis into MPAC, MPAC +3 and MPAC +5: "= -
 * SINY COSX + SINX SINZ COSY", "= - SINX COSZ" and "= COSX COSY - SINX
 * SINY SINZ", each product doubled (twice for the triple ones) to its
 * scale. The Z axis goes to the matrix too, and VXV makes the Y axis, the
 * Z axis times the X axis at ADDRWD; its EXIT comes back at the word after
 * TCF VXV. */
void fleshpot(void)
{
    if (!ts(ADDRESS_OF(TEM2))) xch(EBANK_ADDRESS);
    xch(ADDRESS_OF(TEM2));
    mask(LOW8);
    ad(OCT1400);
    ts(ADDRESS_OF(TEM1));

    /* The X axis. */
    dca(ADDRESS_OF(COSCDUY));
    dxch(ADDRESS_OF(MPAC));
    tc_dmp(FLESHPOT + 011u); /* ADRES COSCDUZ */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    store_matrix(0u); /* "= COSY COSZ" */
    dca(ADDRESS_OF(SINCDUZ));
    store_matrix(2u); /* "= SINZ" */
    dcs(ADDRESS_OF(SINCDUY));
    dxch(ADDRESS_OF(MPAC));
    tc_dmpsub(FLESHPOT + 026u); /* "ADDRWD SET TO COSCDUZ" */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    store_matrix(4u); /* "= - SINY COSZ" */

    /* The Z axis: -SINX COSZ, doubled, into MPAC +3; -SINX SINZ, and
     * SINX SINZ kept in MPAC +5. */
    dcs(ADDRESS_OF(SINCDUX));
    dxch(ADDRESS_OF(MPAC));
    tc_dmpsub(FLESHPOT + 036u); /* "ADDRWD SET TO COSCDUZ STILL" */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    dxch(MPAC_AT(3u));
    dcs(ADDRESS_OF(SINCDUX));
    dxch(ADDRESS_OF(MPAC));
    tc_dmp(FLESHPOT + 045u); /* ADRES SINCDUZ */
    dcs(ADDRESS_OF(MPAC));
    dxch(MPAC_AT(5u));
    /* -SINX SINZ times SINY, doubled twice, into MPAC +5, and SINX SINZ
     * back into MPAC; it times COSY, doubled twice, into BUF. */
    tc_dmp(FLESHPOT + 052u); /* ADRES SINCDUY */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    ddoubl();
    dxch(MPAC_AT(5u));
    dxch(ADDRESS_OF(MPAC));
    tc_dmp(FLESHPOT + 061u); /* ADRES COSCDUY */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    ddoubl();
    dxch(BUF_AT(0u));
    /* COSY COSX, doubled, onto MPAC +5; -SINY COSX, doubled, onto BUF,
     * and BUF into MPAC. */
    dca(ADDRESS_OF(COSCDUY));
    dxch(ADDRESS_OF(MPAC));
    tc_dmp(FLESHPOT + 072u); /* ADRES COSCDUX */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    das(MPAC_AT(5u));
    dca(ADDRESS_OF(SINCDUY));
    dxch(ADDRESS_OF(MPAC));
    tc_dmpsub(FLESHPOT + 0102u); /* "ADDRWD SET TO COSCDUX" */
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    das(BUF_AT(0u));
    dxch(BUF_AT(0u));
    dxch(ADDRESS_OF(MPAC));

    /* The Z axis to the matrix. */
    dca(ADDRESS_OF(MPAC));
    store_matrix(014u); /* "= - SINY COSX + SINX SINZ COSY" */
    dca(MPAC_AT(3u));
    store_matrix(016u); /* "= - SINX COSZ" */
    dca(MPAC_AT(5u));
    store_matrix(020u); /* "= COSX COSY - SINX SINY SINZ" */

    /* VXV, the X axis at ADDRWD, with LOC the word after TCF VXV (DCA Z
     * reads the next word's address, and BBANK into BANKSET) and an EXIT
     * in EDOP. */
    ca(ADDRESS_OF(TEM1));
    ts(ADDRESS_OF(ADDRWD));
    continue_at(FLESHPOT + 0130u);
    dca(Z_ADDRESS);
    ad(FOUR);
    dxch(ADDRESS_OF(LOC));
    set_A(BIT8);
    ts(ADDRESS_OF(EDOP));
    continue_at(VXV);
}

/* After VXV's EXIT: the Y axis, doubled, to the matrix; the user's EBANK
 * back, and SWRETURN. */
void fleshpot_after_vxv(void)
{
    dxch(ADDRESS_OF(MPAC));
    ddoubl();
    store_matrix(6u);
    dxch(MPAC_AT(3u));
    ddoubl();
    store_matrix(010u);
    dxch(MPAC_AT(5u));
    ddoubl();
    store_matrix(012u);
    ca(ADDRESS_OF(TEM2));
    ts(EBANK_ADDRESS);
    continue_at(SWRETURN);
}

#endif
