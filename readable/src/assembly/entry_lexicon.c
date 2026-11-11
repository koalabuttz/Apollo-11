/* SPDX-License-Identifier: MIT */
/* ENTRY_LEXICON.agc, the CM's (Comanche055 pp. 837-843): the entry
 * guidance's glossary, all of it comment. It stands in the listing just
 * before the guidance (REENTRY_CONTROL, src/reentry/) and gives each name
 * of the guidance's flow charts with what it is, its scaling and the
 * program's own name for it where the two differ; then the erasables the
 * flow charts do not show, the switches, the constants and gains, the
 * conversion factors, the quantities the crew is shown and the body's
 * attitude quantities.
 *
 * Each list is a table that keeps the listing's lines and columns, in the
 * same form as the lists of ASSEMBLY_AND_OPERATION_INFORMATION
 * (assembly.h, struct information_table): a row is a line of the listing,
 * and a cell is what the line has in one of the list's columns (NULL, or
 * left off the end of the row, where it has nothing). An entry that takes
 * two lines takes two rows, the second with nothing in cell 0. The words
 * are the listing's, with its spelling. ENTRY_LEXICON, at the end, names
 * the tables in the listing's order.
 *
 * No routine is here and no flight code reads these tables: they are the
 * listing's page for a reader, kept word for word and compared with the
 * listing by make assembly-cases. */
#include <stddef.h>
#include "assembly.h"

#if COMANCHE055

/* The guidance's variables. Cell 0: the name the flow charts use (a line with
 * only _ is the bar over the vector's name under it); cell 1: what it is;
 * cell 2: its maximum value, which is its scaling; cell 3: the program's
 * name for it (COMPUTER NAME), or a note. */
static const char *const VARIABLES[][4] = {
    /* CM p. 837 */
    {"VARIABLE", "DESCRIPTION", "MAXIMUM VALUE *", "COMPUTER NAME"},
    {"--------", "-----------", "------- -------", "-------- ----"},
    {"_"},
    {"URT0", "INITIAL TARGET VECTOR", "2 (UNIT VECTOR)", "= RTINIT"},
    {"_"},
    {"UZ", "UNIT VECTOR NORTH", "1", "= UNITW"},
    {"_"},
    {"V", "VELOCITY VECTOR", "2 VSAT", "= VEL"},
    {"_"},
    {"R", "POSITION VECTOR", "2 EXP 29 METERS", "= RN"},
    {"_"},
    {"VI", "INERTIAL VELOCITY", "128 M/CENTISEC", "= VN"},
    {"_"},
    {"RTE", "VECTOR EAST AT INITIAL TARGET", "2", "= RTEAST"},
    {"_"},
    {"UTR", "NORMAL TO RTE AND UZ", "2", "= RTNORM"},
    {"_"},
    {"URT", "TARGET VECTOR", "2", "= RT"},
    {"_"},
    {"UNI", "UNIT NORMAL TO TRAJECTORY PLANE", "2"},
    {"_"},
    {"DELV", "INTEGRATED ACCEL. FROM PIPAS", "5.85 16384 CM/S"},
    {"_"},
    {"G", "GRAVITY VECTOR", "128 M/CENTISEC", "= GDT/2"},
    {"A0", "INITIAL DRAG FOR UPCONTRL", "805 FPSS", "FPSS=FT/SEC/SEC"},
    {"AHOOKDV", "TERM IN GAMMAL CALC. = AHOOK DVL", "16"},
    {"A1", "DRAG VALUE IN FACTOR CALCULATION", "805 FPSS"},
    {"ALP", "CONST FOR UPCONTRL", "1"},
    {"ASKEP", "KEPLER RANGE", "21600 NM", "NM = NAUTICAL MILE"},
    {"ASP1", "FINAL PHASE RANGE", "21600 NM"},
    {"ASPUP", "UP-RANGE", "21600 NM"},
    {"ASP3", "GAMMA CORRECTION", "21600 NM"},
    {"ASPDWN", "RANGE DOWN TO PULL-UP", "21600 NM"},
    {"ASP", "PREDICTED RANGE", "21600 NM", "NOT STORED"},
    {"COSG", "COSINE(GAMMAL)", "2", "= COSG/2"},
    {"C/D0", "RECIPROCAL DRAG, -4/D0 B-8", "64/FPSS"},
    {"D", "TOTAL ACCELERATION", "805 FPSS"},
    {"D0", "CONTROLLED CONSTANT D", "805 FPSS"},
    {"DHOOK", "TERM IN GAMMAL COMPUTATION", "805 FPSS"},
    {"DIFF", "THETNM-ASP (RANGE DIFFERENCE)", "21600 NM"},
    {"DIFFOLD", "PREVIOUS VALUE OF DIFF", "21600 NM"},
    {"DLEWD", "CHANGE IN LEWD", "1"},
    {"DR", "REFERENCE DRAG FOR DOWNCONTROL", "805 FPSS", "NOT STORED"},
    {"DREFR", "REFERENCE DRAG", "805 FPSS", "NOT STORED"},
    {"DVL", "VS1-VL", "2 VSAT"},
    {"E", "ECCENTRICITY", "4", "NOT STORED"},
    {"F1", "DRANGE/D DRAG (FINAL PHASE)", "2700/805", "= FX +5"},
    {"F2", "DRANGE/D RDOT (FINAL PHASE)", "2700/2VS NM/FPS", "= FX +4"},
    /* CM p. 838 */
    {"F3", "DRANGE/D (L/D)", "2700 NM", "= FX"},
    {"FACT1", "CONST FOR UPCONTRL", "805 FPSS"},
    {"FACT2", "CONST FOR UPCONTRL", "1/805 FPSS"},
    {"FACTOR", "USED IN UPCONTRL", "1", "* MAXIMUM VALUE DENOTES UNSCALED"},
    {"GAMMAL", "FLIGHT PATH ANGLE AT VL", "1 RADIAN", "VARIABLE VALUE WHEN SCALED"},
    {"GAMMAL1", "SIMPLE FORM OF GAMMAL", "1 RADIAN", "VARIABLE HAS MAXIMUM VALUE OF ONE."},
    /* CM p. 839 */
    {"HEADSUP", "INDICATOR FOR INITIAL ROLL", "1"},
    {"KA", "DRAG TO LIFT UP IF DOWN", "805 FPSS", "= KAT"},
    {"KLAT", "LATERAL SWITCH GAIN", "1", "(NOM = .0125)"},
    {"K2ROLL", "INDICATOR FOR ROLL SWITCH"},
    {"LAD", "MAX L/D (MIN ACTUAL VEHICLE L/D)", "1"},
    {"LADPAD", "NOMINAL VEHICLE L/D, SP PAD LOAD", "1", "(NOM = 0.3)"},
    {"LATANG", "LATERAL RANGE", "4 RADIANS"},
    {"LEQ", "EXCESS C.F. OVER GRAV=(VSQ-1)GS", "128.8 FPSS"},
    {"LEWD", "UPCONTROL REFERENCE L/D", "1"},
    {"LOD", "FINAL PHASE L/D", "1", "(NOM = 0.18)"},
    {"LODPAD", "FINAL PHASE L/D, SP PAD LOAD", "1"},
    {"L/D", "DESIRED LIFT TO DRAG RATIO", "1"},
    {NULL, "(VERTICAL PLANE)"},
    {"L/D1", "TEMP STORAGE FOR L/D IN LATERAL", "1"},
    {"L/DCMINR", "LAD COS(15DEG)", "1", "(NOM = 0.2895)"},
    {"PREDANGL", "PREDICTED RANGE (FINAL PHASE)", "2700 NM", "= PREDANG"},
    {"Q2", "FINAL PHASE RANGE -23500 Q3", "21600 NM"},
    {NULL, "Q2 = FCN(LAD)"},
    {"Q7", "MINIMUM DRAG FOR UPCONTROL", "805 FPSS"},
    {"RDOT", "ALTITUDE RATE", "2 VSAT"},
    {"RDOTREF", "REFERENCE RDOT FOR UPCONTROL", "2 VSAT"},
    {"RDTR", "REFERENCE RDOT FOR DOWNCONT", "2 VSAT", "NOT SAVED"},
    {"ROLLC", "ROLL COMMAND", "1 REVOLUTION"},
    {"RTOGO", "RANGE TO GO (FINAL PHASE)", "2700 NM", "= FX +2"},
    {"SL", "SINE OF LATITUDE", "1", "NOT SAVED"},
    {"T", "TIME", "B 28 CENTISEC", "= TIME2,TIME1"},
    {"THETA", "DESIRED RANGE (RADIANS)", "2 PI RADIANS", "= THETAH"},
    {"THETNM", "DESIRED RANGE (NM)", "21600 NM", "NON EXISTENT"},
    {"V", "VELOCITY MAGNITUDE", "2 VSAT"},
    {"V1", "INITIAL VELOCITY FOR UPCONTROL", "2 VSAT"},
    {"VL", "EXIT VELOCITY FOR UPCONTROL", "2 VSAT"},
    {"VREF", "REFERENCE VELOCITY FOR UPCONTROL", "2 VSAT"},
    {"VS1", "VSAT OR V1, WHICHEVER IS SMALLER", "2 VSAT"},
    {NULL, "2 2"},
    {"VBARS", "VL /VSAT", "4"},
    {NULL, "2", "2"},
    {"VSQ", "NORMALISED VEL. SQUARED = V /VSAT", "4", "= VSQUARE"},
    {"WT", "EARTH RATE TIMES TIME", "1 REVOLUTION", "NOT SAVED"},
    {NULL, NULL, NULL, "= WIE (DTEAROT)"},
    {"X", "INTERMEDIATE VARIABLE IN G-LIMITER", "2 VSAT", "NOT SAVED"},
    {"Y", "LATERAL MISS LIMIT", "4 RADIANS", "NOT SAVED"},
};

/* The program's erasables that the flow charts do not show, with the same
 * cells 0 to 2. */
static const char *const EXTRA_ERASABLES[][3] = {
    /* CM p. 840 */
    {"EXTRA COMPUTER ERASABLE LOCATIONS NOT SHOWN ON FLOW CHARTS"},
    {"----------------------------------------------------------"},
    {"VARIABLE", "DESCRIPTION", "MAXIMUM VALUE"},
    {"--------", "-----------", "-------------"},
    {"GOTOADDR", "ADDRESS SELECTED BY SEQUENCER"},
    {"XPIPBUF", "BUFFER TO STORE X PIPA COUNTS"},
    {"YPIPBUF", "BUFFER TO STORE Y PIPA COUNTS"},
    {"ZPIPBUF", "BUFFER TO STORE Z PIPA COUNTS"},
    {"PIPCTR", "COUNTS PASSES THRU PIPA READ ROUTINE"},
    {"JJ", "INDEX IN FINAL PHASE TABLE LOOK-UP"},
    {"MM", "INDEX IN FINAL PHASE TABLE LOOK-UP"},
    {"GRAD", "INTERPOLATION FACTOR IN FINAL PHASE"},
    {"FX", "DRANGE/D L/D = F3", "2700 NM"},
    {"FX +1", "AREF", "805 FPSS"},
    {"FX +2", "RTOGO", "2700 NM"},
    {"FX +3", "RDOTREF", "VSAT/4"},
    {"FX +4", "DRANGE/D RDOT = F2", "21600/2VS NM/FPS"},
    {"FX +5", "DRANGE/D DRAG = F1", "2700/805 NM/FPSS"},
    {"TEM1B", "TEMPORARY LOCATION"},
    {"TIME/RTO", "TIME OF INITIAL TARGET RTINIT", "B 28 CENTISEC"},
    {"DTEAROT", "EST TIME BETWEEN RTINIT AND RT", "B 28 CENTISEC"},
    {"_"},
    {"UNITV", "UNIT V VECTOR", "2"},
    {"_"},
    {"UNITR", "UNIT R VECTOR", "2"},
    {"_"},
    {"-VREL", "NEGATIVE VELOCITY REL TO ATMOSP", "2 VSAT"},
};

/* The switches. Cell 0: the switch; cell 1: what it says; cell 2: its
 * initial state; cell 3: its flag's number and its bit in CM/FLAGS. */
static const char *const SWITCHES[][4] = {
    {"COMPUTER SWITCHES", NULL, "INITIAL STATE", "CM/FLAGS = STATE +6"},
    {"-----------------", NULL, "-------------", "-------------------"},
    {"ENTRYDSP", "DO ENTRY DISPLAY, IF SET", "NON-BRANCH (1)", "92D, BIT13"},
    {"GONEPAST", "INDICATES OVERSHOOT OF TARGET", "NON-BRANCH (0)", "95D, BIT10"},
    {"RELVELSW", "RELATIVE VELOCITY SWITCH", "NON-BRANCH (0)", "96D, BIT 9"},
    {"EGSW", "FINAL PHASE SWITCH", "NON-BRANCH (0)", "97D, BIT 8"},
    {"FIRSTPAS", "INITIAL PASS THRU HUNTEST", "NON-BRANCH (0)", "98D, BIT 7"},
    {"HIND", "INDICATES ITERATION IN HUNTEST", "NON-BRANCH (0)", "99D, BIT 6"},
    {"INRLSW", "INDICATES INIT ROLL ATTITUDE SET", "NON-BRANCH (0)", "100D, BIT 5"},
    {"LATSW", "INHIBIT DOWNLIFT SWITCH IF NOT SET", "BRANCH (1)", "101D, BIT 4"},
    {".05GSW", "INDICATES DRAG EXCEEDS .05 GS", "BRANCH (0)", "102D, BIT 3"},
    {"GONEBY", "INDICATES GONE PAST TARGET (SET)", "SELF-INITIALIZING", "112D, BIT 8"},
};

/* The constants and gains. Cell 0: the name; cell 1: what it is; cell 2: its
 * value. */
static const char *const CONSTANTS_AND_GAINS[][3] = {
    /* CM p. 841 */
    {"CONSTANTS AND GAINS", NULL, "VALUE"},
    {"-------------------", NULL, "-----"},
    {"C1", "FACTOR IN ALP COMPUTATION", "1.25"},
    {"C16", "CONSTD GAIN ON DRAG", ".01"},
    {"C17", "CONSTD GAIN ON RDOT", ".001"},
    {"C18", "BIAS VEL. FOR FINAL PHASE START", "500 FPS"},
    {"C20", "MAX DRAG FOR DOWN-LIFT", "175 FPSS"},
    {"CHOOK", "FACTOR IN AHOOK COMPUTATION", ".25"},
    {"CH1", "FACTOR IN GAMMAL COMPUTATION", "1.0"},
    {"COS15", "COS( 15 DEG)", ".965"},
    {"DLEWD0", "INITIAL VARIATION IN LEWD", "-.05"},
    {"D2", "DRAG TO CHANGE LEWD", "175 FPSS"},
    {"DT", "COMPUTATION CYCLE TIME INTERVAL", "2 SEC."},
    {"GMAX", "MAXIMUM ACCELERATION", "257.6 FPSS (8 G-S)"},
    {"KA1", "FACTOR IN KA CALC", "1.3 GS"},
    {"KA2", "FACTOR IN KA CALC", ".2 GS"},
    {"KA3", "FACTOR IN D0 CALC", "90 FPSS"},
    {"KA4", "FACTOR IN D0 CALC", "40 FPSS"},
    {"KB1", "OPTIMIZED UPCONTROL GAIN", "3.4"},
    {"KB2", "OPTIMIZED UPCONTROL GAIN", ".0034"},
    {"KDMIN", "INCREMENT ON Q7 TO DETECT END OF KEPLER PHASE", ".5 FPSS"},
    {"KTETA", "TIME OF FLIGHT CONSTANT", "1000"},
    {"KLAT1", "FACTOR IN KLAT CALC", "1/24"},
    {"K44", "GAIN USED IN INITIAL ROLL SECTION", "19749550 FPS"},
    {"LATBIAS", "LATERAL SWITCH BIAS TERM", ".41252961 NM"},
    {"LEWD1", "NOMINAL UPCONTROL L/D", ".15"},
    {"POINT1", "FACTOR TO REDUCE UPCONTROL GAIN", ".1"},
    {"Q2", "FINAL PHASE RANGE - 23500 Q3", "-1002 NM"},
    {"Q3", "FINAL PHASE DRANGE/D V", ".07 NM/FPS"},
    {"Q5", "FINAL PHASE DRANGE/D GAMMA", "7050 NM/RAD"},
    {"Q6", "FINAL PHASE INITIAL FLIGHT PATH ANGLE", ".0349 RAD"},
    {"Q7F", "MIN DRAG FOR UPCONTROL", "6 FPSS"},
    {"Q7MIN", "MIN VALUE FOR Q7 IN FACTOR CALCULATION", "40 FPSS"},
    {"Q19", "FACTOR IN GAMMAL1 CALCULATION", ".5"},
    {"Q21", "FACTOR IN Q2 CALCULATION.", "1000 NM"},
    {"Q22", "FACTOR IN Q2 CALCULATION.", "-1302 NM"},
    {"VFINAL1", "VELOCITY TO START FINAL PHASE ON INITIAL ENTRY", "27000 FPS"},
    {"VFINAL", "FACTOR IN INITIAL UP-DOWN CALC", "26600 FPS"},
    {"VLMIN", "MINIMUM VL", "18000 FPS"},
    {"VMIN", "VELOCITY TO SWITCH TO RELATIVE VEL", "VSAT/2"},
    {"VRCONTRL", "RDOT TO START INTO HUNTEST", "700 FPS"},
    {NULL, "VRCONT = COMPUTER NAME"},
    {"25NM", "TOLERANCE TO STOP RANGE ITERATION", "25 NM"},
    {"VQUIT", "VELOCITY TO STOP STEERING", "1000 FPS"},
};

/* The conversion factors and scaling constants, with the same cells. */
static const char *const CONVERSION_FACTORS[][3] = {
    /* CM p. 842 */
    {"CONVERSION FACTORS AND SCALING CONSTANTS"},
    {"---------- ------- --- ------- ---------"},
    {"ATK", "ANGLE IN RAD TO NM", "3437.7468 NM/RAD"},
    {"G5", "NOMINAL G VALUE FOR SCALING", "32.2 FPSS"},
    {"H5", "ATMOSPHERE SCALE HEIGHT", "28500 FT"},
    {"J", "GRAVITY HARMONIC COEFFICIENT", ".00162346"},
    {"KWE", "EQUATORIAL EARTH RATE", "1546.70168 FPS"},
    {"MUE", "EARTH GRAVITATIONAL CONSTANT", "3.986032233 E 14 CUBIC M/ SEC SEC"},
    {"RE", "EARTH RADIUS", "21202900 FT"},
    {"REQ", "EARTH EQUATORIAL RADIUS", "20925738.2 FT"},
    {"VSAT", "SATELLITE VELOCITY AT RE", "25766.1973 FPS"},
    {"WIE", "EARTH RATE", ".0000729211505 RAD/SEC"},
    {NULL, "(END GSOP AS-278, VOL 1, FIG. 5.6-3", "CONSTANTS,GAINS, ETC.)"},
};

/* What the crew is shown. Cell 0: the name; cell 1: what it is; cell 2: its
 * maximum value; cell 3: the nouns that show it. */
static const char *const DISPLAY_QUANTITIES[][4] = {
    {"DISPLAY QUANTITIES"},
    {"------------------"},
    {"(SEE SECTION 4 OF THE GSOP FOR SIGN CONVENTIONS.)"},
    {"VARIABLE", "DESCRIPTION", "MAXIMUM VALUE"},
    {"--------", "-----------", "-------------"},
    {"GMAX", "PREDICTED MAXIMUM ENTRY ACCEL", "163.84 GS", "N 60"},
    {"VPRED", "PREDICTED VELOCITY AT ALTITUDE", "128 M/CENTISEC", "N 60"},
    {NULL, "400K FT ABOVE FISCHER RADIUS."},
    {"GAMMAEI", "PREDICTED GAMMA AT ALTITUDE", "1 REVOLUTION", "N 60"},
    {NULL, "400K FT ABOVE FISCHER RADIUS."},
    {"D", "DRAG ACCELERATION", "805 FPSS", "N 64"},
    {"VMAGI", "INERTIAL VELOCITY MAGNITUDE", "128 M/CENTISEC", "N 64, N 68"},
    {"THETAH", "DESIRED RANGE ANGLE NM", "1 REVOLUTION", "N 64, N 67"},
    {"LAT", "PRESENT LATITUDE", "1 REVOLUTION", "N 67"},
    {"LONG", "PRESENT LONGITUDE", "1 REVOLUTION", "N 67"},
    {"RTOGO", "RANGE ANGLE TO SPLASH FROM", "1 REVOLUTION", "N 63"},
    {NULL, "EMSALT FT ABOVE FISCHER RADIUS. (IN NM)"},
    {"VIO", "PREDICTED VELOCITY AT ALTITUDE", "128 M/CENTISEC", "N 63"},
    {NULL, "EMSALT FT ABOVE FISCHER RADIUS."},
    {"TTE", "TIME OF FREE FALL TO ALT", "B 28 CENTISEC", "N 63"},
    {NULL, "EMSALT FT ABOVE FISCHER RADIUS."},
    {"ROLLC", "ROLL COMMAND", "1 REVOLUTION", "N 66, N 68, N 69"},
    {"LATANG", "CROSS-RANGE ERROR (XRNGERR)", "4 RADIANS", "N 66"},
    {"DNRNGERR", "DOWN RANGE ERROR", "1 REVOLUTION", "N 66"},
    {NULL, "(PREDANG - THETAH IN NM)"},
    {"HDOT", "ALTITUDE RATE", "128 M/CENTISEC", "N 68"},
    {"Q7", "MINIMUM DRAG FOR UPCONTROL", "805 FPSS", "N 69"},
    {"VL", "EXIT VELOCITY FOR UP-CONTROL", "2 VSAT", "N 69"},
};

/* The quantities of the body's attitude, with the same cells 0 to 2. */
static const char *const BODY_ATTITUDE[][3] = {
    /* CM p. 843 */
    {"BODY ATTITUDE QUANTITIES (CM/POSE)"},
    {"----------------------------------"},
    {"VARIABLE", "DESCRIPTION", "MAXIMUM VALUE"},
    {"--------", "----------", "------- -----"},
    {"_"},
    {"-VREL", "NEGATIVE VELOCITY REL TO ATMOS.", "2 VSAT"},
    {"_"},
    {"OLDUYA", "USED FOR UYA BELOW 1000 FPS", "2"},
    {"_"},
    {"UXA/2", "UNIT VECTOR TRIAD", "2"},
    {"_"},
    {"UYA/2", "BASED ON", "2"},
    {"_"},
    {"UZA/2", "THE TRAJECTORY.", "2"},
    {"_"},
    {"UBX/2", "UNIT VECTOR", "2"},
    {"_"},
    {"UBY/2", "BODY TRIAD", "2"},
    {"_"},
    {"UBZ/2", "FOR CM.", "2"},
};

const struct information_table ENTRY_LEXICON[] = {
    {"VARIABLES", 4, COUNT(VARIABLES), &VARIABLES[0][0]},
    {"EXTRA_ERASABLES", 3, COUNT(EXTRA_ERASABLES), &EXTRA_ERASABLES[0][0]},
    {"SWITCHES", 4, COUNT(SWITCHES), &SWITCHES[0][0]},
    {"CONSTANTS_AND_GAINS", 3, COUNT(CONSTANTS_AND_GAINS), &CONSTANTS_AND_GAINS[0][0]},
    {"CONVERSION_FACTORS", 3, COUNT(CONVERSION_FACTORS), &CONVERSION_FACTORS[0][0]},
    {"DISPLAY_QUANTITIES", 4, COUNT(DISPLAY_QUANTITIES), &DISPLAY_QUANTITIES[0][0]},
    {"BODY_ATTITUDE", 3, COUNT(BODY_ATTITUDE), &BODY_ATTITUDE[0][0]},
};
const unsigned ENTRY_LEXICON_COUNT = COUNT(ENTRY_LEXICON);

#endif /* COMANCHE055 */
