/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 17 (Comanche055 pp. 625-626): R23.55 to R23.END, the
 * measurement's incorporation (bank 31).
 *
 * R23.55 takes POINTAXS's line of sight ("RCLL IS IN MPAC"). UCLSTAR is
 * its unit vector plus the CSM's velocity VZC times ONE/C, made a unit
 * vector again; USSTAR the star's (US) plus the velocity less VESO, the
 * same way. BVECTOR is USSTAR less its part along UCLSTAR ("USSTAR -
 * COSQ(UCLSTAR)"), made a unit vector, with two zero vectors after it;
 * the angle between them (ACOS of their dot product) goes to the
 * push-down list complemented.
 *
 * The trunnion angle (TRUNION) is changed from two's complement to ones'
 * (MSU with L +0) in basic code. Back in interpretive code: the angle
 * less TRUNBIAS (the optics calibration), plus the complemented angle,
 * plus TRUN19, times the range (32D) and PI/4.0, to DELTAQ (shifted more
 * in the lunar sphere, CMOONFLG); the range squared times TRUNVAR, plus
 * VARSUBL, to VARIANCE. INCORP1 works out the state vector deviation
 * (DELTAX), GRP2PC makes a restart point, and noun 49 shows the
 * deviation's position and velocity lengths ("DISPLAY IS 2-27 IF IN LUNAR
 * SPHERE.").
 *
 * R23.6 flashes verb 06 noun 49 by GOFLASHR: terminate; proceed, R23.7
 * ("INCORPORATE DATA": VEHUPFLG set and INCORP2); enter, which also ends
 * P23 (GOTOPOOH). GOFLASHR's last return comes at once, with the display
 * up: R3 blanked (BLANKET, "BLAN OUT R3"), a restart point, and the job
 * ends (ENDOFJOB).
 *
 * Each call out of the file (BANKCALL, BLANKET, PHASCHNG, GOTOPOOH,
 * ENDOFJOB) leaves Z at the routine and Q at the word after the TC, where
 * the routine comes back to (after its data word, for PHASCHNG);
 * GOTOPOOH and ENDOFJOB don't come back. GOFLASHR comes back to the word
 * after its CADR on terminate, the next on proceed, the next on enter,
 * and the next once the display is up. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r23_7(void);

/* R23.55, which P23.85 goes to: "RCLL IS IN MPAC". */
static const word r23_55_words[] = {
    ORDERS(UNIT, PUSH),
    ORDER(VLOAD),
    /* "RCLL * RCLL" */
    ADDRESS_PLUS_ONE_D(34),
    /* "PUSH 30-31 =RCLL*RCLL 32-33=ABVAL RCLL" */
    STOVL_D(30),
    ADDRESS_PLUS_ONE(VZC),
    ORDERS(VXSC, VSR),
    ADDRESS_PLUS_ONE(ONE_C),
    SR_COUNT(15),
    /* "PUSH UP RCLL(UNIT)" */
    ORDER(VAD),
    ORDER(UNIT),
    STOVL(UCLSTAR),
    ADDRESS_PLUS_ONE(VZC),
    ORDERS(VSR2, VSU),
    ADDRESS_PLUS_ONE(VESO),
    ORDERS(VXSC, VSR),
    ADDRESS_PLUS_ONE(ONE_C),
    SR_COUNT(13),
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(US),
    STORE(USSTAR),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UCLSTAR),
    /* "PD 0,1 = USSTAR(DOT)UCLSTAR" */
    ORDERS(PUSH, VLOAD),
    ADDRESS_PLUS_ONE(UCLSTAR),
    ORDERS(VXSC, VCOMP),
    ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE(USSTAR),
    ORDER(UNIT),
    /* "USSTAR - COSQ(UCLSTAR)" */
    STOVL(BVECTOR),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(BVECTOR, 6),
    STODL_AT(BVECTOR, 12),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(ACOS, DCOMP),
    ORDERS(PUSH, DLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDER(EXIT),
};

/* R23.55 +045 -- after the EXIT: "PUT FIXED INTO ERASABLE FOR MSU"
 * ("INSTRUCTION COMING UP"): VARSUBL, +0, to L; "TRUNION (2'S)-00000
 * CONVERTS TRUNION TO" "1'S.", to MPAC. (Neither the CA of +0 nor the
 * MSU, which leaves A sign-extended, leaves an overflow for a TS to skip
 * on.) */
void r23_55_after_exit(void)
{
    set_A(VARSUBL);
    if (ts(L_ADDRESS)) not_modelled();
    ca(ADDRESS_OF(TRUNION));
    msu_at(L_ADDRESS);
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    tc_at(R23_55 + 053u);
    intpret();
}

/* R23.55 +054: "PUSH IS DP.  WHEN BDSU IS EXECUTED, 2ND" "HALF OF
 * PUSHLIST IS GUARANTEED ZERO FROM" "DLOAD ZEROVECS ABOVE". The angle
 * worked out to DELTAQ, the variance to VARIANCE, INCORP1, and noun 49's
 * lengths to N49DISP: the velocity's at N49DISP +2, the position's at
 * N49DISP. */
static const word r23_55_trunnion_words[] = {
    ORDERS(PUSH, SLOAD),
    ADDRESS_PLUS_ONE(TRUNBIAS),
    ORDER(BDSU),
    ORDERS(SR3, DAD),
    ORDERS(DAD, DMP),
    ADDRESS_PLUS_ONE(TRUN19),
    ADDRESS_PLUS_ONE_D(32),
    ORDERS(DMP, SL3),
    ADDRESS_PLUS_ONE(PI_4_0),
    ORDERS(BOFF, SL2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(R23_51),
    /* R23.51 */
    STODL(DELTAQ),
    /* "RCLL * RCLL" */
    ADDRESS_PLUS_ONE_D(30),
    ORDERS(DMP, RTB),
    ADDRESS_PLUS_ONE(TRUNVAR),
    ADDRESS(TPMODE),
    ORDER(TAD),
    ADDRESS_PLUS_ONE(VARSUBL),
    STORE(VARIANCE),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, DMENFLG),
    ADDRESS(INCORP1),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    /* "DISPLAY IS 2-27 IF IN LUNAR SPHERE." */
    ORDERS(BOF, SR2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(R23_52),
    /* R23.52 */
    STOVL_AT(N49DISP, 2),
    ADDRESS_PLUS_ONE(DELTAX),
    ORDER(ABVAL),
    ORDERS(BOF, SR2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(R23_53),
    /* R23.53 */
    STORE(N49DISP),
    ORDER(EXIT),
};

/* R23.6 -- after the EXIT: verb 06 noun 49 flashed by GOFLASHR. */
void r23_6(void)
{
    set_A(V6N49);
    tc_to(R23_6 + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* R23.6 +3: terminate. */
void r23_6_terminate(void)
{
    tc_to(R23_6 + 3u, GOTOPOOH);
}

/* R23.6 +4: "INCORPORATE DATA": TC R23.7. */
void r23_6_proceed(void)
{
    tc_at(R23_6 + 4u);
    r23_7();
}

/* R23.6 +5: enter. */
void r23_6_enter(void)
{
    tc_to(R23_6 + 5u, GOTOPOOH);
}

/* R23.6 +6, with the display up: "BLAN OUT R3". */
void r23_6_blank(void)
{
    set_A(BIT3);
    tc_to(R23_6 + 7u, BLANKET);
}

/* R23.6 +010: a restart point. */
void r23_6_after_blanket(void)
{
    tc_to(R23_6 + 010u, PHASCHNG); /* OCT 00012 */
}

/* R23.6 +012: and the job ends. */
void r23_6_after_phaschng(void)
{
    tc_to(R23_6 + 012u, ENDOFJOB);
}

/* R23.7: TC INTPRET, */
static void r23_7(void)
{
    tc_at(R23_7);
    intpret();
}

/* R23.8: VEHUPFLG set, and INCORP2. */
static const word r23_8_words[] = {
    ORDERS(SET, CALL),
    SWITCH(SET, VEHUPFLG),
    ADDRESS(INCORP2),
    ORDER(EXIT),
};

/* R23.END -- after the EXIT: P23 ends. */
void r23_end(void)
{
    tc_to(R23_END, GOTOPOOH);
}

/* ---- The interpretive code ---- */

const struct interpretive_code R23_55_CODE = {031u, R23_55, COUNT(r23_55_words), r23_55_words};
const struct interpretive_code R23_55_TRUNNION_CODE = {
    031u, R23_55 + 054u, COUNT(r23_55_trunnion_words), r23_55_trunnion_words};
const struct interpretive_code R23_8_CODE = {031u, R23_7 + 1u, COUNT(r23_8_words), r23_8_words};

#endif
