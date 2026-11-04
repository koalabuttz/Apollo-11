/* SPDX-License-Identifier: MIT */
/* BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, pp. 750-751 (Luminary099
 * only): KILLTASK, by Covelli, "USED TO REMOVE A TASK FROM THE WAITLIST BY
 * SUBSTITUTING A NULL TASK". The faithful C is c64-port/src/lm_burn_baby.c.
 *
 * The caller's TC KILLTASK is followed by the task's CADR and returns to
 * the word after it ("L+2"), with interrupts still inhibited. KILLTASK
 * itself is in fixed-fixed and goes on in bank 27 (KILLTSK2), which scans
 * the waitlist's 2CADRs, LST2 +0 to +17, for the first with the task's
 * address and bank, and puts TCTSKOVR (a TC TASKOVER) in its place. */
#include "burn_baby.h"
#include "../agc/names.h"

#if LUMINARY099

/* INDEX L; ... LST2: the list's word L. Not modelled: an L past 16, which
 * a DTCB from DEAD into this code (the caller's BBANK in L) can leave. */
static word list_word(word offset)
{
    if (L > 16u) not_modelled();
    return (word)(ADDRESS_OF(LST2) + offset + L);
}

/* KILLTASK: "MUST BE IN FIXED-FIXED". INDEX Q; CA 0 reads the CADR after
 * the caller's TC ("GET CADR"), and LXCH BBANK switches to bank 27 with
 * the caller's BBANK in L. Not modelled: a Q that is not a fixed address. */
void killtask(void)
{
    set_A(KILLBB);
    inhint();
    lxch(A_ADDRESS);
    if (Q < 02000u || Q > 07777u) not_modelled();
    set_A(read_word(Q));
    lxch(BBANK_ADDRESS);
    killtsk2();
}

/* KILLTSK2: "SAVE CALLER'S BBANK" and "RETURN 2CADR IN ITEMP1,ITEMP2" (Q
 * + 1, past the CADR). The CADR splits into the task's GENADR (its low
 * ten bits in the 02000 window, AD BIT11) and its FBANK. Not modelled: a
 * CADR with overflow in A (KILLTASK's CA leaves none), which the TS would
 * skip on. */
void killtsk2(void)
{
    lxch(ADDRESS_OF(ITEMP2));
    incr(Q_ADDRESS);
    qxch(ADDRESS_OF(ITEMP1));
    if (ts(ADDRESS_OF(ITEMP3))) not_modelled(); /* "CADR IS IN A" */
    mask(LOW10);
    ad(BIT11);
    ts(ADDRESS_OF(ITEMP4)); /* "GENADR OF TASK" */
    set_A(negate(LOW10));
    mask_at(ADDRESS_OF(ITEMP3));
    ts(ADDRESS_OF(ITEMP3)); /* "FBANK OF TASK" */
    zl();
    adrscan();
}

/* ADRSCAN: "COMPARE GENADRS" of the 2CADR at LST2 +L; "IF THEY MATCH,
 * COMPARE FBANKS". */
void adrscan(void)
{
    cs(list_word(0u));
    ad_at(ADDRESS_OF(ITEMP4));
    if (A_is_zero())
        tstfbank();
    else
        letitliv();
}

/* LETITLIV: "ARE WE DONE?" after L 16, the ninth 2CADR: "YES - DONE, SO
 * RETURN". Otherwise "CONTINUE LOOP" with the next 2CADR. */
void letitliv(void)
{
    set_A(negate(LSTLIM));
    ad_at(L_ADDRESS);
    if (A_is_zero()) {
        dead();
        return;
    }
    incr(L_ADDRESS);
    incr(L_ADDRESS);
    adrscan();
}

/* DEAD: back to the caller's L+2 in the caller's bank (DXCH ITEMP1; DTCB). */
void dead(void)
{
    dxch(ADDRESS_OF(ITEMP1));
    dtcb((word)(DEAD + 1u));
}

/* TSTFBANK: "COMPARE FBANKS ONLY" (the BBCON's bank bits): "MATCH - KILL
 * IT", or "NO MATCH - CONTINUE". */
void tstfbank(void)
{
    set_A(negate(LOW10));
    mask_at(list_word(1u));
    su_at(ADDRESS_OF(ITEMP3));
    if (A_is_zero())
        killdead();
    else
        letitliv();
}

/* KILLDEAD: "REMOVE TASK BY INSERTING TASKOVER" (TCTSKOVR's TC TASKOVER in
 * the GENADR's place). */
void killdead(void)
{
    set_A(TCTSKOVR);
    ts(list_word(0u));
    dead();
}

#endif
