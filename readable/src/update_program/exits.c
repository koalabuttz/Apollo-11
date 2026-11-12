/* SPDX-License-Identifier: MIT */
/* UPDATE_PROGRAM.agc, part 5: the ways out of P27, UPOUT (the normal
 * finish), UPERROUT (the error exits) and UPACTOFF; LM pages 1394-1396, CM
 * pages 1506-1507. The faithful C is c64-port/src/lm_update.c and
 * cm_update.c. */
#include "update_program.h"

/* UPOUT -- LM p. 1394, CM p. 1506: "NORMAL FINISH OF P27". "RELEASE GRAB
 * OF ORBITAL INTEGRATION" (INTWAKEU). */
void upout(void)
{
    tc_to(UPOUT, INTWAKEU);
}

/* UPOUT +2, with a major mode in A: back to it (NEWMODEX +3). */
void upout_plus_2(void)
{
    tc_to((word)(UPOUT + 2u), (word)(NEWMODEX + 3u));
}

/* UPOUT +1 (UPOUT4), where INTWAKEU returns, the restart point of group
 * 6, and where a V34 "TERMINATE UPDATE": "RESTORE PRIOR P27 MODE". */
void upout4(void)
{
    ca(ADDRESS_OF(UPOLDMOD));
    upout_plus_2();
}

/* UPOUT +3, where NEWMODEX +3 returns: DNLSTCOD back to 0, "TURN OFF
 * 'UPLINK ACTIVITY' LIGHT", "KILL GROUP 6." (-PHASE6, PHASE6 = -0, +0),
 * and "EXTENDED VERB EXIT". */
void upout_after_newmodex(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DNLSTCOD));
    tc_at((word)(UPOUT + 5u)); /* TC UPACTOFF */
    upactoff();
    set_L(ZERO); /* EXTEND; DCA NEG0: NEG0 and the ZERO after it */
    set_A(NEG0);
    dxch(ADDRESS_OF(MINUS_PHASE6));
    tc_to((word)(UPOUT + 011u), ENDEXT);
}

/* UPERROUT -- LM p. 1396, CM p. 1507: "TURN ON *OPERATOR ERROR* LIGHT". */
void uperrout(void)
{
    tc_to(UPERROUT, FALTON);
}

/* UPERROUT +1, where FALTON returns: "GO TO COMMON UPDATE PROGRAM EXIT". */
void uperrout_after_falton(void)
{
    upout();
}

/* UPERROUT +2: "TURN ON 'OPERATOR ERROR' LIGHT". "THE PURPOSE OF UPERROUT
 * +2 EXIT IS TO PROVIDE AN ERROR EXIT WHICH DOES NOT RESET ANY RESTART
 * GROUPS": UPERROR comes here, before P27 has set one. */
void uperrout_quiet(void)
{
    tc_to((word)(UPERROUT + 2u), FALTON);
}

/* UPERROUT +3, where FALTON returns: "TURN OFF'UPLINK ACTIVITY'LIGHT", and
 * "EXTENDED VERB EXIT". */
void uperrout_quiet_after_falton(void)
{
    tc_at((word)(UPERROUT + 3u)); /* TC UPACTOFF */
    upactoff();
    tc_to((word)(UPERROUT + 4u), ENDEXT);
}

/* UPACTOFF -- LM p. 1396, CM p. 1507: "A ROUTINE TO TURN OFF UPLINK
 * ACTIVITY LIGHT ON ALL EXITS FROM UPDATE PROGRAM(P27).": "(BIT 3 OF
 * CHANNEL 11)", and TC Q. The CM's FRESH_START and P20-P25 call it too,
 * through BANKCALL. */
void upactoff(void)
{
    set_A(negate(BIT3));
    wand_dsalmout();
    return_through_q();
}
