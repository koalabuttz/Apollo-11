/* SPDX-License-Identifier: MIT */
/* P11.agc of the CM, part 1 (Comanche055 pp. 548-550): the "SATURN
 * TAKEOVER FUNCTION". "DURING THE COASTING PHASE OF SIVB ATTACHED, THE
 * ASTRONAUT MAY REQUEST SATURN TAKEOVER THROUGH EXTENDED VERB 46": "THE
 * CMC REGARDS RHC COMMANDS AS BODY-AXES RATE COMMANDS AND IT TRANSMITS
 * THESE TO SATURN AS DC ... VOLTAGES" (NEEDLER, from AK, AK1 and AK2), of
 * "0.5 DEG/SEC", or none with the hand controller at rest.
 *
 * SATSTKON (bank 43, from DAPFIG) takes over T5: REDOSAT, the first T5
 * task (and the restart's), starts the needles and SATSTICK, the T5 task
 * every 100 ms that sends the rates. */
#include "p11.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- SATSTKON -- p. 549, bank 43 ---- */

/* SATSTKON: T5 to REDOSAT; TIME5 POSMAX, so that it comes due at the next
 * count; FLAGWRD6's bits 15 and 14 "SATSTICK CONTROL OF T5"; then
 * ZEROJET, "ZERO JET CHANNELS IN 14 MS AND THEN LEAVE THE T6 CLOCK
 * DISABLED". */
void satstkon(void)
{
    dca_constant(TWO_REDOSAT);
    inhint();
    dxch(ADDRESS_OF(T5LOC));
    set_A(POSMAX);
    ts(ADDRESS_OF(TIME5));
    cs(ADDRESS_OF(FLAGWRD6));
    mask(RELTAB11);
    ads(ADDRESS_OF(FLAGWRD6));
    tc_to(SATSTKON + 011u, IBNKCALL); /* CADR ZEROJET */
}

/* SATSTKON +013: "EXIT THUS BECAUSE WE CAME VIA V46" (GOPIN). */
void satstkon_after_zerojet(void)
{
    relint();
    tc_to(SATSTKON + 014u, GOPIN);
}

/* ---- REDOSAT and SATSTICK -- pp. 549-550, bank 23 ---- */

/* The start of a T5 task: BANKRUPT gets L (the interrupted BBANK, which
 * the T5 lead-in's DTCB leaves there) and QRUPT gets Q, for RESUME. */
static void t5_task(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
}

/* REDOSAT: "TURN ON BIT3 OF RCSFLAGX FOR NEEDLER INITIALIZATION" and
 * NEEDLER, "DISABLE IMU ERR COUNTERS ETC.". */
void redosat(void)
{
    t5_task();
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    tc_to(REDOSAT + 6u, IBNKCALL); /* CADR NEEDLER */
}

/* REDOSAT +010: "SIVB TAKEOVER ENABLE" (channel 12's bit 9); "SET UP T5
 * CYCLE", SATSTICK "IN 100 MSECS"; "END OF SATURN STICK
 * INITIALIZATION". */
void redosat_after_needler(void)
{
    set_A(BIT9);
    wor_channel(012u);
    dca_constant(TWO_SATSTCK);
    dxch(ADDRESS_OF(T5LOC));
    set_A(HUNDRED_MST5);
    ts(ADDRESS_OF(TIME5));
    continue_at(RESUME);
}

/* SATSTICK, "EXECUTED EVERY 100 MSECS": itself again in 100 ms ("LO
 * ORDER LOC SET"); then "CHECK IF MAN ROT BITS SAME" (channel 31's
 * rotation bits, inverted) and STICKCHK, "SET RATE INDICES FOR PITCH YAW
 * AND ROLL". */
void satstick(void)
{
    t5_task();
    set_A(TWO_SATSTCK[0]);
    ts(ADDRESS_OF(T5LOC));
    set_A(HUNDRED_MST5);
    ts(ADDRESS_OF(TIME5));
    set_A(STIKBITS);
    rxor_channel(031u);
    mask(STIKBITS);
    tc_to(SATSTICK + 013u, IBNKCALL); /* CADR STICKCHK */
}

/* INDEX k; CA SATRATE: the rate the index picks. The faithful port reads
 * any erasable word but Z; an index the program never sets, onto Z or out
 * of erasable memory, is not modelled. */
static void satrate(unsigned index_address)
{
    word index = fetch(index_address);
    long at = (long)ADDRESS_OF(SATRATE) +
              ((index & SIGN_BIT) ? -(long)(MINUS_ZERO & ~index) : (long)index);

    if (at < 0 || at > 01777 || at == Z_ADDRESS) not_modelled();
    ca((unsigned)at);
}

/* SATSTICK +015: "SET SATURN RATES": roll, pitch and yaw from SATRATE by
 * RMANNDX, PMANNDX and YMANNDX; then NEEDLER, "FOR SATURN INTERFACE AND
 * FDAI DISPLAY". */
void satstick_after_stickchk(void)
{
    satrate(ADDRESS_OF(RMANNDX));
    ts(ADDRESS_OF(AK));
    satrate(ADDRESS_OF(PMANNDX));
    ts(ADDRESS_OF(AK1));
    satrate(ADDRESS_OF(YMANNDX));
    ts(ADDRESS_OF(AK2));
    tc_to(SATSTICK + 026u, IBNKCALL); /* CADR NEEDLER */
}

/* SATSTICK +030: "END OF SATURN STICK CONTROL". */
void satstick_after_needler(void)
{
    continue_at(RESUME);
}

#endif
