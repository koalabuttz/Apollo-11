/* SPDX-License-Identifier: MIT */
/* P34-35_P74-75.agc, part 1 (Luminary099 pp. 682-683, Comanche055 pp.
 * 483-484): the display subroutines the rendezvous programs share.
 * DISPLAYE shows the elevation angle and central angle (verb 06 noun 55);
 * VNPOOH flashes the verb and noun in A. Both come back to the caller on a
 * proceed, and end the program (GOTOPOOH) on a terminate. P3XORP7X
 * tells the active vehicle's programs (P3x) from the passive vehicle's
 * (P7x). The faithful C is c64-port/src/lm_p34_p35.c and cm_p34_p35.c.
 *
 * DISPLAYE and P3XORP7X are in bank 35 in both missions. The LM's VNPOOH
 * is there too; the CM's is in fixed-fixed memory (bank 2), and saves
 * and restores the caller's bank itself: "THIS ROUTINE MUST REMAIN IN
 * FIXED-FIXED". Each flash goes through BANKCALL to GOFLASH, which comes
 * back to the word after its CADR: there to terminate, one on to
 * proceed, two on to recycle (flash again). */
#include "p34_p35.h"
#include "../agc/names.h"

/* TC Q: the routine returns to the address in Q (bits 1-12), and Q
 * becomes 3. Not modelled: a Q that is not such an address (bits 13-15
 * set, or below 010). */
static void tc_q(void)
{
    word at = (word)(Q & 07777u);

    if ((Q & 070000u) || Q < 010u) not_modelled();
    Q = 3u;
    Z = at;
}

/* TC to an erasable word that holds a return (NORMEX, RTRN): the word
 * runs as TC <address>. Not modelled: a word that is not such an
 * address, as for TC Q. */
static void tc_return_in(unsigned address)
{
    word to = fetch(address);

    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_through(address);
}

/* TCR BANKCALL at at, with CADR GOFLASH after it: the flash. */
static void goflash(word at)
{
    tc_at(at);
    continue_at(BANKCALL);
}

/* ---- DISPLAYE -- LM p. 682, CM p. 483 ---- */

/* DISPLAYE +2: verb 06 noun 55 flashed (BANKCALL, CADR GOFLASH). */
static void displaye_flash(void)
{
    set_A(V06N55);
    goflash(DISPLAYE + 3u);
}

/* DISPLAYE: the return to NORMEX (QXCH), and the display. */
void displaye(void)
{
    qxch(ADDRESS_OF(NORMEX)); /* EXTEND; QXCH */
    displaye_flash();
}

/* GOFLASH's returns: terminate (GOTOPOOH), */
void displaye_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* proceed (TC NORMEX: back to the caller), */
void displaye_proceed(void)
{
    tc_at(DISPLAYE + 6u);
    tc_return_in(ADDRESS_OF(NORMEX));
}

/* and recycle (TCF -5): the display again. */
void displaye_recycle(void)
{
    displaye_flash();
}

/* ---- P3XORP7X -- LM p. 683, CM p. 484 ---- */

/* P3XORP7X: the major mode's bits above the sixth (HIGH9 of MODREG) are
 * zero in P34 and P35, and not in P74 and P75: P3x returns to Q, P7x to
 * Q + 1. */
void p3xorp7x(void)
{
    set_A(HIGH9);
    mask(MODREG);
    if (!A_is_zero()) incr(Q_ADDRESS); /* EXTEND; BZF +2 */
    tc_q();                            /* RETURN */
}

/* ---- VNPOOH -- LM p. 683, CM p. 484 ---- */

#if LUMINARY099
/* VNPOOH: the return to RTRN (QXCH), and the verb and noun in A to
 * VERBNOUN, and flashed (BANKCALL, CADR GOFLASH). TS skips the CA after
 * it on an overflow. */
void vnpooh(void)
{
    qxch(ADDRESS_OF(RTRN)); /* EXTEND; QXCH */
    if (!ts(ADDRESS_OF(VERBNOUN))) ca(ADDRESS_OF(VERBNOUN));
    goflash(VNPOOH + 4u);
}

/* GOFLASH's returns: terminate (GOTOPOOH), */
void vnpooh_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* proceed (TC RTRN: back to the caller), */
void vnpooh_proceed(void)
{
    tc_at(VNPOOH + 7u);
    tc_return_in(ADDRESS_OF(RTRN));
}

/* and recycle (TCF -5): VERBNOUN flashed again. */
void vnpooh_recycle(void)
{
    ca(ADDRESS_OF(VERBNOUN));
    goflash(VNPOOH + 4u);
}
#else
/* VNPOOH: the return to RTRN (QXCH), and the verb and noun in A to
 * VERBNOUN. Then bank 2 selected, the word at VNBANK (TC -5) being one
 * of its addresses, and the caller's bank kept in TBASE5; and VERBNOUN
 * flashed (BANKCALL, CADR GOFLASH). The first TS skips the CAF VNBANK on
 * an overflow, and FB is then XCH'd with A as it is. */
void vnpooh(void)
{
    qxch(ADDRESS_OF(RTRN));                    /* EXTEND; QXCH */
    if (!ts(ADDRESS_OF(VERBNOUN))) ca(VNBANK); /* CAF VNBANK */
    xch(FBANK_ADDRESS);
    ts(ADDRESS_OF(TBASE5)); /* the old FB: no overflow */
    ca(ADDRESS_OF(VERBNOUN));
    goflash(VNPOOH + 7u);
}

/* GOFLASH's returns: terminate (GOTOPOOH), */
void vnpooh_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* VNPOOH +014: the caller's bank back (TBASE5 to FBANK), and back to the
 * caller (TC RTRN). */
void vnpooh_restore(void)
{
    ca(ADDRESS_OF(TBASE5));
    ts(FBANK_ADDRESS);
    tc_at(VNPOOH + 016u);
    tc_return_in(ADDRESS_OF(RTRN));
}

/* proceed (TCF +2, to the bank's restoring), */
void vnpooh_proceed(void)
{
    vnpooh_restore();
}

/* and recycle (VNBANK, TC -5, whose return the TCR BANKCALL replaces):
 * VERBNOUN flashed again. */
void vnpooh_recycle(void)
{
    tc_at(VNBANK);
    ca(ADDRESS_OF(VERBNOUN));
    goflash(VNPOOH + 7u);
}
#endif
