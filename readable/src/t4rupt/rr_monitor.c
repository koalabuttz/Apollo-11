/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 3: the rendezvous radar checks and GPMATRIX of
 * T4RUPT_PROGRAM.agc, Luminary099 only (listing pages 182-189). The
 * faithful C is c64-port/src/lm_t4rupt_rr.c. The CM's T4JUMP goes to the
 * optics instead.
 *
 * RADMODES keeps the radar's inbits as last sampled (bit 2, channel 33's
 * RR AUTO-POWER ON, 0 when on; bit 7, channel 30's RR CDU FAIL) and its
 * modes: bit 14 remode, 13 CDUs being zeroed, 11 reposition, 1 turn-on.
 *
 * The binding (src/binding/t4rupt_rr_routines.c) runs these in bank 06,
 * taking SPSIN's and SPCOS's results without overflow, as they leave
 * them. */
#include "rr_monitor.h"
#include "../agc/names.h"

#if LUMINARY099

static void rrcduchk_minus3(void);
static void rrcduchk(void);
static void monrepos(void);

/* RRAUTCHK -- LM p. 182-183.
 * "SEE IF CHANGE IN RR AUTO MODE BIT": with none, RRCDUCHK. Else "UPDATE
 * RADMODES", and "CLR CONT. DES., REMODE, REPOS, CDUZERO, AND TURNON
 * BITS". Just off: "GO DISABLE RR CDU ERROR COUNTERS" (RRCDUCHK -3). Just
 * on: "SET RRCDUZRO AND TURNON BITS" and start RRTURNON as a task in 10
 * ms. */
void rrautchk(void)
{
    ca(ADDRESS_OF(RADMODES));
    rxor_channel(CHAN33); /* EXTEND; RXOR CHAN33 */
    mask(AUTOMBIT);
    if (A_is_zero()) { /* EXTEND; BZF RRCDUCHK */
        rrcduchk();
        return;
    }
    lxch(ADDRESS_OF(RADMODES));
    rxor_lchan(); /* EXTEND; RXOR LCHAN */
    mask(OCT05776);
    ts(ADDRESS_OF(RADMODES));
    mask(BIT2);                          /* "SEE IF JUST ON" */
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* CCS A; TCF RRCDUCHK -3: "OFF" */
        rrcduchk_minus3();
        return;
    }
    set_A(OCT10001); /* CA OCT10001 */
    ads(ADDRESS_OF(RADMODES));
    set_A(ONE);                     /* CAF ONE */
    tc_at((word)(RRAUTCHK + 021u)); /* TC WAITLIST; 2CADR RRTURNON */
    continue_at(WAITLIST);
}

/* RRAUTCHK +024, where WAITLIST returns: TCF NORRGMON. */
void rrautchk_after_waitlist(void)
{
    gpmatrix();
}

/* RRCDUCHK -3 -- LM p. 184: "AT TURNON, DISABLE CDU ERROR COUNTERS"
 * (channel 12 bit 2), and on into RRCDUCHK. */
static void rrcduchk_minus3(void)
{
    set_A(negate(BIT2)); /* CS BIT2 */
    wand_channel(CHAN12);
    rrcduchk();
}

/* RRCDUCHK -- LM p. 184-185.
 * The RR CDU fail bit: with no change, RRGIMON. "IF RR NOT IN AUTO MODE,
 * DON'T CHANGE BIT 7 OF RADMODES. IF THIS WERE NOT DONE, THE TRACKER FAIL
 * MIGHT COME ON WHEN JUST READING LR DATA": NORRGMON. Else update it, and
 * if the CDU failed during P20 or P22 (RNDVZBIT), alarm 515; then the
 * TRACKER lamp (TRKFLCDU). */
static void rrcduchk(void)
{
    ca(ADDRESS_OF(RADMODES)); /* "LAST SAMPLED BIT IN RADMODES" */
    rxor_channel(CHAN30);     /* EXTEND; RXOR CHAN30 */
    mask(RCDUFBIT);
    if (A_is_zero()) { /* EXTEND; BZF RRGIMON */
        rrgimon();
        return;
    }
    set_A(AUTOMBIT);
    mask_at(ADDRESS_OF(RADMODES));
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* TCF NORRGMON */
        gpmatrix();
        return;
    }
    set_A(RCDUFBIT); /* "SET BIT 7 OF RADMODES FOR SETTRKF" */
    lxch(ADDRESS_OF(RADMODES));
    rxor_lchan(); /* EXTEND; RXOR L */
    ts(ADDRESS_OF(RADMODES));
    ca(ADDRESS_OF(RADMODES)); /* "DID RR CDU FAIL" */
    mask(RCDUFBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* "NO" */
        trkflcdu();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD0)); /* "RNDVFLG P20 OR P22 OPERATING" */
    mask(RNDVZBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* "NO" */
        trkflcdu();
        return;
    }
    tc_at((word)(TRKFLCDU - 2u)); /* TC ALARM; OCT 00515: "YES" */
    continue_at(ALARM);
}

/* TRKFLCDU -- LM p. 185: "UPDATE TRAKER FAIL LAMP ON DSKY" (SETTRKF,
 * which returns to RRGIMON). */
void trkflcdu(void)
{
    tc_at(TRKFLCDU); /* TC SETTRKF */
    continue_at(SETTRKF);
}

/* RRGIMON -- LM p. 186-187.
 * "THE RR GIMBAL LIMIT MONITOR". Skipped (NORRGMON) when "NO ANGLE
 * MONITOR FLAG" is set; when SERVICER is running and MUNFLAG is set
 * ("DON'T DO R25"); and when "INHIBIT BY REMODE, ZEROING, MONITOR, OR RR
 * NOT IN AUTO". Else RRLIMCHK checks the radar's CDU angles (from CDUT)
 * against the current mode's limits: it returns out of limits, and skips
 * within them. */
void rrgimon(void)
{
    ca(ADDRESS_OF(FLAGWRD5)); /* CAE FLAGWRD5 */
    mask(NORRMBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* "YES -- SKIP LIMIT CHECK" */
        gpmatrix();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD7)); /* "IS SERVICER RUNNING?" */
    mask(AVEGFBIT);
    if (ccs_at(A_ADDRESS) != POSITIVE) { /* "YES. IS MUNFLAG SET?" */
        ca(ADDRESS_OF(FLAGWRD6));
        mask(MUNFLBIT);
        if (ccs_at(A_ADDRESS) == POSITIVE) { /* "YES. DON'T DO R25" */
            gpmatrix();
            return;
        }
    }
    set_A(OCT32002); /* +5 */
    mask_at(ADDRESS_OF(RADMODES));
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        gpmatrix();
        return;
    }
    tc_at((word)(RRGIMON + 020u)); /* TC RRLIMCHK; ADRES CDUT */
    continue_at(RRLIMCHK);
}

/* RRGIMON +022, where RRLIMCHK returns: TCF MONREPOS. */
void rrgimon_out_of_limits(void)
{
    monrepos();
}

/* RRGIMON +023, where RRLIMCHK skips: TCF NORRGMON. */
void rrgimon_within_limits(void)
{
    gpmatrix();
}

/* MONREPOS -- LM p. 187.
 * "SET FLAG TO SHOW REPOSITION IN PROGRESS", "DISABLE TRACKER AND ERROR
 * COUNTER" (channel 12 bits 14 and 2), and start DORREPOS as a task in 20
 * ms. */
static void monrepos(void)
{
    set_A(REPOSBIT);
    ads(ADDRESS_OF(RADMODES));
    set_A(negate(OCT20002)); /* CS OCT20002 */
    wand_channel(CHAN12);
    set_A(TWO);
    tc_at((word)(MONREPOS + 6u)); /* TC WAITLIST; 2CADR DORREPOS */
    continue_at(WAITLIST);
}

/* MONREPOS +011, where WAITLIST returns: TCF NORRGMON. */
void monrepos_after_waitlist(void)
{
    gpmatrix();
}

/* GPMATRIX (DAPT4S, NORRGMON) -- LM p. 188-189.
 * "THIS PROGRAM CALCULATES ALL THE SINGLE-PRECISION MATRIX ELEMENTS WHICH
 * ARE USED BY LEM DAP TO TRANSFORM VECTORS FROM GIMBAL TO PILOT (BODY)
 * AXES AND BACK AGAIN", from the middle gimbal angle MG (CDUZ) and the
 * outer OG (CDUX), each through SPSIN or SPCOS:
 *     M11 = SIN(MG), COSMG = COS(MG), M22 = SIN(OG),
 *     M31 = -SIN(OG)COS(MG), M32 = COS(OG), M21 = COS(OG)COS(MG). */
void gpmatrix(void)
{
    ca(ADDRESS_OF(CDUZ));         /* CAE CDUZ */
    tc_at((word)(GPMATRIX + 1u)); /* TC SPSIN */
    continue_at(SPSIN);
}

void gpmatrix_after_sin_mg(void)
{
    ts(ADDRESS_OF(M11)); /* "SCALED AT 1" */
    ca(ADDRESS_OF(CDUZ));
    tc_at((word)(GPMATRIX + 4u)); /* TC SPCOS */
    continue_at(SPCOS);
}

void gpmatrix_after_cos_mg(void)
{
    ts(ADDRESS_OF(COSMG)); /* "SCALED AT 1 (ONLY A FACTOR)" */
    ca(ADDRESS_OF(CDUX));
    tc_at((word)(GPMATRIX + 7u)); /* TC SPSIN */
    continue_at(SPSIN);
}

void gpmatrix_after_sin_og(void)
{
    ts(ADDRESS_OF(M22)); /* "SCALED AT 1 (ALSO IS MR22)" */
    cs(ADDRESS_OF(M22));
    mp_at(ADDRESS_OF(COSMG)); /* EXTEND; MP COSMG: "-SIN(OG)COS(MG)" */
    ts(ADDRESS_OF(M31));
    ca(ADDRESS_OF(CDUX));
    tc_at((word)(GPMATRIX + 016u)); /* TC SPCOS */
    continue_at(SPCOS);
}

void gpmatrix_after_cos_og(void)
{
    ts(ADDRESS_OF(M32));      /* "SCALED AT 1 (ALSO IS MR23)" */
    mp_at(ADDRESS_OF(COSMG)); /* EXTEND; MP COSMG: "COS(OG)COS(MG)" */
    ts(ADDRESS_OF(M21));
    tc_at((word)(GPMATRIX + 023u)); /* TC RESUME */
    continue_at(RESUME);
}

#endif
