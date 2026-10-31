/* SPDX-License-Identifier: MIT */
/* PINBALL_NOUN_TABLES.agc: the tables -- LM pp. 305-319, CM pp. 271-284.
 * The faithful port reads these words from its fixed data
 * (c64-port/src/fixed_data.c, fixed_data_lm.inc and fixed_data_cm.inc).
 *
 * Each table as the listing writes it, word by word, with the listing's
 * comments (where the listing carries a comment on over more lines, so do
 * the lines after the word): OCT(n) is OCT n, and ECADR(name) is ECADR name, the word's
 * E-bank bits and address in the bank (E_<name>, from the assembled
 * listing). A 2DEC, a double precision constant the assembler converted, is
 * written as the two words it assembled to, with the listing's notation in
 * the comment. make pinball-noun-tables-cases checks every word against
 * the rope.
 *
 * The two missions' tables differ in nearly every noun, so each has its
 * own, in the #if LUMINARY099 and #else halves. */
#include "pinball_noun_tables.h"
#include "../agc/names.h"

#define OCT(n)      ((word)0##n)
#define ECADR(name) ((word)E_##name)

#if LUMINARY099

/* NNADTAB -- LM pp. 305-308: each noun's address: for a normal noun (00-39) the ECADR of its
 * word (or a code: 40000 machine address to be specified, 77776 channel,
 * 77777 increment address, 0 spare); for a mixed noun (40-99) bits 15-11
 * the component codes and bits 10-1 its first entry in IDADDTAB. */
/* clang-format off */
const word NNADTAB[100] = {
    OCT(00000),          /* 00 NOT IN USE */
    OCT(40000),          /* 01 SPECIFY MACHINE ADDRESS (FRACTIONAL) */
    OCT(40000),          /* 02 SPECIFY MACHINE ADDRESS (WHOLE) */
    OCT(40000),          /* 03 SPECIFY MACHINE ADDRESS (DEGREES) */
    ECADR(DSPTEM1),      /* 04 ANGULAR ERROR/DIFFERENCE */
    ECADR(DSPTEM1),      /* 05 ANGULAR ERROR/DIFFERENCE */
    ECADR(OPTION1),      /* 06 OPTION CODE */
    ECADR(XREG),         /* 07 ECADR OF WORD TO BE MODIFIED */
                         /* ONES FOR BITS TO BE MODIFIED */
                         /* 1 TO SET OR 0 TO RESET SELECTED BITS */
    ECADR(ALMCADR),      /* 08 ALARM DATA */
    ECADR(FAILREG),      /* 09 ALARM CODES */
    OCT(77776),          /* 10 CHANNEL TO BE SPECIFIED */
    ECADR(TCSI),         /* 11 TIG OF CSI (HRS,MIN,SEC) */
    ECADR(OPTIONX),      /* 12 OPTION CODE */
                         /* (USED BY EXTENDED VERBS ONLY) */
    ECADR(TCDH),         /* 13 TIG OF CDH (HRS,MIN,SEC) */
    ECADR(DSPTEMX),      /* 14 CHECKLIST */
                         /* (USED BY EXTENDED VERBS ONLY) */
    OCT(77777),          /* 15 INCREMENT MACHINE ADDRESS */
    ECADR(DSPTEMX),      /* 16 TIME OF EVENT (HRS,MIN,SEC) */
    OCT(00000),          /* 17 SPARE */
    ECADR(FDAIX),        /* 18 AUTO MANEUVER BALL ANGLES */
    OCT(00000),          /* 19 SPARE */
    ECADR(CDUX),         /* 20 ICDU ANGLES */
    ECADR(PIPAX),        /* 21 PIPAS */
    ECADR(THETAD),       /* 22 NEW ICDU ANGLES */
    OCT(00000),          /* 23 SPARE */
    ECADR(DSPTEM2) + 1u, /* 24 DELTA TIME FOR AGC CLOCK (HRS,MIN,SEC) */
    ECADR(DSPTEM1),      /* 25 CHECKLIST */
                         /* (USED WTIH PLEASE PERFORM ONLY) */
    ECADR(DSPTEM1),      /* 26 PRIO/DELAY, ADRES, BBCON */
    ECADR(SMODE),        /* 27 SELF TEST ON/OFF SWITCH */
    OCT(00000),          /* 28 SPARE */
    OCT(00000),          /* 29 SPARE */
    OCT(0),              /* 30 SPARE */
    OCT(0),              /* 31 SPARE */
    ECADR(MINUS_TPER),   /* 32 TIME TO PERIGEE (HRS,MIN,SEC) */
    ECADR(TIG),          /* 33 TIME OF IGNITION (HRS,MIN,SEC) */
    ECADR(DSPTEM1),      /* 34 TIME OF EVENT (HRS,MIN,SEC) */
    ECADR(TTOGO),        /* 35 TIME TO GO TO EVENT (HRS,MIN,SEC) */
    ECADR(TIME2),        /* 36 TIME OF AGC CLOCK (HRS,MIN,SEC) */
    ECADR(TTPI),         /* 37 TIG OF TPI (HRS,MIN,SEC) */
    ECADR(TET),          /* 38 TIME OF STATE BEING INTEGRATED */
    OCT(00000),          /* 39 SPARE */
    /* END OF NNADTAB FOR NORMAL NOUNS */
    /* NN MIXED NOUNS */
    OCT(64000),          /* 40 TIME TO IGNITION/CUTOFF */
                         /* VG */
                         /* DELTA V (ACCUMULATED) */
    OCT(02003),          /* 41 TARGET AZIMUTH */
                         /* ELEVATION */
    OCT(24006),          /* 42 APOGEE */
                         /* PERIGEE */
                         /* DELTA V (REQUIRED) */
    OCT(24011),          /* 43 LATITUDE */
                         /* LONGITUDE */
                         /* ALTITUDE */
    OCT(64014),          /* 44 APOGEE */
                         /* PERIGEE */
                         /* TFF */
    OCT(64017),          /* 45 MARKS */
                         /* TTI OF NEXT BURN */
                         /* MGA */
    OCT(00022),          /* 46 AUTOPILOT CONFIGURATION */
    OCT(22025),          /* 47 LEM WEIGHT */
                         /* CSM WEIGHT */
    OCT(22030),          /* 48 GIMBAL PITCH TRIM */
                         /* GIMBAL ROLL TRIM */
    OCT(24033),          /* 49 DELTA R */
                         /* DELTA V */
                         /* RADAR DATA SOURCE CODE */
    OCT(0),              /* 50 SPARE */
    OCT(22041),          /* 51 S-BAND ANTENNA PITCH */
                         /* YAW */
    OCT(00044),          /* 52 CENTRAL ANGLE OF ACTIVE VEHICLE */
    OCT(00000),          /* 53 SPARE */
    OCT(24052),          /* 54 RANGE */
                         /* RANGE RATE */
                         /* THETA */
    OCT(24055),          /* 55 NO. OF APSIDAL CROSSINGS */
                         /* ELEVATION ANGLE */
                         /* CENTRAL ANGLE */
    OCT(02060),          /* 56 RR LOS AZIMUTH */
                         /* ELEVATION */
    OCT(20063),          /* 57 DELTA R */
    OCT(24066),          /* 58 PERIGEE ALT */
                         /* DELTA V TPI */
                         /* DELTA V TPF */
    OCT(24071),          /* 59 DELTA VELOCITY LOS */
    OCT(24074),          /* 60 HORIZONTAL VELOCITY */
                         /* ALTITUDE RATE */
                         /* COMPUTED ALTITUDE */
    OCT(64077),          /* 61 TIME TO GO IN BRAKING PHASE */
                         /* TIME TO IGNITION */
                         /* CROSS RANGE DISTANCE */
    OCT(64102),          /* 62 ABSOLUTE VALUE OF VELOCITY */
                         /* TIME TO IGNITION */
                         /* DELTA V (ACCUMULATED) */
    OCT(24105),          /* 63 ABSOLUTE VALUE OF VELOCITY */
                         /* ALTITUDE RATE */
                         /* COMPUTED ALTITUDE */
    OCT(64110),          /* 64 TIME LEFT FOR REDESIGNATION -- LPD ANGLE */
                         /* ALTITUDE RATE */
                         /* COMPUTED ALTITUDE */
    OCT(24113),          /* 65 SAMPLED AGC TIME (HRS,MIN,SEC) */
                         /* (FETCHED IN INTERRUPT) */
    OCT(62116),          /* 66 LR RANGE */
                         /* POSITION */
    OCT(04121),          /* 67 LRVX */
                         /* LRVY */
                         /* LRVZ */
    OCT(64124),          /* 68 SLANT RANGE TO LANDING SIGHT */
                         /* TIME TO GO IN BRAKING PHASE */
                         /* LR ALTITUDE -- COMPUTED ALTITUDE */
    OCT(00000),          /* 69 SPARE */
    OCT(04132),          /* 70 AOT DETENT CODE/STAR CODE */
    OCT(04135),          /* 71 AOT DETENT CODE/STAR CODE */
    OCT(02140),          /* 72 RR 360 -- TRUNNION ANGLE */
                         /* SHAFT ANGLE */
    OCT(02143),          /* 73 NEW RR 360 -- TRUNNION ANGLE */
                         /* SHAFT ANGLE */
    OCT(64146),          /* 74 TIME TO IGNITION */
                         /* YAWAFTER VEHICLE RISE */
                         /* PITCH AFTER VEHICLE RISE */
    OCT(64151),          /* 75 DELTA ALTITUDE CDH */
                         /* DELTA TIME (CDH-CSI OR TPI-CDH) */
                         /* DELTA TIME (TPI-CDH OR TPI-NOMTPI) */
    OCT(24154),          /* 76 DESIRED HORIZONTAL VELOCITY */
                         /* DESIRED RADIAL VELOCITY */
                         /* CROSS-RANGE DISTANCE */
    OCT(62157),          /* 77 TIME TO ENGINE CUTOFF */
                         /* VELOCITY NORMAL TO CSM PLANE */
    OCT(02162),          /* 78 RR RANGE */
                         /* RANGE RATE */
    OCT(24165),          /* 79 CURSOR ANGLE */
                         /* SPIRAL ANGLE */
                         /* POSITION CODE */
    OCT(02170),          /* 80 DATA INDICATOR */
                         /* OMEGA */
    OCT(24173),          /* 81 DELTA V (LV) */
    OCT(24176),          /* 82 DELTA V (LV) */
    OCT(24201),          /* 83 DELTA V (BODY) */
    OCT(24204),          /* 84 DELTA V (OTHER VEHICLE) */
    OCT(24207),          /* 85 VG (BODY) */
    OCT(24212),          /* 86 VG (LV) */
    OCT(02215),          /* 87 BACKUP OPTICS LOS AZIMUTH */
                         /* ELEVATION */
    OCT(24220),          /* 88 HALF UNIT SUN OR PLANET VECTOR */
    OCT(24223),          /* 89 LANDMARK LATITUDE */
                         /* LONGITUDE/2 */
                         /* ALTITUDE */
    OCT(24226),          /* 90 Y */
                         /* Y DOT */
                         /* PSI */
    OCT(04231),          /* 91 ALTITUDE */
                         /* VELOCITY */
                         /* FLIGHT PATH ANGLE */
    OCT(00000),          /* 92 SPARE */
    OCT(04237),          /* 93 DELTA GYRO ANGLES */
    OCT(00000),          /* 94 SPARE */
    OCT(0),              /* 95 SPARE */
    OCT(0),              /* 96 SPARE */
    OCT(04253),          /* 97 SYSTEM TEST INPUTS */
    OCT(04256),          /* 98 SYSTEM TEST RESULTS */
    OCT(24261),          /* 99 RMS IN POSITION */
                         /* RMS IN VELOCITY */
                         /* RMS IN BIAS */
    /* END OF NNADTAB FOR MIXED NOUNS */
    /* NN NORMAL NOUNS */
};
/* clang-format on */

/* NNTYPTAB -- LM pp. 308-311: each noun's type: for a normal noun the number of components
 * (bits 15-11) and the scale factor routine and constant (bits 10-6 and
 * 5-1); for a mixed noun the scale factor constants of its components. */
/* clang-format off */
const word NNTYPTAB[100] = {
    OCT(00000), /* 00 NOT IN USE */
    OCT(04040), /* 01 3COMP FRACTIONAL */
    OCT(04140), /* 02 3COMP WHOLE */
    OCT(04102), /* 03 3COMP CDU DEGREES */
    OCT(00504), /* 04 1COMP DPDEG(360) */
    OCT(00504), /* 05 1COMP DPDEG(360) */
    OCT(04000), /* 06 3COMP OCTAL ONLY */
    OCT(04000), /* 07 3COMP OCTAL ONLY */
    OCT(04000), /* 08 3COMP OCTAL ONLY */
    OCT(04000), /* 09 3COMP OCTAL ONLY */
    OCT(00000), /* 10 1COMP OCTAL ONLY */
    OCT(24400), /* 11 3COMP HMS (DEC ONLY) */
    OCT(02000), /* 12 2COMP OCTAL ONLY */
    OCT(24400), /* 13 3COMP HMS (DEC ONLY) */
    OCT(04140), /* 14 3COMP WHOLE */
    OCT(00000), /* 15 1COMP OCTAL ONLY */
    OCT(24400), /* 16 3COMP HMS (DEC ONLY) */
    OCT(0),     /* 17 SPARE */
    OCT(04102), /* 18 3COMP CDU DEG */
    OCT(00000), /* 19 SPARE */
    OCT(04102), /* 20 3COMP CDU DEGREES */
    OCT(04140), /* 21 3COMP WHOLE */
    OCT(04102), /* 22 3COMP CDU DEGREES */
    OCT(00000), /* 23 SPARE */
    OCT(24400), /* 24 3COMP HMS (DEC ONLY) */
    OCT(04140), /* 25 3COMP WHOLE */
    OCT(04000), /* 26 3COMP OCTAL ONLY */
    OCT(00140), /* 27 1COMP WHILE */
    OCT(00000), /* 28 SPARE */
    OCT(00000), /* 29 SPARE */
    OCT(0),     /* 30 SPARE */
    OCT(0),     /* 31 SPARE */
    OCT(24400), /* 32 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 33 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 34 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 35 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 36 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 37 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 38 3COMP HMS (DEC ONLY) */
    OCT(00000), /* 39 SPARE */
    /* END OF NNTYPTAB FOR NORMAL NOUNS */
    /* NN MIXED NOUNS */
    OCT(24500), /* 40 3COMP MIN/SEC, VEL3, VEL3 */
                /* (NO LOAD, DEC ONLY) */
    OCT(00542), /* 41 2COMP CDU DEG, ELEV DEG */
    OCT(24410), /* 42 3COMP POS4, POS4, VEL3 */
                /* (DEC ONLY) */
    OCT(20204), /* 43 3COMP DPDEG(360), DPDEG(360) POS4 */
                /* (DEC ONLY) */
    OCT(00410), /* 44 3COMP POS4, POS4, MIN/SEC */
                /* (NO LOAD, DEC ONLY) */
    OCT(10000), /* 45 3COMP WHOLE, MIN/SEC, DPDEG(360) */
                /* (NO LOAD, DEC ONLY) */
    OCT(00000), /* 46 1COMP OCTAL ONLY */
    OCT(00306), /* 47 2COMP WEIGHT2 FOR EACH */
                /* (DEC ONLY) */
    OCT(01367), /* 48 2COMP TRIM DEG2 FOR EACH */
                /* (DEC ONLY) */
    OCT(00510), /* 49 3COMP POS4, VEL3, WHOLE */
                /* (DEC ONLY) */
    OCT(0),     /* 50 SPARE */
    OCT(00204), /* 51 2COMP DPDEG(360), DPDEG(360) */
                /* (DEC ONLY) */
    OCT(00004), /* 52 1COMP DPDEG(360) */
    OCT(00000), /* 53 SPARE */
    OCT(10507), /* 54 3COMP POS5, VEL3, DPDEG(360) */
                /* (DEC ONLY) */
    OCT(10200), /* 55 3COMP WHOLE, DPDEG(360), DPDEG(360) */
                /* (DEC ONLY) */
    OCT(00204), /* 56 2COMP DPDEG(360), DPDEG(360) */
    OCT(00010), /* 57 1COMP POS4 */
                /* (DEC ONLY) */
    OCT(24510), /* 58 3COMP POS4, VEL3, VEL3 */
                /* (DEC ONLY) */
    OCT(24512), /* 59 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(60512), /* 60 3COMP VEL3, VEL3, COMP ALT */
                /* (DEC ONLY) */
    OCT(54000), /* 61 3COMP MIN/SEC, MIN/SEC, POS7 */
                /* (NO LOAD, DEC ONLY) */
    OCT(24012), /* 62 3COMP VEL3, MIN/SEC, VEL3 */
                /* (NO LOAD, DEC ONLY) */
    OCT(60512), /* 63 3COMP VEL3, VEL3, COMP ALT */
                /* (DEC ONLY) */
    OCT(60500), /* 64 3COMP 2INT, VEL3, COMP ALT */
                /* (NO LOAD, DEC ONLY) */
    OCT(00000), /* 65 3COMP HMS (DEC ONLY) */
    OCT(00016), /* 66 2COMP LANDING RADAR ALT, POSITION */
                /* (NO LOAD, DEC ONLY) */
    OCT(53223), /* 67 3COMP LANDING RADAR VELX, Y, Z */
    OCT(60026), /* 68 3COMP POS7, MIN/SEC, COMP ALT */
                /* (NO LOAD, DEC ONLY) */
    OCT(00000), /* 69 SPARE */
    OCT(0),     /* 70 3COMP OCTAL ONLY FOR EACH */
    OCT(0),     /* 71 3COMP OCTAL ONLY FOR EACH */
    OCT(00102), /* 72 2COMP 360-CDU DEG, CDU DEG */
    OCT(00102), /* 73 2COMP 360-CDU DEG, CDU DEG */
    OCT(10200), /* 74 3COMP MIN/SEC, DPDEG(360), DPDEG(360) */
                /* (NO LOAD, DEC ONLY) */
    OCT(00010), /* 75 3COMP POS4, MIN/SEC, MIN/SEC */
                /* (NO LOAD, DEC ONLY) */
    OCT(20512), /* 76 3COMP VEL3, VEL3, POS4 */
                /* (DEC ONLY) */
    OCT(00500), /* 77 2COMP MIN/SEC, VEL3 */
                /* (NO LOAD, DEC ONLY) */
    OCT(00654), /* 78 2 COMP RR RANGE, RR RANGE RATE */
    OCT(00102), /* 79 3COMP CDU DEG, CDU DEG, WHOLE */
                /* (DEC ONLY) */
    OCT(00200), /* 80 2COMP WHOLE, DPDEG(360) */
    OCT(24512), /* 81 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 82 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 83 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 84 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 85 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 86 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(00102), /* 87 2COMP CDU DEG FOR EACH */
    OCT(0),     /* 88 3COMP FRAC FOR EACH */
                /* (DEC ONLY) */
    OCT(16143), /* 89 3COMP DPDEG(90), DPDEG(90), POS5 */
                /* (DEC ONLY) */
    OCT(10507), /* 90 3COMP POS5, VEL3, DEPDEG(360) */
                /* (DEC ONLY) */
    OCT(10450), /* 91 3COMP POS4, VEL2, DPDEG(360) */
    OCT(00000), /* 92 SPARE */
    OCT(06143), /* 93 3COMP DPDEG(90) FOR EACH */
    OCT(00000), /* 94 SPARE */
    OCT(0),     /* 95 SPARE */
    OCT(0),     /* 96 SPARE */
    OCT(00000), /* 97 3COMP WHOLE FOR EACH */
    OCT(00000), /* 98 3COMP WHOLE, FRAC, WHOLE */
    OCT(71572), /* 99 3COMP POS9, VEL4, RADIANS */
                /* (DEC ONLY) */
    /* END OF NNTYPTAB FOR MIXED NOUNS */
};
/* clang-format on */

/* SFINTAB -- LM pp. 311-312: the scale factors from the decimal the crew loads into the
 * machine's units, a double word each, indexed by twice the scale factor
 * constant code (2X(SFCONUM)). */
/* clang-format off */
const word SFINTAB[58] = {
    OCT(00006), /* WHOLE, DP TIME (SEC) */
    OCT(03240),
    OCT(00000), /* SPARE */
    OCT(00000),
    OCT(00000), /* CDU DEGREES, 360-CDU DEGREES */
    OCT(00000), /* (SFCONS IN DEGINSF) */
    OCT(10707), /* DP DEGREES (90) */
    OCT(03435), /* UPPED BY 1 */
    OCT(13070), /* DP DEGREES (360) (POINT BETWN BITS 11-12) */
    OCT(34345), /* UPPED BY 1 */
    OCT(00005), /* DEGREES (180) */
    OCT(21616),
    OCT(26113), /* WEIGHT2 */
    OCT(31713),
    OCT(00070), /* POSITION5 */
    OCT(20460),
    OCT(01065), /* POSITION4 */
    OCT(05740),
    OCT(11414), /* VELOCITY2 (POINT BETWN BITS 11-12) */
    OCT(31463),
    OCT(07475), /* VELOCITY3 */
    OCT(16051),
    OCT(00001), /* ELEVATION DEGREES */
    OCT(03434),
    OCT(00047), /* RENDEZVOUS RADAR RANGE */
    OCT(21135),
    OCT(77766), /* RENDESVOUS RADAR RANGE RATE */
    OCT(50711),
    OCT(00005), /* 2DEC* .9267840599 E5 B-28*: LANDING RADAR ALTITUDE */
    OCT(25006),
    OCT(00002), /* INITIAL/FINAL ALTITUDE */
    OCT(23224),
    OCT(00014), /* ALTITUDE RATE */
    OCT(06500),
    OCT(00012), /* FORWARD/LATERAL VELOCITY */
    OCT(36455),
    OCT(04256), /* ROT HAND CONT ANGLE RATE */
    OCT(07071),
    OCT(77766), /* 2DEC* -1.552795030 E5 B-28*: LANDING RADAR VELX */
    OCT(60557),
    OCT(00005), /* 2DEC* .8250825087 E5 B-28*: LANDING RADAR VELY */
    OCT(01114),
    OCT(00007), /* 2DEC* 1.153668673 E5 B-28*: LANDING RADAR VELZ */
    OCT(01247),
    OCT(04324), /* POSITION7 */
    OCT(27600),
    OCT(00036), /* TRIM DEGREES2 */
    OCT(20440),
    OCT(00035), /* COMPUTED ALTITUDE */
    OCT(30400),
    OCT(23420), /* DP DEGREES */
    OCT(00000),
    OCT(01670), /* 2DEC 30480 B-19: POSITION 9 */
    OCT(20000),
    OCT(07475), /* 2DEC 30.48 B-7: VELOCITY4 */
    OCT(16051),
    OCT(14400), /* 2DEC 100 B-8: RADIANS */
    OCT(00000),
    /* END OF SFINTAB */
};
/* clang-format on */

/* SFOUTAB -- LM pp. 312-314: the scale factors from the machine's units to the decimal
 * displayed, a double word each, by the same code. */
/* clang-format off */
const word SFOUTAB[58] = {
    OCT(05174), /* WHOLE, DP TIME (SEC) */
    OCT(13261),
    OCT(00000), /* SPARE */
    OCT(00000),
    OCT(00000), /* CDU DEGREES, 360-CDU DEGREES */
    OCT(00000), /* (SFCONS IN DEGOUTSF, 360 CDUO) */
    OCT(00714), /* DP DEGREES (90) (POINT BETWN BITS 7-8) */
    OCT(31463),
    OCT(13412), /* DP DEGREES (360) */
    OCT(07534),
    OCT(05605), /* DEGREES (180) */
    OCT(03656),
    OCT(00001), /* WEIGHT2 */
    OCT(16170),
    OCT(00441), /* POSITION5 */
    OCT(34306),
    OCT(07176), /* POSITION4 (POINT BETWN BITS 7-8) */
    OCT(21603),
    OCT(15340), /* VELOCITY2 */
    OCT(15340),
    OCT(01031), /* VELOCITY3 (POINT BETWN BITS 7-8) */
    OCT(21032),
    OCT(34631), /* ELEVATION DETREES */
    OCT(23146),
    OCT(00636), /* RENDEZVOUS RADAR RANGE */
    OCT(14552),
    OCT(74552), /* RENDEZVOUS RADAR RANGE RATE */
    OCT(70307),
    OCT(05520), /* 2DEC 1.079 E-5 B14: LANDING RADAR ALTITUDE */
    OCT(15312),
    OCT(14226), /* INITIAL/FINAL ALTITUDE */
    OCT(31757),
    OCT(02476), /* ALTITUDE RATE */
    OCT(05531),
    OCT(02727), /* FORWARD/LATERAL VELOCTY */
    OCT(16415),
    OCT(00007), /* ROT HAND CONT ANGLE RATE */
    OCT(13734),
    OCT(74477), /* 2DEC -.6440 E-5 B14: LANDING RADAR VELX */
    OCT(50643),
    OCT(06265), /* 2DEC 1.212 E-5 B14: LANDING RADAR VELY */
    OCT(16004),
    OCT(04426), /* 2DEC .8668 E-5 B14: LANDING RADAR VELZ */
    OCT(31433),
    OCT(34772), /* POSITION7 */
    OCT(07016),
    OCT(01030), /* TRIM DEGREES2 */
    OCT(33675),
    OCT(01046), /* COMPUTED ALTITUDE */
    OCT(15700),
    OCT(00321), /* DP DEGREES */
    OCT(26706),
    OCT(04231), /* 2DEC 17.2010499 B-7: POSITION 9 */
    OCT(27400),
    OCT(01031), /* 2DEC .032808399: VELOCITY4 */
    OCT(21032),
    OCT(12172), /* 2DEC .32: RADIANS */
    OCT(34122),
    /* END OF SFOUTAB */
    /* NN SF CONSTANT SF ROUTINE */
};
/* clang-format on */

/* IDADDTAB -- LM pp. 314-317: the ECADRs of a mixed noun's three components, three words
 * per noun, with the scale factor constant and routine each one takes. */
/* clang-format off */
const word IDADDTAB[180] = {
    ECADR(TTOGO),        /* 40 MIN/SEC M/S */
    ECADR(VGDISP),       /* 40 VEL3 DP3 */
    ECADR(DVTOTAL),      /* 40 VEL3 DP3 */
    ECADR(DSPTEM1),      /* 41 CDU DEG CDU */
    ECADR(DSPTEM1) + 1u, /* 41 ELEV DEG ARTH */
    OCT(0),              /* 41 SPARE COMPONENT */
    ECADR(HAPO),         /* 42 POS4 DP3 */
    ECADR(HPER),         /* 42 POS4 DP3 */
    ECADR(VGDISP),       /* 42 VEL3 DP3 */
    ECADR(LAT),          /* 43 DPDEG(360) DP4 */
    ECADR(LONG),         /* 43 DPDEG(360 DP4 */
    ECADR(ALT),          /* 43 POS4 DP3 */
    ECADR(HAPOX),        /* 44 POS4 DP3 */
    ECADR(HPERX),        /* 44 POS4 DP3 */
    ECADR(TFF),          /* 44 MIN/SEC M/S */
    ECADR(TRKMKCNT),     /* 45 WHOLE ARTH */
    ECADR(TTOGO),        /* 45 MIN/SEC M/S */
    ECADR(PLUS_MGA),     /* 45 DPDEG(360) DP4 */
    ECADR(DAPDATR1),     /* 46 OCTAL ONLY OCT */
    OCT(0),              /* 46 SPARE COMPONENT */
    OCT(0),              /* 46 SPARE COMPONENT */
    ECADR(LEMMASS),      /* 47 WEIGHT2 ARTH1 */
    ECADR(CSMMASS),      /* 47 WEIGHT2 ARTH1 */
    OCT(0),              /* 47 SPARE COMPONENT */
    ECADR(PITTIME),      /* 48 TRIM DEG2 ARTH */
    ECADR(ROLLTIME),     /* 48 TRIM DEG2 ARTH */
    OCT(0),              /* 48 SPARE COMPONENT */
    ECADR(R22DISP),      /* 49 POS4 DP3 */
    ECADR(R22DISP) + 2u, /* 49 VEL3 DP3 */
    ECADR(WHCHREAD),     /* 49 WHOLE ARTH */
    OCT(0),              /* 50 SPARE */
    OCT(0),              /* 50 SPARE */
    OCT(0),              /* 50 SPARE */
    ECADR(ALPHASB),      /* 51 DPDEG(360) DP4 */
    ECADR(BETASB),       /* 51 DPDEG(360) DP4 */
    OCT(0),              /* 51 SPARE COMPONENT */
    ECADR(ACTCENT),      /* 52 DPDEG(360) DP4 */
    OCT(00000),          /* 52 SPARE COMPONENT */
    OCT(00000),          /* 52 SPARE COMPONENT */
    OCT(00000),          /* 53 SPARE */
    OCT(00000),          /* 53 */
    OCT(00000),          /* 53 */
    ECADR(RANGE),        /* 54 POS5 DP1 */
    ECADR(RRATE),        /* 54 VEL3 DP3 */
    ECADR(RTHETA),       /* 54 DPDEG(360) DP4 */
    ECADR(NN),           /* 55 WHOLE ARTH */
    ECADR(ELEV),         /* 55 DPDEG(360) DP4 */
    ECADR(CENTANG),      /* 55 DPDEG(360) DP4 */
    ECADR(RR_AZ),        /* 56 DPDEG(360) DP4 */
    ECADR(RR_ELEV),      /* 56 DPDEG(360) DP4 */
    OCT(0),              /* 56 SPARE COMPONENT */
    ECADR(DELTAR),       /* 57 POS4 DP3 */
    OCT(0),              /* 57 SPARE COMPONENT */
    OCT(0),              /* 57 SPARE COMPONENT */
    ECADR(POSTTPI),      /* 58 POS4 DP3 */
    ECADR(DELVTPI),      /* 58 VEL3 DP3 */
    ECADR(DELVTPF),      /* 58 VEL3 DP3 */
    ECADR(DVLOS),        /* 59 VEL3 DP3 */
    ECADR(DVLOS) + 2u,   /* 59 VEL3 DP3 */
    ECADR(DVLOS) + 4u,   /* 59 VEL3 DP3 */
    ECADR(VHORIZ),       /* 60 VEL3 DP3 */
    ECADR(HDOTDISP),     /* 60 VEL3 DP3 */
    ECADR(HCALC),        /* 60 COMP ALT DP1 */
    ECADR(TTFDISP),      /* 61 MIN/SEC M/S */
    ECADR(TTOGO),        /* 61 MIN/SEC M/S */
    ECADR(OUTOFPLN),     /* 61 POS7 DP4 */
    ECADR(ABVEL),        /* 62 VEL3 DP3 */
    ECADR(TTOGO),        /* 62 MIN/SEC M/S */
    ECADR(DVTOTAL),      /* 62 VEL3 DP3 */
    ECADR(ABVEL),        /* 63 VEL3 DP3 */
    ECADR(HDOTDISP),     /* 63 VEL3 DP3 */
    ECADR(HCALC1),       /* 63 COMP ALT DP1 */
    ECADR(FUNNYDSP),     /* 64 2INT 2INT */
    ECADR(HDOTDISP),     /* 64 VEL3 DP3 */
    ECADR(HCALC),        /* 64 COMP ALT DP1 */
    ECADR(SAMPTIME),     /* 65 HMS (MIXED ONLY TO KEEP CODE 65) HMS */
    ECADR(SAMPTIME),     /* 65 HMS HMS */
    ECADR(SAMPTIME),     /* 65 HMS HMS */
    ECADR(RSTACK) + 6u,  /* 66 LANDING RADAR ALT DP1 */
    OCT(0),              /* 66 LR POSITION LRPOS */
    OCT(0),              /* 66 SPARE COMPONENT */
    ECADR(RSTACK),       /* 67 LANDING RADAR VELX DP1 */
    ECADR(RSTACK) + 2u,  /* 67 LANDING RADAR VELY DP1 */
    ECADR(RSTACK) + 4u,  /* 67 LANDING RADAR VELZ DP1 */
    ECADR(RANGEDSP),     /* 68 POS7 DP4 */
    ECADR(TTFDISP),      /* 68 MIN/SEC M/S */
    ECADR(DELTAH),       /* 68 COMP ALT DP1 */
    OCT(00000),          /* 69 SPARE */
    OCT(00000),          /* 69 */
    OCT(00000),          /* 69 */
    ECADR(AOTCODE),      /* 70 OCTAL ONLY OCT */
    ECADR(AOTCODE) + 1u, /* 70 OCTAL ONLY OCT */
    ECADR(AOTCODE) + 2u, /* 70 OCTAL ONLY OCT */
    ECADR(AOTCODE),      /* 71 OCTAL ONLY OCT */
    ECADR(AOTCODE) + 1u, /* 71 OCTAL ONLY OCT */
    ECADR(AOTCODE) + 2u, /* 71 OCTAL ONLY OCT */
    ECADR(CDUT),         /* 72 360-CDU DEG 360-CDU */
    ECADR(CDUS),         /* 72 CDU DEG CDU */
    OCT(0),              /* 72 SPARE COMPONENT */
    ECADR(TANG),         /* 73 360-CDU DEG 360-CDU */
    ECADR(TANG) + 1u,    /* 73 CDU DEG CDU */
    OCT(0),              /* 73 SPARE COMPONENT */
    ECADR(TTOGO),        /* 74 MIN/SEC M/S */
    ECADR(YAW),          /* 74 DPDEG(360) DP4 */
    ECADR(PITCH),        /* 74 DPDEG(360) DP4 */
    ECADR(DIFFALT),      /* 75 POS4 DP3 */
    ECADR(T1TOT2),       /* 75 MIN/SEC */
    ECADR(T2TOT3),       /* 75 MIN/SEC M/S */
    ECADR(ZDOTD),        /* 76 VEL3 DP3 */
    ECADR(RDOTD),        /* 76 VEL3 DP3 */
    ECADR(XRANGE),       /* 76 POS4 DP3 */
    ECADR(TTOGO),        /* 77 MIN/SEC M/S */
    ECADR(YDOT),         /* 77 VEL3 DP3 */
    OCT(0),              /* 77 SPARE COMPONENT */
    ECADR(RSTACK),       /* 78 RR RANGE DP1 */
    ECADR(RSTACK) + 2u,  /* 78 RR RANGE RATE DP1 */
    OCT(00000),          /* 78 SPARE COMPONENT */
    ECADR(CURSOR),       /* 79 CDU DEG CDU */
    ECADR(SPIRAL),       /* 79 CDU DEG CDU */
    ECADR(POSCODE),      /* 79 WHOLE ARTH */
    ECADR(DATAGOOD),     /* 80 WHOLE ARTH */
    ECADR(OMEGAD),       /* 80 DPDEG(360) DP4 */
    OCT(0),              /* 80 SPARE COMPONENT */
    ECADR(DELVLVC),      /* 81 VEL3 DP3 */
    ECADR(DELVLVC) + 2u, /* 81 VEL3 DP3 */
    ECADR(DELVLVC) + 4u, /* 81 VEL3 DP3 */
    ECADR(DELVLVC),      /* 82 VEL3 DP3 */
    ECADR(DELVLVC) + 2u, /* 82 VEL3 DP3 */
    ECADR(DELVLVC) + 4u, /* 82 VEL3 DP3 */
    ECADR(DELVIMU),      /* 83 VEL3 DP3 */
    ECADR(DELVIMU) + 2u, /* 83 VEL3 DP3 */
    ECADR(DELVIMU) + 4u, /* 83 VEL3 DP3 */
    ECADR(DELVOV),       /* 84 VEL3 DP3 */
    ECADR(DELVOV) + 2u,  /* 84 VEL3 DP3 */
    ECADR(DELVOV) + 4u,  /* 84 VEL3 DP3 */
    ECADR(VGBODY),       /* 85 VEL3 DP3 */
    ECADR(VGBODY) + 2u,  /* 85 VEL3 DP3 */
    ECADR(VGBODY) + 4u,  /* 85 VEL3 DP3 */
    ECADR(DELVLVC),      /* 86 VEL3 DP3 */
    ECADR(DELVLVC) + 2u, /* 86 VEL3 DP3 */
    ECADR(DELVLVC) + 4u, /* 86 VEL3 DP3 */
    ECADR(AZ),           /* 87 CDU DEG CDU */
    ECADR(EL),           /* 87 CDU DEG CDU */
    OCT(0),              /* 87 SPARE COMPONENT */
    ECADR(STARAD),       /* 88 FRAC FRAC */
    ECADR(STARAD) + 2u,  /* 88 FRAC FRAC */
    ECADR(STARAD) + 4u,  /* 88 FRAC FRAC */
    ECADR(LANDLAT),      /* 89 DPDEG(90) DP3 */
    ECADR(LANDLONG),     /* 89 DPDEG(90) DP3 */
    ECADR(LANDALT),      /* 89 POS5 DP1 */
    ECADR(RANGE),        /* 90 POS5 DP1 */
    ECADR(RRATE),        /* 90 VEL3 DP3 */
    ECADR(RTHETA),       /* 90 DPDEG(360) DP4 */
    ECADR(P21ALT),       /* 91 POS4 DP3 */
    ECADR(P21VEL),       /* 91 VEL2 DP4 */
    ECADR(P21GAM),       /* 91 DPDEG(360) DP4 */
    OCT(00000),          /* 92 SPARE */
    OCT(00000),          /* 92 */
    OCT(00000),          /* 92 */
    ECADR(OGC),          /* 93 DPDEG(90) DP3 */
    ECADR(OGC) + 2u,     /* 93 DPDEG(90) DP3 */
    ECADR(OGC) + 4u,     /* 93 DPDEG(90) DP3 */
    OCT(00000),          /* 94 SPARE */
    OCT(00000),          /* 94 */
    OCT(00000),          /* 94 */
    OCT(0),              /* 95 SPARE */
    OCT(0),              /* 95 SPARE */
    OCT(0),              /* 95 SPARE */
    OCT(0),              /* 96 SPARE */
    OCT(0),              /* 96 SPARE */
    OCT(0),              /* 96 SPARE */
    ECADR(DSPTEM1),      /* 97 WHOLE ARTH */
    ECADR(DSPTEM1) + 1u, /* 97 WHOLE ARTH */
    ECADR(DSPTEM1) + 2u, /* 97 WHOLE ARTH */
    ECADR(DSPTEM2),      /* 98 WHOLE ARTH */
    ECADR(DSPTEM2) + 1u, /* 98 FRAC FRAC */
    ECADR(DSPTEM2) + 2u, /* 98 WHOLE ARTH */
    ECADR(WWPOS),        /* 99 POS9 DP3 */
    ECADR(WWVEL),        /* 99 VEL4 DP2 */
    ECADR(WWBIAS),       /* 99 RADIANS DP4 */
    /* END OF IDADDTAB */
    /* NN SF ROUTINES */
};
/* clang-format on */

/* RUTMXTAB -- LM pp. 317-319: each mixed noun's scale factor routines, three 5-bit codes
 * (bits 15-11, 10-6, 5-1 for components 3, 2, 1). */
/* clang-format off */
const word RUTMXTAB[60] = {
    OCT(16351), /* 40 M/S, DP3, DP3 */
    OCT(00142), /* 41 CDU, ARTH */
    OCT(16347), /* 42 DP3, DP3, DP3 */
    OCT(16512), /* 43 DP4, DP4, DP3 */
    OCT(22347), /* 44 DP3, DP3, M/S */
    OCT(24443), /* 45 ARTH, M/S, DP4 */
    OCT(00000), /* 46 OCT */
    OCT(00553), /* 47 ARITH1, ARITH1 */
    OCT(00143), /* 48 ARTH, ARTH */
    OCT(06347), /* 49 DP3, DP3, ARTH */
    OCT(0),     /* 50 SPARE */
    OCT(00512), /* 51 DP4, DP4 */
    OCT(00012), /* 52 DP4 */
    OCT(00000), /* 53 SPARE */
    OCT(24344), /* 54 DP1, DP3, DP4 */
    OCT(24503), /* 55 ARTH, DP4, DP4 */
    OCT(00512), /* 56 DP4, DP4 */
    OCT(00007), /* 57 DP3 */
    OCT(16347), /* 58 DP3, DP3, DP3 */
    OCT(16347), /* 59 DP3, DP3, DP3 */
    OCT(10347), /* 60 DP3, DP3, DP1 */
    OCT(24451), /* 61 M/S, M/S, DP4 */
    OCT(16447), /* 62 DP3, M/S, DP3 */
    OCT(10347), /* 63 DP3, DP3, DP1 */
    OCT(10354), /* 64 2INT, DP3, DP1 */
    OCT(20410), /* 65 HMS, HMS, HMS */
    OCT(00304), /* 66 DP1, LRPOS */
    OCT(10204), /* 67 DP1, DP1, DP1 */
    OCT(10452), /* 68 DP4, M/S, DP1 */
    OCT(00000), /* 69 SPARE */
    OCT(0),     /* 70 OCT, OCT, OCT */
    OCT(0),     /* 71 OCT, OCT, OCT */
    OCT(00115), /* 72 360-CDU, CDU */
    OCT(00115), /* 73 360-CDU, CDU */
    OCT(24511), /* 74 M/S, DP4, DP4 */
    OCT(22447), /* 75 DP3, M/S, M/S */
    OCT(16347), /* 76 DP3, DP3, DP3 */
    OCT(00351), /* 77 M/S, DP3 */
    OCT(00204), /* 78 DP1, DP1 */
    OCT(06102), /* 79 CDU, CDU, ARTH */
    OCT(00503), /* 80 ARTH, DP4 */
    OCT(16347), /* 81 DP3, DP3, DP3 */
    OCT(16347), /* 82 DP3, DP3, DP3 */
    OCT(16347), /* 83 DP3, DP3, DP3 */
    OCT(16347), /* 84 DP3, DP3, DP3 */
    OCT(16347), /* 85 DP3, DP3, DP3 */
    OCT(16347), /* 86 DP3, DP3, DP3 */
    OCT(00102), /* 87 CDU, CDU */
    OCT(02041), /* 88 FRAC FOR EACH */
    OCT(10347), /* 89 DP3, DP3, DP1 */
    OCT(24344), /* 90 DP1, DP3, DP4 */
    OCT(24507), /* 91 DP3, DP4, DP4 */
    OCT(00000), /* 92 SPARE */
    OCT(16347), /* 93 DP3, DP3, DP3 */
    OCT(00000), /* 94 SPARE */
    OCT(0),     /* 95 SPARE */
    OCT(0),     /* 96 SPARE */
    OCT(06143), /* 97 ARTH, ARTH, ARTH */
    OCT(06043), /* 98 ARTH, FRAC, ARTH */
    OCT(24247), /* 99 DP3, DP2, DP4 */
    /* END OF RUTMXTAB */
};
/* clang-format on */

#else

/* NNADTAB -- CM pp. 271-274: each noun's address: for a normal noun (00-39) the ECADR of its
 * word (or a code: 40000 machine address to be specified, 77776 channel,
 * 77777 increment address, 0 spare); for a mixed noun (40-99) bits 15-11
 * the component codes and bits 10-1 its first entry in IDADDTAB. */
/* clang-format off */
const word NNADTAB[100] = {
    OCT(00000),          /* 00 NOT IN USE */
    OCT(40000),          /* 01 SPECIFY MACHINE ADDRESS (FRACTIONAL) */
    OCT(40000),          /* 02 SPECIFY MACHINE ADDRESS (WHOLE) */
    OCT(40000),          /* 03 SPECIFY MACHINE ADDRESS (DEGREES) */
    OCT(0),              /* 04 SPARE */
    ECADR(DSPTEM1),      /* 05 ANGULAR ERROR/DIFFERENCE */
    ECADR(OPTION1),      /* 06 OPTION CODE */
    ECADR(XREG),         /* 07 ECADR OF WORD TO BE MODIFIED */
                         /* ONES FOR BITS TO BE MODIFIED */
                         /* 1 TO SET OR 0 TO RESET SELECTED BITS */
    ECADR(ALMCADR),      /* 08 ALARM DATA */
    ECADR(FAILREG),      /* 09 ALARM CODES */
    OCT(77776),          /* 10 CHANNEL TO BE SPECIFIED */
    ECADR(TCSI),         /* 11 TIG OF CSI (HRS,MIN,SEC) */
    ECADR(OPTIONX),      /* 12 OPTION CODE */
                         /* (USED BY EXTENDED VERBS ONLY) */
    ECADR(TCDH),         /* 13 TIG OF CDH (HRS,MIN,SEC) */
    OCT(0),              /* 14 SPARE */
    OCT(77777),          /* 15 INCREMENT MACHINE ADDRESS */
    ECADR(DSPTEMX),      /* 16 TIME OF EVENT (HRS,MIN,SEC) */
    ECADR(CPHIX),        /* 17 ASTRONAUT TOTAL ATTITUDE */
    ECADR(THETAD),       /* 18 AUTO MANEUVER BALL ANGLES */
    ECADR(THETAD),       /* 19 BYPASS ATTITUDE TRIM MANEUVER */
    ECADR(CDUX),         /* 20 ICDU ANGLES */
    ECADR(PIPAX),        /* 21 PIPAS */
    ECADR(THETAD),       /* 22 NEW ICDU ANGLES */
    OCT(00000),          /* 23 SPARE */
    ECADR(DSPTEM2) + 1u, /* 24 DELTA TIME FOR AGC CLOCK(HRS,MIN,SEC) */
    ECADR(DSPTEM1),      /* 25 CHECKLIST */
                         /* (USED WITH PLEASE PERFORM ONLY) */
    ECADR(DSPTEM1),      /* 26 PRIO/DELAY, ADRES, BBCON */
    ECADR(SMODE),        /* 27 SELF TEST ON/OFF SWITCH */
    OCT(0),              /* 28 SPARE */
    ECADR(DSPTEM1),      /* 29 XSM LAUNCH AZIMUTH */
    ECADR(DSPTEM1),      /* 30 TARGET CODES */
    ECADR(DSPTEM1),      /* 31 TIME OF LANDING SITE (HRS,MIN,SEC) */
    ECADR(MINUS_TPER),   /* 32 TIME TO PERIGEE (HRS,MIN,SEC) */
    ECADR(TIG),          /* 33 TIME OF IGNITION (HRS,MIN,SEC) */
    ECADR(DSPTEM1),      /* 34 TIME OF EVENT (HRS,MIN,SEC) */
    ECADR(TTOGO),        /* 35 TIME TO GO TO EVENT (HRS,MIN,SEC) */
    ECADR(TIME2),        /* 36 TIME OF AGC CLOCK (HRS,MIN,SEC) */
    ECADR(TTPI),         /* 37 TIG OF TPI (HRS,MIN,SEC) */
    ECADR(TET),          /* 38 TIME OF STATE VECTOR */
    ECADR(T3TOT4),       /* 39 DELTA TIME TO TRANSFER (HRS,MIN,SEC) */
    /* END OF NNADTAB FOR NORMAL NOUNS */
    /* NN MIXED NOUNS */
    OCT(64000),          /* 40 TIME TO IGNITION/CUTOFF */
                         /* VG */
                         /* DELTA V (ACCUMULATED) */
    OCT(02003),          /* 41 TARGET AZIMUTH */
                         /* ELEVATION */
    OCT(24006),          /* 42 APOGEE */
                         /* PERIGEE */
                         /* DELTA V (REQUIRED) */
    OCT(24011),          /* 43 LATITUDE */
                         /* LONGITUDE */
                         /* ALTITUDE */
    OCT(64014),          /* 44 APOGEE */
                         /* PERIGEE */
                         /* TFF */
    OCT(64017),          /* 45 MARKS (VHF - OPTICS) */
                         /* TTI OF NEXT BURN */
                         /* MGA */
    OCT(02022),          /* 46 AUTOPILOT CONFIGURATION */
    OCT(22025),          /* 47 THIS VEHICLE WEIGHT */
                         /* OTHER VEHICLE WEIGHT */
    OCT(22030),          /* 48 PITCH TRIM */
                         /* YAW TRIM */
    OCT(24033),          /* 49 DELTA R */
                         /* DELTA V */
                         /* VHF OR OPTICS CODE */
    OCT(64036),          /* 50 SPLASH ERROR */
                         /* PERIGEE */
                         /* TFF */
    OCT(22041),          /* 51 S-BAND ANTENNA PITCH */
                         /* YAW */
    OCT(00044),          /* 52 CENTRAL ANGLE OF ACTIVE VEHICLE */
    OCT(24047),          /* 53 RANGE */
                         /* RANGE RATE */
                         /* PHI */
    OCT(24052),          /* 54 RANGE */
                         /* RANGE RATE */
                         /* THETA */
    OCT(24055),          /* 55 PERIGEE CODE */
                         /* ELEVATION ANGLE */
                         /* CENTRAL ANGLE */
    OCT(22060),          /* 56 REENTRY ANGLE, */
                         /* DELTA V */
    OCT(20063),          /* 57 DELTA R */
    OCT(24066),          /* 58 PERIGEE ALT */
                         /* DELTA V TPI */
                         /* DELTA V TPF */
    OCT(24071),          /* 59 DELTA VELOCITY LOS */
    OCT(24074),          /* 60 GMAX */
                         /* VPRED */
                         /* GAMMA EI */
    OCT(24077),          /* 61 IMPACT LATITUDE */
                         /* IMPACT LONGITUDE */
                         /* HEADS UP/DOWN */
    OCT(24102),          /* 62 INERTIAL VEL MAG (V1) */
                         /* ALT RATE CHANGE (HDOT) */
                         /* ALT ABOVE PAD RADIUS (H) */
    OCT(64105),          /* 63 RANGE 297,431 TO SPLASH (RTGO) */
                         /* PREDICTED INERT VEL (VIO) */
                         /* TIME TO GO TO 297,431 (TTE) */
    OCT(24110),          /* 64 DRAG ACCELERATION */
                         /* INERTIAL VELOCITY (VI) */
                         /* RANGE TO SPLASH */
    OCT(24113),          /* 65 SAMPLED AGC TIME (HRS,MIN,SEC) */
                         /* (FETCHED IN INTERRUPT) */
    OCT(24116),          /* 66 COMMAND BANK ANGLE (BETA) */
                         /* CROSS RANGE ERROR */
                         /* DOWN RANGE ERROR */
    OCT(24121),          /* 67 RANGE TO TARGET */
                         /* PRESENT LATITUDE */
                         /* PRESENT LONGITUDE */
    OCT(24124),          /* 68 COMMAND BANK ANGLE (BETA) */
                         /* INERTIAL VELOCITY (VI) */
                         /* ALT RATE CHANGE (RDOT) */
    OCT(24127),          /* 69 BETA */
                         /* DL */
                         /* VL */
    OCT(04132),          /* 70 STAR CODE */
                         /* LANDMARK DATA */
                         /* HORIZON DATA */
    OCT(04135),          /* 71 STAR CODE */
                         /* LANDMARK */
                         /* HORIZON */
    OCT(24140),          /* 72 DELT ANG */
                         /* DELT ALT */
                         /* SEARCH OPTION */
    OCT(04143),          /* 73 ALTITUDE */
                         /* VELOCITY */
                         /* FLIGHT PATH ANGLE */
    OCT(04146),          /* 74 COMMAND BANK ANGLE (BETA) */
                         /* INERTIAL VELOCITY (VI) */
                         /* DRAG ACCELERATION */
    OCT(64151),          /* 75 DELTA ALTITUDE CDH */
                         /* DELTA TIME (CDH-CSI OR TPI-CDH) */
                         /* DELTA TIME (TPI-CDH OR TPI-NOMTPI) */
    OCT(0),              /* 76 SPARE */
    OCT(0),              /* 77 SPARE */
    OCT(0),              /* 78 SPARE */
    OCT(0),              /* 79 SPARE */
    OCT(64170),          /* 80 TIME TO IGNITION/CUTOFF */
                         /* VG */
                         /* DELTA V (ACCUMULATED) */
    OCT(24173),          /* 81 DELTA V (LV) */
    OCT(24176),          /* 82 DELTA V (LV) */
    OCT(24201),          /* 83 DELTA V (BODY) */
    OCT(24204),          /* 84 DELTA V (OTHER VEHICLE) */
    OCT(24207),          /* 85 VG (BODY) */
    OCT(24212),          /* 86 DELTA V (LV) */
    OCT(02215),          /* 87 MARK DATA SHAFT */
                         /* TRUNION */
    OCT(24220),          /* 88 HALF UNIT SUN OR PLANET VECTOR */
    OCT(24223),          /* 89 LANDMARK LATITUDE */
                         /* LONGITUDE/2 */
                         /* ALTITUDE */
    OCT(24226),          /* 90 Y */
                         /* Y DOT */
                         /* PSI */
    OCT(02231),          /* 91 OCDU ANGLES SHAFT */
                         /* TRUNION */
    OCT(02234),          /* 92 NEW OPTICS ANGLES SHAFT */
                         /* TRUNION */
    OCT(04237),          /* 93 DELTA GYRO ANGLES */
    OCT(02242),          /* 94 NEW OPTICS ANGLES SHAFT */
                         /* TRUNNION */
    OCT(04245),          /* 95 PREFERRED ATTITUDE ICDU ANGLES */
    OCT(04250),          /* 96 +X-AXIS ATTITUDE ICDU ANGLES */
    OCT(04253),          /* 97 SYSTEM TEST INPUTS */
    OCT(04256),          /* 98 SYSTEM TEST RESULTS */
    OCT(24261),          /* 99 RMS IN POSITION */
                         /* RMS IN VELOCITY */
                         /* RMS OPTION */
    /* END OF NNADTAB FOR MIXED NOUNS */
    /* NN NORMAL NOUNS */
};
/* clang-format on */

/* NNTYPTAB -- CM pp. 275-277: each noun's type: for a normal noun the number of components
 * (bits 15-11) and the scale factor routine and constant (bits 10-6 and
 * 5-1); for a mixed noun the scale factor constants of its components. */
/* clang-format off */
const word NNTYPTAB[100] = {
    OCT(00000), /* 00 NOT IN USE */
    OCT(04040), /* 01 3COMP FRACTIONAL */
    OCT(04140), /* 02 3COMP WHOLE */
    OCT(04102), /* 03 3COMP CDU DEGREES */
    OCT(0),     /* 04 SPARE */
    OCT(00504), /* 05 1COMP DPDEG(360) */
    OCT(02000), /* 06 2COMP OCTAL ONLY */
    OCT(04000), /* 07 3COMP OCTAL ONLY */
    OCT(04000), /* 08 3COMP OCTAL ONLY */
    OCT(04000), /* 09 3COMP OCTAL ONLY */
    OCT(00000), /* 10 1COMP OCTAL ONLY */
    OCT(24400), /* 11 3COMP HMS (DEC ONLY) */
    OCT(02000), /* 12 2COMP OCTAL ONLY */
    OCT(24400), /* 13 3COMP HMS (DEC ONLY) */
    OCT(0),     /* 14 SPARE */
    OCT(00000), /* 15 1COMP OCTAL ONLY */
    OCT(24400), /* 16 3COMP HMS (DEC ONLY) */
    OCT(04102), /* 17 3COMP CDU DEG */
    OCT(04102), /* 18 3COMP CDU DEG */
    OCT(04102), /* 19 3COMP CDU DEG */
    OCT(04102), /* 20 3COMP CDU DEGREES */
    OCT(04140), /* 21 3COMP WHOLE */
    OCT(04102), /* 22 3COMP CDU DEGREES */
    OCT(00000), /* 23 SPARE */
    OCT(24400), /* 24 3COMP HMS (DEC ONLY) */
    OCT(04140), /* 25 3COMP WHOLE */
    OCT(04000), /* 26 3COMP OCTAL ONLY */
    OCT(00140), /* 27 1COMP WHOLE */
    OCT(00000), /* 28 SPARE */
    OCT(20102), /* 29 1COMP CDU DEG (DEC ONLY) */
    OCT(04140), /* 30 3COMP WHOLE */
    OCT(24400), /* 31 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 32 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 33 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 34 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 35 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 36 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 37 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 38 3COMP HMS (DEC ONLY) */
    OCT(24400), /* 39 3COMP HMS (DEC ONLY) */
    /* END OF NNTYPTAB FOR NORMAL NOUNS */
    /* NN MIXED NOUNS */
    OCT(24500), /* 40 3COMP MIN/SEC, VEL3, VEL3 */
                /* (NO LOAD, DEC ONLY) */
    OCT(00542), /* 41 2COMP CDU DEG, ELEV DEG */
    OCT(24410), /* 42 3COMP POS4, POS4, VEL3 */
                /* (DEC ONLY) */
    OCT(20204), /* 43 3COMP DPDEG(360), DPDEG(360), POS4 */
                /* (DEC ONLY) */
    OCT(00410), /* 44 3COMP POS4, POS4, MIN/SEC */
                /* (NO LOAD, DEC ONLY) */
    OCT(10000), /* 45 3COMP 2INT, MIN/SEC, DPDEG(360) */
                /* (NO LOAD, DEC ONLY) */
    OCT(00000), /* 46 2COMP OCTAL ONLY FOR EACH */
    OCT(00306), /* 47 2COMP WEIGHT2 FOR EACH */
                /* (DEC ONLY) */
    OCT(00614), /* 48 2COMP TRIM DEG, TRIM DEG */
                /* (DEC ONLY) */
    OCT(00510), /* 49 3COMP POS4, VEL3, WHOLE */
                /* (DEC ONLY) */
    OCT(00417), /* 50 3COMP POS6, POS4, MIN/SEC */
                /* (NO LOAD, DEC ONLY) */
    OCT(00204), /* 51 2COMP DPDEG(360), DPDEG(360) */
                /* (DEC ONLY) */
    OCT(00004), /* 52 1COMP DPDEG(360) */
    OCT(10507), /* 53 3COMP POS5, VEL3, DPDEG(360) */
                /* (DEC ONLY) */
    OCT(10507), /* 54 3COMP POS5, VEL3, DPDEG(360) */
                /* (DEC ONLY) */
    OCT(10200), /* 55 3COMP WHOLE, DPDEG(360), DPDEG(360) */
                /* (DEC ONLY) */
    OCT(00444), /* 56 2COMP DPDEG(360), VEL2 */
                /* (DEC ONLY) */
    OCT(00010), /* 57 1COMP POS4 */
                /* (DEC ONLY) */
    OCT(24510), /* 58 3COMP POS4, VEL3, VEL3 */
                /* (DEC ONLY) */
    OCT(24512), /* 59 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(10440), /* 60 3COMP WHOLE, VEL2, DPDEG(360) */
                /* (DEC ONLY) */
    OCT(00204), /* 61 3COMP DPDEG(360), DPDEG(360), WHOLE */
                /* (DEC ONLY) */
    OCT(20451), /* 62 3COMP VEL2, VEL2, POS4 */
                /* (DEC ONLY) */
    OCT(00457), /* 63 3COMP POS6, VEL2, MIN/SEC */
                /* (NO LOAD, DEC ONLY) */
    OCT(36460), /* 64 3COMP DRAG ACCEL, VEL2, POS6 */
                /* (DEC ONLY) */
    OCT(00000), /* 65 3COMP HMS (DEC ONLY) */
    OCT(37044), /* 66 3COMP DPDEG(360), POS8, POS6 */
                /* (DEC ONLY) */
    OCT(10217), /* 67 3COMP POS6, DPDEG(360), DPDEG(360) */
                /* (DEC ONLY) */
    OCT(34444), /* 68 3COMP DPDEG(360), VEL2, VEL/2VS */
                /* (DEC ONLY) */
    OCT(35004), /* 69 3COMP DPDEG(360), DRAG ACCEL,VEL/2VS */
                /* (DEC ONLY) */
    OCT(00000), /* 70 3COMP OCTAL ONLY FOR EACH */
    OCT(0),     /* 71 3COMP OCTAL ONLY FOR EACH */
    OCT(00404), /* 72 3COMP DPDEG(360), POS4, WHOLE */
                /* (DEC ONLY) */
    OCT(10450), /* 73 3COMP POS4, VEL2, DPDEG(360) */
    OCT(40444), /* 74 3COMP DPDEG(360), VEL2, DRAG ACCEL */
    OCT(00010), /* 75 3COMP POS4, MIN/SEC, MIN/SEC */
                /* # (NO LOAD, DEC ONLY) */
    OCT(0),     /* 76 SPARE */
    OCT(0),     /* 77 SPARE */
    OCT(0),     /* 78 SPARE */
    OCT(0),     /* 79 SPARE */
    OCT(22440), /* 80 3COMP MIN/SEC, VEL2, VEL2 */
                /* (NO LOAD, DEC ONLY) */
    OCT(24512), /* 81 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 82 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 83 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 84 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(24512), /* 85 3COMP VEL3 FOR EACH */
                /* (DEC ONLY) */
    OCT(22451), /* 86 3COMP VEL2 FOR EACH */
                /* (DEC ONLY) */
    OCT(00102), /* 87 2COMP CDU DEG, Y OPTICS DEG */
    OCT(0),     /* 88 3COMP FRAC FOR EACH */
                /* (DEC ONLY) */
    OCT(16143), /* 89 3COMP DPDEG(90), DPDEG(90), POS5 */
                /* (DEC ONLY) */
    OCT(10507), /* 90 3COMP POS5, VEL3, DEPDEG(360) */
                /* (DEC ONLY) */
    OCT(00102), /* 91 2COMP CDUDEG, YOPTICS DEG */
    OCT(00102), /* 92 2COMP CDUDEG, YOPTICS DEG */
    OCT(06143), /* 93 3COMP DPDEG(90) FOR EACH */
    OCT(00102), /* 94 2COMP CDUDEG, YOPTICS DEG */
    OCT(04102), /* 95 3COMP CDU DEG FOR EACH */
    OCT(04102), /* 96 3COMP CDU DEG FOR EACH */
    OCT(00000), /* 97 3COMP WHOLE FOR EACH */
    OCT(00000), /* 98 3COMP WHOLE, FRAC, WHOLE */
    OCT(01162), /* 99 3COMP POS9, VEL4, WHOLE */
                /* (DEC ONLY) */
    /* END OF NNTYPTAB FOR MIXED NOUNS */
};
/* clang-format on */

/* SFINTAB -- CM pp. 277-278: the scale factors from the decimal the crew loads into the
 * machine's units, a double word each, indexed by twice the scale factor
 * constant code (2X(SFCONUM)). */
/* clang-format off */
const word SFINTAB[40] = {
    OCT(00006), /* WHOLE, DP TIME (SEC) */
    OCT(03240),
    OCT(00000), /* SPARE */
    OCT(00000),
    OCT(00000), /* CDU DEGREES, Y OPTICS DEGREES */
    OCT(00000), /* (SFCONS IN DEGINSF, OPTDEGIN) */
    OCT(10707), /* DP DEGREES (90) */
    OCT(03435), /* UPPED BY 1 */
    OCT(13070), /* DP DEGREES (360)(POINT BETWN BITS 11-12) */
    OCT(34345), /* UPPED BY 1 */
    OCT(00005), /* DEGREES (180) */
    OCT(21616),
    OCT(26113), /* WEIGHT2 */
    OCT(31713),
    OCT(00070), /* POSITION5 */
    OCT(20460),
    OCT(01065), /* POSITION4 */
    OCT(05740),
    OCT(11414), /* VELOCITY2 (POINT BETWN BITS 11-12) */
    OCT(31463),
    OCT(07475), /* VELOCITY3 */
    OCT(16051),
    OCT(00001), /* ELEVATION DEGREES */
    OCT(03434),
    OCT(00002), /* TRIM DEGREES */
    OCT(22245),
    OCT(00014), /* INERTIA, THRUST MOMENT */
    OCT(35607),
    OCT(07606), /* VELOCITY/2VS */
    OCT(06300),
    OCT(16631), /* POSITION 6 */
    OCT(11307),
    OCT(12000), /* DRAG ACCELERATION (POINT BETWN BITS 7-8) */
    OCT(00000),
    OCT(27176), /* POSITION 8 */
    OCT(14235),
    OCT(01670), /* 2DEC 30480 B-19: POSITION 9 */
    OCT(20000),
    OCT(07475), /* 2DEC 30.48 B-7: VELOCITY4 */
    OCT(16051),
    /* END OF SFINTAB */
};
/* clang-format on */

/* SFOUTAB -- CM pp. 278-279: the scale factors from the machine's units to the decimal
 * displayed, a double word each, by the same code. */
/* clang-format off */
const word SFOUTAB[40] = {
    OCT(05174), /* WHOLE, DP TIME (SEC) */
    OCT(13261),
    OCT(00000), /* SPARE */
    OCT(00000),
    OCT(00000), /* CDU DEGREES, Y OPTICS DEGREES */
    OCT(00000), /* (SFCONS IN DEGOUTSF, OPTDEGOUT) */
    OCT(00714), /* DP DEGREES (90) (POINT BETWN BITS 7-8) */
    OCT(31463),
    OCT(13412), /* DP DEGREES (360) */
    OCT(07534),
    OCT(05605), /* DEGREES (180) */
    OCT(03656),
    OCT(00001), /* WEIGHT2 */
    OCT(16170),
    OCT(00441), /* POSITION5 */
    OCT(34306),
    OCT(07176), /* POSITION4 */
    OCT(21603),
    OCT(15340), /* VELOCITY2 */
    OCT(15340),
    OCT(01031), /* VELOCITY3 (POINT BETWN BITS 7-8) */
    OCT(21032),
    OCT(34631), /* ELEVATION DEGREES */
    OCT(23146),
    OCT(14340), /* TRIM DEGREES */
    OCT(24145),
    OCT(02363), /* INERTIA, THRUST MOMENT */
    OCT(03721),
    OCT(20373), /* VELOCITY/ZVS */
    OCT(02122),
    OCT(00424), /* POSITION 6 (POINT BETWN BITS 7-8) */
    OCT(30446),
    OCT(00631), /* DRAG ACCELERATION */
    OCT(23146),
    OCT(00260), /* POSITION 8 */
    OCT(06213),
    OCT(04231), /* 2DEC 17.2010499 B-7: POSITION 9 */
    OCT(27400),
    OCT(01031), /* 2DEC .032808399: VELOCITY4 */
    OCT(21032),
    /* END OF SFOUTAB */
    /* NN SF CONSTANT SF ROUTINE */
};
/* clang-format on */

/* IDADDTAB -- CM pp. 279-283: the ECADRs of a mixed noun's three components, three words
 * per noun, with the scale factor constant and routine each one takes. */
/* clang-format off */
const word IDADDTAB[180] = {
    ECADR(TTOGO),         /* 40 MIN/SEC M/S */
    ECADR(VGDISP),        /* 40 VEL3 DP3 */
    ECADR(DVTOTAL),       /* 40 VEL3 DP3 */
    ECADR(DSPTEM1),       /* 41 CDU DEG CDU */
    ECADR(DSPTEM1) + 1u,  /* 41 ELEV DEG ARTH */
    OCT(0),               /* 41 SPARE COMPONENT */
    ECADR(HAPO),          /* 42 POS4 DP3 */
    ECADR(HPER),          /* 42 POS4 DP3 */
    ECADR(VGDISP),        /* 42 VEL3 DP3 */
    ECADR(LAT),           /* 43 DPDEG(360) DP4 */
    ECADR(LONG),          /* 43 DPDEG(360) DP4 */
    ECADR(ALT),           /* 43 POS4 DP3 */
    ECADR(HAPOX),         /* 44 POS4 DP3 */
    ECADR(HPERX),         /* 44 POS4 DP3 */
    ECADR(TFF),           /* 44 MIN/SEC M/S */
    ECADR(VHFCNT),        /* 45 2INT 2INT */
    ECADR(TTOGO),         /* 45 MIN/SEC M/S */
    ECADR(PLUS_MGA),      /* 45 DPDEG(360) DP4 */
    ECADR(DAPDATR1),      /* 46 OCTAL ONLY OCT */
    ECADR(DAPDATR2),      /* 46 OCATAL ONLY OCT */
    OCT(0),               /* 46 SPARE COMPONENT */
    ECADR(CSMMASS),       /* 47 WEIGHT2 ARTH1 */
    ECADR(LEMMASS),       /* 47 WEIGHT2 ARTH1 */
    OCT(00000),           /* 47 SPARE COMPONENT */
    ECADR(PACTOFF),       /* 48 TRIM DEG ARTH */
    ECADR(YACTOFF),       /* 48 TRIM DEG ARTH */
    OCT(00000),           /* 48 SPARE COMPONENT */
    ECADR(N49DISP),       /* 49 POS4 DP3 */
    ECADR(N49DISP) + 2u,  /* 49 VEL3 DP3 */
    ECADR(N49DISP) + 4u,  /* 49 WHOLE ARTH */
    ECADR(RSP_RREC),      /* 50 POS6 DP3 */
    ECADR(HPERX),         /* 50 POS4 DP3 */
    ECADR(TFF),           /* 50 MIN/SEC M/S */
    ECADR(RHOSB),         /* 51 DPDEG(360) */
    ECADR(GAMMASB),       /* 51 DPDEG(360) DP4 */
    OCT(0),               /* 51 SPARE COMPONENT */
    ECADR(ACTCENT),       /* 52 DPDEG(360) DP4 */
    OCT(00000),           /* 52 SPARE COMPONENT */
    OCT(00000),           /* 52 SPARE COMPONENT */
    ECADR(RANGE),         /* 53 POS5 DP1 */
    ECADR(RRATE),         /* 53 VEL3 DP3 */
    ECADR(RTHETA),        /* 53 DPDEG(360) DP4 */
    ECADR(RANGE),         /* 54 POS5 DP1 */
    ECADR(RRATE),         /* 54 VEL3 DP3 */
    ECADR(RTHETA),        /* 54 DPDEG(360) DP4 */
    ECADR(NN1),           /* 55 WHOLE ARTH */
    ECADR(ELEV),          /* 55 DPDEG(360) DP4 */
    ECADR(CENTANG),       /* 55 DPDEG(360) DP4 */
    ECADR(RTEGAM2D),      /* 56 DPDEG(360) DP4 */
    ECADR(RTEDVD),        /* 56 VEL2 DP4 */
    OCT(0),               /* 56 SPARE COMPONENT */
    ECADR(DELTAR),        /* 57 POS4 DP3 */
    OCT(0),               /* 57 SPARE COMPONENT */
    OCT(0),               /* 57 SPARE COMPONENT */
    ECADR(POSTTPI),       /* 58 POS4 DP3 */
    ECADR(DELVTPI),       /* 58 VEL3 DP3 */
    ECADR(DELVTPF),       /* 58 VEL3 DP3 */
    ECADR(DVLOS),         /* 59 VEL3 DP3 */
    ECADR(DVLOS) + 2u,    /* 59 VEL3 DP3 */
    ECADR(DVLOS) + 4u,    /* 59 VEL3 DP3 */
    ECADR(GMAX),          /* 60 WHOLE ARTH */
    ECADR(VPRED),         /* 60 VEL2 DP4 */
    ECADR(GAMMAEI),       /* 60 DPDEG(360) DP4 */
    ECADR(LAT_SPL),       /* 61 DPDEG(360) DP4 */
    ECADR(LNG_SPL),       /* 61 DPDEG(360) DP4 */
    ECADR(HEADSUP),       /* 61 WHOLE ARTH */
    ECADR(VMAGI),         /* 62 VEL2 DP4 */
    ECADR(HDOT),          /* 62 VEL2 DP4 */
    ECADR(ALTI),          /* 62 POS4 DP3 */
    ECADR(RTGO),          /* 63 POS6 DP3 */
    ECADR(VIO),           /* 63 VEL2 DP4 */
    ECADR(TTE),           /* 63 MIN/SEC M/S */
    ECADR(D),             /* 64 DRAG ACCEL DP2 */
    ECADR(VMAGI),         /* 64 VEL2 DP4 */
    ECADR(RTGON64),       /* 64 POS6 DP3 */
    ECADR(SAMPTIME),      /* 65 HMS (MIXED ONLY TO KEEP CODE 65) HMS */
    ECADR(SAMPTIME),      /* 65 HMS HMS */
    ECADR(SAMPTIME),      /* 65 HMS HMS */
    ECADR(ROLLC),         /* 66 DPDEG(360) DP4 */
    ECADR(XRNGERR),       /* 66 POS8 DP3 */
    ECADR(DNRNGERR),      /* 66 POS6 DP3 */
    ECADR(RTGON67),       /* 67 POS6 DP3 */
    ECADR(LAT),           /* 67 DPDEG(360) DP4 */
    ECADR(LONG),          /* 67 DPDEG(360) DP4 */
    ECADR(ROLLC),         /* 68 DPDEG(360) DP4 */
    ECADR(VMAGI),         /* 68 VEL2 DP4 */
    ECADR(RDOT),          /* 68 VEL/2VS DP4 */
    ECADR(ROLLC),         /* 69 DPDEG(360) DP4 */
    ECADR(Q7),            /* 69 DRAG ACCEL DP2 */
    ECADR(VL),            /* 69 VEL/2VS DP4 */
    ECADR(STARCODE),      /* 70 OCTAL ONLY OCT */
    ECADR(LANDMARK),      /* 70 OCTAL ONLY OCT */
    ECADR(HORIZON),       /* 70 OCTAL ONLY OCT */
    ECADR(STARCODE),      /* 71 OCTAL ONLY OCT */
    ECADR(LANDMARK),      /* 71 OCTAL ONLY OCT */
    ECADR(HORIZON),       /* 71 OCTAL ONLY OCT */
    ECADR(THETZERO),      /* 72 DPDEG(360) DP4 */
    ECADR(DELHITE),       /* 72 POS4 DP3 */
    ECADR(OPTION2),       /* 72 WHOLE ARTH */
    ECADR(P21ALT),        /* 73 POS4 DP3 */
    ECADR(P21VEL),        /* 73 VEL2 DP4 */
    ECADR(P21GAM),        /* 73 DPDEG(360) DP4 */
    ECADR(ROLLC),         /* 74 DPDEG(360) DP4 */
    ECADR(VMAGI),         /* 74 VEL2 DP4 */
    ECADR(D),             /* 74 DRAG ACCEL DP2 */
    ECADR(DIFFALT),       /* 75 POS4 DP3 */
    ECADR(T1TOT2),        /* 75 MIN/SEC M/S */
    ECADR(T2TOT3),        /* 75 MIN/SEC M/S */
    OCT(0),               /* 76 SPARE */
    OCT(0),               /* 76 SPARE */
    OCT(0),               /* 76 SPARE */
    OCT(0),               /* 77 SPARE */
    OCT(0),               /* 77 SPARE */
    OCT(0),               /* 77 SPARE */
    OCT(0),               /* 78 SPARE */
    OCT(0),               /* 78 SPARE */
    OCT(0),               /* 78 SPARE */
    OCT(0),               /* 79 SPARE */
    OCT(0),               /* 79 SPARE */
    OCT(0),               /* 79 SPARE */
    ECADR(TTOGO),         /* 80 MIN/SEC M/S */
    ECADR(VGDISP),        /* 80 VEL2 DP4 */
    ECADR(DVTOTAL),       /* 80 VEL2 DP4 */
    ECADR(DELVLVC),       /* 81 VEL3 DP3 */
    ECADR(DELVLVC) + 2u,  /* 81 VEL3 DP3 */
    ECADR(DELVLVC) + 4u,  /* 81 VEL3 DP3 */
    ECADR(DELVLVC),       /* 82 VEL3 DP3 */
    ECADR(DELVLVC) + 2u,  /* 82 VEL3 DP3 */
    ECADR(DELVLVC) + 4u,  /* 82 VEL3 DP3 */
    ECADR(DELVIMU),       /* 83 VEL3 DP3 */
    ECADR(DELVIMU) + 2u,  /* 83 VEL3 DP3 */
    ECADR(DELVIMU) + 4u,  /* 83 VEL3 DP3 */
    ECADR(DELVOV),        /* 84 VEL3 DP3 */
    ECADR(DELVOV) + 2u,   /* 84 VEL3 DP3 */
    ECADR(DELVOV) + 4u,   /* 84 VEL3 DP3 */
    ECADR(VGBODY),        /* 85 VEL3 DP3 */
    ECADR(VGBODY) + 2u,   /* 85 VEL3 DP3 */
    ECADR(VGBODY) + 4u,   /* 85 VEL3 DP3 */
    ECADR(DELVLVC),       /* 86 VEL2 DP4 */
    ECADR(DELVLVC) + 2u,  /* 86 VEL2 DP4 */
    ECADR(DELVLVC) + 4u,  /* 86 VEL2 DP4 */
    ECADR(MRKBUF1) + 3u,  /* 87 CDU DEG CDU */
    ECADR(MRKBUF1) + 5u,  /* 87 Y OPTICS DEG YOPT */
    OCT(0),               /* 87 SPARE COMPONENT */
    ECADR(STARSAV3),      /* 88 DPFRAC DPFRAC */
    ECADR(STARSAV3) + 2u, /* 88 DPFRAC DPFRAC */
    ECADR(STARSAV3) + 4u, /* 88 DPFRAC DPFRAC */
    ECADR(LANDLAT),       /* 89 DPDEG(90) DP3 */
    ECADR(LANDLONG),      /* 89 DPDEG(90) DP3 */
    ECADR(LANDALT),       /* 89 POS5 DP1 */
    ECADR(RANGE),         /* 90 POS5 DP1 */
    ECADR(RRATE),         /* 90 VEL3 DP3 */
    ECADR(RTHETA),        /* 90 DPDEG(360) DP4 */
    ECADR(CDUS),          /* 91 CDU DEG CDU */
    ECADR(CDUT),          /* 91 Y OPTICS DEG YOPT */
    OCT(0),               /* 91 SPARE COMPONENT */
    ECADR(SAC),           /* 92 CDU DEG CDU */
    ECADR(PAC),           /* 92 Y OPTICS DEG YOPT */
    OCT(0),               /* 92 SPARE COMPONENT */
    ECADR(OGC),           /* 93 DPDEG(90) DP3 */
    ECADR(OGC) + 2u,      /* 93 DPDEG(90) DP3 */
    ECADR(OGC) + 4u,      /* 93 DPDEG(90) DP3 */
    ECADR(MRKBUF1) + 3u,  /* 94 CDU DEG CDU */
    ECADR(MRKBUF1) + 5u,  /* 94 Y OPTICS DEG YOPT */
    OCT(00000),           /* 94 SPARE */
    ECADR(PRAXIS),        /* 95 CDU DEG CDU */
    ECADR(PRAXIS) + 1u,   /* 95 CDU DEG CDU */
    ECADR(PRAXIS) + 2u,   /* 95 CDU DEG CDU */
    ECADR(CPHIX),         /* 96 CDU DEG CDU */
    ECADR(CPHIX) + 1u,    /* 96 CDU DEG CDU */
    ECADR(CPHIX) + 2u,    /* 96 CDU DEG CDU */
    ECADR(DSPTEM1),       /* 97 WHOLE ARTH */
    ECADR(DSPTEM1) + 1u,  /* 97 WHOLE ARTH */
    ECADR(DSPTEM1) + 2u,  /* 97 WHOLE ARTH */
    ECADR(DSPTEM2),       /* 98 WHOLE ARTH */
    ECADR(DSPTEM2) + 1u,  /* 98 FRAC FRAC */
    ECADR(DSPTEM2) + 2u,  /* 98 WHOLE ARTH */
    ECADR(WWPOS),         /* 99 POS9 DP3 */
    ECADR(WWVEL),         /* 99 VEL4 DP2 */
    ECADR(WWOPT),         /* 99 WHOLE ARTH */
    /* END OF IDADDTAB */
    /* NN SF ROUTINES */
};
/* clang-format on */

/* RUTMXTAB -- CM pp. 283-284: each mixed noun's scale factor routines, three 5-bit codes
 * (bits 15-11, 10-6, 5-1 for components 3, 2, 1). */
/* clang-format off */
const word RUTMXTAB[60] = {
    OCT(16351), /* 40 M/S, DP3, DP3 */
    OCT(00142), /* 41 CDU, ARTH */
    OCT(16347), /* 42 DP3, DP3, DP3 */
    OCT(16512), /* 43 DP4, DP4, DP3 */
    OCT(22347), /* 44 DP3, DP3, M/S */
    OCT(24454), /* 45 2INT, M/S, DP4 */
    OCT(00000), /* 46 OCT, OCT */
    OCT(00553), /* 47 ARITH1, ARITH1 */
    OCT(00143), /* 48 ARTH, ARTH */
    OCT(06347), /* 49 DP3, DP3, ARTH */
    OCT(22347), /* 50 DP3, DP3, M/S */
    OCT(00512), /* 51 DP4, DP4 */
    OCT(00012), /* 52 DP4 */
    OCT(24344), /* 53 DP1, DP3, DP4 */
    OCT(24344), /* 54 DP1, DP3, DP4 */
    OCT(24503), /* 55 ARTH, DP4 , DP4 */
    OCT(00512), /* 56 DP4, DP4 */
    OCT(00007), /* 57 DP3 */
    OCT(16347), /* 58 DP3, DP3, DP3 */
    OCT(16347), /* 59 DP3, DP3, DP3 */
    OCT(24503), /* 60 ARTH, DP4, DP4 */
    OCT(06512), /* 61 DP4, DP4, ARTH */
    OCT(16512), /* 62 DP4, DP4, DP3 */
    OCT(22507), /* 63 DP3, DP4, M/S */
    OCT(16505), /* 64 DP2, DP4, DP3 */
    OCT(20410), /* 65 HMS, HMS, HMS */
    OCT(16352), /* 66 DP4, DP3, DP3 */
    OCT(24507), /* 67 DP3, DP4, DP4 */
    OCT(24512), /* 68 DP4, DP4, DP4 */
    OCT(24252), /* 69 DP4, DP2, DP4 */
    OCT(00000), /* 70 OCT, OCT, OCT */
    OCT(0),     /* 71 OCT, OCT, OCT */
    OCT(06352), /* 72 DP4, DP3, ARTH */
    OCT(24507), /* 73 DPR, DP4, DP4 */
    OCT(12512), /* 74 DP4, DP4, DP2 */
    OCT(22447), /* 75 DP3, M/S, M/S */
    OCT(0),     /* 76 SPARE */
    OCT(0),     /* 77 SPARE */
    OCT(0),     /* 78 SPARE */
    OCT(0),     /* 79 SPARE */
    OCT(24511), /* 80 M/S, DP4, DP4 */
    OCT(16347), /* 81 DP3, DP3, DP3 */
    OCT(16347), /* 82 DP3, DP3, DP3 */
    OCT(16347), /* 83 DP3, DP3, DP3 */
    OCT(16347), /* 84 DP3, DP3, DP3 */
    OCT(16347), /* 85 DP3, DP3, DP3 */
    OCT(24512), /* 86 DP4, DP4, DP4 */
    OCT(00302), /* 87 CDU, YOPT */
    OCT(32655), /* 88 DPFRAC FOR EACH */
    OCT(10347), /* 89 DP3, DP3, DP1 */
    OCT(24344), /* 90 DP1, DP3, DP4 */
    OCT(00302), /* 91 CDU, YOPT */
    OCT(00302), /* 92 CDU, YOPT */
    OCT(16347), /* 93 DP3, DP3, DP3 */
    OCT(00302), /* 94 CDU, YOPT */
    OCT(04102), /* 95 CDU, CDU, CDU */
    OCT(04102), /* 96 CDU, CDU, CDU */
    OCT(06143), /* 97 ARTH, ARTH, ARTH */
    OCT(06043), /* 98 ARTH, FRAC, ARTH */
    OCT(06247), /* 99 DP3, DP2, ARTH */
    /* END OF RUTMXTAB */
};
/* clang-format on */

#endif
