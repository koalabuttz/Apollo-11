/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 3 (Comanche055 pp. 817-818, bank 10): S62.3,
 * which P62 calls for the entry attitude.
 *
 * "COMPUTE DESIRED GIMBAL ANGLES FOR ENTRY ATTITUDE": from the trajectory
 * triad CM/POSE keeps in reference coordinates (UXA, UYA, UZA, at half
 * scale), "GENERATE A DESIRED BODY TRIAD FOR TRIMMED FLIGHT WITH RESPECT
 * TO THE RELATIVE VELOCITY VECTOR, USING ROLL COMMAND AND TRIM ANGLE OF
 * ATTACK":
 *   UXD = UNIT(UYD*UXA) SIN(ALFATRIM) + UXA COS(ALFATRIM)
 *   UYD = UYA COS(ROLLC) + UZA SIN(ROLLC)
 *   UZD = UXD * UYD
 * as XNB, YNB and ZNB, with REFSMMAT's rows as XSM, YSM and ZSM; then
 * CALCGA, with CPHIFLAG clear, "CAUSE CALCGA TO STORE ANS IN TP CPHI": it
 * returns to S62.3's caller "VIA QPRET WITH 2'S COMP. ANGLES IN CPHI". */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word s62_3_words[] = {
    /* S62.3 */ ORDERS(SETPD, SLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(ALFAPAD), /* "ALFATRIM / 180, ALFA IS NEG." */
    ORDERS(SR1, PUSH),
    ORDERS(COS, PDDL), /* "XCH PDL, COS TO PDL0" */
    ORDERS(SIN, PDDL), /* "SIN TO PDL2" */
    ADDRESS_PLUS_ONE(ROLLC),
    ORDERS(COS, VXSC),
    ADDRESS_PLUS_ONE(UYA_2),
    ORDERS(PDDL, SIN), /* "PUSH VECTOR INTO PDL4...9" */
    ADDRESS_PLUS_ONE(ROLLC),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UZA_2), /* (the VAD's: "VECTOR FROM PDL4...9") */
    ORDER(VSL1),
    STORE(YNB), /* "= UYD" */
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UXA_2),
    ORDERS(VXSC, PDDL), /* "SIN TRIM FROM PDL2", "XCH PDL0 FOR COS TRIM" */
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UXA_2),
    ORDER(VSL1),
    STORE(XNB), /* "X SC AXIS (.5 UNIT)" */
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(YNB),
    STOVL(ZNB), /* "Z SC IN REF COOR. SCALED AT 2" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(XSM),
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 6),
    STOVL(YSM),
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 12),
    STORE(ZSM),
    ORDERS(CLEAR, GOTO),
    SWITCH(CLEAR, CPHIFLAG), /* "CAUSE CALCGA TO STORE ANS IN TP CPHI" */
    ADDRESS(CALCGA),
};

const struct interpretive_code S62_3_CODE = {010u, S62_3, COUNT(s62_3_words), s62_3_words};

#endif
