/* SPDX-License-Identifier: MIT */
/* The interrupt lead-ins: INTERRUPT_LEAD_INS.agc of Luminary099 (LM,
 * listing pages 153-154) and Comanche055 (CM, pages 131-132). The faithful
 * C is c64-port/src/rupt_lead_ins.c, with the LM's T6RUPT lead-in in
 * c64-port/src/lm_t6rupt.c.
 *
 * Most lead-ins are the same four instructions with a different BBCON and
 * handler. The two missions differ in their timers: the LM's T6RUPT jumps
 * to DOT6RUPT (its RCS jet timer, T6-RUPT_PROGRAMS.agc) and its T5RUPT to
 * whatever the autopilot put in T5ADR; the CM's T6RUPT goes where T6LOC
 * says, and its T5RUPT checks TIME5 first (T5RUPT below). Interrupt 10 is
 * the LM's landing guidance (PITFALL) and the CM's unused hand controller
 * interrupt.
 *
 * The binding (src/binding/rupt_routines.c) runs each lead-in inside its
 * interrupt (GO's at a restart), with the 2CADR a DTCB lead-in jumps by in
 * fixed memory, as the program always leaves it. */
#include "lead_ins.h"
#include "../agc/names.h"

/* The banked lead-in: DXCH ARUPT; CAF bbcon; XCH BBANK; TCF handler. A and
 * L go to ARUPT, the handler's banks to BBANK, and the interrupted BBANK
 * to A, for the handler to save in BANKRUPT. */
static void banked_lead_in(word bbcon, word handler)
{
    dxch(ADDRESS_OF(ARUPT));
    set_A(bbcon);
    xch(BBANK_ADDRESS);
    continue_at(handler);
}

/* GO (04000) -- LM p. 153, CM p. 131.
 * Where the AGC starts after a restart: "INHINT; CAF GOBB; XCH BBANK; TCF
 * GOPROG", into FRESH_START_AND_RESTART's restart. */
void go_lead_in(void)
{
    inhint();
    set_A(GOBB);
    xch(BBANK_ADDRESS);
    continue_at(GOPROG);
}

/* T6RUPT (04004) -- LM p. 153, CM p. 131.
 * TIME6 has run down. Jump by DTCB to the 2CADR: in the LM the constant
 * T6ADR, DOT6RUPT, which fires the RCS jets the autopilot queued; in the
 * CM whatever T6LOC holds. */
void t6rupt_lead_in(void)
{
    dxch(ADDRESS_OF(ARUPT));
#if LUMINARY099
    set_A(T6ADR[0]); /* EXTEND; DCA T6ADR */
    set_L(T6ADR[1]);
#else
    dca(ADDRESS_OF(T6LOC));
#endif
    dtcb((word)(LEAD_IN(RUPT_T6) + 3u));
}

/* T5RUPT (04010) -- LM p. 153, CM p. 131.
 * TIME5 has overflowed: the autopilot's clock. The LM jumps by DTCB to the
 * 2CADR in T5ADR, the autopilot's next pass. The CM first asks how soon
 * TIME5 overflows again: "CS TIME5; AD .5SEC; TCF T5RUPT". */
void t5rupt_lead_in(void)
{
    dxch(ADDRESS_OF(ARUPT));
#if LUMINARY099
    dca(ADDRESS_OF(T5ADR));
    dtcb((word)(LEAD_IN(RUPT_T5) + 3u));
#else
    cs(ADDRESS_OF(TIME5));
    ad(HALF_SEC);
    t5rupt();
#endif
}

#if !LUMINARY099
/* T5RUPT -- CM p. 132.
 * A is half a second minus TIME5's count. With TIME5 within half a second
 * of its next overflow (A not positive) the interrupt ends at once
 * (NOQBRSM: the banks were never changed). Else jump by DTCB to the 2CADR
 * in T5LOC. */
void t5rupt(void)
{
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF NOQBRSM */
        continue_at(NOQBRSM);
        return;
    }
    dca(ADDRESS_OF(T5LOC));
    dtcb((word)(T5RUPT + 4u));
}
#endif

/* T4RUPT (04020) -- LM p. 153, CM p. 131.
 * TIME4: the 10-ms clock of the displays, the IMU and the radar. */
void t4rupt_lead_in(void)
{
    banked_lead_in(T4RPTBB, T4RUPT);
}

/* KEYRUPT1 (04024) -- LM p. 153, CM p. 131.
 * A key on the main DSKY (KEYRUPT_UPRUPT.agc). */
void keyrupt1_lead_in(void)
{
    banked_lead_in(KEYRPTBB, KEYRUPT1);
}

/* KEYRUPT2 (04030) -- LM p. 153, CM p. 131.
 * A MARK button, or a key on the CM's navigation DSKY: MARKRUPT. */
void keyrupt2_lead_in(void)
{
    banked_lead_in(MKRUPTBB, MARKRUPT);
}

/* UPRUPT (04034) -- LM p. 153, CM p. 131.
 * A word from the ground (KEYRUPT_UPRUPT.agc). */
void uprupt_lead_in(void)
{
    banked_lead_in(UPRPTBB, UPRUPT);
}

/* DOWNRUPT (04040) -- LM p. 153, CM p. 131.
 * The downlink wants its next two words: DODOWNTM
 * (DOWN-TELEMETRY_PROGRAM.agc). */
void downrupt_lead_in(void)
{
    banked_lead_in(DWNRPTBB, DODOWNTM);
}

/* RADAR RUPT (04044) -- LM p. 153-154, CM p. 131-132.
 * A radar reading is in: the LM's rendezvous or landing radar
 * (RADAREAD), the CM's VHF ranging (VHFREAD). */
void radar_rupt_lead_in(void)
{
#if LUMINARY099
    banked_lead_in(RDRPTBB, RADAREAD);
#else
    banked_lead_in(RDRPTBB, VHFREAD);
#endif
}

#if LUMINARY099
/* RUPT10 (04050) -- LM p. 154.
 * "RUPT10 IS USED ONLY BY LANDING GUIDANCE": PITFALL. The listing loads
 * the BBCON by CA RUPT10BB, the same as CAF here. */
void rupt10_lead_in(void)
{
    banked_lead_in(RUPT10BB, PITFALL);
}
#else
/* HAND CONTROL RUPT (04050) -- CM p. 132.
 * "NOT USED": the interrupt ends at once, at RESUME +3 (XCH BBANK, which
 * puts back the BBANK this lead-in left in A). */
void hand_control_rupt_lead_in(void)
{
    banked_lead_in(HCRUPTBB, (word)(RESUME + 3u));
}
#endif
