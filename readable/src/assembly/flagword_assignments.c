/* SPDX-License-Identifier: MIT */
/* FLAGWORD_ASSIGNMENTS.agc, the LM's: Luminary099 pages 61-88. The listing
 * file that names the program's flags: the bits of the fourteen flagwords,
 * FLAGWRD0 to FLGWRD13, which are the erasable words STATE +0 to STATE
 * +13D. It holds no routine and assembles no word of the rope; every line
 * of code in it gives a name its number.
 *
 * A bit has two names. The flag's name is its number among all the flags,
 * counted from bit 15 of FLAGWRD0 (number 0) down the word and on through
 * the next, fifteen to a word: flag n is bit 15 - n mod 15 of flagword
 * n / 15. Interpretive code names a flag by this number (SET, CLEAR, BON
 * and the other switch orders take it apart into word and bit), and so do
 * UPFLAG and DOWNFLAG in basic. The bit's name is the address of the
 * fixed-fixed constant with that one bit (BIT15 to BIT1), which basic
 * code masks the flagword with.
 *
 * The file has two parts: an alphabetical list of the flags (FLAG_LIST),
 * all comment, and then the assignments, flagword by flagword and bit by
 * bit (FLAGWORDS), each flag with what it means SET and what it means
 * RESET. Here both are tables, in the listing's words and spelling
 * (ERASTALL, IMPUSE). A bit with no flag has no name. A flag's meaning is
 * kept in the listing's lines (a new line of the listing is a new line of
 * the string), so SET and RESET read side by side as they do on the page;
 * where the listing writes one note across both columns, it is the flag's
 * note.
 *
 * In a few places the list, as transcribed, and the assignments disagree,
 * and both stand here as they are: the list gives FLRCS the number 149
 * (bit 10 of flagword 9 is 140), calls MID1FBIT MID1BIT, puts PRIODFLG
 * in flagword 1 (61 is in flagword 4), writes DIMOFLAG and DIMOBIT with
 * the letter O where the assignments have the digit 0, and gives DIDFLAG
 * and NJETSFLG no flagword. make assembly-cases holds the table to
 * exactly these (LIST_SLIPS in tests/assembly_cases.py).
 *
 * The CM's listing has no such file: its flagwords are assigned in its
 * ERASABLE_ASSIGNMENTS. */
#include <stddef.h>
#include "assembly.h"

#if LUMINARY099

/* clang-format off */
#define LISTED(name, number, bit, flagword, bit_name) \
    {name, number, bit, flagword, bit_name, NULL}
#define LISTED_AND_NOTE(name, number, bit, flagword, bit_name, note) \
    {name, number, bit, flagword, bit_name, note}
#define LISTED_WORD(name, text) \
    {name, NOT_GIVEN, NOT_GIVEN, NOT_GIVEN, NULL, text}
/* clang-format on */

const char FLAGWORDS_INTRODUCTION[] =
    "FLAGWORDS 0-11 ARE DOWNLINKED AND CAN BE SET AND CLEARED BY UP-FLAG AND DOWN-FLAG "
    "INSTRUCTIONS IN THE "
    "INTERPRETER. THESE WERE PREVIOUSLY LISTED UNDER \"INTERPRETIVE SWITCH BIT ASSIGNMENTS\" IN "
    "THE ERASABLE LOG SECTION. FLAGWORDS 12 & 13 WERE PREVIOUSLY RADMODES AND DAPBOOLS AND "
    "ARE STILL DOWNLINKED UNDER THOSE NAMES. ";

const char FLAG_LIST_HEADING[] = "ALPHABETICAL LIST OF FLAGWORDS";
const char FLAG_LIST_COLUMNS[] = "FLAGWORD DEC. NUMBER BIT AND FLAG BIT NAME";

const struct listed_flag FLAG_LIST[] = {
    LISTED("ACCOKFLG", 207, 3, 13, "ACCSOKAY"),
    LISTED("ACC4-2FL", 199, 11, 13, "ACC4OR2X"),
    LISTED("ACMODFLG", 32, 13, 2, "ACMODBIT"),
    LISTED("ALTSCALE", 186, 9, 12, "ALTSCBIT"),
    LISTED("ANTENFLG", 183, 12, 12, "ANTENBIT"),
    LISTED("AORBSFLG", 205, 5, 13, "AORBSYST"),
    LISTED("AORBTFLG", 200, 10, 13, "AORBTRAN"),
    LISTED("APSESW", 130, 5, 8, "APSESBIT"),
    LISTED("APSFLAG", 152, 13, 10, "APSFLBIT"),
    LISTED("ASTNFLAG", 108, 12, 7, "ASTNBIT"),
    LISTED("ATTFLAG", 104, 1, 6, "ATTFLBIT"),
    LISTED("AUTOMODE", 193, 2, 12, "AUTOMBIT"),
    LISTED("AUTR1FLG", 209, 1, 13, "AUTRATE1"),
    LISTED("AUTR2FLG", 208, 2, 13, "AUTRATE2"),
    LISTED("AUXFLAG", 103, 2, 6, "AUXFLBIT"),
    LISTED("AVEGFLAG", 115, 5, 7, "AVEGFBIT"),
    LISTED("AVEMIDSW", 149, 1, 9, "AVEMDBIT"),
    LISTED("AVFLAG", 40, 5, 2, "AVFLBIT"),
    LISTED("CALCMAN2", 43, 2, 2, "CALC2BIT"),
    LISTED("CALCMAN3", 42, 3, 2, "CALC3BIT"),
    LISTED("CDESFLAG", 180, 15, 12, "CDESBIT"),
    LISTED("CMOONFLG", 123, 12, 8, "CMOONBIT"),
    LISTED("COGAFLAG", 131, 4, 8, "COGAFBIT"),
    LISTED("CSMDKFLG", 197, 13, 13, "CSMDOCKD"),
    LISTED("CULTFLAG", 53, 7, 3, "CULTBIT"),
    LISTED_WORD("DAPBOOLS", "FLGWRD13"),
    LISTED("DBSELFLG", 206, 4, 13, "DBSELECT"),
    LISTED("DESIGFLG", 185, 10, 12, "DESIGBIT"),
    LISTED("DIDFLAG", 16, 14, NOT_GIVEN, "DIDFLBIT"),
    LISTED("DIMOFLAG", 59, 1, 3, "DIMOBIT"),
    LISTED("DMENFLG", 81, 9, 5, "DMENFBIT"),
    LISTED("DRIFTDFL", 202, 8, 13, "DRIFTBIT"),
    LISTED("DRIFTFLG", 30, 15, 2, "DRFTBIT"),
    LISTED("DSKYFLAG", 75, 15, 5, "DSKYFBIT"),
    /* LM p. 62 */
    LISTED("D6OR9FLG", 58, 2, 3, "D6OR9BIT"),
    LISTED("ENGONFLG", 83, 7, 5, "ENGONBIT"),
    LISTED("ERADFLAG", 17, 13, 1, "ERADFBIT"),
    LISTED_AND_NOTE("ETPIFLAG", 38, 7, 2, "ETPIBIT", "EQUIVALENT FLAG NAME: DPTNSW"),
    LISTED("FINALFLG", 39, 6, 2, "FINALBIT"),
    LISTED_WORD("FLAGWRD0", "(000-014) (STATE +0)"),
    LISTED_WORD("FLAGWRD1", "(015-029) (STATE +1)"),
    LISTED_WORD("FLAGWRD2", "(030-044) (STATE +2)"),
    LISTED_WORD("FLAGWRD3", "(045-059) (STATE +3)"),
    LISTED_WORD("FLAGWRD4", "(060-074) (STATE +4)"),
    LISTED_WORD("FLAGWRD5", "(075-089) (STATE +5)"),
    LISTED_WORD("FLAGWRD6", "(090-104) (STATE +6)"),
    LISTED_WORD("FLAGWRD7", "(105-119) (STATE +7)"),
    LISTED_WORD("FLAGWRD8", "(120-134) (STATE +8D)"),
    LISTED_WORD("FLAGWRD9", "(135-149) (STATE +9D)"),
    LISTED("FLAP", 142, 8, 9, "FLAPBIT"),
    LISTED_WORD("FLGWRD10", "(150-164) (STATE +10D)"),
    LISTED_WORD("FLGWRD11", "(165-179) (STATE +11D)"),
    LISTED_WORD("FLGWRD12", "(180-194) (STATE +12D)"),
    LISTED_WORD("FLGWRD13", "(195-209) (STATE +13D)"),
    LISTED("FLPC", 138, 12, 9, "FLPCBIT"),
    LISTED("FLPI", 139, 11, 9, "FLPIBIT"),
    LISTED("FLRCS", 149, 10, 9, "FLRCSBIT"),
    LISTED("FLUNDISP", 125, 10, 8, "FLUNDBIT"),
    LISTED("FLVR", 136, 14, 9, "FLVRBIT"),
    LISTED("FREEFLAG", 12, 3, 0, "FREEFBIT"),
    LISTED("FSPASFLG", 5, 10, 0, "FSPASBIT"),
    LISTED("GLOKFAIL", 46, 14, 3, "GLOKFBIT"),
    LISTED("GMBDRVSW", 95, 10, 6, "GMBDRBIT"),
    LISTED("GUESSW", 28, 2, 1, "GUESSBIT"),
    LISTED("HFLSHFLG", 179, 1, 11, "HFLSHBIT"),
    LISTED("IDLEFLAG", 113, 7, 7, "IDLEFBIT"),
    LISTED("IGNFLAG", 107, 13, 7, "IGNFLBIT"),
    LISTED("IMPULSW", 36, 9, 2, "IMPULBIT"),
    LISTED("IMUSE", 7, 8, 0, "IMUSEBIT"),
    LISTED("INFINFLG", 128, 7, 8, "INFINBIT"),
    LISTED("INITALGN", 133, 2, 8, "INITABIT"),
    LISTED("INTFLAG", 151, 14, 10, "INTFLBIT"),
    LISTED("INTYPFLG", 56, 4, 3, "INTYPBIT"),
    LISTED("ITSWICH", 105, 15, 7, "ITSWBIT"),
    LISTED("JSWITCH", 1, 14, 0, "JSWCHBIT"),
    LISTED("LETABORT", 141, 9, 9, "LETABBIT"),
    LISTED("LMOONFLG", 124, 11, 8, "LMOONBIT"),
    LISTED("LOKONSW", 10, 5, 0, "LOKONBIT"),
    LISTED("LOSCMFLG", 33, 12, 2, "LOSCMBIT"),
    LISTED("LRALTFLG", 190, 5, 12, "LRALTBIT"),
    LISTED("LRBYPASS", 165, 15, 11, "LRBYBIT"),
    LISTED("LRINH", 172, 8, 11, "LRINHBIT"),
    LISTED("LRPOSFLG", 189, 6, 12, "LRPOSBIT"),
    LISTED("LRVELFLG", 187, 8, 12, "LRVELBIT"),
    /* LM p. 63 */
    LISTED("LUNAFLAG", 48, 12, 3, "LUNABIT"),
    LISTED("MANUFLAG", 106, 14, 7, "MANUFBIT"),
    LISTED("MGLVFLAG", 88, 2, 5, "MGLVFBIT"),
    LISTED("MIDAVFLG", 148, 2, 9, "MIDAVBIT"),
    LISTED("MIDFLAG", 2, 13, 0, "MIDFLBIT"),
    LISTED("MID1FLAG", 147, 3, 9, "MID1BIT"),
    LISTED("MKOVFLAG", 72, 3, 4, "MKOVBIT"),
    LISTED("MOONFLAG", 3, 12, 0, "MOONBIT"),
    LISTED("MRKIDFLG", 60, 15, 4, "MRKIDBIT"),
    LISTED("MRKNVFLG", 66, 9, 4, "MRKNVBIT"),
    LISTED("MRUPTFLG", 70, 5, 4, "MRUPTBIT"),
    LISTED("MUNFLAG", 97, 8, 6, "MUNFLBIT"),
    LISTED("MWAITFLG", 64, 11, 4, "MWAITBIT"),
    LISTED("NEEDLFLG", 11, 4, 0, "NEEDLBIT"),
    LISTED("NEWIFLG", 122, 13, 8, "NEWIBIT"),
    LISTED("NJETSFLG", 15, 15, NOT_GIVEN, "NJETSBIT"),
    LISTED("NODOFLAG", 44, 1, 2, "NODOBIT"),
    LISTED("NOLRREAD", 170, 10, 11, "NOLRRBIT"),
    LISTED("NORMSW", 110, 10, 7, "NORMSBIT"),
    LISTED("NORRMON", 86, 4, 5, "NORRMBIT"),
    LISTED("NOR29FLG", 49, 11, 3, "NR29FBIT"),
    LISTED("NOTHROTL", 78, 12, 5, "NOTHRBIT"),
    LISTED("NOUPFLAG", 24, 6, 1, "NOUPFBIT"),
    LISTED("NRMNVFLG", 67, 8, 4, "NRMNVBIT"),
    LISTED("NRMIDFLG", 62, 13, 4, "NRMIDBIT"),
    LISTED("NRUPTFLG", 71, 4, 4, "NRUPTBIT"),
    LISTED("NTARGFLG", 102, 3, 6, "NTARGBIT"),
    LISTED("NWAITFLG", 65, 10, 4, "NWAITBIT"),
    LISTED("OLDESFLG", 14, 1, 0, "OLDESBIT"),
    LISTED_AND_NOTE("OPTNSW", 38, 7, 2, "OPTNBIT", "EQUIVALENT FLAG NAME: ETPIFLAG"),
    LISTED("ORBWFLAG", 54, 6, 3, "ORBWFBIT"),
    LISTED("ORDERSW", 129, 6, 8, "ORDERBIT"),
    LISTED("OURRCFLG", 198, 12, 13, "OURRCBIT"),
    LISTED("PDSPFLAG", 63, 12, 4, "PDSPFBIT"),
    LISTED("PFRATFLG", 41, 4, 2, "PFRATBIT"),
    LISTED("PINBRFLG", 69, 6, 4, "PINBRBIT"),
    LISTED("PRECIFLG", 52, 8, 3, "PRECIBIT"),
    LISTED("PRIODFLG", 61, 14, 1, "PRIODBIT"),
    LISTED("PRONVFLG", 68, 7, 4, "PRONVBIT"),
    LISTED("PSTHIGAT", 169, 11, 11, "PSTHIBIT"),
    LISTED("PULSEFLG", 195, 15, 13, "PULSES"),
    LISTED("P21FLAG", 4, 11, 0, "P21FLBIT"),
    LISTED("P25FLAG", 6, 9, 0, "P25FLBIT"),
    LISTED("P39/79SW", 126, 9, 8, "P39SWBIT"),
    LISTED("QUITFLAG", 145, 5, 9, "QUITBIT"),
    LISTED_WORD("RADMODES", "FLGWRD12"),
    LISTED_WORD("RASFLAG", "FLGWRD10"),
    LISTED("RCDUFAIL", 188, 7, 12, "RCDUFBIT"),
    LISTED("RCDU0FLG", 182, 13, 12, "RCDU0BIT"),
    LISTED("READLR", 174, 6, 11, "READLBIT"),
    /* LM p. 64 */
    LISTED_AND_NOTE("READRFLG", 51, 9, 3, "READRBIT", "EQUIVALENT FLAG NAME FOR R04FLAG"),
    LISTED("READVEL", 175, 5, 11, "READVBIT"),
    LISTED("REDFLAG", 99, 6, 6, "REDFLBIT"),
    LISTED("REFSMFLG", 47, 13, 3, "REFSMBIT"),
    LISTED("REINTFLG", 158, 7, 10, "REINTBIT"),
    LISTED("REMODFLG", 181, 14, 12, "REMODBIT"),
    LISTED("RENDWFLG", 89, 1, 5, "RENDWBIT"),
    LISTED("REPOSMON", 184, 11, 12, "REPOSBIT"),
    LISTED("RHCSCFLG", 203, 7, 13, "RHCSCALE"),
    LISTED("RNDVZFLG", 8, 7, 0, "RNDVZBIT"),
    LISTED("RNGEDATA", 176, 4, 11, "RNGEDBIT"),
    LISTED("RNGSCFLG", 80, 10, 5, "RNGSCBIT"),
    LISTED("RODFLAG", 18, 12, 1, "RODFLBIT"),
    LISTED("ROTFLAG", 144, 6, 9, "ROTFLBIT"),
    LISTED("RPQFLAG", 120, 15, 8, "RPQFLBIT"),
    LISTED("RRDATAFL", 191, 4, 12, "RRDATABT"),
    LISTED("RRNBSW", 9, 6, 0, "RRNBBIT"),
    LISTED("RRRSFLAG", 192, 3, 12, "RRRSBIT"),
    LISTED("RVSW", 111, 9, 7, "RVSWBIT"),
    LISTED_AND_NOTE("R04FLAG", 51, 9, 3, "R04FLBIT", "EQUIVALENT FLAG NAME: READRFLG"),
    LISTED("R10FLAG", 13, 2, 0, "R10FLBIT"),
    LISTED("R61FLAG", 20, 10, 1, "R61FLBIT"),
    LISTED("R77FLAG", 79, 11, 5, "R77FLBIT"),
    LISTED("SCALBAD", 177, 3, 11, "SCABBIT"),
    LISTED("SLOPESW", 27, 3, 1, "SLOPEBIT"),
    LISTED("SNUFFER", 77, 13, 5, "SNUFFBIT"),
    LISTED("SOLNSW", 87, 3, 5, "SOLNSBIT"),
    LISTED("SRCHOPTN", 31, 14, 2, "SRCHOBIT"),
    LISTED("STATEFLG", 55, 5, 3, "STATEBIT"),
    LISTED("STEERSW", 34, 11, 2, "STEERBIT"),
    LISTED("SURFFLAG", 127, 8, 8, "SURFFBIT"),
    LISTED("SWANDISP", 109, 11, 7, "SWANDBIT"),
    LISTED("S32.1F1", 90, 15, 6, "S32BIT1"),
    LISTED("S32.1F2", 91, 14, 6, "S32BIT2"),
    LISTED("S32.1F3A", 92, 13, 6, "S32BIT3A"),
    LISTED("S32.1F3B", 93, 12, 6, "S32BIT3B"),
    LISTED("TFFSW", 119, 1, 7, "TFFSWBIT"),
    LISTED("TRACKFLG", 25, 5, 1, "TRACKBIT"),
    LISTED("TURNONFL", 194, 1, 12, "TURNONBT"),
    LISTED("ULLAGFLG", 204, 6, 13, "ULLAGER"),
    LISTED("UPDATFLG", 23, 7, 1, "UPDATBIT"),
    LISTED("UPLOCKFL", 116, 4, 7, "UPLOCBIT"),
    LISTED("USEQRFLG", 196, 14, 13, "USEQRJTS"),
    LISTED("VEHUPFLG", 22, 8, 1, "VEHUPBIT"),
    LISTED("VELDATA", 173, 7, 11, "VELDABIT"),
    LISTED("VERIFLAG", 117, 3, 7, "VERIFBIT"),
    LISTED("VFLAG", 50, 10, 3, "VFLAGBIT"),
    LISTED("VFLSHFLG", 178, 2, 11, "VFLSHBIT"),
    LISTED("VINTFLAG", 57, 3, 3, "VINTFBIT"),
    LISTED("VXINH", 168, 12, 11, "VXINHBIT"),
    /* LM p. 65 */
    LISTED("V37FLAG", 114, 6, 7, "V37FLBIT"),
    LISTED("V67FLAG", 112, 8, 7, "V67FLBIT"),
    LISTED("V82EMFLG", 118, 2, 7, "V82EMBIT"),
    LISTED("XDELVFLG", 37, 8, 2, "XDELVBIT"),
    LISTED("XDSPFLAG", 74, 1, 4, "XDSPBIT"),
    LISTED("XORFLG", 171, 9, 11, "XORFLBIT"),
    LISTED("XOVINFLG", 201, 9, 13, "XOVINHIB"),
    LISTED("3AXISFLG", 84, 6, 5, "3AXISBIT"),
    LISTED("360SW", 134, 1, 8, "360SWBIT"),
};
const unsigned FLAG_LIST_COUNT = COUNT(FLAG_LIST);

const char FLAG_ASSIGNMENTS_HEADING[] = "ASSIGNMENT AND DESCRIPTION OF FLAGWORDS";

static const struct flag FLAGWRD0_FLAGS[] = {
    {.number = 0, .bit = 15, .marks = "(S)"},
    {.name = "JSWITCH",
     .number = 1,
     .bit_name = "JSWCHBIT",
     .bit = 14,
     .marks = "(S)",
     .set = "INTEGRATION OF W\n"
            "MATRIX",
     .reset = "INTEGRATION OF STATE\n"
              "VECTOR"},
    {.name = "MIDFLAG",
     .number = 2,
     .bit_name = "MIDFLBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "INTEGRATION WITH\n"
            "SECONDARY BODY AND\n"
            "SOLAR PERTURBATIONS",
     .reset = "INTEGRATION WITHOUT\n"
              "SOLAR PERTURBATIONS"},
    {.name = "MOONFLAG",
     .number = 3,
     .bit_name = "MOONBIT",
     .bit = 12,
     .marks = "(L)",
     .set = "MOON IS SPHERE OF\n"
            "INFLUENCE",
     .reset = "EARTH IS SPHERE OF\n"
              "INFLUENCE"},
    {.name = "P21FLAG",
     .number = 4,
     .bit_name = "P21FLBIT",
     .bit = 11,
     .set = "USE BASE VECTORS\n"
            "ALREADY CALCULATED",
     .reset = "1ST PASS -- CALC-\n"
              "ULATE BASE VECTORS"},
    {.name = "FSPASFLG",
     .number = 5,
     .bit_name = "FSPASBIT",
     .bit = 10,
     .set = "FIRST PASS THROUGH\n"
            "REPOSITION ROUTINE",
     .reset = "NOT FIRST PASS THRU\n"
              "REPOSITION ROUTINE"},
    /* LM p. 66 */
    {.name = "P25FLAG",
     .number = 6,
     .bit_name = "P25FLBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "P25 OPERATING",
     .reset = "P25 NOT OPERATING"},
    {.name = "IMUSE",
     .number = 7,
     .bit_name = "IMUSEBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "IMU IN USE",
     .reset = "IMU NOT IN USE"},
    {.name = "RNDVZFLG",
     .number = 8,
     .bit_name = "RNDVZBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "P20 RUNNING (RADAR\n"
            "IN USE)",
     .reset = "P20 NOT RUNNING"},
    {.name = "RRNBSW",
     .number = 9,
     .bit_name = "RRNBBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "RADAR TARGET IN\n"
            "NB COORDINATES",
     .reset = "RADAR TARGET IN\n"
              "SM COORDINATES"},
    {.name = "LOKONSW",
     .number = 10,
     .bit_name = "LOKONBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "RADAR LOCK-ON\n"
            "DESIRED",
     .reset = "RADAR LOCK-ON NOT\n"
              "DESIRED"},
    {.name = "NEEDLFLG",
     .number = 11,
     .bit_name = "NEEDLBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "TOTAL ATTITUDE\n"
            "ERROR DISPLAYED",
     .reset = "A/P FOLLOWING\n"
              "ERROR DISPLAYED"},
    {.name = "FREEFLAG",
     .number = 12,
     .bit_name = "FREEFBIT",
     .bit = 3,
     .note = "(USED BY P51-53 TEMP IN MANY DIFFERENT\n"
             "ROUTINES & BY LUNAR + SOLAR EPHEMERIDES)"},
    {.name = "R10FLAG",
     .number = 13,
     .bit_name = "R10FLBIT",
     .bit = 2,
     .set = "R10 OUTPUTS DATA TO\n"
            "ALTITUDE & ALTITUDE\n"
            "RATE METERS ONLY",
     .reset = "BESIDES OUTPUT WHEN\n"
              "SET, R10 ALSO OUTPUT\n"
              "TO FORWARD & LATERAL\n"
              "VELOCITY CROSSPOINTER"},
    {.name = "OLDESFLG",
     .number = 14,
     .bit_name = "OLDESBIT",
     .bit = 1,
     .marks = "(L)",
     .set = "R29 GYRO CMD LOOP\n"
            "REQUESTED",
     .reset = "R29 GYRO CMD LOOP\n"
              "NOT REQUESTED"},
};

static const struct flag FLAGWRD1_FLAGS[] = {
    /* LM p. 67 */
    {.name = "NJETSFLG",
     .number = 15,
     .bit_name = "NJETSBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "TWO JET RCS BURN",
     .reset = "FOUR JET RCS BURN"},
    {.name = "DIDFLAG",
     .number = 16,
     .bit_name = "DIDFLBIT",
     .bit = 14,
     .marks = "(L)",
     .set = "INERTIAL DATA IS\n"
            "AVAILABLE",
     .reset = "PERFORM DATA DISPLAY\n"
              "INITIALIZATION FUNCS"},
    {.name = "ERADFLAG",
     .number = 17,
     .bit_name = "ERADFBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "COMPUTE REARTH\n"
            "FISCHER ELLIPSOID",
     .reset = "USE CONSTANT REARTH\n"
              "PAD RADIUS"},
    {.name = "RODFLAG",
     .number = 18,
     .bit_name = "RODFLBIT",
     .bit = 12,
     .set = "IF IN P66, NORMAL\n"
            "OPERATION CONTINUES.\n"
            "RESTART CLEARS FLAG",
     .reset = "IF IN P66, RE-INIT-\n"
              "IALIZATION IS PER-\n"
              "FORMED AND FLAG IS"},
    {.number = 19, .bit = 11},
    {.name = "R61FLAG",
     .number = 20,
     .bit_name = "R61FLBIT",
     .bit = 10,
     .marks = "(L)",
     .set = "RUN R61 LEM",
     .reset = "RUN R65 LEM"},
    {.number = 21, .bit = 9},
    {.name = "VEHUPFLG",
     .number = 22,
     .bit_name = "VEHUPBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "CSM STATE-VECTOR\n"
            "BEING UPDATED",
     .reset = "LEM STATE VECTOR\n"
              "BEING UPDATED"},
    {.name = "UPDATFLG",
     .number = 23,
     .bit_name = "UPDATBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "UPDATING BY MARKS\n"
            "ALLOWED",
     .reset = "UPDATING BY MARKS\n"
              "NOT ALLOWED"},
    {.name = "NOUPFLAG",
     .number = 24,
     .bit_name = "NOUPFBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "NEITHER CSM\n"
            "NOR LM STATE VECTOR\n"
            "MAY BE UPDATED",
     .reset = "EITHER STATE\n"
              "VECTOR MAY BE\n"
              "UPDATED"},
    /* LM p. 68 */
    {.name = "TRACKFLG",
     .number = 25,
     .bit_name = "TRACKBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "TRACKING ALLOWED",
     .reset = "TRACKING NOT ALLOWED"},
    {.number = 26, .bit = 4},
    {.name = "SLOPESW",
     .number = 27,
     .bit_name = "SLOPEBIT",
     .bit = 3,
     .marks = "(S)",
     .set = "ITERATE WITH BIAS\n"
            "METHOD IN ITERATOR",
     .reset = "ITERATE WITH REGULAR\n"
              "FALSI METHOD IN\n"
              "ITERATOR"},
    {.name = "GUESSW",
     .number = 28,
     .bit_name = "GUESSBIT",
     .bit = 2,
     .marks = "(S)",
     .set = "NO STARTING VALUE\n"
            "FOR ITERATION",
     .reset = "STARTING VALUE FOR\n"
              "ITERATION EXISTS"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 29, .bit = 1},
};

static const struct flag FLAGWRD2_FLAGS[] = {
    {.name = "DRIFTFLG",
     .number = 30,
     .bit_name = "DRFTBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "T3RUPT CALLS GYRO\n"
            "COMPENSATION",
     .reset = "T3RUPT DOES NO GYRO\n"
              "COMPENSATION"},
    {.name = "SRCHOPTN",
     .number = 31,
     .bit_name = "SRCHOBIT",
     .bit = 14,
     .marks = "(S)",
     .set = "RADAR IN AUTOMATIC\n"
            "SEARCH OPTION (R24)",
     .reset = "RADAR NOT IN AUTO-\n"
              "MATIC SEARCH OPTION"},
    {.name = "ACMODFLG",
     .number = 32,
     .bit_name = "ACMODBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "MANUAL ACQUISITION\n"
            "BY RENDEZVOUS RADAR",
     .reset = "AUTO ACQUISITION\n"
              "BY RENDEZVOUS RADAR"},
    {.name = "LOSCMFLG",
     .number = 33,
     .bit_name = "LOSCMBIT",
     .bit = 12,
     .marks = "(S)",
     .set = "LINE OF SIGHT BEING\n"
            "COMPUTED (R21)",
     .reset = "LINE OF SIGHT NOT\n"
              "BEING COMPUTED"},
    /* LM p. 69 */
    {.name = "STEERSW",
     .number = 34,
     .bit_name = "STEERBIT",
     .bit = 11,
     .marks = "(S)",
     .set = "SUFFICIENT THRUST\n"
            "IS PRESENT",
     .reset = "INSUFFICIENT THRUST\n"
              "IS PRESENT"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 35, .bit = 10, .marks = "(S)"},
    {.name = "IMPULSW",
     .number = 36,
     .bit_name = "IMPULBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "MINIMUM IMPULSE\n"
            "BURN (CUTOFF TIME\n"
            "SPECIFIED)",
     .reset = "STEERING BURN (NO\n"
              "CUTOFF TIME YET\n"
              "AVAILABLE)"},
    {.name = "XDELVFLG",
     .number = 37,
     .bit_name = "XDELVBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "EXTERNAL DELTAV VG\n"
            "COMPUTATION",
     .reset = "LAMBERT (AIMPOINT)\n"
              "VG COMPUTATION"},
    {.name = "ETPIFLAG",
     .number = 38,
     .bit_name = "ETPIBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "ELEVATION ANGLE\n"
            "SUPPLIED FOR\n"
            "P34,74",
     .reset = "TPI TIME SUPPLIED\n"
              "FOR P34,74 TO COMPUTE\n"
              "ELEVATION"},
    {.name = "OPTNSW",
     .same_as = "ETPIFLAG",
     .number = 38,
     .bit_name = "OPTNBIT",
     .bit = 7,
     .marks = "(L)",
     .set = "SOI PHASE OF P38/78",
     .reset = "SOR PHASE OF P38/78"},
    {.name = "FINALFLG",
     .number = 39,
     .bit_name = "FINALBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "LAST PASS THROUGH\n"
            "RENDEZVOUS PROGRAM\n"
            "COMPUTATIONS",
     .reset = "INTERIM PASS THROUGH\n"
              "RENDEZVOUS PROGRAM\n"
              "COMPUTATIONS"},
    {.name = "AVFLAG",
     .number = 40,
     .bit_name = "AVFLBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "LEM IS ACTIVE\n"
            "VEHICLE",
     .reset = "CSM IS ACTIVE\n"
              "VEHICLE"},
    {.name = "PFRATFLG",
     .number = 41,
     .bit_name = "PFRATBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "PREFERRED ATTITUDE\n"
            "COMPUTED",
     .reset = "PREFERRED ATTITUDE\n"
              "NOT COMPUTED"},
    {.name = "CALCMAN3",
     .number = 42,
     .bit_name = "CALC3BIT",
     .bit = 3,
     .marks = "(S)",
     .set = "NO FINAL ROLL",
     .reset = "FINAL ROLL IS\n"
              "NECESSARY"},
    {.name = "CALCMAN2",
     .number = 43,
     .bit_name = "CALC2BIT",
     .bit = 2,
     .marks = "(S)",
     .set = "PERFORM MANEUVER\n"
            "STARTING PROCEDURE",
     .reset = "BYPASS STARTING\n"
              "PROCEDURE"},
    {.name = "NODOFLAG",
     .number = 44,
     .bit_name = "NODOBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "V37 NOT PERMITTED",
     .reset = "V37 PERMITTED"},
};

static const struct flag FLAGWRD3_FLAGS[] = {
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 45, .bit = 15},
    {.name = "GLOKFAIL",
     .number = 46,
     .bit_name = "GLOKFBIT",
     .bit = 14,
     .marks = "(S)",
     .set = "GIMBAL LOCK HAS\n"
            "OCCURRED",
     .reset = "NOT IN GIMBAL LOCK"},
    {.name = "REFSMFLG",
     .number = 47,
     .bit_name = "REFSMBIT",
     .bit = 13,
     .marks = "*** PROTECTED FROM FRESH START ***",
     .set = "REFSMMAT GOOD",
     .reset = "REFSMMAT NO GOOD"},
    {.name = "LUNAFLAG",
     .number = 48,
     .bit_name = "LUNABIT",
     .bit = 12,
     .marks = "(S)",
     .set = "LUNAR LAT-LONG",
     .reset = "EARTH LAT-LONG"},
    {.name = "NOR29FLG",
     .number = 49,
     .bit_name = "NR29FBIT",
     .bit = 11,
     .marks = "(L)",
     .set = "R29 NOT ALLOWED",
     .reset = "R29 ALLOWED (RR DES-\n"
              "IGNATED POWERED FLT)"},
    {.name = "VFLAG",
     .number = 50,
     .bit_name = "VFLAGBIT",
     .bit = 10,
     .marks = "(S)",
     .set = "LESS THAN TWO STARS\n"
            "IN FIELD OF VIEW",
     .reset = "TWO STARS IN FIELD\n"
              "OF VIEW"},
    {.name = "R04FLAG",
     .number = 51,
     .bit_name = "R04FLBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "ALARM 521\n"
            "SUPPRESSED",
     .reset = "ALARM 521 ALLOWED"},
    {.name = "READRFLG",
     .same_as = "R04FLAG",
     .number = 51,
     .bit_name = "READRBIT",
     .bit = 9,
     .marks = "(L)",
     .set = "READING RR DATA\n"
            "PURSUANT TO R29",
     .reset = "NOT READING RR DATA\n"
              "PURSUANT TO R29"},
    {.name = "PRECIFLG",
     .number = 52,
     .bit_name = "PRECIBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "NORMAL INTEGRATION\n"
            "IN P00",
     .reset = "ENGAGES 4-TIME STEP\n"
              "(P00) LOGIC IN INTE-\n"
              "GRATION"},
    {.name = "CULTFLAG",
     .number = 53,
     .bit_name = "CULTBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "STAR OCCULTED",
     .reset = "STAR NOT OCCULTED"},
    {.name = "ORBWFLAG",
     .number = 54,
     .bit_name = "ORBWFBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "W MATRIX VALID FOR\n"
            "ORBITAL NAVIGATION",
     .reset = "W MATRIX INVALID FOR\n"
              "ORBITAL NAVIGATION"},
    {.name = "STATEFLG",
     .number = 55,
     .bit_name = "STATEBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "PERMANENT STATE\n"
            "VECTOR UPDATED",
     .reset = "PERMANENT STATE\n"
              "VECTOR NOT UPDATED"},
    {.name = "INTYPFLG",
     .number = 56,
     .bit_name = "INTYPBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "CONIC INTEGRATION",
     .reset = "ENCKE INTEGRATION"},
    {.name = "VINTFLAG",
     .number = 57,
     .bit_name = "VINTFBIT",
     .bit = 3,
     .marks = "(S)",
     .set = "CSM STATE VECTOR\n"
            "BEING INTEGRATED",
     .reset = "LEM STATE VECTOR\n"
              "BEING INTEGRATED"},
    {.name = "D6OR9FLG",
     .number = 58,
     .bit_name = "D6OR9BIT",
     .bit = 2,
     .marks = "(S)",
     .set = "DIMENSION OF W IS 9\n"
            "FOR INTEGRATION",
     .reset = "DIMENSION OF W IS 6\n"
              "FOR INTEGRATION"},
    {.name = "DIM0FLAG",
     .number = 59,
     .bit_name = "DIM0BIT",
     .bit = 1,
     .marks = "(S)",
     .set = "W MATRIX IS TO BE\n"
            "USED",
     .reset = "W MATRIX IS NOT TO\n"
              "USED"},
};

static const struct flag FLAGWRD4_FLAGS[] = {
    /* LM p. 72 */
    {.name = "MRKIDFLG",
     .number = 60,
     .bit_name = "MRKIDBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "MARK DISPLAY IN\n"
            "ENDIDLE",
     .reset = "NO MARK DISPLAY IN\n"
              "ENDIDLE"},
    {.name = "PRIODFLG",
     .number = 61,
     .bit_name = "PRIODBIT",
     .bit = 14,
     .marks = "(S)",
     .set = "PRIORITY DISPLAY IN\n"
            "ENDIDLE",
     .reset = "NO PRIORITY DISPLAY\n"
              "IN ENDIDLE"},
    {.name = "NRMIDFLG",
     .number = 62,
     .bit_name = "NRMIDBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "NORMAL DISPLAY IN\n"
            "ENDIDLE",
     .reset = "NO NORMAL DISPLAY\n"
              "IN ENDIDLE"},
    {.name = "PDSPFLAG",
     .number = 63,
     .bit_name = "PDSPFBIT",
     .bit = 12,
     .marks = "(S)",
     .set = "P20 SETS SO AS TO\n"
            "TURN A NORMAL DIS-\n"
            "PLAY INTO A PRIORITY\n"
            "DISPLAY IN R60",
     .reset = "LEAVE AS NORMAL DISP"},
    {.name = "MWAITFLG",
     .number = 64,
     .bit_name = "MWAITBIT",
     .bit = 11,
     .marks = "(S)",
     .set = "HIGHER PRIORITY\n"
            "DISPLAY OPERATING\n"
            "WHEN MARK\n"
            "DISPLAY INITIATED",
     .reset = "NO HIGHER PRIORITY\n"
              "DISPLAY OPERATING\n"
              "WHEN MARK DISPLAY\n"
              "INITIATED"},
    {.name = "NWAITFLG",
     .number = 65,
     .bit_name = "NWAITBIT",
     .bit = 10,
     .marks = "(S)",
     .set = "HIGHER PRIORITY\n"
            "DISPLAY OPERATING\n"
            "WHEN NORMAL\n"
            "DISPLAY INITIATED",
     .reset = "NO HIGHER PRIORITY\n"
              "DISPLAY OPERATING\n"
              "WHEN NORMAL DISPLAY\n"
              "INITIATED"},
    {.name = "MRKNVFLG",
     .number = 66,
     .bit_name = "MRKNVBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "ASTRONAUT USING\n"
            "KEYBOARD WHEN MARK\n"
            "DISPLAY INITIATED",
     .reset = "ASTRONAUT NOT USING\n"
              "KEYBOARD WHEN MARK\n"
              "DISPLAY INITIATED"},
    {.name = "NRMNVFLG",
     .number = 67,
     .bit_name = "NRMNVBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "ASTRONAUT USING\n"
            "KEYBOARD WHEN\n"
            "NORMAL DISPLAY\n"
            "INITIATED",
     .reset = "ASTRONAUT NOT USING\n"
              "KEYBOARD WHEN\n"
              "NORMAL DISPLAY\n"
              "INITIATED"},
    {.name = "PRONVFLG",
     .number = 68,
     .bit_name = "PRONVBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "ASTRONAUT USING\n"
            "KEYBOARD WHEN\n"
            "PRIORITY DISPLAY\n"
            "INITIATED",
     .reset = "ASTRONAUT NOT USING\n"
              "KEYBOARD WHEN\n"
              "PRIORITY DISPLAY\n"
              "INITIATED"},
    {.name = "PINBRFLG",
     .number = 69,
     .bit_name = "PINBRBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "ASTRONAUT HAS\n"
            "INTERFERED WITH\n"
            "EXISTING DISPLAY",
     .reset = "ASTRONAUT HAS NOT\n"
              "INTERFERED WITH\n"
              "EXISTING DISPLAY"},
    {.name = "MRUPTFLG",
     .number = 70,
     .bit_name = "MRUPTBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "MARK DISPLAY\n"
            "INTERRUPTED BY\n"
            "PRIORITY DISPLAY",
     .reset = "MARK DISPLAY NOT\n"
              "INTERRUPTED BY\n"
              "PRIORITY DISPLAY"},
    {.name = "NRUPTFLG",
     .number = 71,
     .bit_name = "NRUPTBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "NORMAL DISPLAY\n"
            "INTERRUPTED BY\n"
            "PRIORITY OR MARK\n"
            "DISPLAY",
     .reset = "NORMAL DISPLAY NOT\n"
              "INTERRUPTED BY\n"
              "PRIORITY OR MARK\n"
              "DISPLAY"},
    {.name = "MKOVFLAG",
     .number = 72,
     .bit_name = "MKOVBIT",
     .bit = 3,
     .marks = "(S)",
     .set = "MARK DISPLAY OVER\n"
            "NORMAL",
     .reset = "NO MARK DISPLAY OVER\n"
              "NORMAL"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 73, .bit = 2},
    {.name = "XDSPFLAG",
     .number = 74,
     .bit_name = "XDSPBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "MARK DISPLAY NOT\n"
            "TO BE INTERRUPTED",
     .reset = "NO SPECIAL MARK\n"
              "INFORMATION"},
};

static const struct flag FLAGWRD5_FLAGS[] = {
    {.name = "DSKYFLAG",
     .number = 75,
     .bit_name = "DSKYFBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "DISPLAYS SENT TO\n"
            "DSKY",
     .reset = "NO DISPLAYS TO DSKY"},
    {.number = 76, .bit = 14},
    /* LM p. 74 */
    {.name = "SNUFFER",
     .number = 77,
     .bit_name = "SNUFFBIT",
     .bit = 13,
     .marks = "(S,L)",
     .set = "U,V JETS DISABLED\n"
            "DURING DPS\n"
            "BURNS (V65)",
     .reset = "U,V JETS ENABLED\n"
              "DURING DPS\n"
              "BURNS (V75)"},
    {.name = "NOTHROTL",
     .number = 78,
     .bit_name = "NOTHRBIT",
     .bit = 12,
     .marks = "(S)",
     .set = "INHIBIT FULL\n"
            "THROTTLE",
     .reset = "PERMIT FULL THROTTLE"},
    {.name = "R77FLAG",
     .number = 79,
     .bit_name = "R77FLBIT",
     .bit = 11,
     .marks = "(S,L)",
     .set = "R77 IS ON,\n"
            "SUPPRESS ALL RADAR\n"
            "ALARMS AND TRACKER\n"
            "FAILS",
     .reset = "R77 IS NOT ON."},
    {.name = "RNGSCFLG",
     .number = 80,
     .bit_name = "RNGSCBIT",
     .bit = 10,
     .marks = "(S)",
     .set = "SCALE CHANGE HAS\n"
            "OCCURRED DURING\n"
            "RR READING",
     .reset = "NO SCALE CHANGE HAS\n"
              "OCCURRED DURING\n"
              "RR READING"},
    {.name = "DMENFLG",
     .number = 81,
     .bit_name = "DMENFBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "DIMENSION OF W IS 9\n"
            "FOR INCORPORATION",
     .reset = "DIMENSION OF W IS 6\n"
              "FOR INCORPORATION"},
    {.number = 82, .bit = 8, .marks = "(S)"},
    {.name = "ENGONFLG",
     .number = 83,
     .bit_name = "ENGONBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "ENGINE TURNED ON",
     .reset = "ENGINE TURNED OFF"},
    {.name = "3AXISFLG",
     .number = 84,
     .bit_name = "3AXISBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "MANEUVER SPECIFIED\n"
            "BY THREE AXES",
     .reset = "MANEUVER SPECIFIED\n"
              "BY ONE AXIS; R60\n"
              "CALLS VECPOINT."},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 85, .bit = 5},
    {.name = "NORRMON",
     .number = 86,
     .bit_name = "NORRMBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "BYPASS RR GIMBAL\n"
            "MONITOR",
     .reset = "PERFORM\n"
              "RR GIMBAL MONITOR"},
    {.name = "SOLNSW",
     .number = 87,
     .bit_name = "SOLNSBIT",
     .bit = 3,
     .marks = "(S)",
     .set = "LAMBERT DOES NOT\n"
            "CONVERGE, OR TIME-RAD\n"
            "NEARLY CIRCULAR",
     .reset = "LAMBERT CONVERGES OR\n"
              "TIME-RADIUS NON-\n"
              "CIRCULAR"},
    {.name = "MGLVFLAG",
     .number = 88,
     .bit_name = "MGLVFBIT",
     .bit = 2,
     .marks = "(S)",
     .set = "LOCAL VERTICAL\n"
            "COORDINATES\n"
            "COMPUTED",
     .reset = "MIDDLE GIMBAL ANGLE\n"
              "COMPUTED"},
    {.name = "RENDWFLG",
     .number = 89,
     .bit_name = "RENDWBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "W MATRIX VALID\n"
            "FOR RENDEZVOUS\n"
            "NAVIGATION",
     .reset = "W MATRIX INVALID\n"
              "FOR RENDEZVOUS\n"
              "NAVIGATION"},
};

static const struct flag FLAGWRD6_FLAGS[] = {
    {.name = "S32.1F1",
     .number = 90,
     .bit_name = "S32BIT1",
     .bit = 15,
     .marks = "(S)",
     .set = "DELTA V AT CSI TIME\n"
            "ONE EXCEEDS MAX",
     .reset = "DVT1 LESS THAN MAX"},
    {.name = "S32.1F2",
     .number = 91,
     .bit_name = "S32BIT2",
     .bit = 14,
     .marks = "(S)",
     .set = "FIRST PASS OF\n"
            "NEWTON ITERATION",
     .reset = "REITERATION OF\n"
              "NEWTON"},
    {.name = "S32.1F3A",
     .number = 92,
     .bit_name = "S32BIT3A",
     .bit = 13,
     .marks = "(S)",
     .note = "BIT 13 AND BIT 12 FUNCTION AS AN ORDERED\n"
             "PAIR (13,12) INDICATING THE POSSIBLE OC-\n"
             "CURRENCE OF 2 NEWTON ITERATIONS FOR S32.1\n"
             "IN THE PROGRAM IN THE FOLLOWING ORDER:"},
    {.name = "S32.1F3B",
     .number = 93,
     .bit_name = "S32BIT3B",
     .bit = 12,
     .marks = "(S) (0,1) (I.E. BIT 13 RESET, BIT 12 SET)",
     .note = "= FIRST NEWTON ITERATION BEING DONE\n"
             "(0,0)= FIRST PASS OF SECOND NEWTON ITERATION\n"
             "(1,1)= 50 FT/SEC STAGE OF SECOND NEWTON ITERATION\n"
             "(1,0)= REMAINDER OF SECOND NEWTON ITERATION"},
    {.number = 94, .bit = 11, .marks = "(S)"},
    /* LM p. 76 */
    {.name = "GMBDRVSW",
     .number = 95,
     .bit_name = "GMBDRBIT",
     .bit = 10,
     .marks = "(S)",
     .set = "TRIMGIMB OVER",
     .reset = "TRIMGIMB NOT OVER"},
    {.number = 96, .bit = 9},
    {.name = "MUNFLAG",
     .number = 97,
     .bit_name = "MUNFLBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "SERVICER CALLS\n"
            "MUNRVG",
     .reset = "SERVICER CALLS\n"
              "CALCRVG"},
    {.number = 98, .bit = 7, .marks = "(L)"},
    {.name = "REDFLAG",
     .number = 99,
     .bit_name = "REDFLBIT",
     .bit = 6,
     .marks = "(L)",
     .set = "LANDING SITE\n"
            "REDESIGNATION\n"
            "PERMITTED",
     .reset = "LANDING SITE\n"
              "REDESIGNATION NOT\n"
              "PERMITTED"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 100, .bit = 5},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 101, .bit = 4},
    {.name = "NTARGFLG",
     .number = 102,
     .bit_name = "NTARGBIT",
     .bit = 3,
     .marks = "(S)",
     .set = "ASTRONAUT DID\n"
            "OVERWRITE DELTA\n"
            "VELOCITY AT TPI\n"
            "OR TPM (P34,35)",
     .reset = "ASTRONAUT DID NOT\n"
              "OVERWRITE DELTA\n"
              "VELOCITY"},
    {.name = "AUXFLAG",
     .number = 103,
     .bit_name = "AUXFLBIT",
     .bit = 2,
     .set = "PROVIDING IDLEFLAG\n"
            "IS NOT SET, SERV-\n"
            "ICER WILL EXERCISE\n"
            "DVMON ON ITS NEXT\n"
            "PASS.",
     .reset = "SERVICER WILL SKIP\n"
              "DVMON ON ITS NEXT\n"
              "PASS EVEN IF THE\n"
              "IDLEFLAG IS NOT SET.\n"
              "IT WILL THEN SET\n"
              "AUXFLAG."},
    {.name = "ATTFLAG",
     .number = 104,
     .bit_name = "ATTFLBIT",
     .bit = 1,
     .marks = "(L)",
     .set = "LEM ATTITUDE EXISTS\n"
            "IN MOON-FIXED\n"
            "COORDINATES",
     .reset = "NO LEM ATTITUDE\n"
              "AVAILABLE IN MOON-\n"
              "FIXED COORDINATES"},
};

static const struct flag FLAGWRD7_FLAGS[] = {
    {.name = "ITSWICH",
     .number = 105,
     .bit_name = "ITSWBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "R34;TPI TIME TO BE\n"
            "COMPUTED",
     .reset = "TPI HAS BEEN\n"
              "COMPUTED"},
    {.name = "MANUFLAG",
     .number = 106,
     .bit_name = "MANUFBIT",
     .bit = 14,
     .marks = "(S)",
     .set = "ATTITUDE MANEUVER\n"
            "GOING DURING RR\n"
            "SEARCH",
     .reset = "NO ATTITUDE MANEUVER\n"
              "DURING RR SEARCH"},
    {.name = "IGNFLAG",
     .number = 107,
     .bit_name = "IGNFLBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "TIG HAS ARRIVED",
     .reset = "TIG HAS NOT ARRIVED"},
    {.name = "ASTNFLAG",
     .number = 108,
     .bit_name = "ASTNBIT",
     .bit = 12,
     .marks = "(S)",
     .set = "ASTRONAUT HAS\n"
            "OKAYED IGNITION",
     .reset = "ASTRONAUT HAS NOT\n"
              "OKAYED IGNITION"},
    {.name = "SWANDISP",
     .number = 109,
     .bit_name = "SWANDBIT",
     .bit = 11,
     .marks = "(L)",
     .set = "LANDING ANALOG\n"
            "DISPLAYS ENABLED",
     .reset = "LANDING ANALOG\n"
              "DISPLAYS SUPPRESSED"},
    {.name = "NORMSW",
     .number = 110,
     .bit_name = "NORMSBIT",
     .bit = 10,
     .marks = "(S)",
     .set = "UNIT NORMAL INPUT\n"
            "TO LAMBERT",
     .reset = "LAMBERT COMPUTES ITS\n"
              "OWN UNIT NORMAL"},
    {.name = "RVSW",
     .number = 111,
     .bit_name = "RVSWBIT",
     .bit = 9,
     .marks = "(S)",
     .set = "DO NOT COMPUTE\n"
            "FINAL STATE VECTOR\n"
            "IN TIME-DELTA",
     .reset = "COMPUTE FINAL STATE\n"
              "VECTOR IN TIME-THETA"},
    {.name = "V67FLAG",
     .number = 112,
     .bit_name = "V67FLBIT",
     .bit = 8,
     .marks = "(S)",
     .set = "ASTRONAUT OVERWRITE\n"
            "W-MATRIX INITIAL\n"
            "VALUES",
     .reset = "ASTRONAUT DOES NOT\n"
              "OVERWRITE W-MATRIX\n"
              "INITIAL VALUES"},
    /* LM p. 78 */
    {.name = "IDLEFLAG",
     .number = 113,
     .bit_name = "IDLEFBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "NO DV MONITOR",
     .reset = "CONNECT DV MONITOR"},
    {.name = "V37FLAG",
     .number = 114,
     .bit_name = "V37FLBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "AVERAGEG (SERVICER)\n"
            "RUNNING",
     .reset = "AVERAGEG (SERVICER)\n"
              "OFF"},
    {.name = "AVEGFLAG",
     .number = 115,
     .bit_name = "AVEGFBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "AVERAGEG (SERVICER)\n"
            "DESIRED",
     .reset = "AVERAGEG (SERVICER)\n"
              "NOT DESIRED"},
    {.name = "UPLOCKFL",
     .number = 116,
     .bit_name = "UPLOCBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "K-KBAR-K FAIL",
     .reset = "NO K-KBAR-K FAIL"},
    {.name = "VERIFLAG",
     .number = 117,
     .bit_name = "VERIFBIT",
     .bit = 3,
     .marks = "(S)",
     .note = "CHANGED WHEN V33E OCCURS AT END OF P27"},
    {.name = "V82EMFLG",
     .number = 118,
     .bit_name = "V82EMBIT",
     .bit = 2,
     .marks = "(L,C)",
     .set = "MOON VICINITY",
     .reset = "EARTH VICINITY"},
    {.name = "TFFSW",
     .number = 119,
     .bit_name = "TFFSWBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "CALCULATE TPERIGEE",
     .reset = "CALCULATE TFF"},
};

static const struct flag FLAGWRD8_FLAGS[] = {
    {.name = "RPQFLAG",
     .number = 120,
     .bit_name = "RPQFLBIT",
     .bit = 15,
     .marks = "(S)",
     .set = "RPQ NOT COMPUTED\n"
            "(RPQ = VECTOR BE-\n"
            "TWEEN SECONDARY BODY\n"
            "AND PRIMARY BODY)",
     .reset = "RPQ COMPUTED"},
    {.number = 121, .bit = 14},
    /* LM p. 79 */
    {.name = "NEWIFLG",
     .number = 122,
     .bit_name = "NEWIBIT",
     .bit = 13,
     .marks = "(S)",
     .set = "FIRST PASS THROUGH\n"
            "INTEGRATION",
     .reset = "SUCCEEDING ITERATION\n"
              "OF INTEGRATION"},
    {.name = "CMOONFLG",
     .number = 123,
     .bit_name = "CMOONBIT",
     .bit = 12,
     .marks = "*** PROTECTED FROM FRESH START ***",
     .set = "PERMANENT CSM STATE\n"
            "IN LUNAR SPHERE",
     .reset = "PERMANENT CSM STATE\n"
              "IN EARTH SPHERE"},
    {.name = "LMOONFLG",
     .number = 124,
     .bit_name = "LMOONBIT",
     .bit = 11,
     .marks = "*** PROTECTED FROM FRESH START ***",
     .set = "PERMANENT LM STATE\n"
            "IN LUNAR SPHERE",
     .reset = "PERMANENT LM STATE\n"
              "IN EARTH SPHERE"},
    {.name = "FLUNDISP",
     .number = 125,
     .bit_name = "FLUNDBIT",
     .bit = 10,
     .marks = "(L)",
     .set = "CURRENT GUIDANCE\n"
            "DISPLAYS INHIBITED",
     .reset = "CURRENT GUIDANCE\n"
              "DISPLAYS PERMITTED"},
    {.name = "P39/79SW",
     .number = 126,
     .bit_name = "P39SWBIT",
     .bit = 9,
     .marks = "(L)",
     .set = "P39/79 OPERATING",
     .reset = "P38/78 OPERATING"},
    {.name = "SURFFLAG",
     .number = 127,
     .bit_name = "SURFFBIT",
     .bit = 8,
     .marks = "*** PROTECTED FROM FRESH START ***",
     .set = "LM ON LUNAR SURFACE",
     .reset = "LM NOT ON LUNAR\n"
              "SURFACE"},
    {.name = "INFINFLG",
     .number = 128,
     .bit_name = "INFINBIT",
     .bit = 7,
     .marks = "(S)",
     .set = "NO CONIC SOLUTION\n"
            "(CLOSURE THROUGH\n"
            "INFINITY REQUIRED)",
     .reset = "CONIC SOLUTION\n"
              "EXISTS"},
    {.name = "ORDERSW",
     .number = 129,
     .bit_name = "ORDERBIT",
     .bit = 6,
     .marks = "(S)",
     .set = "ITERATOR USES 2ND\n"
            "ORDER MINIMUM MODE",
     .reset = "ITERATOR USES 1ST\n"
              "ORDER STANDARD MODE"},
    {.name = "APSESW",
     .number = 130,
     .bit_name = "APSESBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "RDESIRED OUTSIDE\n"
            "PERICENTER-APOCENTER\n"
            "RANGE IN TIME-RADIUS",
     .reset = "RDESIRED INSIDE\n"
              "PERICENTER-APOCENTER\n"
              "RANGE IN TIME-RADIUS"},
    {.name = "COGAFLAG",
     .number = 131,
     .bit_name = "COGAFBIT",
     .bit = 4,
     .marks = "(S)",
     .set = "NO CONIC SOLUTION --\n"
            "TOO CLOSE TO RECTI-\n"
            "LINEAR (COGA OVERFLWS)",
     .reset = "CONIC SOLUTION\n"
              "EXISTS (COGA DOES NOT\n"
              "OVERFLOW)"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 132, .bit = 3, .marks = "(S)"},
    {.name = "INITALGN",
     .number = 133,
     .bit_name = "INITABIT",
     .bit = 2,
     .marks = "(L)",
     .set = "INITIAL PASS THRU\n"
            "P57",
     .reset = "SECOND PASS THRU P57\n"
              "(CHECK RESET-MILLARD)"},
    {.name = "360SW",
     .number = 134,
     .bit_name = "360SWBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "TRANSFER ANGLE NEAR\n"
            "360 DEGREES",
     .reset = "TRANSFER ANGLE NOT\n"
              "NEAR 360 DEGREES"},
};

static const struct flag FLAGWRD9_FLAGS[] = {
    {.number = 135, .bit = 15},
    {.name = "FLVR",
     .number = 136,
     .bit_name = "FLVRBIT",
     .bit = 14,
     .marks = "(L)",
     .set = "VERTICAL RISE\n"
            "(ASCENT GUIDANCE)",
     .reset = "NON-VERTICAL RISE"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 137, .bit = 13},
    {.name = "FLPC",
     .number = 138,
     .bit_name = "FLPCBIT",
     .bit = 12,
     .marks = "(L)",
     .set = "NO POSITION CONTROL\n"
            "(ASCENT GUIDANCE)",
     .reset = "POSITION CONTROL"},
    {.name = "FLPI",
     .number = 139,
     .bit_name = "FLPIBIT",
     .bit = 11,
     .marks = "(L)",
     .set = "PRE-IGNITION PHASE\n"
            "(ASCENT GUIDANCE)",
     .reset = "REGULAR GUIDANCE"},
    {.name = "FLRCS",
     .number = 140,
     .bit_name = "FLRCSBIT",
     .bit = 10,
     .marks = "(L)",
     .set = "RCS INJECTION MODE\n"
            "(ASCENT GUIDANCE)",
     .reset = "MAIN ENGINE MODE"},
    {.name = "LETABORT",
     .number = 141,
     .bit_name = "LETABBIT",
     .bit = 9,
     .marks = "(L)",
     .set = "ABORT PROGRAMS\n"
            "ARE ENABLED",
     .reset = "ABORT PROGRAMS\n"
              "ARE NOT ENABLED"},
    {.name = "FLAP",
     .number = 142,
     .bit_name = "FLAPBIT",
     .bit = 8,
     .marks = "(L)",
     .set = "APS CONTINUED ABORT\n"
            "AFTER DPS STAGING\n"
            "(ASCENT GUIDANCE)",
     .reset = "APS ABORT IS NOT A\n"
              "CONTINUATION"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 143, .bit = 7, .marks = "(L)"},
    {.name = "ROTFLAG",
     .number = 144,
     .bit_name = "ROTFLBIT",
     .bit = 6,
     .marks = "(L)",
     .set = "P70 AND P71 WILL\n"
            "FORCE VEHICLE\n"
            "ROTATION IN THE\n"
            "PREFERRED DIRECTION",
     .reset = "P70 AND P71 WILL NOT\n"
              "FORCE VEHICLE\n"
              "ROTATION IN THE\n"
              "PREFERRED DIRECTION"},
    {.name = "QUITFLAG",
     .number = 145,
     .bit_name = "QUITBIT",
     .bit = 5,
     .marks = "(S)",
     .set = "DISCONTINUE INTEGR.",
     .reset = "CONTINUE INTEGRATION"},
    {.number = 146, .bit = 4},
    {.name = "MID1FLAG",
     .number = 147,
     .bit_name = "MID1FBIT",
     .bit = 3,
     .marks = "(L)",
     .set = "INTEGRAT TO TDEC",
     .reset = "INTEGRATE TO THE\n"
              "THEN-PRESENT TIME"},
    {.name = "MIDAVFLG",
     .number = 148,
     .bit_name = "MIDAVBIT",
     .bit = 2,
     .marks = "(L)",
     .set = "INTEGRATION ENTERED\n"
            "FROM ONE OF MIDTOAV\n"
            "PORTALS",
     .reset = "INTEGRATION WAS\n"
              "NOT ENTERED VIA\n"
              "MIDTOAV"},
    {.name = "AVEMIDSW",
     .number = 149,
     .bit_name = "AVEMDBIT",
     .bit = 1,
     .marks = "(S)",
     .set = "AVETOMID CALLING\n"
            "FOR W.MATRIX INTEGR\n"
            "DON'T WRITE OVER RN,\n"
            "VN,PIPTIME",
     .reset = "NO AVETOMID W INTEGR\n"
              "ALLOW SET UP RM, VN\n"
              "PIPTIME"},
};

static const struct flag FLGWRD10_FLAGS[] = {
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 150, .bit = 15, .marks = "(S)"},
    {.name = "INTFLAG",
     .number = 151,
     .bit_name = "INTFLBIT",
     .bit = 14,
     .marks = "(L,C)",
     .set = "INTEGRATION IN\n"
            "PROGRESS",
     .reset = "INTEGRATION NOT IN\n"
              "PROGRESS"},
    {.name = "APSFLAG",
     .number = 152,
     .bit_name = "APSFLBIT",
     .bit = 13,
     .marks = "(S,L)",
     .set = "ASCENT STAGE\n"
            "*** PROTECTED FROM FRESH START ***",
     .reset = "DESCENT STAGE"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 153, .bit = 12},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 154, .bit = 11},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 155, .bit = 10},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 156, .bit = 9},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 157, .bit = 8},
    {.name = "REINTFLG",
     .number = 158,
     .bit_name = "REINTBIT",
     .bit = 7,
     .marks = "(L,C)",
     .set = "INTEGRATION ROUTINE\n"
            "TO BE RESTARTED",
     .reset = "INTEGRATION ROUTINE\n"
              "NOT TO BE RESTARTED"},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 159, .bit = 6},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 160, .bit = 5},
    /* LM p. 83 */
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 161, .bit = 4},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 162, .bit = 3},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 163, .bit = 2},
    /* (The transcription notes this bit's BIT line as not in the scan.) */
    {.number = 164, .bit = 1},
};

static const struct flag FLGWRD11_FLAGS[] = {
    {.name = "LRBYPASS",
     .number = 165,
     .bit_name = "LRBYBIT",
     .bit = 15,
     .marks = "(L)(R12)",
     .set = "BYPASS ALL LANDING\n"
            "RADAR UPDATES",
     .reset = "DO NOT BYPASS LR\n"
              "UPDATES"},
    {.number = 166, .bit = 14},
    {.number = 167, .bit = 13},
    {.name = "VXINH",
     .number = 168,
     .bit_name = "VXINHBIT",
     .bit = 12,
     .marks = "(L)(R12)",
     .set = "IF Z VELOCITY DATA\n"
            "UNREASONABLE,\n"
            "BYPASS X VELOCITY\n"
            "UPDATE ON NEXT PASS",
     .reset = "UPDATE X AXIS\n"
              "VELOCITY"},
    {.name = "PSTHIGAT",
     .number = 169,
     .bit_name = "PSTHIBIT",
     .bit = 11,
     .marks = "(L)(R12)",
     .set = "PAST HIGATE",
     .reset = "PREHIGATE"},
    {.name = "NOLRREAD",
     .number = 170,
     .bit_name = "NOLRRBIT",
     .bit = 10,
     .marks = "(L)(R12)",
     .set = "LANDING RADAR\n"
            "REPOSITIONING;\n"
            "BYPASS UPDATE",
     .reset = "LR NOT REPOSITIONING"},
    {.name = "XORFLG",
     .number = 171,
     .bit_name = "XORFLBIT",
     .bit = 9,
     .marks = "(L)(R12)",
     .set = "BELOW LIMIT\n"
            "INHIBIT X AXIS\n"
            "OVERRIDE",
     .reset = "ABOVE LIMIT DO\n"
              "NOT INHIBIT"},
    {.name = "LRINH",
     .number = 172,
     .bit_name = "LRINHBIT",
     .bit = 8,
     .set = "LANDING RADAR UP-\n"
            "DATES PERMITTED\n"
            "BY ASTRONAUT",
     .reset = "LR UPDATES INHIBITED\n"
              "BY ASTRONAUT"},
    {.name = "VELDATA",
     .number = 173,
     .bit_name = "VELDABIT",
     .bit = 7,
     .marks = "(L)(R12)",
     .set = "LR VELOCITY\n"
            "MEASUREMENT MADE",
     .reset = "LR VELOCITY MEASURE\n"
              "NOT MADE"},
    {.name = "READLR",
     .number = 174,
     .bit_name = "READLBIT",
     .bit = 6,
     .marks = "(L)(R12)",
     .set = "OK TO READ LR\n"
            "RANGE DATA",
     .reset = "DO NOT READ LR RANGE\n"
              "DATA"},
    {.name = "READVEL",
     .number = 175,
     .bit_name = "READVBIT",
     .bit = 5,
     .marks = "(L)(R12)",
     .set = "OK TO READ LR\n"
            "VELOCITY DATA",
     .reset = "DO NOT READ LR\n"
              "VELOCITY DATA"},
    {.name = "RNGEDATA",
     .number = 176,
     .bit_name = "RNGEDBIT",
     .bit = 4,
     .marks = "(L)(R12)",
     .set = "LR ALTITUDE\n"
            "MEASUREMENT MADE",
     .reset = "LR ALTITUDE MEASURE\n"
              "NOT MADE"},
    {.name = "SCALBAD",
     .number = 177,
     .bit_name = "SCABBIT",
     .bit = 3,
     .set = "LR LOW SCALE DISP-\n"
            "CRETE NOT PRESENT\n"
            "WHEN IT SHOULD",
     .reset = "LS SCALE DISCRETE\n"
              "APPEARS OK"},
    {.name = "VFLSHFLG",
     .number = 178,
     .bit_name = "VFLSHBIT",
     .bit = 2,
     .marks = "(L)(R12)",
     .set = "LR VELOCITY FAIL\n"
            "LAMP SHOULD BE\n"
            "FLASHING",
     .reset = "LR VEL FAIL LAMP\n"
              "SHOULDN'T FLASH"},
    {.name = "HFLSHFLG",
     .number = 179,
     .bit_name = "HFLSHBIT",
     .bit = 1,
     .marks = "(L)(R12)",
     .set = "LR ALTITUDE FAIL\n"
            "LAMP SHOULD BE\n"
            "FLASHING",
     .reset = "LR ALTITUDE FAIL\n"
              "LAMP SHOULD NOT BE\n"
              "FLASHING"},
};

static const struct flag FLGWRD12_FLAGS[] = {
    {.name = "CDESFLAG",
     .number = 180,
     .bit_name = "CDESBIT",
     .bit = 15,
     .set = "CONTINUOUS DESIG-\n"
            "NATE, LGC COMMANDS\n"
            "RR REGARDLESS OF\n"
            "LOCK-ON",
     .reset = "LGC CHECKS FOR LOCK-\n"
              "ON WHEN ANTENNA\n"
              "BEING DESIGNATED"},
    {.name = "REMODFLG",
     .number = 181,
     .bit_name = "REMODBIT",
     .bit = 14,
     .set = "CHANGE IN ANTENNA\n"
            "MODE BEEN REQUESTED\n"
            "I.E., REMODE",
     .reset = "NO REMODE REQUESTED\n"
              "OR OCCURRING"},
    {.name = "RCDU0FLG",
     .number = 182,
     .bit_name = "RCDU0BIT",
     .bit = 13,
     .set = "RR CDU'S BEING\n"
            "ZEROED",
     .reset = "RR CDU'S NOT BEING\n"
              "ZEROED"},
    {.name = "ANTENFLG",
     .number = 183,
     .bit_name = "ANTENBIT",
     .bit = 12,
     .set = "RR ANTENNA MODE IS\n"
            "MODE 2",
     .reset = "RR ANTENNA IN MODE 1"},
    {.name = "REPOSMON",
     .number = 184,
     .bit_name = "REPOSBIT",
     .bit = 11,
     .set = "REPOSITION MONITOR.\n"
            "RR REPOSITION IS\n"
            "TAKING PLACE",
     .reset = "NO REPOSITION TAKING\n"
              "PLACE"},
    {.name = "DESIGFLG",
     .number = 185,
     .bit_name = "DESIGBIT",
     .bit = 10,
     .set = "RR DESIGNATE\n"
            "REQUESTED OR IN\n"
            "PROGRESS",
     .reset = "RR DESIGNATE NOT\n"
              "REQUESTED OR IN\n"
              "PROGRESS"},
    {.name = "ALTSCALE",
     .number = 186,
     .bit_name = "ALTSCBIT",
     .bit = 9,
     .set = "LR ALTITUDE READING\n"
            "IS ON HIGH SCALE",
     .reset = "LR ALTITUDE READING\n"
              "IS ON LOW SCALE"},
    /* LM p. 86 */
    {.name = "LRVELFLG",
     .number = 187,
     .bit_name = "LRVELBIT",
     .bit = 8,
     .set = "LR VELOCITY DATA\n"
            "FAIL",
     .reset = "NO LR VELOCITY DATA\n"
              "FAIL"},
    {.name = "RCDUFAIL",
     .number = 188,
     .bit_name = "RCDUFBIT",
     .bit = 7,
     .set = "RR CDU FAIL HAS\n"
            "NOT OCCURRED",
     .reset = "RR CDU FAIL OCCURRED"},
    {.name = "LRPOSFLG",
     .number = 189,
     .bit_name = "LRPOSBIT",
     .bit = 6,
     .set = "LANDING RADAR\n"
            "POSITION 2",
     .reset = "LR POSITION 1"},
    {.name = "LRALTFLG",
     .number = 190,
     .bit_name = "LRALTBIT",
     .bit = 5,
     .set = "LR ALTITUDE DATA\n"
            "FAIL.  COULD NOT BE\n"
            "READ SUCCESSFULLY.",
     .reset = "NO LR ALTITUDE DATA\n"
              "FAIL"},
    {.name = "RRDATAFL",
     .number = 191,
     .bit_name = "RRDATABT",
     .bit = 4,
     .set = "RR DATA FAIL.\n"
            "DATA COULD NOT BE\n"
            "READ SUCCESSFULLY",
     .reset = "NO RR DATA FAIL"},
    {.name = "RRRSFLAG",
     .number = 192,
     .bit_name = "RRRSBIT",
     .bit = 3,
     .set = "RR RANGE READING\n"
            "ON THE HIGH SCALE",
     .reset = "RR RANGE READING ON\n"
              "THE LOW SCALE"},
    {.name = "AUTOMODE",
     .number = 193,
     .bit_name = "AUTOMBIT",
     .bit = 2,
     .set = "RR NOT IN AUTO MODE.\n"
            "AUTO MODE DISCRETE\n"
            "IS NOT PRESENT",
     .reset = "RR IN AUTO MODE"},
    {.name = "TURNONFL",
     .number = 194,
     .bit_name = "TURNONBT",
     .bit = 1,
     .set = "RR TURN-ON SEQUENCE\n"
            "IN PROGRESS.  (ZERO\n"
            "CDU'S, FIX ANTENNA\n"
            "MODE)",
     .reset = "NO RR TURN-ON\n"
              "SEQUENCE IN PROGRESS"},
};

static const struct flag FLGWRD13_FLAGS[] = {
    {.name = "PULSEFLG",
     .number = 195,
     .bit_name = "PULSES",
     .bit = 15,
     .set = "MINIMUM IMPUSE\n"
            "COMMAND MODE IN\n"
            "\"ATT HOLD\" (V76)",
     .reset = "NOT IN MINIMUM\n"
              "IMPULSE COMMAND MODE\n"
              "(V77)"},
    {.name = "USEQRFLG",
     .number = 196,
     .bit_name = "USEQRJTS",
     .bit = 14,
     .set = "GIMBAL UNUSABLE.\n"
            "USE JETS ONLY.",
     .reset = "TRIM GIMBAL MAY BE\n"
              "USED."},
    {.name = "CSMDKFLG",
     .number = 197,
     .bit_name = "CSMDOCKD",
     .bit = 13,
     .set = "CSM DOCKED.  USE\n"
            "BACKUP DAP",
     .reset = "CSM NOT DOCKED TO LM"},
    {.name = "OURRCFLG",
     .number = 198,
     .bit_name = "OURRCBIT",
     .bit = 12,
     .set = "CURRENT DAP PASS\n"
            "IS RATE COMMAND",
     .reset = "CURRENT DAP PASS IS\n"
              "NOT RATE COMMAND"},
    {.name = "ACC4-2FL",
     .number = 199,
     .bit_name = "ACC4OR2X",
     .bit = 11,
     .set = "4 JET X-AXIS TRANS-\n"
            "LATION REQUESTED",
     .reset = "2 JET X-AXIS TRANS-\n"
              "LATION REQUESTED"},
    {.name = "AORBTFLG",
     .number = 200,
     .bit_name = "AORBTRAN",
     .bit = 10,
     .set = "B SYSTEM FOR X-\n"
            "TRANSLATION",
     .reset = "A SYSTEM FOR X-\n"
              "TRANSLATION PREFER'D"},
    {.name = "XOVINFLG",
     .number = 201,
     .bit_name = "XOVINHIB",
     .bit = 9,
     .set = "X-AXIS OVERRIDE\n"
            "LOCKED OUT",
     .reset = "X-AXIS OVERRIDE OKAY"},
    {.name = "DRIFTDFL",
     .number = 202,
     .bit_name = "DRIFTBIT",
     .bit = 8,
     .set = "ASSUME 0 OFFSET\n"
            "DRIFTING FLIGHT",
     .reset = "USE OFFSET ACCELERA-\n"
              "ION ESTIMATE"},
    {.name = "RHCSCFLG",
     .number = 203,
     .bit_name = "RHCSCALE",
     .bit = 7,
     .set = "NORMAL RHC SCALING\n"
            "REQUESTED",
     .reset = "FINE RHC SCALING\n"
              "REQUESTED"},
    /* LM p. 88 */
    {.name = "ULLAGFLG",
     .number = 204,
     .bit_name = "ULLAGER",
     .bit = 6,
     .set = "ULLAGE REQUEST BY\n"
            "MISSION PROGRAM",
     .reset = "NO INTERNAL ULLAGE\n"
              "REQUEST"},
    {.name = "AORBSFLG",
     .number = 205,
     .bit_name = "AORBSYST",
     .bit = 5,
     .set = "P-AXIS COUPLES 7.15\n"
            "AND 8.16 PREFERRED",
     .reset = "P-AXIS COUPLES 4.12\n"
              "AND 3.11 PREFERRED"},
    {.name = "DBSELFLG",
     .number = 206,
     .bit_name = "DBSELECT",
     .bit = 4,
     .set = "MAX DB SELECTED\n"
            "BY CREW (5 DEG)",
     .reset = "MIN DB SELECTED BY\n"
              "CREW (0.3 DEG)"},
    {.name = "ACCOKFLG",
     .number = 207,
     .bit_name = "ACCSOKAY",
     .bit = 3,
     .set = "CONTROL AUTHORITY\n"
            "VALUES FROM 1/ACCS\n"
            "USABLE",
     .reset = "RESTART OR FRESH ST.\n"
              "SINCE LAST 1/ACCS;\n"
              "OUTPUTS SUSPECT."},
    {.name = "AUTR2FLG",
     .number = 208,
     .bit_name = "AUTRATE2",
     .bit = 2,
     .note = "THESE FLAGS ARE USED TOGETHER TO INDICATE\n"
             "ASTRONAUT-CHOSEN KALCMANU MANEUVER RATES\n"
             "(0,0)=(BIT2,BIT1)= 0.2 DEG/SEC"},
    {.name = "AUTR1FLG",
     .number = 209,
     .bit_name = "AUTRATE1",
     .bit = 1,
     .marks = "(0,1)= 0.5 DEG/SEC",
     .note = "(1,0)= 2.0 DEG/SEC\n"
             "(1,1)= 10.0 DEG/SEC"},
};

const struct flagword FLAGWORDS[] = {
    {NULL, NULL, "FLAGWRD0", 0, NULL, FLAGWRD0_FLAGS, COUNT(FLAGWRD0_FLAGS)},
    {NULL, NULL, "FLAGWRD1", 1, NULL, FLAGWRD1_FLAGS, COUNT(FLAGWRD1_FLAGS)},
    {NULL, NULL, "FLAGWRD2", 2, NULL, FLAGWRD2_FLAGS, COUNT(FLAGWRD2_FLAGS)},
    {NULL, NULL, "FLAGWRD3", 3, NULL, FLAGWRD3_FLAGS, COUNT(FLAGWRD3_FLAGS)},
    {NULL, NULL, "FLAGWRD4", 4, NULL, FLAGWRD4_FLAGS, COUNT(FLAGWRD4_FLAGS)},
    {NULL, NULL, "FLAGWRD5", 5, NULL, FLAGWRD5_FLAGS, COUNT(FLAGWRD5_FLAGS)},
    {NULL, NULL, "FLAGWRD6", 6, NULL, FLAGWRD6_FLAGS, COUNT(FLAGWRD6_FLAGS)},
    {NULL, NULL, "FLAGWRD7", 7, NULL, FLAGWRD7_FLAGS, COUNT(FLAGWRD7_FLAGS)},
    {NULL, NULL, "FLAGWRD8", 8, NULL, FLAGWRD8_FLAGS, COUNT(FLAGWRD8_FLAGS)},
    {NULL, NULL, "FLAGWRD9", 9, NULL, FLAGWRD9_FLAGS, COUNT(FLAGWRD9_FLAGS)},
    {"RASFLAG", "WAS ONLY AN INSTALL-ERASTALL FLAG", "FLGWRD10", 10, NULL, FLGWRD10_FLAGS,
     COUNT(FLGWRD10_FLAGS)},
    {NULL, NULL, "FLGWRD11", 11, NULL, FLGWRD11_FLAGS, COUNT(FLGWRD11_FLAGS)},
    {"RADMODES", "RADAR FLAG WORD", "FLGWRD12", 12, "WAS RADMODES", FLGWRD12_FLAGS,
     COUNT(FLGWRD12_FLAGS)},
    {"DAPBOOLS", "DIGITAL AUTOPILOT FLAGWORD", "FLGWRD13", 13, "WAS DAPBOOLS", FLGWRD13_FLAGS,
     COUNT(FLGWRD13_FLAGS)},
};
const unsigned FLAGWORD_COUNT = COUNT(FLAGWORDS);

#endif
