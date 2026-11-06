/* SPDX-License-Identifier: MIT */
/* P61-P67.agc of the CM, part 1 (Comanche055 pp. 795-802, bank 26): P63
 * to P67, the entry guidance's programs.
 *
 * P63 initializes the entry equations: the entry guidance (STARTENT) goes
 * on after CM/POSE, and the entry display shows G, VI and the range to
 * splash (V06N64). The guidance itself, once it runs, starts the others
 * by RTB as it passes from phase to phase: P64 at .05G, P65 the up-control
 * phase, P66 the ballistic phase, P67 the final phase; each shows its own
 * display through OVERNOUT (ENTRYVN). P67.1 ends the entry: the entry DAP
 * off, SERVEXIT the average-G exit, P00.
 *
 * Calls out of the file leave Z at the routine and Q at the word after the
 * TC, as in p61_p62.c; DANZIG (TC DANZIG) goes back to the guidance's
 * interpretive code. */
#include "p61_p67.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p65_1_flash(void);
static void p66end(void);
static void p67_1_dap_off(void);

/* ---- P63 -- pp. 795-796 ---- */

/* P63: "TO INITIALIZE THE ENTRY EQUATIONS." (NEWMODEX, MM 63). */
void p63(void)
{
    tc_to(P63, NEWMODEX); /* MM 63 */
}

/* P63 +2: "CONTINUE AT STARTENT AFTER CM/POSE." (ENTCADR to POSEXIT);
 * V06N64 to ENTRYVN, "FOR DISPLAY CALL IN OVERNOUT"; P63FLAG -1, so "THAT
 * EXO-ATM DAP WILL NOT CALL P63 OUT OF SEQUENCE IN P66."; a restart point
 * (PHASCHNG, OCT 00004), "REQUIRED TO PRESERVE CLEANDSP RETURN IN EVENT OF
 * AN EXTENDED VERB". (A holds a constant at each TS: none skips.) */
void p63_after_newmodex(void)
{
    set_A(ENTCADR);
    ts(ADDRESS_OF(POSEXIT));
    set_A(V06N64);
    ts(ADDRESS_OF(ENTRYVN));
    set_A(negate(ONE));
    ts(ADDRESS_OF(P63FLAG));
    tc_to(P63 + 010u, PHASCHNG); /* OCT 00004 */
}

/* P63 +012: "FLUSH 'N22' DISPLAY, IF ON" (CLEANDSP). */
void p63_after_phaschng(void)
{
    tc_to(P63 + 012u, BANKCALL); /* CADR CLEANDSP */
}

/* P63.1: group 4 disabled (PHASCHNG, OCT 00004): "DISPLAY RESTARTED VIA
 * ENTRY."; then ENDOFJOB. P62 and P65.1 come here too. */
void p63_1(void)
{
    tc_to(P63_1, PHASCHNG); /* OCT 00004 */
}

void p63_1_after_phaschng(void)
{
    tc_to(P63_1 + 2u, ENDOFJOB);
}

/* ---- P64 -- p. 797 ---- */

/* P64: "ENTER VIA RTB WHEN .05G IS EXCEEDED." (NEWMODEX, MM 64). */
void p64(void)
{
    tc_to(P64, NEWMODEX); /* MM 64 */
}

/* P64 +2: ROLLC, VI and D (V06N74) as the entry display, "DISPLAY VIA
 * OVERNOUT."; "... AND CONTINUE IN INITROLL ..." (DANZIG). (A is a
 * constant: TS does not skip.) */
void p64_after_newmodex(void)
{
    set_A(V06N74);
    ts(ADDRESS_OF(ENTRYVN));
    tc_to(P64 + 4u, DANZIG);
}

/* ---- P65 -- p. 798 ---- */

/* P65: "ENTER VIA RTB WHEN RANGE < 25 N M OF TARGET." (NEWMODEX, MM 65). */
void p65(void)
{
    tc_to(P65, NEWMODEX); /* MM 65 */
}

/* P65 +2: P65.1, the display, as a job (NOVAC, priority 13). */
void p65_after_newmodex(void)
{
    set_A(PRIO13);
    tc_to(P65 + 3u, NOVAC); /* 2CADR P65.1 */
}

/* P65 +6: two restart points (2PHSCHNG, OCT 00554 and OCT 10035), "TO
 * PREVENT RE-" starting the flashing display twice. */
void p65_after_novac(void)
{
    tc_to(P65 + 6u, TWO_PHSCHNG); /* OCT 00554, OCT 10035 */
}

void p65_after_2phschng(void)
{
    tc_at(P65 + 011u);
    intpret();
}

/* P65 +012: "CHANGE ENTRY MODE TO UPCONTRL." (GOTOADDR), and REFAZE10:
 * "GO HERE TO REESTABLISH ENTRY SEQUENCER." */
static const word p65_words[] = {
    /* P65 +012 */ ORDERS(SSP, RTB),
    ADDRESS_PLUS_ONE(GOTOADDR),
    ADDRESS(UPCONTRL),
    ADDRESS(REFAZE10),
};

const struct interpretive_code P65_CODE = {026u, P65 + 012u, COUNT(p65_words), p65_words};

/* P65.1: the entry display off (DOWNFLAG ENTRYDSP); ROLLC, DL (Q7) and VL
 * flashed (V16N69, GOFLASHR). */
void p65_1(void)
{
    tc_to(P65_1, DOWNFLAG); /* ADRES ENTRYDSP */
}

void p65_1_display(void)
{
    p65_1_flash();
}

static void p65_1_flash(void)
{
    set_A(V16N69);
    tc_to(P65_1 + 3u, BANKCALL); /* CADR GOFLASHR */
}

/* P65.1 +5: terminate (TC -3), "NODOFLAG IS SET ...": flash it again. */
void p65_1_terminate(void)
{
    tc_at(P65_1 + 5u);
    p65_1_flash();
}

/* P65.1 +6: proceed (TC +3): "IF PROCEED, CONTINUE": the entry display on
 * again (UPFLAG ENTRYDSP). */
void p65_1_proceed(void)
{
    tc_at(P65_1 + 6u);
    tc_to(P65_1 + 011u, UPFLAG); /* ADRES ENTRYDSP */
}

/* P65.1 +7: enter (TC -5): flash it again. */
void p65_1_enter(void)
{
    tc_at(P65_1 + 7u);
    p65_1_flash();
}

/* P65.1 +010: the immediate return: "EST. GRP 4 FOR DISPLAY AND DO
 * ENDOFJOB" (TC P61.3). */
void p65_1_immediate_return(void)
{
    tc_at(P65_1 + 010u);
    p61_3();
}

/* P65.1 +013: "DISABLE GRP 4, START UP ENTRY DISPLAY" (TC P63.1). */
void p65_1_after_upflag(void)
{
    tc_at(P65_1 + 013u);
    p63_1();
}

/* ---- P66 -- p. 799 ---- */

/* P66: "ENTER VIA RTB WHEN D < Q7 FPSS" (NEWMODEX, MM 66). */
void p66(void)
{
    tc_to(P66, NEWMODEX); /* MM 66 */
}

/* P66 +2: OGA, IGA and MGA (V06N22) as the entry display: "IN CASE CAME
 * FROM P65, GO TO DISABLE GRP 4, AND SET ENTRYDSP TO DO DISPLAY VIA
 * OVERNOUT." (TC P66END); "... AND CONTINUE AT KEP2". */
void p66_after_newmodex(void)
{
    set_A(V06N22);
    tc_at(P66 + 3u);
    p66end();
}

/* ---- P67 -- pp. 800-802 ---- */

/* P67: "TO TERMINATE STEERING WHEN THE CM VELOCITY WRT EARTH = 1000
 * FT/SEC" (NEWMODEX, MM 67). */
void p67(void)
{
    tc_to(P67, NEWMODEX); /* MM 67 */
}

/* P67 +2: ROLLC, XRNGERR and DNRNGERR (V06N66) as the entry display. */
void p67_after_newmodex(void)
{
    set_A(V06N66);
    p66end();
}

/* P66END: ENTRYVN, "DISPLAY VIA OVERNOUT."; the entry display on (UPFLAG
 * ENTRYDSP): "IN CASE CAME FROM P65. ENTRY DISPLAY WILL FLUSH FLASHING
 * DISP. IF STILL ON". (A holds a VN code: TS does not skip.) */
static void p66end(void)
{
    ts(ADDRESS_OF(ENTRYVN));
    tc_to(P66END + 1u, UPFLAG); /* ADRES ENTRYDSP */
}

/* KILLGRP4: "DISABLE GRP4, IN CASE CAME FROM HUNTEST." (PHASCHNG, OCT
 * 00004): "(COME TO KILLGRP4 VIA RTB, RET TO CALLER)"; "... AND CONTINUE
 * AT PREDICT3 ..." (DANZIG). */
void killgrp4(void)
{
    tc_to(KILLGRP4, PHASCHNG); /* OCT 00004 */
}

void killgrp4_after_phaschng(void)
{
    tc_to(KILLGRP4 + 2u, DANZIG);
}

/* P67.1: RTOGO, LAT and LONG flashed (V16N67, GOFLASH). */
void p67_1(void)
{
    set_A(V16N67);
    tc_to(P67_1 + 1u, BANKCALL); /* CADR GOFLASH */
}

/* P67.1 +3: terminate, the "EFFECTIVE GOTOPOOH" (TC +3), and +4, proceed
 * (TC +2): both end the entry. */
void p67_1_terminate(void)
{
    tc_at(P67_1 + 3u);
    p67_1_dap_off();
}

void p67_1_proceed(void)
{
    tc_at(P67_1 + 4u);
    p67_1_dap_off();
}

/* P67.1 +5: "REDO" (TC P67.1). */
void p67_1_enter(void)
{
    tc_at(P67_1 + 5u);
    p67_1();
}

/* P67.1 +6: "TURN OFF ENTRY DAP": CM/DSTBY and GAMDIFSW, bits 1 and 2 of
 * CM/FLAGS, cleared with the interrupts inhibited; SERVEXIT the average-G
 * exit again (SERVCAD2 = SERVCAD1, to AVEGEXIT); then P00 (TCF GOTOPOOH).
 * (After the MASK, A has no overflow: TS does not skip.) */
static void p67_1_dap_off(void)
{
    set_A(negate(THREE));
    inhint();
    mask_at(ADDRESS_OF(CM_FLAGS));
    ts(ADDRESS_OF(CM_FLAGS));
    relint();
    dca_2cadr(SERVCAD1);
    dxch(ADDRESS_OF(AVEGEXIT));
    continue_at(GOTOPOOH);
}

/* P67.2: "CALC PRESENT LAT, LONG, ALT." from RN at PIPTIME, "USE TIME OF
 * RN", "USE PAD RAD FOR ALT. (NOT SEEN ANYWAY)" (ERADFLAG clear), about
 * the earth (LUNAFLAG clear): LAT-LONG. P67.3: the "ENTRY EXIT THAT OMITS
 * DISPLAY." (SERVNOUT). */
static const word p67_2_words[] = {
    /* P67.2 */ ORDERS(VLOAD, CLEAR), ADDRESS_PLUS_ONE(RN),
    SWITCH(CLEAR, ERADFLAG),          STODL(ALPHAV),
    ADDRESS_PLUS_ONE(PIPTIME),        ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, LUNAFLAG),          ADDRESS(LAT_LONG),
    /* P67.3 */ ORDER(RTB),           ADDRESS(SERVNOUT),
};

const struct interpretive_code P67_2_CODE = {026u, P67_2, COUNT(p67_2_words), p67_2_words};

#endif
