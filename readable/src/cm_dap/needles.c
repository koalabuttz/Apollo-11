/* SPDX-License-Identifier: MIT */
/* RCS-CSM_DIGITAL_AUTOPILOT.agc (Comanche055 pp. 1009-1013, bank 21):
 * "RCS-CSM AUTOPILOT ATTITUDE ERROR DISPLAY", the end of phase 1 of the
 * T5 program, and the "FDAI ATTITUDE ERROR DISPLAY SUBROUTINE", NEEDLER,
 * which puts the attitude errors in AK, AK1 and AK2 on the FDAI's needles
 * "VIA THE DIGITAL TO ANALOG CONVERTERS (DACS) IN THE CDUS". It meters "OUT
 * THE APPROPRIATE NUMBER OF PULSES TO THE IMU ERROR COUNTERS", sequences
 * their relays, and keeps off the DACs while the coarse align loop uses
 * them. The faithful C is c64-port/src/cm_rcs_dap.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* ---- DACNDLS, the end of phase 1 -- p. 1009 ---- */

/* DACNDLS: "ALTERNATE BETWEEN FDAIDSP1 AND FDAIDSP2" by bit 4 of
 * RCSFLAGS. FDAIDSP1 sets the bit and calls NEEDLER, and NEEDLER's TC Q
 * comes back to KRESUME1. (DELOOP's CCS SPNDX comes in at CS RCSFLAGS on
 * +0, at MASK BIT4 on a negative SPNDX and at EXTEND; BZF FDAIDSP2, with A
 * +0, on -0, where MASK BIT4 changes nothing.) */
void dacndls_ccs(enum sign entry)
{
    if (entry == IS_PLUS_ZERO) cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT4);
    if (A_is_zero()) { /* EXTEND; BZF */
        continue_at(FDAIDSP2);
        return;
    }
    ads(ADDRESS_OF(RCSFLAGS)); /* FDAIDSP1 */
    tc_at(FDAIDSP1 + 1u);      /* TC NEEDLER */
    needler();
}

void dacndls(void)
{
    dacndls_ccs(IS_PLUS_ZERO);
}

/* KRESUME1: "END PHASE 1". */
void kresume1(void)
{
    continue_at(RESUME);
}

/* ---- NEEDLER -- pp. 1010-1012 ---- */

/* NEEDLER: "CHECK FOR COARSE ALIGN ENABLE" (bit 4 of channel 12): "IF IN
 * COARSE ALIGN DO NOT USE IMU ERROR COUNTERS. DONT USE NEEDLES". Bits 3
 * and 2 of RCSFLAGS step the passes: on to NEEDLES3 with both clear, to
 * NEEDLER2 with bit 2 alone, and the first pass, bit 3, disables the IMU
 * error counters "TO ZERO DACS" ("MUST WAIT AT LEAST 60 MS BEFORE ENABLING
 * COUNTERS"). */
void needler(void)
{
    set_A(BIT4);
    rand_channel(012);
    if (!A_is_zero()) { /* EXTEND; BZF NEEDLER1 */
        needler_first_pass();
        return;
    }
    ca(ADDRESS_OF(RCSFLAGS)); /* NEEDLER1 */
    mask(SIX);
    if (A_is_zero()) { /* EXTEND; BZF */
        continue_at(NEEDLES3);
        return;
    }
    mask(BIT3);
    if (A_is_zero()) { /* EXTEND; BZF: "BIT3 = 0, BIT2 = 1" */
        needler2();
        return;
    }
    set_A(negate(BIT6)); /* CS: "FIRST PASS BIT3 = 1" */
    wand_channel(012);   /* "DISABLE IMU ERROR COUNTER TO ZERO DACS" */
    needle11();
}

/* NEEDLER +5, in coarse align: "SET BIT3 FOR INITIALIZATION PASS". */
void needler_first_pass(void)
{
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    tc_q();
}

/* NEEDLE11: "ZERO THE INPUTS ON FIRST PASS", "ZERO THE DISPLAY
 * REGISTERS", "ZERO THE OUT COUNTERS" (each -0), and "RESET RCSFLAGS FOR
 * PASS2": bit 2 alone. "END PASS1". (The sum cannot overflow: bits 2 and 3
 * are clear.) */
void needle11(void)
{
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(AK));
    ts(ADDRESS_OF(AK1));
    ts(ADDRESS_OF(AK2));
    ts(ADDRESS_OF(EDRIVEX));
    ts(ADDRESS_OF(EDRIVEY));
    ts(ADDRESS_OF(EDRIVEZ));
    ts(ADDRESS_OF(CDUXCMD));
    ts(ADDRESS_OF(CDUYCMD));
    ts(ADDRESS_OF(CDUZCMD));
    set_A(negate(SIX)); /* CS */
    mask_at(ADDRESS_OF(RCSFLAGS));
    ad(BIT2);
    ts(ADDRESS_OF(RCSFLAGS));
    tc_q();
}

/* NEEDLER2: "ENABLE IMU ERROR COUNTERS". */
void needler2(void)
{
    set_A(BIT6);
    wor_channel(012);
    continue_at(NEEDLER2 + 3u);
}

/* NEEDLER2 +3: "RESET RCSFLAGS TO DISPLAY ATTITUDE" errors (bits 3 and 2
 * clear), "WAIT AT LEAST 4 MS FOR RELAY CLOSURE". */
void needler2_reset(void)
{
    set_A(negate(SIX)); /* CS */
    needler2_mask();
}

/* NEEDLER2 +4: MASK RCSFLAGS. */
void needler2_mask(void)
{
    mask_at(ADDRESS_OF(RCSFLAGS));
    ts(ADDRESS_OF(RCSFLAGS));
    tc_q();
}

/* NEEDLES3: "CHECK TO SEE IF IMU ERROR COUNTER IS ENABLED". */
void needles3(void)
{
    set_A(BIT6);
    rand_channel(012);
    needles3_test();
}

/* NEEDLES3 +3: "IF NOT RECYCLE NEEDLES" (NEEDLER +5). */
void needles3_test(void)
{
    if (A_is_zero()) /* EXTEND; BZF NEEDLER +5 */
        continue_at(NEEDLER + 5u);
    else
        needles();
}

/* ---- NEEDLES, the DACs -- p. 1012 ---- */

/* NEEDLES: the three axes, SPNDX 2, 1 and 0. */
void needles(void)
{
    set_A(TWO);
    dacloop();
}

/* DACLOOP: the error scaled for the DAC, -QUARTER times AK, into L. (TS
 * SPNDX overflows only for an A that holds an overflow, and skips the CS
 * QUARTER: A is then +1 or -1.) */
static void dac_input(void)
{
    if (!ts(ADDRESS_OF(SPNDX))) set_A(negate(QUARTER)); /* CS */
    mp_at(index_read(ADDRESS_OF(AK), spndx()));
    ts(L_ADDRESS);
}

/* Channel 14's bits 13, 14 and 15, "SET DAC ACTIVITY BITS", and TC Q.
 * (The end of DACLOOP's CCS SPNDX: +0 comes in at CAF 13,14,15, a negative
 * SPNDX at EXTEND; WOR CHAN14, and -0 at WOR CHAN14 without its EXTEND,
 * which runs as TC to the word at 05014, BIT4's.) */
static void dac_activity(enum sign entry)
{
    if (entry == IS_MINUS_ZERO) {
        tc_to(DACLOOP + 033u, ADDRESS_OF(BIT4));
        return;
    }
    if (entry == IS_PLUS_ZERO) set_A(BITS13_14_15);
    wor_channel(014);
    dacloop_return();
}

void dacloop(void)
{
    dac_input();
    dacloop_limit();
}

/* DACLOOP +6, each axis' drive: the new input held to DACLIMIT ("ON
 * OVERFLOW LIMIT OUTPUT TO +-384") or, with no overflow, as it is; the
 * pulses to send, the change from the "CURRENT VALUE OF DAC" in EDRIVEX,
 * added into CDUXCMD; and the new value kept in EDRIVEX. CCS SPNDX: the
 * next axis, or the DAC activity bits. */
void dacloop_limit(void)
{
    enum sign axis;

    for (;;) {
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: ca(ADDRESS_OF(DACLIMIT)); break;
        case NEGATIVE: cs(ADDRESS_OF(DACLIMIT)); break;
        case IS_PLUS_ZERO:
        case IS_MINUS_ZERO: break; /* A +0 */
        }
        ad_at(L_ADDRESS);
        if (ts(ADDRESS_OF(T5TEMP))) { /* "OVFLO CHK" */
            ca(index_read(ADDRESS_OF(DACLIMIT), a_word()));
            ts(L_ADDRESS);
        }
        cs(index_read(ADDRESS_OF(EDRIVEX), spndx()));
        ad_at(L_ADDRESS);
        ads(index_store(ADDRESS_OF(CDUXCMD), spndx()));
        lxch(index_store(ADDRESS_OF(EDRIVEX), spndx()));
        axis = ccs_at(ADDRESS_OF(SPNDX));
        if (axis != POSITIVE) break;
        dac_input(); /* TCF DACLOOP */
    }
    dac_activity(axis);
}

/* DACLOOP +034: TC Q. */
void dacloop_return(void)
{
    tc_q();
}

#endif
