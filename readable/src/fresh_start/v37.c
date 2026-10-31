/* SPDX-License-Identifier: MIT */
/* Verb 37 and P00: FRESH_START_AND_RESTART.agc of Luminary099 (LM, listing
 * pages 224-236) and Comanche055 (CM, pages 194-209), its part in bank 04
 * and the CM's VAC5STOR in bank 20. The faithful C is c64-port/src/
 * lm_v37.c and cm_v37.c. (R00's two interpretive words, CALL INTSTALL and
 * EXIT, run in the interpreter between r00() and r00_after_intpret().)
 *
 * V37 is how the astronaut changes program: "VERB 37 ENTER, NN ENTER"
 * with NN the major mode. Pinball's MMCHANG comes here with NN in A. The
 * listing's own account (LM p. 226, CM p. 195):
 *   "1. CHECK IF NEW PROGRAM ALLOWED. ... 2. CHECK FOR VALIDITY OF
 *    PROGRAM SELECTED. ... 3. SERVICER IS TERMINATED IF IT HAS BEEN
 *    RUNNING. 4. INSTALL IS EXECUTED TO AVOID INTERRUPTING INTEGRATION.
 *    5. THE ENGINE IS TURNED OFF AND THE DAP IS INITIALIZED FOR COAST.
 *    6. TRACK AND UPDATE FLAGS ARE SET TO ZERO. 7. DISPLAY SYSTEM IS
 *    RELEASED."
 * Then P00 (POOH) starts the 9-minute coasting integration, and any other
 * program is started by V37XEQ as a restart group 4 job, at the priority
 * and E bank its PREMM1 word gives, from its FCADRMM1 address.
 *
 * Every TC out of the file leaves through Z (Q holds the return), and the
 * place it returns to is its own routine. A CCS of a word that is never
 * negative (a MASK by a positive constant, a count, a major mode) uses
 * positive(), which does not model the other two cases. */
#include "v37.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#define MPAC1 (ADDRESS_OF(MPAC) + 1u)

/* Where the routines TC out of the file: label + offset, the TC's own
 * word (Q gets the word after it, or two after for a TC with a CADR or
 * ADRES after it). */
#if LUMINARY099
#define TC_DOWNFLAG_XOVINFLG (GOP00FIX + 0u)
#define TC_DOWNFLAG_ULLAGFLG (GOP00FIX + 2u)
#define TC_CLEARMRK          (GOP00FIX + 4u)
#define TC_GOFLASH           (GOP00FIX + 6u)
#define TC_PINBRNCH          (V37BAD + 1u)
#define TC_DOWNFLAG_AVEGFLAG (ISSERVON + 4u)
#define TC_PHSCHNGA          (CANV37 - 1u)
#define TC_PHASCHNG          (R00 - 2u)
#define TC_DOWNFLAG_3AXISFLG (R00 + 4u)
#define TC_DOWNFLAG_R04FLAG  (R00 + 010u)
#define TC_DOWNFLAG_MUNFLAG  (R00 + 012u)
#define TC_DOWNFLAG_XOVIN2   (R00 + 014u)
#define TC_CLRADMOD_POOH     (POOH + 3u)
#define TC_ENGINOF1          (SEUDOP00 + 2u)
#define TC_ALLCOAST          (SEUDOP00 + 4u)
#define TC_V37KLEAN          (SEUDOP00 + 012u)
#define TC_P00KLEAN          (GOMOD + 0u)
#define TC_CLRADMOD_RESET22  (RESET22 + 3u)
#define TC_DOWNFLAG_IMUSE    (NOUVEAU + 4u)
#define TC_SPVAC             (V37XEQ + 026u)
#define DTCB_P70             (SETUP70 + 4u)
#else
#define TC_INITSUB_GOP00FIX  (GOP00FIX + 0u)
#define TC_CLEARMRK          (GOP00FIX + 1u)
#define TC_GOFLASH           (GOP00FIX + 3u)
#define TC_SPSOFF            (R00TOP00 + 5u)
#define TC_MASSPROP          (R00TOP00 + 7u)
#define TC_RCSDAPON          (R00TOP00 + 012u)
#define TC_TVCZAP            (R00TOP00 + 014u)
#define TC_DELAYJOB          (R00TOP00 + 022u)
#define TC_PINBRNCH          (V37BAD + 1u)
#define TC_PHASCHNG          (CANV37 + 2u)
#define TC_INITSUB_ROC       (ROC + 015u)
#define TC_CLEARMRK_ROC      (ROC + 016u)
#define TC_DOWNFLAG_STIKFLAG (ROC + 017u)
#define TC_UPACTOFF          (ROC + 021u)
#define TC_DOWNFLAG_VHFRFLAG (ROC + 023u)
#define TC_DOWNFLAG_R21MARK  (ROC + 025u)
#define TC_V37KLEAN          (GROUPKIL + 0u)
#define TC_P00KLEAN          (GROUPKIL + 4u)
#define TC_DOWNFLAG_IMUSE    (NOUVEAU + 4u)
#define TC_SPVAC             (V37XEQ + 026u)
#define TC_STOPRATE          (INITSUB + 5u)
#define TC_SETMINDB          (SETMAXER - 3u)
#define TC_SETMAXDB          (SETMAXER + 0u)
#define TC_UPFLAG_IMPULSW    (RAKE + 013u)
#define TC_GENTRAN           (V5OUT1 + 3u)
#define TC_SWRETURN          (V5OUT1 + 036u)
#endif
#define TC_ALARM_1520     (V37BAD - 2u) /* CANTROD (LM), CANTR00 (CM) */
#define TC_RELDSP_V37BAD  (V37BAD + 0u)
#define TC_INTPRET        (DUMMYAD - 3u) /* R00 (CM: ROC): TC INTPRET; CALL; INTSTALL */
#define TC_RELDSP_POOH    (POOH + 0u)
#define TC_POSTJUMP_PROG2 (GOGOPROG + 0u)
#define TC_FALTON         (V37NONO + 0u)
#define TC_NEWMODEA       (V37XEQC + 2u)
#define TC_RELDSP_V37XEQC (V37XEQC + 3u)
#define TC_ENDOFJOB       (V37XEQC + 4u)

/* ---- The tables ---- */

/* "FOR VERB 37 TWO TABLES ARE MAINTAINED. EACH TABLE HAS AN ENTRY FOR
 * EACH MAJOR MODE THAT CAN BE STARTED FROM THE KEYBOARD. THE ENTRIES ARE
 * PUT INTO THE TABLE WITH THE ENTRY FOR THE HIGHEST MAJOR MODE COMING
 * FIRST, TO THE LOWEST MAJOR MODE WHICH IS THE LAST ENTRY IN EACH TABLE."
 * (LM p. 233, CM p. 206.) A third, DNLADMM1, gives each one's downlist. */

/* The downlists (DNLSTCOD values), by the listing's = names. */
#if LUMINARY099
enum { COSTALIN = 0, AGSUPDAT = 1, RENDEZVU = 2, ORBMANUV = 3, DESASCNT = 4, LUNRSALN = 5 };
#else
enum { COSTALIN = 0, ENTRYUPD = 1, RENDEZVU = 2, POWERED = 3, P22DNLST = 4 };
#endif

/* FCADRMM1 -- LM pp. 233-234, CM pp. 207-208.
 * "THE FCADRMM TABLE CONTAINS THE FCADR OF THE STARTING JOB OF THE MAJOR
 * MODE." */
/* clang-format off */
const word FCADRMM1[] = {
#if LUMINARY099
    F_P79,   F_P78,    F_P76,   F_P75,    F_P74,   F_P73,  F_P72,    F_LANDJUNK, F_P63LM, F_P57,
    F_PROG52, F_P51,   F_P47LM, F_P42LM,  F_P41LM, F_P40LM, F_P39,   F_P38,      F_P35,   F_P34,
    F_P33,   F_P32,    F_P31,   F_P30,    F_PROG25, F_PROG22, F_PROG21, F_PROG20, F_P12LM, F_P06,
#else
    F_P79,    F_P78,    F_P77,    F_P76,  F_P75,    F_P74,    F_P73,    F_P72,  F_P62,
    F_P61,    F_P54,    F_P53,    F_PROG52, F_P51,  F_P47CSM, F_P41CSM, F_P40CSM, F_P39,
    F_P38,    F_P37,    F_P35,    F_P34,  F_P33,    F_P32,    F_P31,    F_P30,  F_P23,
    F_PROG22, F_PROG21, F_PROG20, F_P17,  F_P06,
    F_GTSCPSS1, /* "GYROCOMPASS STANDARD LEAD IN" (P01) */
#endif
};
/* clang-format on */

/* PREMM1 -- LM pp. 234-235, CM pp. 206-207.
 * "THE PREMM TABLE CONTAINS THE E-BANK, MAJOR MODE, AND PRIORITY
 * INFORMATION, IT IS IN THE FOLLOWING FORM, PPP PPE EEM MMM MMM WHERE THE
 * 7 M BITS CONTAIN THE MAJOR MODE NUMBER, 3 E BITS CONTAIN THE E-BANK
 * NUMBER, 5 P BITS CONTAIN THE PRIORITY AT WHICH THE JOB IS TO BE
 * STARTED." */
const word PREMM1[] = {
#if LUMINARY099
    027717u, /* MM 79 EBANK 7 PRIO 13 */
    027716u, /* MM 78 EBANK 7 PRIO 13 */
    027714u, /* MM 76 EBANK 7 PRIO 13 */
    027713u, /* MM 75 EBANK 7 PRIO 13 */
    027712u, /* MM 74 EBANK 7 PRIO 13 */
    027711u, /* MM 73 EBANK 7 PRIO 13 */
    027710u, /* MM 72 EBANK 7 PRIO 13 */
    027704u, /* MM 68 EBANK 7 PRIO 13 */
    027677u, /* MM 63 EBANK 7 PRIO 13 */
    027271u, /* MM 57 EBANK 5 PRIO 13 */
    027264u, /* MM 52 EBANK 5 PRIO 13 */
    027263u, /* MM 51 EBANK 5 PRIO 13 */
    027657u, /* MM 47 EBANK 7 PRIO 13 */
    027652u, /* MM 42 EBANK 7 PRIO 13 */
    027651u, /* MM 41 EBANK 7 PRIO 13 */
    027650u, /* MM 40 EBANK 7 PRIO 13 */
    027647u, /* MM 39 EBANK 7 PRIO 13 */
    027646u, /* MM 38 EBANK 7 PRIO 13 */
    027643u, /* MM 35 EBANK 7 PRIO 13 */
    027642u, /* MM 34 EBANK 7 PRIO 13 */
    027641u, /* MM 33 EBANK 7 PRIO 13 */
    027640u, /* MM 32 EBANK 7 PRIO 13 */
    027637u, /* (MM 31) */
    027636u, /* MM 30 EBANK 7 PRIO 13 */
    027631u, /* MM 25 EBANK 7 PRIO 13 */
    027626u, /* MM 22 EBANK 7 PRIO 13 */
    027625u, /* MM 21 EBANK 7 PRIO 13 */
    027624u, /* MM 20 EBANK 7 PRIO 13 */
    027614u, /* MM 12 EBANK 7 PRIO 13 */
    027006u, /* MM 06 EBANK 4 PRIO 13 */
#else
    027117u, /* MM 79 EBANK 4 PRIO 13 */
    027116u, /* MM 78 EBANK 4 PRIO 13 */
    027115u, /* MM 77 EBANK 4 PRIO 13 */
    027714u, /* MM 76 EBANK 4 PRIO 13 */
    027113u, /* MM 75 EBANK 4 PRIO 13 */
    027112u, /* MM 74 EBANK 4 PRIO 13 */
    027111u, /* MM 73 EBANK 4 PRIO 13 */
    027110u, /* MM 72 EBANK 4 PRIO 13 */
    027476u, /* MM 62 EBANK 6 PRIO 13 */
    027475u, /* MM 61 EBANK 6 PRIO 13 */
    027266u, /* MM 54 EBANK 5 PRIO 13 */
    027265u, /* MM 53 EBANK 5 PRIO 13 */
    027264u, /* MM 52 EBANK 5 PRIO 13 */
    027263u, /* MM 51 EBANK 5 PRIO 13 */
    027657u, /* MM 47 EBANK 7 PRIO 13 */
    027451u, /* MM 41 EBANK 6 PRIO 13 */
    027450u, /* MM 40 EBANK 6 PRIO 13 */
    027047u, /* MM 39 EBANK 4 PRIO 13 */
    027046u, /* MM 38 EBANK 4 PRIO 13 */
    027645u, /* MM 37 EBANK 7 PRIO 13 */
    027043u, /* MM 35 EBANK 4 PRIO 13 */
    027042u, /* MM 34 EBANK 4 PRIO 13 */
    027041u, /* MM 33 EBANK 4 PRIO 13 */
    027040u, /* MM 32 EBANK 4 PRIO 13 */
    027637u, /* MM 31 EBANK 7 PRIO 13 */
    027636u, /* MM 30 EBANK 7 PRIO 13 */
    027227u, /* MM 23 EBANK 5 PRIO 13 */
    027226u, /* MM 22 EBANK 5 PRIO 13 */
    027025u, /* MM 21 EBANK 4 PRIO 13 */
    027424u, /* MM 20 EBANK 6 PRIO 13 */
    027021u, /* MM 17 EBANK 6 PRIO 13 */
    027006u, /* MM 06 EBANK 4 PRIO 13 */
    041201u, /* MM 01 EBANK 5 PRIO 20 */
#endif
};

/* DNLADMM1 -- LM p. 235, CM pp. 208-209.
 * Each program's downlist, "OBTAIN APPROPIRATE DOWNLIST ADDRESS". */
/* clang-format off */
const word DNLADMM1[] = {
#if LUMINARY099
    RENDEZVU, /* P79 */ RENDEZVU, /* P78 */ RENDEZVU, /* P76 */ RENDEZVU, /* P75 */
    RENDEZVU, /* P74 */ RENDEZVU, /* P73 */ RENDEZVU, /* P72 */ DESASCNT, /* P68 */
    DESASCNT, /* P63 */ LUNRSALN, /* P57 */ COSTALIN, /* P52 */ COSTALIN, /* P51 */
    ORBMANUV, /* P47 */ ORBMANUV, /* P42 */ ORBMANUV, /* P41 */ ORBMANUV, /* P40 */
    RENDEZVU, /* P39 */ RENDEZVU, /* P38 */ RENDEZVU, /* P35 */ RENDEZVU, /* P34 */
    RENDEZVU, /* P33 */ RENDEZVU, /* P32 */ RENDEZVU, /* P31LM */ RENDEZVU, /* P30 */
    RENDEZVU, /* P25 */ LUNRSALN, /* P22 */ RENDEZVU, /* P21 */ RENDEZVU, /* P20 */
    DESASCNT, /* P12 */ COSTALIN, /* P06 */
#else
    RENDEZVU, /* P79 */ RENDEZVU, /* P78 */ RENDEZVU, /* P77 */ RENDEZVU, /* P76 */
    RENDEZVU, /* P75 */ RENDEZVU, /* P74 */ RENDEZVU, /* P73 */ RENDEZVU, /* P72 */
    ENTRYUPD, /* P62 */ POWERED,  /* P61 */ COSTALIN, /* P54 */ COSTALIN, /* P53 */
    COSTALIN, /* P52 */ COSTALIN, /* P51 */ POWERED,  /* P47 */ POWERED,  /* P41 */
    POWERED,  /* P40 */ RENDEZVU, /* P39 */ RENDEZVU, /* P38 */ RENDEZVU, /* P37 */
    RENDEZVU, /* P35 */ RENDEZVU, /* P34 */ RENDEZVU, /* P33 */ RENDEZVU, /* P32 */
    RENDEZVU, /* P31 */ RENDEZVU, /* P30 */ RENDEZVU, /* P23 */ P22DNLST, /* P22 */
    RENDEZVU, /* P21 */ RENDEZVU, /* P20 */ RENDEZVU, /* P17 */ COSTALIN, /* P06 */
    COSTALIN, /* P01 */
#endif
};
/* clang-format on */

/* NOV37MM: "THE NUMBER OF ENTRIES IN EACH OF THE ABOVE LISTS-1". */
const unsigned V37_PROGRAMS = COUNT(PREMM1);

#if COMANCHE055
/* FLAGTABL -- CM p. 204. The bits RAKE clears in flag words 0-11. */
const word FLAGTABL[12] = {
    000000u, 000040u,          /* IDLEFAIL */
    006000u,                   /* P21FLAG, STEERSW */
    000000u, 000000u, 004140u, /* V59FLAG, ENGONFLG, 3AXISFLG */
    010000u,                   /* STRULLSW */
    016000u, 000000u, 042000u, /* SWTOVER, V94FLAG */
    000000u, 000000u,
};
#endif

/* INDEX k; CAF table: the table's word for the index in k. An index
 * outside the table reads the words after it, which this does not model. */
static word table_word(const word *table, unsigned address)
{
    word index = fetch(address);
    if (index >= V37_PROGRAMS) not_modelled();
    return table[index];
}

/* CCS of a word that is never negative: true if positive (A gets it less
 * one), false for +0 (A gets +0). */
static bool positive(enum sign sign)
{
    if (sign == NEGATIVE || sign == IS_MINUS_ZERO) not_modelled();
    return sign == POSITIVE;
}

/* EXTEND; DCA NEG0; DXCH -PHASEn: end restart group n (-PHASEn gets -0,
 * PHASEn +0). */
static void kill_group(unsigned minus_phase)
{
    set_L(ZERO);
    set_A(NEG0);
    dxch(minus_phase);
}

/* TC to a routine outside the file. */
static void tc_out(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

static void isitp00(void);
static void cantrod(void);
static void checktab(void);
static void isservon(void);
static void seudop00(void);
static void gogoprog(void);
static void rendv00(void);
static void nouveau(void);
static void v37nono(void);
#if LUMINARY099
static void setup70(void);
static void setup71(void);
#else
static void r00top00(void);
static void initsub(void);
static void return_from_initsub(void);
#endif

/* ---- GOP00FIX ---- */

/* GOP00FIX -- LM pp. 224-225, CM p. 194.
 * Where GOTOPOOH sends P00: the LM allows the X-axis override and makes
 * sure the ullage is off, the CM does INITSUB; then "RELEASE MARK DISPLAY
 * SYSTEM" and flash V37 N99, asking for a program. */
void gop00fix(void)
{
#if LUMINARY099
    tc_out(TC_DOWNFLAG_XOVINFLG, DOWNFLAG); /* "ALLOW X-AXIS OVERRIDE" */
#else
    tc_at(TC_INITSUB_GOP00FIX);
    initsub();
#endif
}

#if LUMINARY099
/* GOP00FIX +2 -- LM p. 224. "INSURE THAT ULLAGE IS OFF". */
void gop00fix_after_downflag_xovinflg(void)
{
    tc_out(TC_DOWNFLAG_ULLAGFLG, DOWNFLAG);
}

/* GOP00FIX +4 -- LM p. 225. */
void gop00fix_after_downflag_ullagflg(void)
{
    tc_out(TC_CLEARMRK, CLEARMRK + 2u); /* "RELEASE MARK DISPLAY SYSTEM." */
}
#endif

/* GOP00FIX, after CLEARMRK -- LM p. 225, CM p. 194.
 * Flash V37 N99 (GOFLASH). Whatever the astronaut keys (terminate, proceed
 * or enter: TCF -3, -4 or -5), it flashes again, until a V37 comes. */
void gop00fix_after_clearmrk(void)
{
    set_A(V37N99);
    tc_out(TC_GOFLASH, BANKCALL); /* CADR GOFLASH */
}

/* GOP00FIX, after GOFLASH -- LM p. 225, CM p. 194. */
void gop00fix_after_goflash(void)
{
    gop00fix_after_clearmrk();
}

/* ---- V37 ---- */

/* V37 -- LM p. 227, CM pp. 196-197.
 * A holds the major mode asked for. "IS IMU BEING INITIALIZED" (IMODES30
 * bit 6): then not now (CANTROD). The LM goes to SETUP70 or SETUP71 for
 * P70 and P71, its aborts; the CM, with the engine on or its TVC DAP on,
 * first goes back to the RCS DAP (R00TOP00). */
void v37(void)
{
    ts(ADDRESS_OF(MMNUMBER)); /* "SAVE MAJOR MODE" */
    set_A(PRIO30);            /* "RESTART AT PINBALL PRIORITY" */
    ts(ADDRESS_OF(RESTREG));
    ca(ADDRESS_OF(IMODES30)); /* "IS IMU BEING INITIALIZED" */
    mask(BIT6);
    if (positive(ccs_register(A_ADDRESS))) {
        cantrod();
        return;
    }
#if LUMINARY099
    cs(ADDRESS_OF(MMNUMBER)); /* "IS P70 REQUESTED?" */
    ad(DEC70);
    if (A_is_zero()) {
        setup70();
        return;
    }
    ad(ONE); /* "IS P71 REQUESTED?" */
    if (A_is_zero()) {
        setup71();
        return;
    }
#else
    set_A(BIT13); /* "IS ENGINE ON" */
    rand_channel(DSALMOUT);
    if (positive(ccs_register(A_ADDRESS))) {
        r00top00(); /* "YES, SET UP FOR P00" */
        return;
    }
    cs(ADDRESS_OF(FLAGWRD6)); /* "NO, IS TVC DAP ON" */
    mask(OCT60000);
    if (!A_is_zero_or_minus()) {
        r00top00();
        return;
    }
#endif
    isitp00(); /* "NO, CONTINUE WITH R00" */
}

/* ISITP00 (the LM's has no label) -- LM p. 227, CM p. 197.
 * "IS NEW REQUEST P00": then straight to ISSERVON. Any other program only
 * if "NODO V37" (NODOFLAG) is clear, and only if it is listed (CHECKTAB). */
static void isitp00(void)
{
    ca(ADDRESS_OF(MMNUMBER));
    if (A_is_zero()) {
        isservon(); /* "YES, CHECK SERVICER STATUS" */
        return;
    }
    cs(ADDRESS_OF(FLAGWRD2)); /* "NO, IS NODO V37 FLAG SET" */
    mask(NODOBIT);
    if (positive(ccs_register(A_ADDRESS))) {
        checktab(); /* "NO" */
        return;
    }
    cantrod();
}

/* CANTROD (CM: CANTR00) -- LM p. 227, CM p. 197.
 * Alarm 1520, "MAJOR MODE CHANGE NOT PERMITTED". ALARM returns to V37BAD. */
static void cantrod(void)
{
    tc_out(TC_ALARM_1520, ALARM); /* OCT 1520 */
}

/* V37BAD -- LM p. 227, CM p. 197.
 * "RELEASES DISPLAY FROM ASTRONAUT", then "BRING BACK LAST NORMAL DISPLAY
 * IF THERE WAS ONE. OTHERWISE DO AN EOJ." (PINBRNCH). */
void v37bad(void)
{
    tc_out(TC_RELDSP_V37BAD, RELDSP);
}

void v37bad_after_reldsp(void)
{
    tc_out(TC_PINBRNCH, POSTJUMP); /* CADR PINBRNCH */
}

/* CHECKTAB, AGAINMM -- LM pp. 227-228, CM p. 197.
 * Look for the major mode in PREMM1, from its last word (the lowest
 * program) up: COM; AD MMNUMBER leaves MMNUMBER less the listed mode,
 * which is -0 when they are equal (x + -x is -0 in ones' complement).
 * Greater: on to the next ("IF GR, SEE IF ANY MORE IN LIST"); less, or no
 * more: not listed ("LAST TIME OR PASSED MM", V37NONO). The index of the
 * one found goes to MINDEX "FOR LATER". */
static void checktab(void)
{
    set_A(NOV37MM); /* "INDEX FOR MM TABLES." */
    for (;;) {      /* AGAINMM */
        ts(MPAC1);
        set_A(table_word(PREMM1, MPAC1)); /* "OBTAIN WHICH MM THIS IS FOR" */
        mask(LOW7);
        com();
        ad_at(ADDRESS_OF(MMNUMBER));
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE:
            switch (ccs_at(MPAC1)) {
            case POSITIVE: continue; /* "YES, GET NEXT ONE" */
            case IS_PLUS_ZERO: v37nono(); return;
            case NEGATIVE: ca(MPAC1); break; /* never: MPAC +1 counts down */
            case IS_MINUS_ZERO: break;       /* never */
            }
            break;
        case IS_PLUS_ZERO: continue;      /* TCF AGAINMM, with +0 */
        case NEGATIVE: v37nono(); return; /* "LAST TIME OR PASSED MM" */
        case IS_MINUS_ZERO: ca(MPAC1); break;
        }
        ts(ADDRESS_OF(MINDEX)); /* "SAVE INDEX FOR LATER" */
        isservon();
        return;
    }
}

/* ISSERVON -- LM p. 228, CM p. 197.
 * "V37 FLAG SET -- I.E., IS SERVICER GOING": then "TURN OFF THE AVERAGE
 * FLAG AND WAIT FOR SERVICER TO RETURN TO CANV37" (the LM through V37RET,
 * which OUTROUTE names, the CM straight). */
static void isservon(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(V37FLBIT);
    if (positive(ccs_register(A_ADDRESS))) {
        canv37(); /* "NO" */
        return;
    }
#if LUMINARY099
    tc_out(TC_DOWNFLAG_AVEGFLAG, DOWNFLAG); /* ADRES AVEGFLAG */
#else
    inhint();
    set_A(negate(AVEGBIT));
    mask_at(ADDRESS_OF(FLAGWRD1));
    ts(ADDRESS_OF(FLAGWRD1));
    continue_at(ENDOFJOB);
#endif
}

#if LUMINARY099
/* ISSERVON, after DOWNFLAG -- LM p. 228. */
void isservon_after_downflag(void)
{
    set_A(V37RETAD);
    ts(ADDRESS_OF(OUTROUTE));
    continue_at(ENDOFJOB);
}

/* V37RET -- LM p. 228.
 * SERVICER's last pass comes here. Restart group 2 gets its phase: 2.7
 * ("IS P20 OR P22 RUNNING?"), 2.11 (P25 running) or 2.0 (neither), each
 * with its own restart point; then CANV37. */
void v37ret(void)
{
    cs(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (positive(ccs_register(A_ADDRESS))) { /* "NO. CHECK FOR P25." */
        cs(ADDRESS_OF(FLAGWRD0));
        mask(P25FLBIT);
        if (positive(ccs_register(A_ADDRESS))) set_A(OCT37667); /* 2.0SPT */
        ad(BIT5);                                               /* 2.11SPT */
    }
    ad(OCT40072); /* 2.7SPT */
    tc_out(TC_PHSCHNGA, PHSCHNGA);
}
#endif

/* CANV37 -- LM p. 228, CM p. 197.
 * The LM selects superbank 0 (WRITE SUPERBNK). The restart point R00AD
 * goes to TEMPFLSH, and group 1 to phase 4 (PHASCHNG, OCT 14). */
void canv37(void)
{
#if LUMINARY099
    set_A(ZERO);
    write_superbnk();
#endif
    set_A(R00AD);
    ts(ADDRESS_OF(TEMPFLSH));
    tc_out(TC_PHASCHNG, PHASCHNG); /* OCT 14 */
}

/* R00 (CM: ROC) -- LM p. 229, CM p. 197.
 * "WAIT FOR INTEGRATION TO FINISH": CALL INTSTALL, interpretive, then EXIT
 * (DUMMYAD) back to the word after. */
#if LUMINARY099
void r00(void)
#else
void roc(void)
#endif
{
    tc_out(TC_INTPRET, INTPRET);
}

#if LUMINARY099

/* R00, after the interpretive CALL INTSTALL; EXIT -- LM p. 229.
 * Clear the 3-axis flag, R12's (FLGWRD11: "CLEAN UP THE R12 FLAGWORD"),
 * R04's, MUNFLAG and the X-axis override's. */
void r00_after_intpret(void)
{
    tc_out(TC_DOWNFLAG_3AXISFLG, DOWNFLAG); /* "RESET 3-AXIT FLAG" */
}

void r00_after_downflag_3axisflg(void)
{
    set_A(LRBYBIT);
    ts(ADDRESS_OF(FLGWRD11));
    tc_out(TC_DOWNFLAG_R04FLAG, DOWNFLAG); /* "INSURE THAT THE R04FLAG IS CLEAR." */
}

void r00_after_downflag_r04flag(void)
{
    tc_out(TC_DOWNFLAG_MUNFLAG, DOWNFLAG); /* "INSURE MUNFLAG IS CLEAR." */
}

void r00_after_downflag_munflag(void)
{
    tc_out(TC_DOWNFLAG_XOVIN2, DOWNFLAG); /* "ALLOW X-AXIS OVERRIDE." */
}

/* "IS THIS A POOH REQUEST": no, NOUVEAU; yes, POOH. */
void r00_after_downflag_xovinflg(void)
{
    if (positive(ccs_at(ADDRESS_OF(MMNUMBER)))) {
        nouveau(); /* "NO, PICK UP NEW PROGRAM" */
        return;
    }
    tc_out(TC_RELDSP_POOH, RELDSP); /* POOH: "RELEASE DISPLAY SYSTEM" */
}

#else

/* ROC, after the interpretive CALL INTSTALL; EXIT -- CM pp. 197-198.
 * "CLEAR CAUTION RESET AND TEST CONNECTOR OUTBIT" (channel 11); clear
 * the optics error counter's enable, star trackers, TVC enable, zero
 * optics, optics DAP and the S-IVB bits (channel 12) and the unused bits
 * of channel 13; then INITSUB. */
void roc_after_intpret(void)
{
    set_A(negate(OCT1400));
    wand_channel(011u);
    set_A(OCT44571);
    wand_channel(012u);
    set_A(negate(OCT600)); /* "CLEAR UNUSED BITS" */
    wand_channel(013u);
    tc_at(TC_INITSUB_ROC);
    initsub();
}

/* After INITSUB: "TC CLEARMRK". */
static void roc_after_initsub(void)
{
    tc_out(TC_CLEARMRK_ROC, CLEARMRK);
}

void roc_after_clearmrk(void)
{
    tc_out(TC_DOWNFLAG_STIKFLAG, DOWNFLAG); /* ADRES STIKFLAG */
}

void roc_after_downflag_stikflag(void)
{
    tc_out(TC_UPACTOFF, BANKCALL); /* "TURN OFF UPLINK ACTIV LIGHT" */
}

void roc_after_upactoff(void)
{
    tc_out(TC_DOWNFLAG_VHFRFLAG, DOWNFLAG); /* ADRES VHFRFLAG */
}

void roc_after_downflag_vhfrflag(void)
{
    tc_out(TC_DOWNFLAG_R21MARK, DOWNFLAG); /* ADRES R21MARK */
}

/* "IS THIS A POOH REQUEST": no, NOUVEAU; yes, POOH. */
void roc_after_downflag_r21mark(void)
{
    if (positive(ccs_at(ADDRESS_OF(MMNUMBER)))) {
        nouveau(); /* "NO, PICK UP NEW PROGRAM" */
        return;
    }
    tc_out(TC_RELDSP_POOH, RELDSP); /* POOH: "RELEASE DISPLAY SYSTEM" */
}

#endif

/* ---- P00 ---- */

/* POOH, after RELDSP -- LM p. 229, CM p. 198.
 * "SET VARIABLE RESTART PRIORITY FOR P00 INTEGRATION", interrupts off
 * (the LM's CLRADMOD "DOES AN INHINT"), "TURN OFF NODOFLAG", restart
 * group 2 to phase 5 ("SET RESTART FOR STATEINT1"), and clear the
 * rendezvous and P25 flags and IMUSE: P00 starts with downlist 0. */
void pooh_after_reldsp(void)
{
    set_A(PRIO5);
    ts(ADDRESS_OF(PHSPRDT2));
#if LUMINARY099
    tc_out(TC_CLRADMOD_POOH, CLRADMOD);
}

void pooh_after_clradmod(void)
{
#else
    inhint();
#endif
    set_A(negate(NODOBIT)); /* "TURN OFF NODOFLAG." */
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));
    set_A(FIVE);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE2));
#if LUMINARY099
    set_A(negate(OCT700)); /* "TURN OFF P20, P25, IMU IN USE FLAG" */
#else
    set_A(negate(BIT7_8)); /* "RESET IMUSE + KILL P20" */
#endif
    mask_at(ADDRESS_OF(FLAGWRD0));
    ts(ADDRESS_OF(FLAGWRD0));
    set_A(DNLADP00);
    seudop00();
}

/* SEUDOP00 -- LM pp. 229-230, CM p. 198.
 * "SET UP APPROPRIATE DOWNLIST CODE" (the current list finishes first).
 * The LM turns the engine off (ENGINOF1) and the DAP to coast (ALLCOAST);
 * both "TURN OFF TRACK, UPDATE FLAGS" (the CM also TARG1) and end
 * restart groups 1, 3, 5 and 6 (CM: 3, 5, 6) with V37KLEAN. */
static void seudop00(void)
{
    ts(ADDRESS_OF(DNLSTCOD));
#if LUMINARY099
    ts(ADDRESS_OF(AGSWORD));
    tc_out(TC_ENGINOF1, IBNKCALL); /* CADR ENGINOF1 */
}

void seudop00_after_enginof1(void)
{
    tc_out(TC_ALLCOAST, IBNKCALL); /* "INSURE ALLCOAST." */
}

void seudop00_after_allcoast(void)
{
    set_A(negate(OCT120));
#else
    set_A(negate(OCT01120));
#endif
    ts(ADDRESS_OF(EBANKTEM));
    mask_at(ADDRESS_OF(FLAGWRD1));
    ts(ADDRESS_OF(FLAGWRD1));
    tc_out(TC_V37KLEAN, IBNKCALL); /* GROUPKIL: CADR V37KLEAN */
}

/* After V37KLEAN -- LM p. 230, CM p. 198.
 * "IS IT POOH": no, RENDV00; yes, P00KLEAN ("REDUNDANT EXCEPT FOR GROUP
 * 4") and GOMOD. */
#if LUMINARY099
void seudop00_after_v37klean(void)
#else
void groupkil_after_v37klean(void)
#endif
{
    if (positive(ccs_at(ADDRESS_OF(MMNUMBER)))) {
        rendv00();
        return;
    }
    tc_out(TC_P00KLEAN, IBNKCALL); /* CADR P00KLEAN */
}

/* GOMOD (LM: after P00KLEAN) -- LM p. 230, CM p. 198.
 * The new major mode into MODREG, and the software restart (GOPROG2)
 * starts the restart groups still set. */
#if LUMINARY099
void gomod_after_p00klean(void)
#else
void gomod(void)
#endif
{
    ca(ADDRESS_OF(MMNUMBER));
    ts(ADDRESS_OF(MODREG));
    gogoprog();
}

/* GOGOPROG -- LM p. 230, CM p. 198. */
static void gogoprog(void)
{
    tc_out(TC_POSTJUMP_PROG2, POSTJUMP); /* CADR GOPROG2 */
}

/* P00FIZZ and the like: CAF V37QCAD; TS TEMPFLSH: the "RESTART POINT FOR
 * V37XEQ", then GOGOPROG. */
static void p00fizz(void)
{
    set_A(V37QCAD);
    ts(ADDRESS_OF(TEMPFLSH));
    gogoprog();
}

#if LUMINARY099

/* KILL2 -- LM p. 231. "NO, KILL 2" (restart group 2), then P00FIZZ. */
void kill2(void)
{
    kill_group(ADDRESS_OF(MINUS_PHASE2));
    p00fizz();
}

/* RESET22 -- LM p. 230. "CLEAR RENDEZVOUS, P25 AND IMU IN USE FLAGS",
 * CLRADMOD, KILL2. */
static void reset22(void)
{
    set_A(negate(OCT700));
    mask_at(ADDRESS_OF(FLAGWRD0));
    ts(ADDRESS_OF(FLAGWRD0));
    tc_out(TC_CLRADMOD_RESET22, CLRADMOD);
}

/* STATQUO -- LM p. 231. "SET TRACKFLAG, UPDATE FLAG", then GOMOD. */
static void statquo(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(OCT120);
    ads(ADDRESS_OF(FLAGWRD1));
    tc_out(TC_P00KLEAN, IBNKCALL); /* GOMOD: CADR P00KLEAN */
}

/* RENDN00 -- LM p. 231. P20 or P25 asked for. "P20 OR P25 ON TOP OF P20
 * OR P25" (MODREG 20 or 25): KILL2. The one asked for already running in
 * the background (P20 with the rendezvous flag, P25 with P25's): leave it
 * as it is (STATQUO); else KILL2. */
static void rendn00(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(OCT24);
    if (A_is_zero()) {
        kill2();
        return;
    }
    ad(FIVE);
    if (A_is_zero()) {
        kill2();
        return;
    }
    set_A(OCT500);
    mask_at(ADDRESS_OF(FLAGWRD0));
    ad_at(ADDRESS_OF(MMNUMBER));
    com();
    ad(P20REG); /* "IS IT 20 AND IS RENDEZVOUS FLAG ON" */
    if (A_is_zero()) {
        statquo();
        return;
    }
    ad(OCT305); /* "IS IT 25 AND IS P25 BIT ON" */
    if (A_is_zero()) {
        statquo(); /* "YES, LEAVE AS IS" */
        return;
    }
    kill2();
}

/* RENDV00 -- LM p. 230.
 * P22 now or asked for: RESET22. P20 or P25 asked for: RENDN00. Another
 * program with P20 or P25 running: "LEAVE GROUP 2 TO PICK UP P20 OR
 * P25" (P00FIZZ); with neither, RESET22. */
static void rendv00(void)
{
    cs(ADDRESS_OF(MODREG)); /* "IS CURRENT PROGRAM 22" */
    ad(OCT26);
    if (A_is_zero()) {
        reset22(); /* "YES -- CLEAR RENDEZVOUS FLAG" */
        return;
    }
    cs(ADDRESS_OF(MMNUMBER)); /* "IS NE PROGRAM P22" */
    ad(OCT26);
    if (A_is_zero()) {
        reset22();
        return;
    }
    ad(NEG2); /* "IS NEW PROGRAM = P20 OR P25" */
    if (A_is_zero()) {
        rendn00();
        return;
    }
    ad(FIVE); /* 25 */
    if (A_is_zero()) {
        rendn00();
        return;
    }
    set_A(OCT500); /* "NO, IS EITHER P20 OR P25 RUNNING" */
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (positive(ccs_register(A_ADDRESS))) {
        p00fizz(); /* "YES, LEAVE GROUP 2 TO PICK UP P20 OR P25" */
        return;
    }
    reset22();
}

#else

/* KILL20 -- CM p. 199. "KILL GROUPS 1 + 2", then REV37 (P00FIZZ's
 * steps). */
static void kill20(void)
{
    kill_group(ADDRESS_OF(MINUS_PHASE1));
    kill_group(ADDRESS_OF(MINUS_PHASE2));
    p00fizz();
}

/* P00FIZZ -- CM p. 199. With the rendezvous flag on, keep groups 1 and
 * 2 (REV37); else KILL20. */
static void p00fizz_cm(void)
{
    set_A(RNDVZBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (positive(ccs_register(A_ADDRESS))) {
        p00fizz(); /* REV37 */
        return;
    }
    kill20();
}

/* RENDV00, RENDN00 -- CM p. 199.
 * "IS NEW PROG = 20": no, P00FIZZ. P20 again (MODREG 20): KILL20. P20
 * with the rendezvous flag set ("IS RENDEZVOO FLAG SET"): STATQUO, "SET
 * TRACKFLAG AND UPDATE FLAG", "KILL GROUP 4" and GOMOD. */
static void rendv00(void)
{
    cs(ADDRESS_OF(MMNUMBER));
    ad(OCT24); /* 20 */
    if (!A_is_zero()) {
        p00fizz_cm();
        return;
    }
    cs(ADDRESS_OF(MMNUMBER)); /* RENDN00 */
    ad_at(ADDRESS_OF(MODREG));
    if (A_is_zero()) {
        kill20();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (!positive(ccs_register(A_ADDRESS))) {
        p00fizz_cm();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD1)); /* STATQUO */
    mask(OCT120);
    ads(ADDRESS_OF(FLAGWRD1));
    kill_group(ADDRESS_OF(MINUS_PHASE4)); /* "KILL GROUP 4" */
    gomod();
}

#endif

/* NOUVEAU -- LM p. 231, CM p. 199.
 * A new program, not P00. With neither P20 nor P25 running (CM: no
 * rendezvous) "RESET IMUINUSE FLAG" first. */
static void nouveau(void)
{
#if LUMINARY099
    set_A(OCT500); /* "IS P20 OR P25 FLAG SET" */
#else
    set_A(RNDVZBIT);
#endif
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (positive(ccs_register(A_ADDRESS))) {
        nouveau_after_downflag(); /* TCF +3: "YES" */
        return;
    }
    tc_out(TC_DOWNFLAG_IMUSE, DOWNFLAG); /* ADRES IMUSE */
}

/* NOUVEAU +3 -- LM p. 231, CM p. 199.
 * The new program's downlist, then SEUDOP00. */
void nouveau_after_downflag(void)
{
    set_A(table_word(DNLADMM1, ADDRESS_OF(MINDEX))); /* "OBTAIN APPROPIRATE DOWNLIST ADDRESS" */
    inhint();
    seudop00();
}

/* V37NONO -- LM pp. 231-232, CM p. 199.
 * "COME HERE IF MM REQUESTED DOESN'T EXIST": the OPR ERR light (FALTON),
 * then V37BAD. */
static void v37nono(void)
{
    tc_out(TC_FALTON, FALTON);
}

void v37nono_after_falton(void)
{
    v37bad();
}

/* ---- Starting the new program ---- */

/* V37XEQ -- LM p. 232, CM p. 200.
 * Group 4's restart job: start the program at MINDEX. Its PREMM1 word,
 * cycled right four places (CYR), gives the priority; times BIT8 the E
 * bank; FCADRMM1 the bank and address of its first job. "TC SPVAC" (the
 * Executive's) makes it a job and returns to V37XEQC. */
void v37xeq(void)
{
    inhint();
    set_A(table_word(PREMM1, ADDRESS_OF(MINDEX))); /* "OBTAIN PRIO, EBANK, AND MM" */
    ts(ADDRESS_OF(MMTEMP));
    ts(ADDRESS_OF(CYR)); /* "SHIFT RIGHT TO BITS 14-10" */
    ca(ADDRESS_OF(CYR));
    mask(PRIO37);
    ts(ADDRESS_OF(PHSPRDT4)); /* "PRESET GROUP 4 RESTART PRIORITY" */
    ts(ADDRESS_OF(NEWPRIO));  /* "STORE PRIO FOR SPVAC" */
    ca(ADDRESS_OF(MMTEMP));   /* "OBTAIN EBANK -- BITS 8, 9, 10 OF MMTEMP." */
    mp(BIT8);
    mask(LOW3);
    ts(L_ADDRESS);
    set_A(table_word(FCADRMM1, ADDRESS_OF(MINDEX)));
    ts(ADDRESS_OF(BASETEMP)); /* "MAKE BBCON BY ADDING HI5 OR FCADR" */
    mask(HI5);
    ads(L_ADDRESS);
    ca(ADDRESS_OF(BASETEMP)); /* "OBTAIN GENADR PORTION OF 2CADR." */
    mask(LOW10);
    ad(BIT11);
    tc_out(TC_SPVAC, SPVAC);
}

/* V37XEQC -- LM p. 232, CM p. 200.
 * "UPON RETURN FROM FINDVAC PLACE THE NEW MM IN MODREG" (NEWMODEA),
 * "RELEASE DISPLAY" "AND EXIT". */
void v37xeqc(void)
{
    ca(ADDRESS_OF(MMTEMP));
    mask(LOW7);
    tc_out(TC_NEWMODEA, NEWMODEA);
}

void v37xeqc_after_newmodea(void)
{
    tc_out(TC_RELDSP_V37XEQC, RELDSP);
}

void v37xeqc_after_reldsp(void)
{
    tc_out(TC_ENDOFJOB, ENDOFJOB);
}

#if LUMINARY099

/* SETUP71, SETUP70 -- LM p. 233.
 * The aborts: straight to P70 (DTCB to its 2CADR), or to P71, three words
 * on. A is +0 or -0 for P70, which TS Q keeps. */
static void setup71(void)
{
    set_A(THREE);
    setup70();
}

static void setup70(void)
{
    ts(Q_ADDRESS);
    set_L(P70CADR[1]);
    set_A(P70CADR[0]);
    ad_at(Q_ADDRESS);
    dtcb(DTCB_P70);
}

#else

/* ---- The CM's R00TOP00 and INITSUB ---- */

/* R00TOP00 -- CM p. 197.
 * Back to coasting before P00 or the new program: "INHINT", E bank 6, save
 * the CSM mass, turn the SPS off (SPSOFF), compute the mass properties
 * (MASSPROP), start the RCS DAP in 3.1 seconds and disable the TVC DAP
 * (TVCZAP). */
static void r00top00(void)
{
    inhint();
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(CSMMASS));
    ts(ADDRESS_OF(MASSTMP));
    tc_out(TC_SPSOFF, IBNKCALL); /* CADR SPSOFF */
}

void r00top00_after_spsoff(void)
{
    tc_out(TC_MASSPROP, IBNKCALL); /* CADR MASSPROP */
}

void r00top00_after_massprop(void)
{
    set_A(THREE_1SEC);             /* "2.5 + 0.6 SEC" */
    tc_out(TC_RCSDAPON, IBNKCALL); /* CADR RCSDAPON +1 */
}

void r00top00_after_rcsdapon(void)
{
    tc_out(TC_TVCZAP, IBNKCALL); /* "DISABLE TVC" */
}

/* MMNUMBER +0 (P00 now), wait 50 ms (DELAYJOB). */
void r00top00_after_tvczap(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MMNUMBER));
    relint();
    set_A(FIVE);
    tc_out(TC_DELAYJOB, BANKCALL); /* CADR DELAYJOB */
}

/* RCS jets off (channels 5 and 6), then ISITP00. */
void r00top00_after_delayjob(void)
{
    set_A(ZERO);
    write_channel(05u);
    write_channel(06u);
    isitp00();
}

/* INITSUB -- CM p. 200.
 * A subroutine: its return goes to MPAC +1 (QXCH). "SET E6 FOR DEADBAND
 * CODING" and stop the DAP's rates (STOPRATE). */
static void initsub(void)
{
    qxch(MPAC1);
    set_A(EBANK6);
    ts(EBANK_ADDRESS); /* "WILL BE RESET IN STARTSB2." */
    inhint();
    tc_out(TC_STOPRATE, STOPRATE);
}

/* INITSUB, after STOPRATE -- CM p. 200.
 * "RESTORE DEADBAND": the minimum (SETMINDB) or, with MAXDBFLG, the
 * maximum ("MAX DB SELECTED", SETMAXER). */
void initsub_after_stoprate(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(MAXDBBIT);
    if (positive(ccs_register(A_ADDRESS))) {
        tc_out(TC_SETMAXDB, BANKCALL); /* SETMAXER: CADR SETMAXDB */
        return;
    }
    tc_out(TC_SETMINDB, BANKCALL); /* "MIN DB SELECTED": CADR SETMINDB */
}

void initsub_after_setmindb(void)
{
    rake(); /* TCF RAKE */
}

/* RAKE -- CM p. 200. "THIS PART CLEARS FLAGWORD BITS": FLAGTABL's bits
 * in flag words 11 down to 0. Then "NOW SET IMPULSW" (UPFLAG). */
void rake(void)
{
    set_A(ELEVEN);
    do {
        ts(ADDRESS_OF(MPAC)); /* RAKE +1: "LOOP COMES HERE." */
        set_A(negate(FLAGTABL[fetch(ADDRESS_OF(MPAC))]));
        mask_at(indexed(ADDRESS_OF(FLAGWRD0), fetch(ADDRESS_OF(MPAC))));
        /* "PUT REVISED FLAGWORD BACK." */
        ts(indexed(ADDRESS_OF(FLAGWRD0), fetch(ADDRESS_OF(MPAC))));
    } while (positive(ccs_at(ADDRESS_OF(MPAC)))); /* "GET THE NEXT FLAGWORD." */
    relint();
    tc_out(TC_UPFLAG_IMPULSW, UPFLAG); /* ADRES IMPULSW */
}

/* "KILL COARSE OPTICS" (OPTIND -1), and return from INITSUB. */
void rake_after_upflag(void)
{
    set_A(NEGONE);
    ts(ADDRESS_OF(OPTIND));
    return_from_initsub();
}

/* TC MPAC +1, "RETURN FROM INITSUB": MPAC +1 holds the return INITSUB's
 * QXCH saved, which the AGC runs as the instruction TC <return>. Its two
 * callers are GOP00FIX and ROC. */
static void return_from_initsub(void)
{
    tc_through(MPAC1);
    if (Z == TC_INITSUB_GOP00FIX + 1u) {
        tc_out(TC_CLEARMRK, CLEARMRK + 2u); /* "TC CLEARMRK +2" */
    } else if (Z == TC_INITSUB_ROC + 1u) {
        roc_after_initsub();
    } else {
        not_modelled();
    }
}

/* ---- VAC5STOR ---- */

/* VAC5STOR -- CM pp. 204-205.
 * A hardware restart's record for debugging, in VAC area 5: each core
 * set's LOC, BANKSET and PRIORITY ("LOOP TO STORE LOCS, BANKSETS, AND
 * PRIOS"), then the restart groups' 2CADRs (GENTRAN of PHSNAME1). */
void vac5stor(void)
{
    set_A(ZERO); /* "INITIALIZE INDEX REGISTERS" */
    ts(ADDRESS_OF(ITEMP1));
    ts(ADDRESS_OF(ITEMP2));
    for (;;) { /* V5LOOP1 */
        dca(indexed(ADDRESS_OF(LOC), fetch(ADDRESS_OF(ITEMP1))));
        dxch(indexed(ADDRESS_OF(VAC5), fetch(ADDRESS_OF(ITEMP2))));
        ca(indexed(ADDRESS_OF(PRIORITY), fetch(ADDRESS_OF(ITEMP1))));
        ts(indexed(ADDRESS_OF(VAC5) + 2u, fetch(ADDRESS_OF(ITEMP2))));
        cs(ADDRESS_OF(ITEMP2)); /* "HAVE WE STORED THEM ALL?" */
        ad(EIGHTEEN);
        if (A_is_zero()) break;
        set_A(TWELVE); /* "NO, INCREMENT INDEXES AND LOOP." */
        ads(ADDRESS_OF(ITEMP1));
        set_A(THREE);
        ads(ADDRESS_OF(ITEMP2));
    }
    set_A(EBANK3); /* V5OUT1: "PHSNAME REGISTERS ARE IN EBANK3." */
    ts(EBANK_ADDRESS);
    set_A(ELEVEN);               /* "GET PHASE 2CADRS." */
    tc_out(TC_GENTRAN, GENTRAN); /* ADRES PHSNAME1; ADRES VAC5 +21D */
}

/* VAC5STOR, after GENTRAN -- CM p. 205.
 * The phase table's six PHASEn words (V5LOOP2), then MPAC +3, NEWLOC,
 * NEWJOB and NEWPRIO; back through SWRETURN. */
void vac5stor_after_gentran(void)
{
    set_A(ZERO); /* "NOW INITIALIZE INDEXES AGAIN." */
    ts(ADDRESS_OF(ITEMP1));
    ts(ADDRESS_OF(ITEMP2));
    for (;;) { /* V5LOOP2: "LOOP TO GET PHASE TABLES." */
        ca(indexed(ADDRESS_OF(PHASE1), fetch(ADDRESS_OF(ITEMP1))));
        ts(indexed(ADDRESS_OF(VAC5) + 041u, fetch(ADDRESS_OF(ITEMP2)))); /* VAC5 +33D */
        cs(ADDRESS_OF(ITEMP2)); /* "DO WE HAVE THEM ALL?" */
        ad(FIVE);
        if (A_is_zero()) break;
        set_A(TWO); /* "NO, INCREMENT INDEXES AND LOOP." */
        ads(ADDRESS_OF(ITEMP1));
        incr(ADDRESS_OF(ITEMP2));
    }
    ca(ADDRESS_OF(MPAC) + 3u); /* V5OUT2 */
    ts(ADDRESS_OF(VAC5) + 047u);
    dca(ADDRESS_OF(NEWLOC));
    dxch(ADDRESS_OF(VAC5) + 050u);
    ca(ADDRESS_OF(NEWJOB));
    ts(ADDRESS_OF(VAC5) + 026u);
    ca(ADDRESS_OF(NEWPRIO));
    ts(ADDRESS_OF(VAC5) + 032u);
    tc_out(TC_SWRETURN, SWRETURN);
}

#endif
