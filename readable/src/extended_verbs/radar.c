/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 4: the LM's radar test verbs -- LM pp. 276-279.
 * The faithful C is c64-port/src/lanec_ev_radar.c.
 *
 * R04, the radar test (verb 63), samples the rendezvous or the landing
 * radar once a second (the task RADSAMP, every RSAMPDT) and shows what it
 * reads: the RR's CDU angles (noun 72) then its range and range rate (noun
 * 78), or the LR's range and position (noun 66) then its velocities (noun
 * 67). Proceed (V33) on the second display starts the other radar's
 * samples. R77 (verb 78) is the LR's spurious-return test: the same
 * sampling of the LR, four times a second, with no display; R77END (verb
 * 79) stops it.
 *
 * The comments name the R04 entries' crew responses as the listing does:
 * V34 terminate, V33 proceed, V32 recycle and, for GOMARKFR and GOMARK4,
 * enter (a load). Each call out of the file ends a C function with Q and Z
 * set (tc() below, or continue_at()), and the word it returns to is an
 * entry of its own; the binding goes on from one entry of this part to the
 * next while Z is one. A routine that falls into the next label calls it.
 * All of it is basic code in bank 43, the LM's only. */
#include "extended_verbs.h"
#include "../agc/names.h"

#if LUMINARY099
/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* CCS A: the masked word is positive (not +0). */
static bool masked_is_set(void)
{
    return ccs_register(A_ADDRESS) == POSITIVE;
}

/* A display, flashing or not, from the word at: CAF <verb-noun>, TC
 * BANKCALL, CADR GOMARKF (or GOMARKFR, GOMARK4). */
static void show(word at, word verb_noun)
{
    set_A(verb_noun);
    tc(at + 1u, BANKCALL);
}

/* ---- Starting ---- */

/* R77 -- LM p. 276.
 * Verb 78, the LR spurious-return test: with neither radar in use ("TRY
 * TO AVOID THE 1210.") and R04 not running, R77FLAG up and the sampling
 * set up (R04Z). */
void r77(void)
{
    tc(R77, RDRUSECK);
}

/* R77 +1 -- LM p. 276: "IS R04 RUNNING?" */
void r77_after_rdruseck(void)
{
    ca(ADDRESS_OF(FLAGWRD3));
    mask(R04FLBIT);
    if (masked_is_set()) {
        tc(R77 + 4u, ALM_END); /* "YES." */
        return;
    }
    tc(R77 + 5u, UPFLAG); /* ADRES R77FLAG */
}

/* R77 +7 -- LM p. 276. */
void r77_after_upflag(void)
{
    continue_at(R04Z); /* TCF R04Z */
}

/* R04 -- LM p. 276.
 * Verb 63, the radar test: with neither radar in use, claim the display
 * and raise R04FLAG ("FOR ALARMS"); UPFLAG returns to R04Z. */
void r04(void)
{
    tc(R04, RDRUSECK);
}

/* R04 +1 -- LM p. 276. */
void r04_after_rdruseck(void)
{
    tc(R04 + 1u, TESTXACT);
}

/* R04 +2 -- LM p. 276. */
void r04_after_testxact(void)
{
    tc(R04 + 2u, UPFLAG); /* ADRES R04FLAG */
}

/* R04Z -- LM p. 276.
 * E-bank 4 for the sampling words. "SAMPLE ONCE PER SECOND" (RSAMPDT),
 * from the first slot (RTSTLOC) with no bad samples yet (RFAILCNT). With
 * interrupts held, RADMODES's bits 9, 6 and 3 (LRPOSCAL: the LR's range
 * scale and position 1, the RR's range scale) taken from channel 33. Then
 * R77 (R04FLAG down) goes to R04K; R04 asks which radar, the RR first
 * ("INDICATES RENDEZVOUS DESIRED"). */
void r04z(void)
{
    set_A(EBANK4);
    ts(EBANK_ADDRESS);
    set_A(ONE_SEC_PLUS_1);
    ts(ADDRESS_OF(RSAMPDT));
    set_A(ZERO);
    ts(ADDRESS_OF(RTSTLOC));
    ts(ADDRESS_OF(RFAILCNT)); /* "ZERO BAD SAMPLE COUNTER" */

    inhint();
    set_A(negate(LRPOSCAL)); /* CS LRPOSCAL: "INITIALIZE" */
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    set_A(LRPOSCAL);
    rand_channel(033);
    ads(ADDRESS_OF(RADMODES));
    relint();

    cs(ADDRESS_OF(FLAGWRD3)); /* "CHECK R04FLAG R04 =1 R77 =0" */
    mask(R04FLBIT);
    if (masked_is_set()) {
        continue_at(R04K); /* TCF R04K */
        return;
    }
    set_A(ONE);
    ts(ADDRESS_OF(OPTIONX) + 1u);
    r04a();
}

/* ---- R04: which radar ---- */

/* R04A -- LM pp. 276-277.
 * V04 N12, option code 3 (BIT3), flashing: R2 00001 the rendezvous radar,
 * 00002 the landing radar. */
void r04a(void)
{
    set_A(BIT3); /* "OPTION CODE FOR V04N12" */
    ts(ADDRESS_OF(OPTIONX));
    r04a_plus2();
}

/* R04A +2 -- LM p. 277: where V32 asks again. */
void r04a_plus2(void)
{
    show(R04A + 2u, V04N12X); /* CADR GOMARKFR */
}

/* R04A +5 -- LM p. 277: V34. */
void r04a_after_terminate(void)
{
    continue_at(R04END); /* TCF R04END */
}

/* R04A +6 -- LM p. 277: V33 (TCF +5). */
void r04a_after_proceed(void)
{
    continue_at(R04A + 11u);
}

/* R04A +7 -- LM p. 277: V32. */
void r04a_after_recycle(void)
{
    continue_at(R04A + 2u); /* TCF R04A +2 */
}

/* R04A +8 -- LM p. 277: a load: blank R3 and wait. */
void r04a_after_enter(void)
{
    set_A(BIT3);
    tc(R04A + 9u, BLANKET);
}

/* R04A +10 -- LM p. 277. */
void r04a_after_blanket(void)
{
    tc(R04A + 10u, ENDOFJOB);
}

/* R04A +11 -- LM p. 277: "SAVE DESIRED OPTION RR =1 LR =2". */
void r04a_plus11(void)
{
    ca(ADDRESS_OF(OPTIONX) + 1u);
    r04x_minus1();
}

/* R04X -1 -- LM p. 277: TS RTSTDEX, where R04Y comes back with the other
 * radar. Its callers never leave an overflow in A, whose TS would skip
 * R04X's first word; the readable routine does not model one. */
void r04x_minus1(void)
{
    if (ts(ADDRESS_OF(RTSTDEX))) not_modelled();
    r04x();
}

/* R04X -- LM p. 277: "RR OR LR DESIRED": RTSTDEX's bits 2 and 3 (SIX) set
 * for the LR (R04L), else the RR's samples go from RTSTBASE 0. */
void r04x(void)
{
    set_A(SIX);
    mask_at(ADDRESS_OF(RTSTDEX));
    if (masked_is_set()) {
        continue_at(R04L); /* "LANDING RADAR" */
        return;
    }
    ts(ADDRESS_OF(RTSTBASE)); /* "FOR RR BASE =0, MAX =1" */
    r04b();
}

/* R04B -- LM p. 277.
 * "IS RR AUTO MODE DISCRETE PRESENT" (channel 33's bit 2, 0 when it is):
 * if not, "REQUEST SELECTION OF RR AUTO MODE", V50 N25 with R1 00201. */
void r04b(void)
{
    set_A(BIT2);
    rand_channel(033);
    if (A_is_zero()) { /* EXTEND; BZF R04C: "YES" */
        continue_at(R04C);
        return;
    }
    r04b_plus5();
}

/* R04B +5 -- LM p. 277: where enter asks again (TCF -7). */
void r04b_plus5(void)
{
    set_A(OCT201_R04);
    ts(ADDRESS_OF(DSPTEM1));
    show(R04B + 7u, V50N25X); /* CADR GOMARK4 */
}

/* R04B +10 -- LM p. 277: V34. */
void r04b_after_terminate(void)
{
    continue_at(R04END); /* TCF R04END */
}

/* R04B +11 -- LM p. 277: V33, look again. */
void r04b_after_proceed(void)
{
    continue_at(R04B); /* TCF R04B */
}

/* R04B +12 -- LM p. 277: enter (TCF -7). */
void r04b_after_enter(void)
{
    continue_at(R04B + 5u);
}

/* R04C -- LM p. 277.
 * "ENABLE RR AUTO TRACKER" (channel 12's bit 14), then RTSTMAX 2 "FOR
 * SEQUENTIAL STORAGE" (R04L comes to R04C +4 with 6, the LR's). */
void r04c(void)
{
    set_A(BIT14);
    wor_channel(012);
    set_A(TWO);
    r04c_plus4();
}

/* R04C +4 -- LM p. 277.
 * The sampling task, RADSAMP, starts in A's centiseconds (the 2 or 6 just
 * stored). Its callers never leave an overflow in A, whose TS would skip
 * the TC WAITLIST; the readable routine does not model one. */
void r04c_plus4(void)
{
    if (ts(ADDRESS_OF(RTSTMAX))) not_modelled();
    tc(R04C + 5u, WAITLIST); /* EBANK= RSTACK; 2CADR RADSAMP */
}

/* R04C +8 -- LM pp. 277-278.
 * R77 (R04FLAG down) goes back to PINBALL, sampling; R04 shows the radar
 * RTSTDEX names. */
void r04c_after_waitlist(void)
{
    relint();
    cs(ADDRESS_OF(FLAGWRD3)); /* "CHECK R04FLAG R04 =1 R77 =0" */
    mask(R04FLBIT);
    if (masked_is_set()) {
        continue_at(GOPIN); /* "R77" */
        return;
    }
    set_A(SIX); /* "RR OR LR" */
    mask_at(ADDRESS_OF(RTSTDEX));
    if (masked_is_set()) {
        continue_at(R04LR); /* "LR" */
        return;
    }
    r04rr();
}

/* ---- R04: the displays ---- */

/* R04RR -- LM p. 278.
 * "DISPLAY RR CDU ANGLES (1/SEC)": V16 N72, trunnion and shaft. V33 goes
 * on to the range and range rate, V32 shows the angles again. */
void r04rr(void)
{
    show(R04RR, V16N72); /* CADR GOMARKF */
}

/* R04RR +3 -- LM p. 278: V34. */
void r04rr_after_terminate(void)
{
    continue_at(R04END);
}

/* R04RR +4 -- LM p. 278: V33 (TCF +2). */
void r04rr_after_proceed(void)
{
    continue_at(R04RR + 6u);
}

/* R04RR +5 -- LM p. 278: V32. */
void r04rr_after_recycle(void)
{
    continue_at(R04RR);
}

/* R04RR +6 -- LM p. 278.
 * "DISPLAY RR RANGE AND RANGE RATE (1/SEC)": V16 N78. V33 goes over to
 * the LR (R04Y), V32 back to the angles. */
void r04rr_plus6(void)
{
    show(R04RR + 6u, V16N78); /* CADR GOMARKF */
}

/* R04RR +9 -- LM p. 278: V34. */
void r04rr_plus6_after_terminate(void)
{
    continue_at(R04END);
}

/* R04RR +10 -- LM p. 278: V33. */
void r04rr_plus6_after_proceed(void)
{
    continue_at(R04Y);
}

/* R04RR +11 -- LM p. 278: V32. */
void r04rr_plus6_after_recycle(void)
{
    continue_at(R04RR);
}

/* R04LR -- LM p. 278.
 * "DISPLAY LR RANGE AND POSITON (1/SEC)": V16 N66. V33 goes on to the
 * velocities, V32 shows the range again. */
void r04lr(void)
{
    show(R04LR, V16N66); /* CADR GOMARKF */
}

/* R04LR +3 -- LM p. 278: V34. */
void r04lr_after_terminate(void)
{
    continue_at(R04END);
}

/* R04LR +4 -- LM p. 278: V33 (TCF +2). */
void r04lr_after_proceed(void)
{
    continue_at(R04LR + 6u);
}

/* R04LR +5 -- LM p. 278: V32. */
void r04lr_after_recycle(void)
{
    continue_at(R04LR);
}

/* R04LR +6 -- LM p. 278.
 * "DISPLAY LR VELX, VELY, VELZ (1/SEC)": V16 N67. V33 goes over to the RR
 * (R04Y), V32 back to the range. */
void r04lr_plus6(void)
{
    show(R04LR + 6u, V16N67); /* CADR GOMARKF */
}

/* R04LR +9 -- LM p. 278: V34. */
void r04lr_plus6_after_terminate(void)
{
    continue_at(R04END);
}

/* R04LR +10 -- LM p. 278: V33. */
void r04lr_plus6_after_proceed(void)
{
    continue_at(R04Y);
}

/* R04LR +11 -- LM p. 278: V32. */
void r04lr_plus6_after_recycle(void)
{
    continue_at(R04LR);
}

/* ---- R04: the other radar ---- */

/* R04Y -- LM p. 278.
 * Sampling stopped ("TO TERMINATE SAMPLING.", RSAMPDT +0) and two seconds'
 * wait "FOR LAST RADARUP". */
void r04y(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RSAMPDT));
    set_A(TWO_SECS);
    tc(R04Y + 3u, BANKCALL); /* CADR DELAYJOB */
}

/* R04Y +5 -- LM pp. 278-279.
 * Sampling again, once a second, from the first slot, of the other radar:
 * RTSTDEX 1 if it was the LR (CS ONE; AD TWO), 2 if it was the RR (the
 * CCS's +0, AD TWO). */
void r04y_after_delayjob(void)
{
    set_A(ONE_SEC_PLUS_1); /* "SAMPLE ONCE PER SECOND" */
    ts(ADDRESS_OF(RSAMPDT));
    set_A(ZERO); /* "FOR STORING RESULTS" */
    ts(ADDRESS_OF(RTSTLOC));
    set_A(SIX);
    mask_at(ADDRESS_OF(RTSTDEX));
    if (masked_is_set()) set_A(negate(ONE)); /* CS ONE: "WAS LR" */
    ad(TWO);                                 /* "WAS RR" */
    continue_at(R04X - 1u);                  /* TCF R04X -1 */
}

/* R04K -- LM p. 279: R77's sampling, "4 LR COMPONENTS PER SECOND." */
void r04k(void)
{
    set_A(MS250_PLUS_1);
    ts(ADDRESS_OF(RSAMPDT));
    r04l();
}

/* R04L -- LM p. 279: the LR's samples, "BASE =2, MAX =3", the task in 6
 * centiseconds. */
void r04l(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(RTSTBASE));
    set_A(SIX);
    continue_at(R04C + 4u); /* TCF R04C +4 */
}

/* ---- Ending ---- */

/* R04END -- LM p. 279.
 * "ZERO RSAMPDT TO TERMINATE SAMPLING", and "WAIT 1.28 SECONDS FOR
 * POSSIBLE PENDING RUPT." (BIT8's 128 centiseconds). */
void r04end(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RSAMPDT));
    set_A(BIT8);
    tc(R04END + 3u, BANKCALL); /* CADR DELAYJOB */
}

/* R04END +5 -- LM p. 279.
 * "DISABLE RR AUTO TRACKER." with interrupts held, R04FLAG down ("SIGNAL
 * END OF R04."; DOWNFLAG allows interrupts again), and the display
 * released. */
void r04end_after_delayjob(void)
{
    inhint();
    set_A(negate(BIT14)); /* CS BIT14 */
    wand_channel(012);
    tc(R04END + 9u, DOWNFLAG); /* ADRES R04FLAG */
}

/* R04END +11 -- LM p. 279. */
void r04end_after_downflag(void)
{
    tc(R04END + 11u, ENDEXT);
}

/* R77END -- LM p. 279.
 * Verb 79: "TO TERMINATE SAMPLING", then "WAIT 320 MS FOR POSSIBLE
 * PENDING RUPT." (BIT6's 32 centiseconds) and R77FLAG down. */
void r77end(void)
{
    set_A(EBANK4);
    ts(EBANK_ADDRESS);
    set_A(ZERO);
    ts(ADDRESS_OF(RSAMPDT));
    set_A(BIT6);
    tc(R77END + 5u, BANKCALL); /* CADR DELAYJOB */
}

/* R77END +7 -- LM p. 279. */
void r77end_after_delayjob(void)
{
    tc(R77END + 7u, DOWNFLAG); /* ADRES R77FLAG */
}

/* R77END +9 -- LM p. 279. */
void r77end_after_downflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}
#endif
