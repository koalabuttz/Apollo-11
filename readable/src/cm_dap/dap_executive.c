/* SPDX-License-Identifier: MIT */
/* RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc (Comanche055 pp. 1037-1038, bank 22):
 * "CALCULATION OF  AMGB, AMBG  ONCE EVERY SECOND", the job the RCS DAP
 * starts each second (AMBGUPDT, by NOVAC). AMGB turns gimbal angle
 * differences into body ones, for the attitude errors (ATTHOLD,
 * attitude_hold.c):
 *
 *     AMGB =  1   SIN(PSI)            0
 *             0   COS(PSI)COS(PHI)    SIN(PHI)
 *             0   -COS(PSI)SIN(PHI)   COS(PHI)
 *
 * "WHERE PHI AND PSI ARE CDU ANGLES": PSI CDUZ, PHI CDUX less the jet
 * quads' 7.25 degree offset. Only its five varying elements are kept,
 * AMGB1 (SIN(PSI)), AMGB4, AMGB5, AMGB7 and AMGB8. The sines and cosines
 * are the single precision ones (SPSIN, SPCOS, src/single_precision),
 * under the DAP's names, SPSIN1, SPSIN2, SPCOS1 and SPCOS2 (MYSUBS). Each
 * call leaves the file, and the job goes on at the word after it. The
 * faithful C is c64-port/src/cm_rcs_dap_exec.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* PHI, "CDUX - 7.25 DEG": EXTEND, MSU CDUX, COM, after the CAF QUADANGL
 * that puts the offset in A. */
static void phi(void)
{
    msu_at(ADDRESS_OF(CDUX));
    com(); /* "CDUX - 7.25 DEG" */
}

/* AMBGUPDT: "CHECK FOR RCS AUTOPILOT": FLAGWRD6's bit 15 clear and bit 14
 * set ("BIT15 = 0, BIT14 = 1"); "IF NOT RCS, EXIT" (ENDOFJOB), "TO PROTECT
 * TVC DAP ON SWITCHOVER". Then SIN(PSI). */
void ambgupdt(void)
{
    ca(ADDRESS_OF(FLAGWRD6));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    mask(BIT14);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    ca(ADDRESS_OF(CDUZ));
    tc_to(AMBGUPDT + 7u, SPSIN); /* TC SPSIN2 */
}

/* AMBGUPDT +010, after SPSIN: "CALCULATE AMGB": AMGB1, SIN(PSI). Then
 * COS(PSI). (A TS that overflows, when SPSIN's caller left one in A,
 * skips the CA CDUZ: SPCOS runs on +1 or -1.) */
void ambgupdt_sin_psi(void)
{
    if (!ts(ADDRESS_OF(AMGB1))) ca(ADDRESS_OF(CDUZ));
    tc_to(AMBGUPDT + 012u, SPCOS); /* TC SPCOS2 */
}

/* AMBGUPDT +013, after SPCOS: COS(PSI) in CAPSI, "MUST CHECK FOR GIMBAL
 * LOCK". Then COS(PHI), "= 7.25  DEGREES JET QUAD ANGULAR OFFSET". (An
 * overflow at the TS skips the CAF: PHI from +1 or -1.) */
void ambgupdt_cos_psi(void)
{
    if (!ts(ADDRESS_OF(CAPSI))) set_A(QUADANGL);
    phi();
    tc_to(AMBGUPDT + 020u, SPCOS); /* TC SPCOS1 */
}

/* AMBGUPDT +021, after SPCOS: AMGB8, COS(PHI), and AMGB4, COS(PSI)COS(PHI).
 * Then SIN(PHI). (An overflow at the TS skips the EXTEND: MP CAPSI runs as
 * MASK CAPSI.) */
void ambgupdt_cos_phi(void)
{
    if (ts(ADDRESS_OF(AMGB8)))
        mask_at(ADDRESS_OF(CAPSI));
    else
        mp_at(ADDRESS_OF(CAPSI));
    ts(ADDRESS_OF(AMGB4));
    set_A(QUADANGL);
    phi();
    tc_to(AMBGUPDT + 031u, SPSIN); /* TC SPSIN1 */
}

/* AMBGUPDT +032, after SPSIN: AMGB5, SIN(PHI), and AMGB7,
 * -COS(PSI)SIN(PHI); then ENDOFJOB. (An overflow at the TS skips the
 * EXTEND, as at AMGB8. A holds no overflow at the TS AMGB7: a product, or
 * the MASK of +1 or -1, complemented.) */
void ambgupdt_sin_phi(void)
{
    if (ts(ADDRESS_OF(AMGB5)))
        mask_at(ADDRESS_OF(CAPSI));
    else
        mp_at(ADDRESS_OF(CAPSI));
    com();
    ts(ADDRESS_OF(AMGB7));
    continue_at(ENDOFJOB);
}

#endif
