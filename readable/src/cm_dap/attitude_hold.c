/* SPDX-License-Identifier: MIT */
/* AUTOMATIC_MANEUVERS.agc (Comanche055 pp. 1025-1028, bank 21): where the
 * T5 program's phase 2 goes with no manual rotation (AHFNOROT). In free
 * mode, "MINIMUM IMPULSE CONTROL" (FREECONT): the RHC's minimum impulse
 * commands in channel 32, a 14 ms firing for each axis commanded. With
 * the autopilot's hold mode (HOLDFUNC), the attitude held: the CDU angles
 * when rate damping is over (GRABANG, ENDDAMP), and from them the
 * attitude errors (ATTHOLD), "AK = AMGB (CDUX - THETADX) + BIAS", for the
 * jet switching logic (JETS, switching.c). The faithful C is
 * c64-port/src/cm_auto_man.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void freecont(void);
static void enddamp(void);

/* ---- AHFNOROT -- p. 1025 ---- */

/* AHFNOROT: with bit 14 of channel 31 clear, free mode: FREECONT. With the
 * rate filter not yet initialized (bit 14 of RCSFLAGS set), "RECYCLE TO
 * INITIALIZE FILTER": REINIT. With bit 13 of channel 31 clear, the hold
 * mode: HOLDFUNC. Otherwise AUTOCONT: "IF HOLDFLAG IS +, GO TO GRABANG.
 * OTHERWISE, GO TO ATTHOLD." */
void ahfnorot(void)
{
    read_channel(031);
    mask(BIT14);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF FREECONT */
        freecont();
        return;
    }
    ca(ADDRESS_OF(RCSFLAGS));
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* CCS A; TCF REINIT */
        continue_at(REINIT);
        return;
    }
    read_channel(031);
    mask(BIT13);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF HOLDFUNC */
        continue_at(HOLDFUNC);
        return;
    }
    ca(ADDRESS_OF(HOLDFLAG)); /* AUTOCONT */
    if (A_is_zero_or_minus())
        atthold();
    else
        continue_at(GRABANG);
}

/* FREECONT: HOLDFLAG 1, "RESET HOLDFLAG", "INHIBIT AUTOMATIC STEERING".
 * The RHC's minimum impulse bits, channel 32's MANROT bits (clear for a
 * command), that are new since CHANTEMP's (the last pass's, kept by the
 * LXCH) go to STICKCHK in A; it sets the axes' PMANNDX, YMANNDX and
 * RMANNDX. */
static void freecont(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(HOLDFLAG));
    read_channel(032);
    ts(L_ADDRESS);
    com();
    mask(MANROT);
    mask_at(ADDRESS_OF(CHANTEMP));
    lxch(ADDRESS_OF(CHANTEMP));
    tc_to(FREECONT + 011u, STICKCHK);
}

/* FREECONT +012, after STICKCHK: each axis' firing time from MINTAU
 * ("+14MS MINIMUM IMPULSE", "-14MS TIME", or none), indexed by the axis'
 * index: TAU (roll), TAU1 (pitch), TAU2 (yaw), for T6PROGM. */
void freecont_after_stickchk(void)
{
    ca(maneuver_index_read(ADDRESS_OF(MINTAU), fetch(ADDRESS_OF(RMANNDX))));
    ts(ADDRESS_OF(TAU));
    ca(maneuver_index_read(ADDRESS_OF(MINTAU), fetch(ADDRESS_OF(PMANNDX))));
    ts(ADDRESS_OF(TAU1));
    ca(maneuver_index_read(ADDRESS_OF(MINTAU), fetch(ADDRESS_OF(YMANNDX))));
    ts(ADDRESS_OF(TAU2));
    continue_at(T6PROGM);
}

/* ---- ATTHOLD, the attitude errors -- pp. 1027-1028 ---- */

/* ATTHOLD: each CDU angle less the angle held (THETADX), into the
 * attitude errors ERRORX, ERRORY, ERRORZ through the gimbal-to-body
 * matrix (AMGB1, AMGB4, AMGB7 for the Y difference, AMGB5, AMGB8 for the
 * Z one; the X difference goes to ERRORX alone). The biases are added only
 * in an automatic maneuver (HOLDFLAG -): "AD BIASES ONLY IF PERFORMING
 * AUTOMATIC". */
void atthold(void)
{
    ca(ADDRESS_OF(CDUX));
    msu_at(ADDRESS_OF(THETADX));
    ts(ADDRESS_OF(ERRORX));
    ca(ADDRESS_OF(CDUY));
    msu_at(ADDRESS_OF(THETADY));
    ts(ADDRESS_OF(T5TEMP));
    mp_at(ADDRESS_OF(AMGB1));
    ads(ADDRESS_OF(ERRORX));
    ca(ADDRESS_OF(T5TEMP));
    mp_at(ADDRESS_OF(AMGB4));
    ts(ADDRESS_OF(ERRORY));
    ca(ADDRESS_OF(T5TEMP));
    mp_at(ADDRESS_OF(AMGB7));
    ts(ADDRESS_OF(ERRORZ));
    ca(ADDRESS_OF(CDUZ));
    msu_at(ADDRESS_OF(THETADZ));
    ts(ADDRESS_OF(T5TEMP));
    mp_at(ADDRESS_OF(AMGB5));
    ads(ADDRESS_OF(ERRORY));
    ca(ADDRESS_OF(T5TEMP));
    mp_at(ADDRESS_OF(AMGB8));
    ads(ADDRESS_OF(ERRORZ));
    cs(ADDRESS_OF(HOLDFLAG));
    if (A_is_zero_or_minus()) /* EXTEND; BZMF JETS */
        continue_at(JETS);
    else
        continue_at(ATTHOLD + 041u);
}

/* ATTHOLD +041: CA BIAS, then the biases added. */
void atthold_bias(void)
{
    ca(ADDRESS_OF(BIAS));
    atthold_bias_errorx();
}

/* ATTHOLD +042: ADS ERRORX, and the other two axes' biases. */
void atthold_bias_errorx(void)
{
    ads(ADDRESS_OF(ERRORX));
    ca(ADDRESS_OF(BIAS1));
    ads(ADDRESS_OF(ERRORY));
    ca(ADDRESS_OF(BIAS2));
    ads(ADDRESS_OF(ERRORZ));
    jets();
}

/* ---- HOLDFUNC, GRABANG -- p. 1028 ---- */

/* HOLDFUNC: HOLDFLAG +0, attitude hold: ATTHOLD. Otherwise GRABANG. */
void holdfunc(void)
{
    if (ccs_at(ADDRESS_OF(HOLDFLAG)) == IS_PLUS_ZERO) {
        continue_at(ATTHOLD);
        return;
    }
    grabang();
}

/* GRABANG: "ZERO WBODYS AND BIASES". "IS RATE DAMPING COMPLETED" (the rate
 * damping flags, bits 13-11 of RCSFLAGS, clear): "IF SO, GO TO ENDDAMP".
 * "OTHERWISE, ZERO ERRORS", and the jets damp the rates alone. */
void grabang(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(WBODY));
    ts(ADDRESS_OF(WBODY) + 1u);
    ts(ADDRESS_OF(WBODY1));
    ts(ADDRESS_OF(WBODY1) + 1u);
    ts(ADDRESS_OF(WBODY2));
    ts(ADDRESS_OF(WBODY2) + 1u);
    ts(ADDRESS_OF(BIAS));
    ts(ADDRESS_OF(BIAS1));
    ts(ADDRESS_OF(BIAS2));
    ca(ADDRESS_OF(RCSFLAGS));
    mask(OCT16000);
    if (A_is_zero()) { /* EXTEND; BZF ENDDAMP */
        enddamp();
        return;
    }
    set_A(ZERO);
    ts(ADDRESS_OF(ERRORX));
    ts(ADDRESS_OF(ERRORY));
    ts(ADDRESS_OF(ERRORZ));
    jets();
}

/* ENDDAMP: "SET HOLDFLAG +0", and "PICK UP CDU ANGLES FOR ATTITUDE HOLD"
 * as THETADX, THETADY and THETADZ. */
static void enddamp(void)
{
    ts(ADDRESS_OF(HOLDFLAG));
    dca(ADDRESS_OF(CDUX));
    dxch(ADDRESS_OF(THETADX));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(THETADZ));
    continue_at(ATTHOLD);
}

#endif
