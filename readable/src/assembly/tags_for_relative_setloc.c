/* SPDX-License-Identifier: MIT */
/* TAGS_FOR_RELATIVE_SETLOC.agc: Luminary099 pages 28-37, Comanche055 pages
 * 27-35. The listing file that says which bank of fixed memory each part
 * of the program goes in. It holds no routine.
 *
 * For each bank it defines names, the tags: a tag is a label with no word
 * under it, so every tag of a bank stands for the bank's first word. A
 * part of the program elsewhere in the listing opens with SETLOC and its
 * tag, which goes to the tag's bank, and then BANK with no number, which
 * goes on at that bank's next free word. To move a part to another bank
 * only its tag moves, here. BLOCK 02 and BLOCK 03 are fixed-fixed memory
 * (04000-07777), BANK 00 to BANK 43 the switched banks. After a bank's
 * tags its BNKSUM card asks the assembler for the bank's sum check.
 *
 * After the banks come names that stand for other names (EQUALS, or =):
 * HI... and LO... for the constants held twice, once in high and once in
 * low fixed memory, and the names by which the code both spacecraft share
 * (orbital integration) reaches this spacecraft's own words and the other
 * one's. The CM's file ends with the ECADRs its programs use to switch
 * erasable banks: seven words of the rope.
 *
 * The bank numbers are octal, as the listing writes them, and the
 * listing's spelling stands (the CM's DETVRUZVING). */
#include <stddef.h>
#include "../agc/names.h"
#include "assembly.h"

/* clang-format off */
#define TAG(name)                        {name, NULL}
#define TAG_AND_REMARK(name, remark)     {name, remark}

#define EQUALS(name, of)                    {name, "EQUALS", of, NULL}
#define EQUALS_AND_REMARK(name, of, remark) {name, "EQUALS", of, remark}
#define IS(name, of)                        {name, "=", of, NULL}
#define CARD(card, operand)                 {NULL, card, operand, NULL}
#define REMARK(remark)                      {NULL, NULL, NULL, remark}
/* clang-format on */

#if LUMINARY099

const char *const TAGS_HEADING[] = {
    "TAGS FOR RELATIVE SETLOC AND BLANK BANK CARDS",
    "COUNT BANKSUM",
};
const unsigned TAGS_HEADING_COUNT = COUNT(TAGS_HEADING);

static const struct setloc_tag BLOCK_02_TAGS[] = {
    TAG("RADARFF"), TAG("FFTAG1"), TAG("FFTAG2"),  TAG("FFTAG3"),  TAG("FFTAG4"),  TAG("FFTAG7"),
    TAG("FFTAG8"),  TAG("FFTAG9"), TAG("FFTAG10"), TAG("FFTAG11"), TAG("FFTAG12"), TAG("FFTAG13"),
};

static const struct setloc_tag BLOCK_03_TAGS[] = {
    TAG("FFTAG5"),
    TAG("FFTAG6"),
};

static const struct setloc_tag BANK_00_TAGS[] = {
    TAG("DLAYJOB"),
};

static const struct setloc_tag BANK_01_TAGS[] = {
    TAG("RESTART"),
    TAG("LOADDAP1"),
};

static const struct setloc_tag BANK_04_TAGS[] = {
    TAG("R02"),   TAG("VERB37"),  TAG("PINBALL4"), TAG("CONICS1"),  TAG("KEYRUPT"),
    TAG("R36LM"), TAG("UPDATE2"), TAG("E/PROG"),   TAG("AOTMARK2"),
};

static const struct setloc_tag BANK_05_TAGS[] = {
    TAG("FRANDRES"), TAG("DOWNTELM"), TAG("ABORTS1"), TAG("EPHEM1"), TAG("ASENT3"),
};

static const struct setloc_tag BANK_06_TAGS[] = {
    TAG("IMUCOMP"), TAG("T4RUP"), TAG("RCSMONT"), TAG("MIDDGIM"), TAG("EARTHLOC"),
};

static const struct setloc_tag BANK_07_TAGS[] = {
    TAG("AOTMARK1"),
    TAG("MODESW"),
    TAG("ASENT2"),
};

static const struct setloc_tag BANK_10_TAGS[] = {
    TAG("RTBCODES"), TAG("DISPLAYS"), TAG("PHASETAB"), TAG("FLESHLOC"), TAG("SLCTMU"),
};

static const struct setloc_tag BANK_11_TAGS[] = {
    TAG("ORBITAL"),
    TAG("F2DPS*11"),
    TAG("INTVEL"),
};

static const struct setloc_tag BANK_12_TAGS[] = {
    TAG("CONICS"),
    TAG("ORBITAL1"),
    /* LM p. 30 */
    TAG("INTPRET2"),
};

static const struct setloc_tag BANK_13_TAGS[] = {
    TAG("LATLONG"), TAG("INTINIT"), TAG("LEMGEOM"), TAG("P76LOC"), TAG("ORBITAL2"), TAG("ABTFLGS"),
};

static const struct setloc_tag BANK_14_TAGS[] = {
    TAG("P50S1"),
    TAG("STARTAB"),
    TAG("ASENT4"),
};

static const struct setloc_tag BANK_15_TAGS[] = {
    TAG("P50S"),
    TAG("EPHEM"),
};

static const struct setloc_tag BANK_16_TAGS[] = {
    TAG("DAPS1"),
};

static const struct setloc_tag BANK_17_TAGS[] = {
    TAG("DAPS2"),
    TAG("P40S3"),
};

static const struct setloc_tag BANK_20_TAGS[] = {
    TAG("DAPS3"),
    TAG("LOADDAP"),
    TAG("RODTRAP"),
};

static const struct setloc_tag BANK_21_TAGS[] = {
    TAG("DAPS4"),
    TAG("R10"),
    TAG("R11"),
};

static const struct setloc_tag BANK_22_TAGS[] = {
    TAG("KALCMON1"), TAG("KALCMON2"), TAG("R30LOC"), TAG("RENDEZ"), TAG("SERV2"), TAG("LANDCNST"),
};

static const struct setloc_tag BANK_23_TAGS[] = {
    TAG("POWFLITE"), TAG("POWFLIT1"), TAG("INFLIGHT"), TAG("APOPERI"),  TAG("R61"),
    TAG("R62"),      TAG("INTPRET1"), TAG("MEASINC"),  TAG("MEASINC1"), TAG("EXTVB1"),
    TAG("P12A"),     TAG("NORMLIZ"),  TAG("ASENT7"),
};

static const struct setloc_tag BANK_24_TAGS[] = {
    TAG("PLANTIN"),
    TAG("P20S"),
};

static const struct setloc_tag BANK_25_TAGS[] = {
    TAG("P20S1"), TAG("P20S2"), TAG("RADARUPT"), TAG("RRLEADIN"), TAG("R29S1"), TAG("PLANTIN3"),
};

static const struct setloc_tag BANK_26_TAGS[] = {
    /* LM p. 33 */
    TAG("P20S3"),    TAG("BAWLANGS"), TAG("MANUVER"),
    TAG("MANUVER1"), TAG("PLANTIN1"), TAG("PLANTIN2"),
};

static const struct setloc_tag BANK_27_TAGS[] = {
    TAG("TOF-FF"), TAG("TOF-FF1"), TAG("P40S1"), TAG("VECPT"), TAG("ASENT1"), TAG("SERV3"),
};

static const struct setloc_tag BANK_30_TAGS[] = {
    TAG("LOWSUPER"), TAG("P12"),   TAG("ASENT"),  TAG("FCDUW"),
    TAG("FLOGSUB"),  TAG("VB67A"), TAG("ASENT5"),
};

static const struct setloc_tag BANK_31_TAGS[] = {
    TAG("FTHROT"),
    TAG("F2DPS*31"),
    TAG("VB67"),
};

static const struct setloc_tag BANK_32_TAGS[] = {
    TAG("P20S4"),  TAG("F2DPS*32"), TAG("ABORTS"), TAG("LRS22"),
    TAG("P66LOC"), TAG("R47"),      TAG("SERV"),
};

static const struct setloc_tag BANK_33_TAGS[] = {
    TAG("SERVICES"),
    TAG("R29/SERV"),
    TAG("ASENT6"),
};

static const struct setloc_tag BANK_34_TAGS[] = {
    TAG("STBLEORB"), TAG("P30S1"), TAG("CSI/CDH1"), TAG("ASCFILT"), TAG("R12STUFF"), TAG("SERV4"),
};

static const struct setloc_tag BANK_35_TAGS[] = {
    TAG("CSI/CDH"),
    TAG("P30S"),
    TAG("GLM"),
    TAG("P40S2"),
};

static const struct setloc_tag BANK_36_TAGS[] = {
    TAG("P40S"),
};

static const struct setloc_tag BANK_37_TAGS[] = {
    TAG("P05P06"), TAG("IMU2"), TAG("IMU4"), TAG("R31"), TAG("IMUSUPER"), TAG("SERV1"),
};

static const struct setloc_tag BANK_40_TAGS[] = {
    TAG("PINBALL1"),
    TAG("SELFSUPR"),
    TAG("PINSUPER"),
    TAG("R31LOC"),
};

static const struct setloc_tag BANK_41_TAGS[] = {
    TAG("PINBALL2"),
};

static const struct setloc_tag BANK_42_TAGS[] = {
    TAG("SBAND"),
    TAG("PINBALL3"),
};

static const struct setloc_tag BANK_43_TAGS[] = {
    TAG("EXTVERBS"),
    TAG("SELFCHEC"),
};

const struct setloc_bank SETLOC_BANKS[] = {
    {"MODULE 1 CONTAINS BANKS 0 THROUGH 5", FIXED_FIXED, 02, BLOCK_02_TAGS, COUNT(BLOCK_02_TAGS)},
    {NULL, FIXED_FIXED, 03, BLOCK_03_TAGS, COUNT(BLOCK_03_TAGS)},
    {NULL, SWITCHED, 00, BANK_00_TAGS, COUNT(BANK_00_TAGS)},
    {NULL, SWITCHED, 01, BANK_01_TAGS, COUNT(BANK_01_TAGS)},
    {NULL, SWITCHED, 04, BANK_04_TAGS, COUNT(BANK_04_TAGS)},
    {NULL, SWITCHED, 05, BANK_05_TAGS, COUNT(BANK_05_TAGS)},
    {"MODULE 2 CONTAINS BANKS 6 THROUGH 13", SWITCHED, 06, BANK_06_TAGS, COUNT(BANK_06_TAGS)},
    {NULL, SWITCHED, 07, BANK_07_TAGS, COUNT(BANK_07_TAGS)},
    {NULL, SWITCHED, 010, BANK_10_TAGS, COUNT(BANK_10_TAGS)},
    {NULL, SWITCHED, 011, BANK_11_TAGS, COUNT(BANK_11_TAGS)},
    {NULL, SWITCHED, 012, BANK_12_TAGS, COUNT(BANK_12_TAGS)},
    {NULL, SWITCHED, 013, BANK_13_TAGS, COUNT(BANK_13_TAGS)},
    {"MODULE 3 CONTAINS BANKS 14 THROUGH 21", SWITCHED, 014, BANK_14_TAGS, COUNT(BANK_14_TAGS)},
    {NULL, SWITCHED, 015, BANK_15_TAGS, COUNT(BANK_15_TAGS)},
    {NULL, SWITCHED, 016, BANK_16_TAGS, COUNT(BANK_16_TAGS)},
    {NULL, SWITCHED, 017, BANK_17_TAGS, COUNT(BANK_17_TAGS)},
    {NULL, SWITCHED, 020, BANK_20_TAGS, COUNT(BANK_20_TAGS)},
    {NULL, SWITCHED, 021, BANK_21_TAGS, COUNT(BANK_21_TAGS)},
    {"MODULE 4 CONTAINS BANKS 22 THROUGH 27", SWITCHED, 022, BANK_22_TAGS, COUNT(BANK_22_TAGS)},
    {NULL, SWITCHED, 023, BANK_23_TAGS, COUNT(BANK_23_TAGS)},
    {NULL, SWITCHED, 024, BANK_24_TAGS, COUNT(BANK_24_TAGS)},
    {NULL, SWITCHED, 025, BANK_25_TAGS, COUNT(BANK_25_TAGS)},
    {NULL, SWITCHED, 026, BANK_26_TAGS, COUNT(BANK_26_TAGS)},
    {NULL, SWITCHED, 027, BANK_27_TAGS, COUNT(BANK_27_TAGS)},
    {"MODULE 5 CONTAINS BANKS 30 THROUGH 35", SWITCHED, 030, BANK_30_TAGS, COUNT(BANK_30_TAGS)},
    {NULL, SWITCHED, 031, BANK_31_TAGS, COUNT(BANK_31_TAGS)},
    {NULL, SWITCHED, 032, BANK_32_TAGS, COUNT(BANK_32_TAGS)},
    {NULL, SWITCHED, 033, BANK_33_TAGS, COUNT(BANK_33_TAGS)},
    {NULL, SWITCHED, 034, BANK_34_TAGS, COUNT(BANK_34_TAGS)},
    {NULL, SWITCHED, 035, BANK_35_TAGS, COUNT(BANK_35_TAGS)},
    {"MODULE 6 CONTAINS BANKS 36 THROUGH 43", SWITCHED, 036, BANK_36_TAGS, COUNT(BANK_36_TAGS)},
    {NULL, SWITCHED, 037, BANK_37_TAGS, COUNT(BANK_37_TAGS)},
    {NULL, SWITCHED, 040, BANK_40_TAGS, COUNT(BANK_40_TAGS)},
    {NULL, SWITCHED, 041, BANK_41_TAGS, COUNT(BANK_41_TAGS)},
    {NULL, SWITCHED, 042, BANK_42_TAGS, COUNT(BANK_42_TAGS)},
    {NULL, SWITCHED, 043, BANK_43_TAGS, COUNT(BANK_43_TAGS)},
};
const unsigned SETLOC_BANK_COUNT = COUNT(SETLOC_BANKS);

const struct setloc_equate SETLOC_EQUATES[] = {
    EQUALS_AND_REMARK("HI6ZEROS", "ZEROVECS", "ZERO VECTOR ALWAYS IN HIGH MEMORY"),
    EQUALS_AND_REMARK("LO6ZEROS", "ZEROVEC", "ZERO VECTOR ALWAYS IN LOW MEMORY"),
    EQUALS("HIDPHALF", "UNITX"),
    EQUALS("LODPHALF", "XUNIT"),
    EQUALS("HIDP1/4", "DP1/4TH"),
    EQUALS_AND_REMARK("LODP1/4", "D1/4", "2DEC .25"),
    EQUALS("HIUNITX", "UNITX"),
    EQUALS("HIUNITY", "UNITY"),
    EQUALS("HIUNITZ", "UNITZ"),
    EQUALS_AND_REMARK("LOUNITX", "XUNIT", "2DEC .5"),
    EQUALS_AND_REMARK("LOUNITY", "YUNIT", "2DEC 0"),
    EQUALS_AND_REMARK("LOUNITZ", "ZUNIT", "2DEC 0"),
    EQUALS_AND_REMARK("DELRSPL", "SPLRET", "COL PGM, ALSO CALLED BY R30 IN LUMINARY."),
    REMARK("ROPE-SPECIFIC ASSIGNS OBVIATING NEED TO CHECK COMPUTER FLAG IN DETERMINING INTEGRATION "
           "AREA ENTRIES."),
    EQUALS("ATOPTHIS", "ATOPLEM"),
    EQUALS("ATOPOTH", "ATOPCSM"),
    EQUALS("OTHPREC", "CSMPREC"),
    EQUALS("MOONTHIS", "LMOONFLG"),
    EQUALS("MOONOTH", "CMOONFLG"),
    EQUALS("MOVATHIS", "MOVEALEM"),
    IS("RMM", "LODPMAX"),
    IS("RME", "LODPMAX1"),
    EQUALS("THISPREC", "LEMPREC"),
    IS("THISAXIS", "UNITZ"),
    EQUALS_AND_REMARK("NB1NB2", "THISAXIS", "FOR R31"),
    EQUALS_AND_REMARK("ERASID", "BITS2-10", "DOWNLINK ERASABLE DUMP ID"),
    EQUALS("DELAYNUM", "TWO"),
};
const unsigned SETLOC_EQUATE_COUNT = COUNT(SETLOC_EQUATES);

#else

const char *const TAGS_HEADING[] = {
    "TAGS FOR RELATIVE SETLOC AND BLANK BANK CARDS",
    "FIXED MEMORY 120000 - 167777",
    "COUNT BANKSUM",
};
const unsigned TAGS_HEADING_COUNT = COUNT(TAGS_HEADING);

static const struct setloc_tag BLOCK_02_TAGS[] = {
    TAG("FFTAG1"), TAG("FFTAG2"),  TAG("FFTAG3"),  TAG("FFTAG4"),  TAG("FFTAG7"),  TAG("FFTAG8"),
    TAG("FFTAG9"), TAG("FFTAG10"), TAG("FFTAG12"), TAG("P30SUBS"), TAG("STOPRAT"), TAG("P23S"),
};

static const struct setloc_tag BLOCK_03_TAGS[] = {
    TAG("FFTAG5"),
    TAG("FFTAG6"),
    TAG("DAPS9"),
    TAG("FFTAG13"),
};

static const struct setloc_tag BANK_00_TAGS[] = {
    TAG("DLAYJOB"),
};

static const struct setloc_tag BANK_01_TAGS[] = {
    TAG("RESTART"),
};

static const struct setloc_tag BANK_04_TAGS[] = {
    TAG("VERB37"),
    TAG("CONICS1"),
    TAG("PINBALL4"),
    TAG("CSI/CDH1"),
    TAG("INTPRET2"),
    TAG("IMUCAL1"),
    /* CM p. 28 */
    TAG("STBLEORB"),
    TAG("E/PROG"),
    TAG("MIDDGIM"),
};

static const struct setloc_tag BANK_05_TAGS[] = {
    TAG("FRANDRES"),
    TAG("DOWNTELM"),
    TAG("DAPMASS"),
    TAG("CDHTAG"),
};

static const struct setloc_tag BANK_06_TAGS[] = {
    TAG("IMUCOMP"),
    TAG("T4RUP"),
    TAG("IMUCAL2"),
    TAG("CSIPROG"),
};

static const struct setloc_tag BANK_07_TAGS[] = {
    TAG("SXTMARKE"), TAG("R02"), TAG("MODESW"), TAG("XANG"), TAG("KEYRUPT"), TAG("CSIPROG6"),
};

static const struct setloc_tag BANK_10_TAGS[] = {
    TAG("DISPLAYS"), TAG("PHASETAB"), TAG("COMGEOM2"), TAG("SXTMARK1"),
    TAG("P60S4"),    TAG("OPTDRV"),   TAG("CSIPROG8"),
};

static const struct setloc_tag BANK_11_TAGS[] = {
    TAG("ORBITAL"),
    TAG_AND_REMARK("ORBITAL1", "CONSTANTS"),
    /* CM p. 29 */
    TAG("INTVEL"),
    TAG("S52/2"),
    TAG("CSIPROG5"),
    TAG("INTINIT1"),
};

static const struct setloc_tag BANK_12_TAGS[] = {
    TAG("CONICS"),
    TAG("CSIPROG2"),
    TAG("CSI/CDH2"),
    TAG("MODCHG2"),
};

static const struct setloc_tag BANK_13_TAGS[] = {
    TAG("P76LOC"),   TAG("LATLONG"), TAG("INTINIT"), TAG("SR52/1"),
    TAG("ORBITAL2"), TAG("CDHTAGS"), TAG("E/PROG1"), TAG("MODCHG3"),
};

static const struct setloc_tag BANK_14_TAGS[] = {
    TAG("STARTAB"), TAG("RT53"), TAG("P50S1"), TAG("MEASINC2"), TAG("CSI/CDH3"),
};

static const struct setloc_tag BANK_15_TAGS[] = {
    TAG("P50S"),
    TAG("ETRYDAP"),
    TAG("S52/3"),
};

static const struct setloc_tag BANK_16_TAGS[] = {
    TAG("P40S1"),
    /* CM p. 30 */
    TAG("DAPROLL"),
    TAG("P50S2"),
    TAG("P23S1"),
    TAG("RTE2"),
};

static const struct setloc_tag BANK_17_TAGS[] = {
    TAG("DAPS4"),
    TAG("DAPS5"),
    TAG("DAPS7"),
    TAG("P50S3"),
};

static const struct setloc_tag BANK_20_TAGS[] = {
    TAG("DAPS6"), TAG("DAPS1"), TAG("DAPS2"), TAG("MANUSTUF"), TAG("R36CM"), TAG("VAC5LOC"),
};

static const struct setloc_tag BANK_21_TAGS[] = {
    TAG("DAPS3"),
    TAG("MYSUBS"),
    TAG("KALCMON3"),
};

static const struct setloc_tag BANK_22_TAGS[] = {
    TAG("RTBCODES"), TAG("RTBCODE1"), TAG("DAPS8"),    TAG("APOPERI"),
    TAG("P40S5"),    TAG("KALCMON2"), TAG("KALCMON1"), TAG("CSIPROG3"),
};

static const struct setloc_tag BANK_23_TAGS[] = {
    TAG("P20S2"),    TAG("INFLIGHT"), TAG("COMGEOM1"), TAG("POWFLITE"), TAG("POWFLIT1"),
    TAG("RENDGUID"), TAG("POWFLIT2"), TAG("R30LOC"),   TAG("P11FOUR"),  TAG("CSIPROG4"),
};

static const struct setloc_tag BANK_24_TAGS[] = {
    TAG("LOADDAP"),
    TAG("P40S"),
    TAG("CSIPROG7"),
};

static const struct setloc_tag BANK_25_TAGS[] = {
    TAG("REENTRY"),
    TAG("CDHTAG1"),
};

static const struct setloc_tag BANK_26_TAGS[] = {
    TAG("INTPRET1"),
    TAG("REENTRY1"),
    TAG("P60S"),
    TAG("P60S1"),
    TAG("P60S2"),
    TAG("P60S3"),
    TAG_AND_REMARK("PLANTIN", "LUNAR ROT"),
    TAG("EPHEM"),
    TAG("P05P06"),
    TAG("26P50S"),
};

static const struct setloc_tag BANK_27_TAGS[] = {
    TAG("TOF-FF"),
    TAG("TOF-FF1"),
    TAG("MANUVER"),
    TAG("MANUVER1"),
    /* CM p. 32 */
    TAG("VECPT"),
    TAG("UPDATE1"),
    TAG("UPDATE2"),
    TAG("R22S1"),
    TAG("P60S5"),
    TAG("P40S2"),
};

static const struct setloc_tag BANK_30_TAGS[] = {
    TAG("IMUSUPER"),
    TAG("LOWSUPER"),
    TAG_AND_REMARK("FCSTART", "STANDARD LOCATION FOR THIS. (FOR EXTV8)"),
    TAG("LOPC"),
    TAG("P20S1"),
    TAG("P20S6"),
    TAG("P40S3"),
    TAG("R35A"),
};

static const struct setloc_tag BANK_31_TAGS[] = {
    TAG("R35"),      TAG("RT23"), TAG("P30S1A"), TAG("R34"),  TAG("CDHTAG2"),
    TAG("CSIPROG9"), TAG("R31"),  TAG("P22S"),   TAG("RTE3"),
};

static const struct setloc_tag BANK_32_TAGS[] = {
    TAG("MSGSCAN1"),
    TAG("RTE"),
    TAG("DELRSPL1"),
    TAG("IMUCAL3"),
};

static const struct setloc_tag BANK_33_TAGS[] = {
    TAG("TESTLEAD"),
    /* CM p. 33 */
    TAG("IMUCAL"),
};

static const struct setloc_tag BANK_34_TAGS[] = {
    TAG("P11ONE"),
    TAG("P20S3"),
    TAG("P20S4"),
    TAG("RTECON"),
};

static const struct setloc_tag BANK_35_TAGS[] = {
    TAG("RTECON1"), TAG("CSI/CDH"),  TAG("P30S1"),    TAG("P30S"),
    TAG("P17S1"),   TAG("MEASINC3"), TAG("INTINIT2"),
};

static const struct setloc_tag BANK_36_TAGS[] = {
    TAG("MEASINC"), TAG("MEASINC1"), TAG("P17S"), TAG("RTE1"), TAG("S3435LOC"),
};

static const struct setloc_tag BANK_37_TAGS[] = {
    TAG("P20S"), TAG("BODYATT"), TAG("RENDEZ"), TAG("SERVICES"), TAG("P11TWO"), TAG("CDHTAG3"),
};

static const struct setloc_tag BANK_40_TAGS[] = {
    TAG("PINSUPER"),
    /* CM p. 34 */
    TAG("SELFSUPR"),
    TAG("PINBALL1"),
    TAG("R36CM1"),
};

static const struct setloc_tag BANK_41_TAGS[] = {
    TAG("PINBALL2"),
    TAG("R36LM"),
};

static const struct setloc_tag BANK_42_TAGS[] = {
    TAG("SBAND"),
    TAG("PINBALL3"),
    TAG("EXTVBS"),
    TAG("R36LM1"),
};

static const struct setloc_tag BANK_43_TAGS[] = {
    TAG("SELFCHEC"),
    TAG("EXTVERBS"),
};

const struct setloc_bank SETLOC_BANKS[] = {
    {"MODULE 1 CONTAINS BANKS 0 THROUGH 5", FIXED_FIXED, 02, BLOCK_02_TAGS, COUNT(BLOCK_02_TAGS)},
    {NULL, FIXED_FIXED, 03, BLOCK_03_TAGS, COUNT(BLOCK_03_TAGS)},
    {NULL, SWITCHED, 00, BANK_00_TAGS, COUNT(BANK_00_TAGS)},
    {NULL, SWITCHED, 01, BANK_01_TAGS, COUNT(BANK_01_TAGS)},
    {NULL, SWITCHED, 04, BANK_04_TAGS, COUNT(BANK_04_TAGS)},
    {NULL, SWITCHED, 05, BANK_05_TAGS, COUNT(BANK_05_TAGS)},
    {"MODULE 2 CONTAINS BANKS 6 THROUGH 13", SWITCHED, 06, BANK_06_TAGS, COUNT(BANK_06_TAGS)},
    {NULL, SWITCHED, 07, BANK_07_TAGS, COUNT(BANK_07_TAGS)},
    {NULL, SWITCHED, 010, BANK_10_TAGS, COUNT(BANK_10_TAGS)},
    {NULL, SWITCHED, 011, BANK_11_TAGS, COUNT(BANK_11_TAGS)},
    {NULL, SWITCHED, 012, BANK_12_TAGS, COUNT(BANK_12_TAGS)},
    {NULL, SWITCHED, 013, BANK_13_TAGS, COUNT(BANK_13_TAGS)},
    {"SPACER\nMODULE 3 CONTAINS BANKS 14 THROUGH 21", SWITCHED, 014, BANK_14_TAGS,
     COUNT(BANK_14_TAGS)},
    {NULL, SWITCHED, 015, BANK_15_TAGS, COUNT(BANK_15_TAGS)},
    {NULL, SWITCHED, 016, BANK_16_TAGS, COUNT(BANK_16_TAGS)},
    {NULL, SWITCHED, 017, BANK_17_TAGS, COUNT(BANK_17_TAGS)},
    {NULL, SWITCHED, 020, BANK_20_TAGS, COUNT(BANK_20_TAGS)},
    {NULL, SWITCHED, 021, BANK_21_TAGS, COUNT(BANK_21_TAGS)},
    {"MODULE 4 CONTAINS BANKS 22 THROUGH 27", SWITCHED, 022, BANK_22_TAGS, COUNT(BANK_22_TAGS)},
    {NULL, SWITCHED, 023, BANK_23_TAGS, COUNT(BANK_23_TAGS)},
    {NULL, SWITCHED, 024, BANK_24_TAGS, COUNT(BANK_24_TAGS)},
    {NULL, SWITCHED, 025, BANK_25_TAGS, COUNT(BANK_25_TAGS)},
    {NULL, SWITCHED, 026, BANK_26_TAGS, COUNT(BANK_26_TAGS)},
    {NULL, SWITCHED, 027, BANK_27_TAGS, COUNT(BANK_27_TAGS)},
    {"MODULE 5 CONTAINS BANKS 30 THROUGH 35", SWITCHED, 030, BANK_30_TAGS, COUNT(BANK_30_TAGS)},
    {NULL, SWITCHED, 031, BANK_31_TAGS, COUNT(BANK_31_TAGS)},
    {NULL, SWITCHED, 032, BANK_32_TAGS, COUNT(BANK_32_TAGS)},
    {NULL, SWITCHED, 033, BANK_33_TAGS, COUNT(BANK_33_TAGS)},
    {NULL, SWITCHED, 034, BANK_34_TAGS, COUNT(BANK_34_TAGS)},
    {NULL, SWITCHED, 035, BANK_35_TAGS, COUNT(BANK_35_TAGS)},
    {"MODULE 6 CONTAINS BANKS 36 THROUGH 43", SWITCHED, 036, BANK_36_TAGS, COUNT(BANK_36_TAGS)},
    {NULL, SWITCHED, 037, BANK_37_TAGS, COUNT(BANK_37_TAGS)},
    {NULL, SWITCHED, 040, BANK_40_TAGS, COUNT(BANK_40_TAGS)},
    {NULL, SWITCHED, 041, BANK_41_TAGS, COUNT(BANK_41_TAGS)},
    {NULL, SWITCHED, 042, BANK_42_TAGS, COUNT(BANK_42_TAGS)},
    {NULL, SWITCHED, 043, BANK_43_TAGS, COUNT(BANK_43_TAGS)},
};
const unsigned SETLOC_BANK_COUNT = COUNT(SETLOC_BANKS);

const struct setloc_equate SETLOC_EQUATES[] = {
    EQUALS_AND_REMARK("HI6ZEROS", "ZEROVECS", "ZERO VECTOR ALWAYS IN HIGH MEMORY"),
    EQUALS_AND_REMARK("LO6ZEROS", "ZEROVEC", "ZERO VECTOR ALWAYS IN LOW MEMORY"),
    EQUALS("HIDPHALF", "UNITX"),
    EQUALS("LODPHALF", "XUNIT"),
    EQUALS("HIDP1/4", "DP1/4TH"),
    EQUALS_AND_REMARK("LODP1/4", "D1/4", "2DEC .25"),
    EQUALS("HIUNITX", "UNITX"),
    EQUALS("HIUNITY", "UNITY"),
    EQUALS("HIUNITZ", "UNITZ"),
    EQUALS_AND_REMARK("LOUNITX", "XUNIT", "2DEC .5"),
    EQUALS_AND_REMARK("LOUNITY", "YUNIT", "2DEC 0"),
    EQUALS_AND_REMARK("LOUNITZ", "ZUNIT", "2DEC 0"),
    EQUALS_AND_REMARK("3/4LOWDP", "3/4", "2DEC 3.0 B-2"),
    CARD("SBANK=", "LOWSUPER"),
    REMARK("ROPE SPECIFIC ASSIGNS OBVIATING NEED TO CHECK COMPUTER FLAG IN DETVRUZVING INTEGRATION "
           "AREA ENTRIES"),
    EQUALS("OTHPREC", "LEMPREC"),
    EQUALS("ATOPOTH", "ATOPLEM"),
    EQUALS("ATOPTHIS", "ATOPCSM"),
    EQUALS("MOONTHIS", "CMOONFLG"),
    /* CM p. 35 */
    EQUALS("MOONOTH", "LMOONFLG"),
    EQUALS("MOVATHIS", "MOVEACSM"),
    EQUALS_AND_REMARK("STATEST", "V83CALL", "* TEMPORARY"),
    EQUALS("THISPREC", "CSMPREC"),
    IS("THISAXIS", "UNITX"),
    EQUALS_AND_REMARK("ERASID", "LOW10", "DOWNLINK ERASABLE DUMP ID"),
    EQUALS("DELAYNUM", "THREE"),
};
const unsigned SETLOC_EQUATE_COUNT = COUNT(SETLOC_EQUATES);

/* The comment before the ECADRs. */
const char EBANK_ECADRS_REMARK[] =
    "THE FOLLOWING ECADRS ARE DEFINED TO FACILITATE EBANK SWITCHING. THEY ALSO MAKE IT EASIER FOR "
    "ERASABLE CONTROL TO REARRANGE ERASABLE MEMORY WITHOUT DISRUPTING THE PROGRAMS WHICH SET "
    "EBANKS. PRIOR TO ROPE RELEASE FIXED MEMORY CAN BE SAVED BY SETTING EACH EBXXXX =EBANKX "
    "(X=4,5,6,7).EBANKX OF COURSE WILL BE THE BANK WHERE THE ERASABLES REFERENCED IN EBXXXX WILL "
    "BE STORED.";

/* The ECADRs: the only words of the rope this file assembles, each bank's
 * first words (the tags above took none). */
const struct ebank_ecadr EBANK_ECADRS[] = {
    {07, "EBMARKDO", K_EBMARKDO, "MARKDOWN", E_MARKDOWN},
    {07, "EBMRKBUF", K_EBMRKBUF, "MRKBUF1", E_MRKBUF1},
    {024, "EBDVCNTR", K_EBDVCNTR, "DVCNTR", E_DVCNTR},
    {024, "EBP40TMP", K_EBP40TMP, "P40TMP", E_P40TMP},
    {034, "EBDVCNT", K_EBDVCNT, "DVCNTR", E_DVCNTR},
    {034, "EBQPLACE", K_EBQPLACE, "QPLACES", E_QPLACES},
    {037, "EBRN1", K_EBRN1, "RN1", E_RN1},
};
const unsigned EBANK_ECADR_COUNT = COUNT(EBANK_ECADRS);

/* The listing's last line (CM p. 36). */
const char TAGS_END[] = "*** END OF MAIN PROGRAM ***";

#endif
