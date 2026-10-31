/* SPDX-License-Identifier: MIT */
/* DOWNLINK_LISTS.agc -- LM pp. 193-205, CM pp. 170-180.
 * The faithful port reads these words from its compiled listing data
 * (c64-port/src/lm_downtelemetry_data.inc, cm_downtelemetry_data.inc).
 *
 * The downlists: what DOWNRUPT (down-telemetry.c) sends to the ground. The
 * program chooses one with DNLSTCOD (an index into DNTABLE), and each list
 * is a control list of special words, sent one by one, two AGC words per
 * DOWNRUPT. The listing's table of them (p. 193; CM p. 170):
 *
 *   OP CODE  ADDRESS (EXAMPLE)  SENDS...                BITS 14-12  BITS 11-0
 *   1DNADR   TIME2              2 AGC words             0           ECADR
 *   2DNADR   TEPHEM             4 AGC words             1           ECADR
 *   ...
 *   6DNADR   DSPTAB             12 AGC words            5           ECADR
 *   DNPTR    NEXTLIST           points to the next list 6           ADRES
 *   DNCHAN   30                 channels 30 and 31      7           CHANNEL
 *
 * (A DNPTR is the instruction CA NEXTLIST: operation code 3 is bits 14-12
 * = 6.) Its rules:
 *   "1. END OF A LIST = -XDNADR (X = 1 TO 6), -DNPTR, OR -DNCHAN.
 *    2. SNAPSHOT SUBLIST = LIST WHICH STARTS WITH A -1DNADR.
 *    3. SNAPSHOT SUBLIST CAN ONLY CONTAIN 1DNADRS.
 *    4. TIME2 1DNADR MUST BE LOCATED IN THE CONTROL LIST OF A DOWNLIST."
 * So a word written with a leading "-" (complemented) ends its list, but a
 * sublist that starts with one is a snapshot: its words are copied into
 * DNTMBUFF all at once, so they belong to the same moment, and sent later
 * by the 6DNADR DNTMBUFF that follows the DNPTR.
 *
 * Here each list is written as the listing has it, word by word, with the
 * listing's comments; downlink_list_word() lays the words out from
 * LOWIDCOD, where the listing assembles them, and gives each one's value.
 * DNADR operands are ECADRs (E_<name>, from the assembled listing: the
 * E-bank and the address in the bank); E(R_OTHER) + 2 is the listing's
 * R-OTHER +2. The LM's UNKNOWN and both programs' SPARE are ERASZERO, the
 * word 7 that is always 0: E(SPARE). */
#include "downtelemetry.h"
#include "../agc/names.h"

/* The kinds of word in a list, by the listing's op code. A LABEL_ONLY is
 * not a word: it names the next word, as `LMORBMDL EQUALS` with no operand
 * does in the listing. */
enum kind { LABEL_ONLY, OCT_WORD, DNADR_WORD, DNCHAN_WORD, DNPTR_WORD, GENADR_WORD };

struct list_word {
    enum kind kind;
    bool minus;   /* written with a leading "-": the last word of its list */
    word count;   /* nDNADR: the n, how many double words (1 to 6) */
    word operand; /* the ECADR, channel, octal value or list */
};

#define E(name)               ((word)E_##name)
#define LIST(label)           {LABEL_ONLY, false, 0, label}
#define OCT(value)            {OCT_WORD, false, 0, value}
#define DNADR(n, ecadr)       {DNADR_WORD, false, n, ecadr}
#define MINUS_DNADR(n, ecadr) {DNADR_WORD, true, n, ecadr}
#define DNCHAN(channel)       {DNCHAN_WORD, false, 0, channel}
#define MINUS_DNCHAN(channel) {DNCHAN_WORD, true, 0, channel}
#define DNPTR(list)           {DNPTR_WORD, false, 0, list}
#define GENADR(list)          {GENADR_WORD, false, 0, list}

#if LUMINARY099

/* The LM's lists, by label. */
enum list {
    LMORBMDL,
    LMORBM01,
    LMORBM02,
    LMORBM03,
    LMORBM04,
    LMORBM05,
    LMORBM06,
    LMCSTADL,
    LMCSTA06,
    LMCSTA07,
    LMRENDDL,
    LMREND07,
    LMDSASDL,
    LMDSAS07,
    LMDSAS08,
    LMLSALDL,
    LMAGSIDL,
    LISTS, /* how many labels */
    /* The names the listing gives the same lists (EQUALS). */
    NOMDNLST = LMCSTADL, /* FRESH START AND POST P27 DOWNLIST */
    AGSLIST = LMAGSIDL,
    UPDNLIST = LMAGSIDL, /* UPDATE PROGRAM (P27) DOWNLIST */
    LMCSTA01 = LMORBM01, /* COMMON DOWNLIST DATA */
    LMCSTA02 = LMORBM02, /* COMMON DOWNLIST DATA */
    LMCSTA03 = LMORBM03, /* COMMON DOWNLIST DATA */
    LMCSTA04 = LMORBM04, /* COMMON DOWNLIST DATA */
    LMCSTA05 = LMORBM05, /* COMMON DOWNLIST DATA */
    LMREND01 = LMORBM01, /* COMMON DOWNLIST DATA */
    LMREND02 = LMORBM02, /* COMMON DOWNLIST DATA */
    LMREND03 = LMORBM03, /* COMMON DOWNLIST DATA */
    LMREND04 = LMORBM04, /* COMMON DOWNLIST DATA */
    LMREND05 = LMORBM05, /* COMMON DOWNLIST DATA */
    LMREND06 = LMCSTA06, /* COMMON DOWNLIST DATA */
    LMDSAS02 = LMORBM02, /* COMMON DOWNLIST DATA */
    LMDSAS03 = LMORBM03, /* COMMON DOWNLIST DATA */
    LMDSAS04 = LMORBM04, /* COMMON DOWNLIST DATA */
    LMDSAS05 = LMORBM05, /* COMMON DOWNLIST DATA */
    LMDSAS06 = LMORBM06, /* COMMON DOWNLIST DATA */
    LMDSAS09 = LMCSTA06, /* COMMON DOWNLIST DATA */
    LMLSAL01 = LMORBM01, /* COMMON DOWNLIST DATA */
    LMLSAL02 = LMORBM02, /* COMMON DOWNLIST DATA */
    LMLSAL03 = LMORBM03, /* COMMON DOWNLIST DATA */
    LMLSAL04 = LMORBM04, /* COMMON DOWNLIST DATA */
    LMLSAL05 = LMORBM05, /* COMMON DOWNLIST DATA */
    LMLSAL06 = LMORBM06, /* COMMON DOWNLIST DATA */
    LMLSAL07 = LMREND07, /* COMMON DOWNLIST DATA */
    LMLSAL08 = LMCSTA06, /* COMMON DOWNLIST DATA */
    LMLSAL09 = LMCSTA07, /* COMMON DOWNLIST DATA */
    LMAGSI02 = LMORBM02, /* COMMON DOWNLIST DATA */
    LMAGSI03 = LMORBM03, /* COMMON DOWNLIST DATA */
    LMAGSI04 = LMORBM04, /* COMMON DOWNLIST DATA */
    LMAGSI05 = LMORBM05, /* COMMON DOWNLIST DATA */
};

static const struct list_word lists[] = {
    OCT(077340), /* LOWIDCOD: LOW ID CODE */

    /* ---- LM ORBITAL MANEUVERS LIST (p. 194) ---- */
    /* The control list. */
    LIST(LMORBMDL),        /* SEND ID BY SPECIAL CODING */
    DNPTR(LMORBM01),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNADR(1, E(DELLT4)),   /* DELLT4,+1 */
    DNADR(3, E(RTARG)),    /* RTARG,+1...+5 */
    DNADR(1, E(ELEV)),     /* ELEV,+1 */
    DNADR(1, E(TEVENT)),   /* TEVENT,+1 */
    DNADR(6, E(REFSMMAT)), /* REFSMMAT +0...+11D */
    DNADR(1, E(TCSI)),     /* TCSI,+1 */
    DNADR(3, E(DELVEET1)), /* DELVEET1 +0...+5 */
    DNADR(3, E(VGTIG)),    /* VGTIG +0...+5 */
    DNADR(1, E(DNLRVELZ)), /* DNLRVELZ,DNLRALT */
    DNADR(1, E(TPASS4)),   /* TPASS4,+1 */
    DNPTR(LMORBM02),       /* COMMON DATA */
    DNADR(1, E(TIME2)),    /* TIME2/1 */
    DNPTR(LMORBM03),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(LMORBM04),       /* COMMON DATA */
    DNADR(2, E(POSTORKU)), /* POSTORKU,NEGTORKU,POSTORKV,NEGTORKV */
    DNADR(1, E(SPARE)),
    DNADR(1, E(TCDH)),      /* TCDH,+1 */
    DNADR(3, E(DELVEET2)),  /* DELVEET2 +0...+5 */
    DNADR(1, E(TTPI)),      /* TTPI,+1 */
    DNADR(3, E(DELVEET3)),  /* DELVEET3 +0...+5 */
    DNADR(1, E(DNRRANGE)),  /* DNRRANGE,DNRRDOT */
    DNADR(2, E(DNLRVELX)),  /* DNLRVELX,DNLRVELY,DNLRVELZ,DNLRALT */
    DNADR(1, E(DIFFALT)),   /* DIFFALT,+1 */
    DNADR(1, E(LEMMASS)),   /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)),  /* IMODES30,IMODES33 */
    DNADR(1, E(TIG)),       /* TIG,+1 */
    DNPTR(LMORBM05),        /* COMMON DATA */
    DNPTR(LMORBM06),        /* COMMON DATA */
    DNADR(1, E(SPARE)),     /* FORMERLY PIF */
    MINUS_DNADR(1, E(TGO)), /* TGO,+1 */
    /* Its sublists. */
    LIST(LMORBM01),
    MINUS_DNADR(1, E(R_OTHER) + 2), /* R-OTHER +2,+3 SNAPSHOT */
    DNADR(1, E(R_OTHER) + 4),       /* R-OTHER +4,+5 */
    DNADR(1, E(V_OTHER)),           /* V-OTHER,+1 */
    DNADR(1, E(V_OTHER) + 2),       /* V-OTHER +2,+3 */
    DNADR(1, E(V_OTHER) + 4),       /* V-OTHER +4,+5 */
    DNADR(1, E(T_OTHER)),           /* T-OTHER,+1 */
    MINUS_DNADR(1, E(R_OTHER)),     /* R-OTHER +0,+1 */
    LIST(LMORBM02),
    DNADR(2, E(REDOCTR)),      /* REDOCTR,THETAD,+1,+2 COMMON DATA */
    DNADR(1, E(RSBBQ)),        /* RSBBQ,+1 */
    DNADR(2, E(OMEGAP)),       /* OMEGAP,OMEGAQ,OMEGAR,GARBAGE */
    DNADR(2, E(CDUXD)),        /* CDUXD,CDUYD,CDUZD,GARBAGE */
    DNADR(2, E(CDUX)),         /* CDUX,CDUY,CDUZ,CDUT */
    DNADR(6, E(STATE)),        /* STATE +0...+11D (FLAGWORDS) */
    MINUS_DNADR(6, E(DSPTAB)), /* DSPTAB TABLES */
    LIST(LMORBM03),
    MINUS_DNADR(1, E(RN) + 2), /* RN +2,+3 SNAPSHOT */
    DNADR(1, E(RN) + 4),       /* RN +4,+5 */
    DNADR(1, E(VN)),           /* VN,+1 */
    DNADR(1, E(VN) + 2),       /* VN +2,+3 */
    DNADR(1, E(VN) + 4),       /* VN +4,+5 */
    DNADR(1, E(PIPTIME)),      /* PIPTIME,+1 */
    MINUS_DNADR(1, E(RN)),     /* RN,+1 */
    LIST(LMORBM04),
    DNADR(2, E(OMEGAPD)),        /* OMEGAPD,OMEGAQD,OMEGARD,GARBAGE */
    DNADR(3, E(CADRFLSH)),       /* CADRFLSH,+1,+2,FAILREG,+1,+2 */
    MINUS_DNADR(1, E(RADMODES)), /* RADMODES,DAPBOOLS COMMON DATA */
    LIST(LMORBM05),
    DNADR(2, E(OMEGAP)),   /* OMEGAP,OMEGAQ,OMEGAR,GARBAGE */
    DNADR(2, E(CDUXD)),    /* CDUXD,CDUYD,CDUZD,GARBAGE */
    DNADR(2, E(CDUX)),     /* CDUX,CDUY,CDUZ,CDUT */
    DNADR(1, E(ALPHAQ)),   /* ALPHAQ,ALPHAR COMMON DATA */
    DNADR(1, E(POSTORKP)), /* POSTORKP,NEGTORKP */
    DNCHAN(011),           /* CHANNELS 11,12 */
    DNCHAN(013),           /* CHANNELS 13,14 */
    DNCHAN(030),           /* CHANNELS 30,31 */
    MINUS_DNCHAN(032),     /* CHANNELS 32,33 */
    LIST(LMORBM06),
    DNADR(1, E(PIPTIME1)),   /* PIPTIME,+1 COMMON DATA */
    MINUS_DNADR(3, E(DELV)), /* DELV +0...+5 */

    /* ---- LM COAST AND ALIGNMENT DOWNLIST (p. 196) ---- */
    /* The control list. */
    LIST(LMCSTADL),            /* SEND ID BY SPECIAL CODING */
    DNPTR(LMCSTA01),           /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNADR(1, E(AGSK)),         /* AGSK,+1 */
    DNADR(1, E(TALIGN)),       /* TALIGN,+1 */
    DNADR(2, E(POSTORKU)),     /* POSTORKU,NEGTORKU,POSTORKV,NEGTORKV */
    DNADR(1, E(DNRRANGE)),     /* DNRRANGE,DNRRDOT */
    DNADR(1, E(TEVENT)),       /* TEVENT,+1 */
    DNADR(6, E(REFSMMAT)),     /* REFSMMAT +0...+11D */
    DNADR(1, E(AOTCODE)),      /* AOTCODE,GARBAGE */
    DNADR(3, E(RLS)),          /* RLS +0...+5 */
    DNADR(2, E(DNLRVELX)),     /* DNLRVELX,DNLRVELY,DNLRVELZ,DNLRALT */
    DNPTR(LMCSTA06),           /* COMMON DATA */
    DNPTR(LMCSTA02),           /* COMMON DATA */
    DNADR(1, E(TIME2)),        /* TIME2/1 */
    DNPTR(LMCSTA03),           /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(LMCSTA04),           /* COMMON DATA */
    DNPTR(LMCSTA07),           /* COMMON DATA */
    DNADR(2, E(DNLRVELX)),     /* DNLRVELX,DNLRVELY,DNLRVELZ,DNLRALT */
    DNADR(2, E(CDUS)),         /* CDUS,PIPAX,PIPAY,PIPAZ */
    DNADR(1, E(LASTYCMD)),     /* LASTYCMD,LASTXCMD */
    DNADR(1, E(LEMMASS)),      /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)),     /* IMODES30,IMODES33 */
    DNADR(1, E(TIG)),          /* TIG,+1 */
    DNPTR(LMCSTA05),           /* COMMON DATA */
    MINUS_DNADR(6, E(DSPTAB)), /* DSPTAB +0...+11D TABLE */
    /* Its sublists. */
    /* LMCSTA01 EQUALS LMORBM01: COMMON DOWNLIST DATA */
    /* LMCSTA02 EQUALS LMORBM02: COMMON DOWNLIST DATA */
    /* LMCSTA03 EQUALS LMORBM03: COMMON DOWNLIST DATA */
    /* LMCSTA04 EQUALS LMORBM04: COMMON DOWNLIST DATA */
    /* LMCSTA05 EQUALS LMORBM05: COMMON DOWNLIST DATA */
    LIST(LMCSTA06),
    DNADR(2, E(X789)),           /* X789 +0...+3 COMMON DATA */
    MINUS_DNADR(1, E(LASTYCMD)), /* LASTYCMD,LASTXCMD */
    LIST(LMCSTA07),
    DNADR(3, E(OGC)),            /* OGC,+1,IGC,+1,MGC,+1 COMMON DATA */
    DNADR(1, E(BESTI)),          /* BESTI,BESTJ */
    DNADR(3, E(STARSAV1)),       /* STARSAV1 +0...+5 */
    MINUS_DNADR(3, E(STARSAV2)), /* STARSAV2 +0...+5 */

    /* ---- LM RENDEZVOUS AND PRE-THRUST DOWNLIST (p. 198) ---- */
    /* The control list. */
    LIST(LMRENDDL),        /* SEND ID BY SPECIAL CODING */
    DNPTR(LMREND01),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(LMREND07),       /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNADR(1, E(DELLT4)),   /* DELLT4,+1 */
    DNADR(3, E(RTARG)),    /* RTARG +0...+5 */
    DNADR(3, E(DELVSLV)),  /* DELVSLV +0...+5 */
    DNADR(1, E(TCSI)),     /* TCSI,+1 */
    DNADR(3, E(DELVEET1)), /* DELVEET +0...+5 */
    DNADR(1, E(SPARE)),
    DNADR(1, E(TPASS4)),   /* TPASS4,+1 */
    DNPTR(LMREND06),       /* COMMON DATA */
    DNPTR(LMREND02),       /* COMMON DATA */
    DNADR(1, E(TIME2)),    /* TIME2/1 */
    DNPTR(LMREND03),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(LMREND04),       /* COMMON DATA */
    DNADR(2, E(POSTORKU)), /* POSTORKU,NEGTORKU,POSTORKV,NEGTORKV */
    DNADR(1, E(SPARE)),
    DNADR(1, E(TCDH)),     /* TCDH,+1 */
    DNADR(3, E(DELVEET2)), /* DELVEET2 +0...+5 */
    DNADR(1, E(TTPI)),     /* TTPI,+1 */
    DNADR(3, E(DELVEET3)), /* DELVEET3 +0...+5 */
    DNADR(1, E(ELEV)),     /* ELEV,+1 */
    DNADR(2, E(CDUS)),     /* CDUS,PIPAX,PIPAY,PIPAZ */
    DNADR(1, E(LASTYCMD)), /* LASTYCMD,LASTXCMD */
    DNADR(1, E(LEMMASS)),  /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)), /* IMODES30,IMODES33 */
    DNADR(1, E(TIG)),      /* TIG,+1 */
    DNPTR(LMREND05),       /* COMMON DATA */
    DNADR(1, E(DELTAR)),   /* DELTAR,+1 */
    DNADR(1, E(CENTANG)),  /* CENTANG,+1 */
    DNADR(1, E(NN)),       /* NN,+1 */
    DNADR(1, E(DIFFALT)),  /* DIFFALT,+1 */
    DNADR(1, E(DELVTPF)),  /* DELVTPF,+1 */
    MINUS_DNADR(1, E(SPARE)),
    /* Its sublists. */
    /* LMREND01 EQUALS LMORBM01: COMMON DOWNLIST DATA */
    /* LMREND02 EQUALS LMORBM02: COMMON DOWNLIST DATA */
    /* LMREND03 EQUALS LMORBM03: COMMON DOWNLIST DATA */
    /* LMREND04 EQUALS LMORBM04: COMMON DOWNLIST DATA */
    /* LMREND05 EQUALS LMORBM05: COMMON DOWNLIST DATA */
    /* LMREND06 EQUALS LMCSTA06: COMMON DOWNLIST DATA */
    LIST(LMREND07),
    MINUS_DNADR(1, E(AIG)),      /* AIG,AMG SNAPSHOT */
    DNADR(1, E(AOG)),            /* AOG,TRKMKCNT */
    DNADR(1, E(TANGNB)),         /* TANGNB,+1 */
    DNADR(1, E(MKTIME)),         /* MKTIME,+1 */
    MINUS_DNADR(1, E(RANGRDOT)), /* DNRRANGE,DNRRDOT */

    /* ---- LM DESCENT AND ASCENT DOWNLIST (p. 200) ---- */
    /* The control list. */
    LIST(LMDSASDL),      /* SEND ID BY SPECIAL CODING */
    DNPTR(LMDSAS07),     /* COLLECT SNAPSHOT */
    DNPTR(LMDSAS08),     /* SEND SNAPSHOT */
    DNADR(1, E(TEVENT)), /* TEVENT,+1 */
    DNADR(3, E(UNFC_2)), /* UNFC/2 +0...+5 */
    DNADR(3, E(VGVECT)), /* VGVECT +0...+5 */
    DNADR(1, E(TTF_8)),  /* TTF/8,+1 */
    DNADR(1, E(DELTAH)), /* DELTAH,+1 */
    DNADR(3, E(RLS)),    /* RLS +0...+5 */
    DNADR(1, E(SPARE)),
    DNPTR(LMDSAS09),          /* COMMON DATA */
    DNPTR(LMDSAS02),          /* COMMON DATA */
    DNADR(1, E(TIME2)),       /* TIME2/1 */
    DNPTR(LMDSAS03),          /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),    /* SEND SNAPSHOT */
    DNPTR(LMDSAS04),          /* COMMON DATA */
    DNADR(2, E(POSTORKU)),    /* POSTORKU,NEGTORKU,POSTORKV,NEGTORKV */
    DNADR(3, E(RGU)),         /* RGU +0...+5 */
    DNADR(3, E(VGU)),         /* VGU +0...+5 */
    DNADR(3, E(LAND)),        /* LAND +0...+5 */
    DNADR(1, E(AT)),          /* AT,+1 */
    DNADR(1, E(TLAND)),       /* TLAND,+1 */
    DNADR(1, E(FC)),          /* FC,GARBAGE */
    DNADR(1, E(LASTYCMD)),    /* LASTYCMD,LASTXCMD */
    DNADR(1, E(LEMMASS)),     /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)),    /* IMODES30,IMODES33 */
    DNADR(1, E(TIG)),         /* TIG,+1 */
    DNPTR(LMDSAS05),          /* COMMON DATA */
    DNPTR(LMDSAS06),          /* COMMON DATA */
    DNADR(1, E(PSEUDO55)),    /* PSEUDO55,GARBAGE */
    MINUS_DNADR(1, E(TTOGO)), /* TTOGO,+1 */
    /* Its sublists. */
    /* LMDSAS02 EQUALS LMORBM02: COMMON DOWNLIST DATA */
    /* LMDSAS03 EQUALS LMORBM03: COMMON DOWNLIST DATA */
    /* LMDSAS04 EQUALS LMORBM04: COMMON DOWNLIST DATA */
    /* LMDSAS05 EQUALS LMORBM05: COMMON DOWNLIST DATA */
    /* LMDSAS06 EQUALS LMORBM06: COMMON DOWNLIST DATA */
    LIST(LMDSAS07),
    MINUS_DNADR(1, E(LRZCDUDL)), /* LRZCDUDL,GARBAGE SNAPSHOT */
    DNADR(1, E(VSELECT)),        /* VSELECT,GARBAGE */
    DNADR(1, E(LRVTIMDL)),       /* LRVTIMDL,+1 */
    DNADR(1, E(VMEAS)),          /* VMEAS,+1 */
    DNADR(1, E(MKTIME)),         /* MKTIME,+1 */
    DNADR(1, E(HMEAS)),          /* HMEAS,+1 */
    DNADR(1, E(RM)),             /* RM,+1 */
    DNADR(1, E(AIG)),            /* AIG,AMG */
    DNADR(1, E(AOG)),            /* AOG,TRKMKCNT */
    DNADR(1, E(TANGNB)),         /* TANGNB,+1 */
    DNADR(1, E(MKTIME)),         /* MKTIME,+1 */
    MINUS_DNADR(1, E(LRXCDUDL)), /* LRXCDUDL,LRYCDUDL */
    LIST(LMDSAS08),
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    MINUS_DNADR(5, E(DNTMBUFF) + 12),
    /* LMDSAS09 EQUALS LMCSTA06: COMMON DOWNLIST DATA */

    /* ---- LM LUNAR SURFACE ALIGN DOWNLIST (p. 202) ---- */
    /* The control list. */
    LIST(LMLSALDL),        /* SEND ID BY SPECIAL CODING */
    DNPTR(LMLSAL01),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(LMLSAL07),       /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)), /* SEND SHAPSHOT */
    DNADR(1, E(TALIGN)),   /* TALIGN,+1 */
    DNADR(6, E(REFSMMAT)), /* REFSMMAT +0...+11D */
    DNADR(6, E(YNBSAV)),   /* YNBSAV +0...+5,SNBSAV +0...+5 */
    DNPTR(LMLSAL08),       /* COMMON DATA */
    DNPTR(LMLSAL02),       /* COMMON DATA */
    DNADR(1, E(TIME2)),    /* TIME2/1 */
    DNPTR(LMLSAL03),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SHAPSHOT */
    DNPTR(LMLSAL04),       /* COMMON DATA */
    DNPTR(LMLSAL09),       /* COMMON DATA */
    DNADR(3, E(GSAV)),     /* GSAV +0...+5 */
    DNADR(1, E(AGSK)),     /* AGSK,+1 */
    DNADR(1, E(LASTYCMD)), /* LASTYCMD,LASTXCMD */
    DNADR(1, E(LEMMASS)),  /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)), /* IMODES30,IMODES33 */
    DNADR(1, E(TIG)),      /* TIG,+1 */
    DNPTR(LMLSAL05),       /* COMMON DATA */
    DNPTR(LMLSAL06),       /* COMMON DATA */
    DNADR(1, E(SPARE)),
    MINUS_DNADR(1, E(SPARE)),
    /* Its sublists. */
    /* LMLSAL01 EQUALS LMORBM01: COMMON DOWNLIST DATA */
    /* LMLSAL02 EQUALS LMORBM02: COMMON DOWNLIST DATA */
    /* LMLSAL03 EQUALS LMORBM03: COMMON DOWNLIST DATA */
    /* LMLSAL04 EQUALS LMORBM04: COMMON DOWNLIST DATA */
    /* LMLSAL05 EQUALS LMORBM05: COMMON DOWNLIST DATA */
    /* LMLSAL06 EQUALS LMORBM06: COMMON DOWNLIST DATA */
    /* LMLSAL07 EQUALS LMREND07: COMMON DOWNLIST DATA */
    /* LMLSAL08 EQUALS LMCSTA06: COMMON DOWNLIST DATA */
    /* LMLSAL09 EQUALS LMCSTA07: COMMON DOWNLIST DATA */

    /* ---- LM AGS INITIALIZATION AND UPDATE DOWNLIST (p. 204) ---- */
    /* The control list. */
    LIST(LMAGSIDL),            /* SEND IO BY SPECIAL CODING */
    DNADR(3, E(AGSBUFF)),      /* AGSBUFF +0...+5 */
    DNADR(1, E(AGSBUFF) + 12), /* AGSBUFF +12D,GARBAGE */
    DNADR(3, E(AGSBUFF) + 1),  /* AGSBUFF +1...+6 */
    DNADR(1, E(AGSBUFF) + 13), /* AGSBUFF +13D, GARBAGE */
    DNADR(3, E(AGSBUFF) + 6),  /* AGSBUFF +6...+11 */
    DNADR(1, E(AGSBUFF) + 12), /* AGSBUFF +12D,GARBAGE */
    DNADR(3, E(AGSBUFF) + 7),  /* AGSBUFF +7...+12D */
    DNADR(1, E(AGSBUFF) + 13), /* AGSBUFF +13D,GARBAGE */
    DNADR(6, E(COMPNUMB)),     /* COMPNUMB,UPOLDMOD,UPVERB,UPCOUNT, */
    DNADR(6, E(UPBUFF) + 8),   /* UPBUFF +8D...+19D */
    DNPTR(LMAGSI02),           /* COMMON DATA */
    DNADR(1, E(TIME2)),        /* TIME2/1 */
    DNPTR(LMAGSI03),           /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(LMAGSI04),           /* COMMON DATA */
    DNADR(2, E(POSTORKU)),     /* POSTORKU,NEGTORKU,POSTORKV,NEGTORKV */
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(AGSK)),        /* AGSK,+1 */
    DNADR(6, E(UPBUFF)),      /* UPBUFF +0...+11D */
    DNADR(4, E(UPBUFF) + 12), /* UPBUFF +12D...+19D */
    DNADR(1, E(LEMMASS)),     /* LEMMASS,CSMMASS */
    DNADR(1, E(IMODES30)),    /* IMODES30,IMODES33 */
    DNADR(1, E(SPARE)),
    DNPTR(LMAGSI05),           /* COMMON DATA */
    MINUS_DNADR(6, E(DSPTAB)), /* DSPTAB +0...+11D */
    /* Its sublists. */
    /* LMAGSI02 EQUALS LMORBM02: COMMON DOWNLIST DATA */
    /* LMAGSI03 EQUALS LMORBM03: COMMON DOWNLIST DATA */
    /* LMAGSI04 EQUALS LMORBM04: COMMON DOWNLIST DATA */
    /* LMAGSI05 EQUALS LMORBM05: COMMON DOWNLIST DATA */

    /* DNTABLE (p. 204): the downlists, by DNLSTCOD. */
    GENADR(LMCSTADL), /* LM COAST AND ALIGN DOWNLIST */
    GENADR(LMAGSIDL), /* LM AGS INITIALIZATION/UPDATE DOWNLIST */
    GENADR(LMRENDDL), /* LM RENDEZVOUS AND PRE-THRUST DOWNLIST */
    GENADR(LMORBMDL), /* LM ORBITAL MANEUVERS DOWNLIST */
    GENADR(LMDSASDL), /* LM DESCENT AND ASCENT DOWNLIST */
    GENADR(LMLSALDL), /* LM LUNAR SURFACE ALIGN DOWNLIST */
};

#else

/* The CM's lists, by label. */
enum list {
    CMPOWEDL,
    CMPOWE01,
    CMPOWE02,
    CMPOWE03,
    CMPOWE04,
    CMPOWE05,
    CMPOWE06,
    CMPOWE07,
    CMCSTADL,
    CMRENDDL,
    CMENTRDL,
    CMENTR05,
    CMPG22DL,
    CMPG2205,
    LISTS, /* how many labels */
    /* The names the listing gives the same lists (EQUALS). */
    NOMDNLST = CMCSTADL, /* FRESH START AND POST P27 DOWNLIST */
    UPDNLIST = CMENTRDL, /* UPDATE PROGRAM (P27) DOWNLIST */
    CMCSTA01 = CMPOWE01, /* COMMON DOWNLIST DATA */
    CMCSTA02 = CMPOWE02, /* COMMON DOWNLIST DATA */
    CMCSTA03 = CMPOWE03, /* COMMON DOWNLIST DATA */
    CMCSTA04 = CMPOWE04, /* COMMON DOWNLIST DATA */
    CMCSTA05 = CMPOWE05, /* COMMON DOWNLIST DATA */
    CMCSTA06 = CMPOWE06, /* COMMON DOWNLIST DATA */
    CMCSTA07 = CMPOWE07, /* COMMON DOWNLIST DATA */
    CMREND01 = CMPOWE01, /* COMMON DOWNLIST DATA */
    CMREND02 = CMPOWE02, /* COMMON DOWNLIST DATA */
    CMREND03 = CMPOWE03, /* COMMON DOWNLIST DATA */
    CMREND04 = CMPOWE04, /* COMMON DOWNLIST DATA */
    CMREND05 = CMPOWE05, /* COMMON DOWNLIST DATA */
    CMREND06 = CMPOWE06, /* COMMON DOWNLIST DATA */
    CMREND07 = CMPOWE07, /* COMMON DOWNLIST DATA */
    CMENTR01 = CMPOWE01, /* COMMON DOWNLIST DATA */
    CMENTR02 = CMPOWE02, /* COMMON DOWNLIST DATA */
    CMENTR03 = CMPOWE03, /* COMMON DOWNLIST DATA */
    CMENTR04 = CMPOWE04, /* COMMON DOWNLIST DATA */
    CMENTR07 = CMPOWE07, /* COMMON DOWNLIST DATA */
    CMPG2201 = CMPOWE01, /* COMMON DOWNLIST DATA */
    CMPG2202 = CMPOWE02, /* COMMON DOWNLIST DATA */
    CMPG2203 = CMPOWE03, /* COMMON DOWNLIST DATA */
    CMPG2204 = CMPOWE04, /* COMMON DOWNLIST DATA */
    CMPG2206 = CMPOWE06, /* COMMON DOWNLIST DATA */
    CMPG2207 = CMPOWE07, /* COMMON DOWNLIST DATA */
};

static const struct list_word lists[] = {
    OCT(077340), /* LOWIDCOD: LOW ID CODE */

    /* ---- CSM POWERED FLIGHT DOWNLIST (p. 171) ---- */
    /* The control list. */
    LIST(CMPOWEDL),
    DNPTR(CMPOWE01),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(CMPOWE02),       /* COLLECT SECOND SNAPSHOT */
    DNADR(4, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(CMPOWE03),       /* COMMON DATA */
    DNADR(1, E(TIG)),      /* TIG,+1 */
    DNADR(1, E(DELLT4)),   /* DELLT4,+1 */
    DNADR(3, E(RTARG)),    /* RTARG,+1,+2,...+5 */
    DNADR(1, E(TGO)),      /* TGO,+1 */
    DNADR(1, E(PIPTIME1)), /* PIPTIME1,+1 */
    DNADR(3, E(DELV)),     /* DELV,+1,...+4,+5 */
    DNADR(1, E(PACTOFF)),  /* PACTOFF,YACTOFF */
    DNADR(1, E(PCMD)),     /* PCMD,YCMD */
    DNADR(1, E(CSTEER)),   /* CSTEER,+1 */
    DNADR(3, E(DELVEET1)), /* CSI DELTA VELOCITY COMPONENTS (31-33) */
    DNADR(6, E(REFSMMAT)), /* REFSMMAT,+1,...+10,+11 */
    DNPTR(CMPOWE04),       /* COMMON DATA */
    DNADR(1, E(TIME2)),    /* TIME2,TIME1 */
    DNPTR(CMPOWE05),       /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(CMPOWE02),       /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNPTR(CMPOWE03),
    DNPTR(CMPOWE06),             /* COMMON DATA */
    DNADR(1, E(ELEV)),           /* ELEV,+1 */
    DNADR(1, E(CENTANG)),        /* CENTANG,+1 */
    DNADR(1, E(DELTAR)),         /* DELTAR,+1 */
    DNADR(1, E(STATE) + 10),     /* FLAGWRDS 10 AND 11 */
    DNADR(1, E(TEVENT)),         /* TEVENT,+1 */
    DNADR(1, E(PCMD)),           /* PCMD,YCMD */
    DNADR(1, E(OPTMODES)),       /* OPTMODES,HOLDFLAG */
    DNPTR(CMPOWE07),             /* COMMON DATA */
    DNADR(3, E(VGTIG)),          /* VGTIG,+1,...+4,+5 */
    MINUS_DNADR(3, E(DELVEET2)), /* CDH DELTA VELOCITY COMPONENTS (98-100) */
    LIST(CMPOWE01),
    MINUS_DNADR(1, E(RN) + 2), /* RN+2,+3 SNAPSHOT DATA */
    DNADR(1, E(RN) + 4),       /* RN+4,+5 */
    DNADR(1, E(VN)),           /* VN,+1 */
    DNADR(1, E(VN) + 2),       /* VN+2,+3 */
    DNADR(1, E(VN) + 4),       /* VN+4,+5 */
    DNADR(1, E(PIPTIME)),      /* PIPTIME,+1 */
    MINUS_DNADR(1, E(RN)),     /* RN,+1 */
    LIST(CMPOWE02),
    MINUS_DNADR(1, E(CDUZ)), /* CDUZ,CDUT SNAPSHOT DATA */
    DNADR(1, E(ADOT)),       /* ADOT,+1/OGARATE,+1 */
    DNADR(1, E(ADOT) + 2),   /* ADOT+2,+3/OMEGAB+2,+3 */
    DNADR(1, E(ADOT) + 4),   /* ADOT+4,+5/OMEGAB+4,+5 */
    MINUS_DNADR(1, E(CDUX)), /* CDUX,CDUY */
    LIST(CMPOWE03),
    DNADR(2, E(AK)),            /* AK,AK1,AK2,RCSFLAGS COMMON DATA */
    MINUS_DNADR(2, E(THETADX)), /* THETADX,THETADY,THETADZ,GARBAGE */
    LIST(CMPOWE04),
    DNADR(5, E(STATE)),        /* FLAGWRD0 THRU FLAGWRD9 COMMON DATA */
    MINUS_DNADR(6, E(DSPTAB)), /* DISPLAY TABLES */
    LIST(CMPOWE05),
    MINUS_DNADR(1, E(R_OTHER) + 2), /* R-OTHER+2,+3 SNAPSHOT DATA */
    DNADR(1, E(R_OTHER) + 4),       /* R-OTHER+4,+5 */
    DNADR(1, E(V_OTHER)),           /* V-OTHER,+1 */
    DNADR(1, E(V_OTHER) + 2),       /* V-OTHER+2,+3 */
    DNADR(1, E(V_OTHER) + 4),       /* V-OTHER+4,+5 */
    DNADR(1, E(T_OTHER)),           /* T-OTHER,+1 */
    MINUS_DNADR(1, E(R_OTHER)),     /* R-OTHER,+1 */
    LIST(CMPOWE06),
    DNADR(1, E(RSBBQ)),      /* RSBBQ,+1 COMMON DATA */
    DNADR(3, E(CADRFLSH)),   /* CADRFLSH,+1,+2,FAILREG,+1,+2 */
    MINUS_DNADR(2, E(CDUS)), /* CDUS,PIPAX,PIPAY,PIPAZ */
    LIST(CMPOWE07),
    DNADR(1, E(LEMMASS)),  /* LEMMASS,CSMMASS COMMON DATA */
    DNADR(1, E(DAPDATR1)), /* DAPDATR1,DAPDATR2 */
    DNADR(2, E(ERRORX)),   /* ERRORX,ERRORY,ERRORZ,GARBAGE */
    DNADR(3, E(WBODY)),    /* WBODY,...+5/OMEGAC,...+5 */
    DNADR(2, E(REDOCTR)),  /* REDOCTR,THETAD,+1,+2 */
    DNADR(1, E(IMODES30)), /* IMODES30,IMODES33 */
    DNCHAN(011),           /* CHANNELS 11,12 */
    DNCHAN(013),           /* CHANNELS 13,14 */
    DNCHAN(030),           /* CHANNELS 30,31 */
    MINUS_DNCHAN(032),     /* CHANNELS 32,33 */

    /* ---- CSM COAST AND ALIGNMENT DOWNLIST (p. 173) ---- */
    /* The control list. */
    LIST(CMCSTADL),            /* SEND ID BY SPECIAL CODING */
    DNPTR(CMCSTA01),           /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(CMCSTA02),           /* COLLECT SECOND SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(CMCSTA03),           /* COMMON DATA */
    DNADR(1, E(TIG)),          /* TIG,+1 */
    DNADR(1, E(BESTI)),        /* BESTI,BESTJ */
    DNADR(4, E(MARKDOWN)),     /* MARKDOWN,+1,...+5,+6,GARBAGE */
    DNADR(4, E(MARK2DWN)),     /* MARK2DWN,+1,...+5,+6 */
    DNADR(2, E(HAPOX)),        /* APOGEE AND PERIGEE FROM R30 (28-29) */
    DNADR(1, E(PACTOFF)),      /* PACTOFF, YACTOFF (30) */
    DNADR(3, E(VGTIG)),        /* VGTIG,...+5 */
    DNADR(6, E(REFSMMAT)),     /* REFSMMAT,+1,...+10,+11 */
    DNPTR(CMCSTA04),           /* COMMON DATA */
    DNADR(1, E(TIME2)),        /* TIME2,TIME1 */
    DNPTR(CMCSTA05),           /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(CMCSTA02),           /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),     /* SEND SNAPSHOT */
    DNPTR(CMCSTA03),           /* COMMON DATA */
    DNPTR(CMCSTA06),           /* COMMON DATA */
    DNADR(3, E(OGC)),          /* OGC,+1,IGC,+1,MGC,+1 */
    DNADR(1, E(STATE) + 10),   /* FLAGWRDS 10 AND 11 */
    DNADR(1, E(TEVENT)),       /* TEVENT,+1 */
    DNADR(1, E(LAUNCHAZ)),     /* LAUNCHAZ,+1 */
    DNADR(1, E(OPTMODES)),     /* OPTMODES,HOLDFLAG */
    DNPTR(CMCSTA07),           /* COMMON DATA */
    MINUS_DNADR(6, E(DSPTAB)), /* DISPLAY TABLES */
    /* CMCSTA01 EQUALS CMPOWE01: COMMON DOWNLIST DATA */
    /* CMCSTA02 EQUALS CMPOWE02: COMMON DOWNLIST DATA */
    /* CMCSTA03 EQUALS CMPOWE03: COMMON DOWNLIST DATA */
    /* CMCSTA04 EQUALS CMPOWE04: COMMON DOWNLIST DATA */
    /* CMCSTA05 EQUALS CMPOWE05: COMMON DOWNLIST DATA */
    /* CMCSTA06 EQUALS CMPOWE06: COMMON DOWNLIST DATA */
    /* CMCSTA07 EQUALS CMPOWE07: COMMON DOWNLIST DATA */

    /* ---- CSM RENDEZVOUS AND PRETHRUST LIST (p. 175) ---- */
    /* The control list. */
    LIST(CMRENDDL),                /* SEND ID BY SPECIAL CODING */
    DNPTR(CMREND01),               /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),         /* SEND SNAPSHOT */
    DNPTR(CMREND02),               /* COLLECT SECOND SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),         /* SEND SNAPSHOT */
    DNPTR(CMREND03),               /* COMMON DATA */
    DNADR(1, E(TIG)),              /* TIG,+1 */
    DNADR(1, E(DELLT4)),           /* DELLT4,+1 */
    DNADR(3, E(RTARG)),            /* RTARG,+1,...+4,+5 */
    DNADR(1, E(VHFTIME)),          /* VHFTIME,+1 */
    DNADR(4, E(MARKDOWN)),         /* MARKTIME(DP),YCDU,SCDU,ZCDU,TCDU,XCDU,RM */
    DNADR(1, E(VHFCNT)),           /* VHFCNT,+1 */
    DNADR(1, E(TTPI)),             /* TTPI,+1 */
    DNADR(1, E(ECSTEER)),          /* ECSTEER,+1 */
    DNADR(1, E(DELVTPF)),          /* DELVTPF,+1 */
    DNADR(2, E(TCDH)),             /* CDH AND CSI TIME (32-33) */
    DNADR(1, E(TPASS4)),           /* TPASS4,+1 */
    DNADR(3, E(DELVSLV)),          /* DELVSLV,+1...+4,+5 */
    DNADR(2, E(RANGE)),            /* RANGE,+1,RRATE,+1 */
    DNPTR(CMREND04),               /* COMMON DATA */
    DNADR(1, E(TIME2)),            /* TIME2,TIME1 */
    DNPTR(CMREND05),               /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),         /* SEND SNAPSHOT */
    DNPTR(CMREND02),               /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),         /* SEND SNAPSHOT */
    DNPTR(CMREND03),               /* COMMON DATA */
    DNPTR(CMREND06),               /* COMMON DATA */
    DNADR(1, E(DIFFALT)),          /* CDH DELTA ALTITUDE */
    DNADR(1, E(CENTANG)),          /* CENTANG,+1 */
    DNADR(1, E(DELTAR)),           /* DELTAR,+1 */
    DNADR(3, E(DELVEET3)),         /* DELVEET3,+1,...+4,+5 */
    DNADR(1, E(OPTMODES)),         /* OPTMODES,HOLDFLAG */
    DNPTR(CMREND07),               /* COMMON DATA */
    DNADR(1, E(RTHETA)),           /* RTHETA,+1 */
    DNADR(2, E(LAT_SPL)),          /* LAT(SPL),LNG(SPL),+1 */
    DNADR(2, E(VPRED)),            /* VPRED,+1,GAMMAEI,+1 */
    MINUS_DNADR(1, E(STATE) + 10), /* FLAGWRDS 10 AND 11 */
    /* CMREND01 EQUALS CMPOWE01: COMMON DOWNLIST DATA */
    /* CMREND02 EQUALS CMPOWE02: COMMON DOWNLIST DATA */
    /* CMREND03 EQUALS CMPOWE03: COMMON DOWNLIST DATA */
    /* CMREND04 EQUALS CMPOWE04: COMMON DOWNLIST DATA */
    /* CMREND05 EQUALS CMPOWE05: COMMON DOWNLIST DATA */
    /* CMREND06 EQUALS CMPOWE06: COMMON DOWNLIST DATA */
    /* CMREND07 EQUALS CMPOWE07: COMMON DOWNLIST DATA */

    /* ---- CSM ENTRY AND UPDATE DOWNLIST (p. 177) ---- */
    /* The control list. */
    LIST(CMENTRDL),             /* SEND ID BY SPECIAL CODING */
    DNPTR(CMENTR01),            /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNPTR(CMENTR02),            /* COLLECT SECOND SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNPTR(CMENTR03),            /* COMMON DATA */
    DNADR(2, E(CMDAPMOD)),      /* CMDAPMOD,PREL,QREL,RREL */
    DNADR(1, E(L_D1)),          /* L/D1,+1 */
    DNADR(6, E(UPBUFF)),        /* UPBUFF,+1,...+10,+11 */
    DNADR(4, E(UPBUFF) + 12),   /* UPBUFF+12,13,...+18,+19D */
    DNADR(2, E(COMPNUMB)),      /* COMPNUMB,UPOLDMOD,UPVERB,UPCOUNT */
    DNADR(1, E(PAXERR1)),       /* PAXERR1,ROLLTM */
    DNADR(3, E(LATANG)),        /* LATANG,+1,RDOT,+1,THETAH,+1 */
    DNADR(2, E(LAT_SPL)),       /* LAT(SPL),+1,LNG(SPL),+1 */
    DNADR(1, E(ALFA_180)),      /* ALFA/180,BETA/180 */
    DNPTR(CMENTR04),            /* COMMON DATA */
    DNADR(1, E(TIME2)),         /* TIME2,TIME1 */
    DNPTR(CMENTR05),            /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNPTR(CMENTR02),            /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNADR(2, E(AK)),            /* AK,AK1,AK2,RCSFLAGS */
    DNADR(3, E(ERRORX)),        /* ERRORX/Y/Z,THETADX/Y/Z */
    DNADR(2, E(CMDAPMOD)),      /* CMDAPMOD,PREL,QREL,RREL */
    DNADR(6, E(UPBUFF)),        /* UPBUFF+0,+1,...+10,+11D */
    DNADR(4, E(UPBUFF) + 12),   /* UPBUFF+12,+13,...+18,+19D */
    DNADR(1, E(LEMMASS)),       /* LEMMASS,CSMMASS */
    DNADR(1, E(DAPDATR1)),      /* DAPDATR1,DAPDATR2 */
    DNADR(1, E(ROLLTM)),        /* ROLLTM,ROLLC */
    DNADR(1, E(OPTMODES)),      /* OPTMODES,HOLDFLAG */
    DNADR(3, E(WBODY)),         /* WBODY,...+5/OMEGAC,...+5 */
    DNADR(2, E(REDOCTR)),       /* REDOCTR,THETAD+0,+1,+2 */
    DNADR(1, E(IMODES30)),      /* IMODES30,IMODES33 */
    DNCHAN(011),                /* CHANNELS 11,12 */
    DNCHAN(013),                /* CHANNELS 13,14 */
    DNCHAN(030),                /* CHANNELS 30,31 */
    DNCHAN(032),                /* CHANNELS 32,33 */
    DNADR(1, E(RSBBQ)),         /* RSBBQ,+1 */
    DNADR(3, E(CADRFLSH)),      /* CADRFLSH,+1,+2,FAILREG,+1,+2 */
    DNADR(1, E(STATE) + 10),    /* FLAGWRDS 10 AND 11 */
    MINUS_DNADR(1, E(GAMMAEI)), /* GAMMAEI,+1 */
    /* CMENTR01 EQUALS CMPOWE01: COMMON DOWNLIST DATA */
    /* CMENTR02 EQUALS CMPOWE02: COMMON DOWNLIST DATA */
    /* CMENTR03 EQUALS CMPOWE03: COMMON DOWNLIST DATA */
    /* CMENTR04 EQUALS CMPOWE04: COMMON DOWNLIST DATA */
    LIST(CMENTR05),
    MINUS_DNADR(1, E(DELV)),     /* DELV,+1 SNAPSHOT DATA */
    DNADR(1, E(DELV) + 2),       /* DELV+2,+3 */
    DNADR(1, E(DELV) + 4),       /* DELV+4,+5 */
    DNADR(1, E(TTE)),            /* TTE,+1 */
    DNADR(1, E(VIO)),            /* VIO,+1 */
    DNADR(1, E(VPRED)),          /* VPRED,+1 */
    MINUS_DNADR(1, E(PIPTIME1)), /* PIPTIME1,+1 */
    /* CMENTR07 EQUALS CMPOWE07: COMMON DOWNLIST DATA */

    /* ---- P22 DOWNLISTS (p. 179) ---- */
    /* The control list. */
    LIST(CMPG22DL),             /* SEND ID BY SPECIAL CODING */
    DNPTR(CMPG2201),            /* COLLECT SNAPSHOT */
    DNADR(6, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNPTR(CMPG2202),            /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),      /* SEND SNAPSHOT */
    DNPTR(CMPG2203),            /* COMMON DATA */
    DNADR(6, E(SVMRKDAT)),      /* LANDING SITE MARK DATA */
    DNADR(6, E(SVMRKDAT) + 12), /* SVMRKDAT+0...+34 */
    DNADR(6, E(SVMRKDAT) + 24), /* LANDING SITE MARK DATA */
    DNADR(1, E(LANDMARK)),      /* LANDMARK,GARBAGE */
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNPTR(CMPG2204),       /* COMMON DATA */
    DNADR(1, E(TIME2)),    /* TIME2,TIME1 */
    DNPTR(CMPG2205),       /* COLLECT SNAPSHOT */
    DNADR(2, E(DNTMBUFF)), /* SEND SNAPSHOT */
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNPTR(CMPG2202),         /* COLLECT SNAPSHOT */
    DNADR(4, E(DNTMBUFF)),   /* SEND SNAPSHOT */
    DNPTR(CMPG2203),         /* COMMON DATA */
    DNPTR(CMPG2206),         /* COMMON DATA */
    DNADR(1, E(8NN)),        /* 8NN,GARBAGE */
    DNADR(1, E(STATE) + 10), /* FLAGWRDS 10 AND 11 */
    DNADR(3, E(RLS)),        /* RLS,+1,...+4,+5 */
    DNADR(1, E(SPARE)),
    DNADR(1, E(OPTMODES)), /* OPTMODES,HOLDFLAG */
    DNPTR(CMPG2207),       /* COMMON DATA */
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    DNADR(1, E(SPARE)),
    MINUS_DNADR(1, E(SPARE)),
    /* CMPG2201 EQUALS CMPOWE01: COMMON DOWNLIST DATA */
    /* CMPG2202 EQUALS CMPOWE02: COMMON DOWNLIST DATA */
    /* CMPG2203 EQUALS CMPOWE03: COMMON DOWNLIST DATA */
    /* CMPG2204 EQUALS CMPOWE04: COMMON DOWNLIST DATA */
    LIST(CMPG2205),
    MINUS_DNADR(1, E(LONG)), /* LONG,+1 SNAPSHOT DATA */
    DNADR(1, E(ALT)),        /* ALT,+1 */
    MINUS_DNADR(1, E(LAT)),  /* LAT,+1 */
    /* CMPG2206 EQUALS CMPOWE06: COMMON DOWNLIST DATA */
    /* CMPG2207 EQUALS CMPOWE07: COMMON DOWNLIST DATA */

    /* DNTABLE (p. 180): the downlists, by DNLSTCOD. */
    GENADR(CMCSTADL),
    GENADR(CMENTRDL),
    GENADR(CMRENDDL),
    GENADR(CMPOWEDL),
    GENADR(CMPG22DL),
};

#endif

/* The word a list entry assembles to, given where each list starts. */
static word assembled(const struct list_word *w, const word *list_address)
{
    word value = 0;

    switch (w->kind) {
    case OCT_WORD: value = w->operand; break;
    case DNADR_WORD: value = (word)(((w->count - 1u) << 11) | w->operand); break;
    case DNCHAN_WORD: value = (word)(034000u | w->operand); break;              /* bits 14-12 = 7 */
    case DNPTR_WORD: value = (word)(030000u | list_address[w->operand]); break; /* CA list */
    case GENADR_WORD: value = list_address[w->operand]; break;
    case LABEL_ONLY: break;
    }
    return w->minus ? negate(value) : value;
}

/* The lists as assembled, from LOWIDCOD on (bank 05). */
#define ENTRIES (sizeof lists / sizeof lists[0])
static word rope[ENTRIES];
static word rope_words;

static void assemble(void)
{
    word list_address[LISTS];
    word at = ADDRESS_OF(LOWIDCOD);
    unsigned i;

    for (i = 0; i < ENTRIES; ++i) {
        if (lists[i].kind == LABEL_ONLY)
            list_address[lists[i].operand] = at;
        else
            ++at;
    }
    for (i = 0; i < ENTRIES; ++i)
        if (lists[i].kind != LABEL_ONLY) rope[rope_words++] = assembled(&lists[i], list_address);
}

/* The downlist word at a bank 05 address (02000-03777), if there is one:
 * LOWIDCOD, the lists and DNTABLE. */
bool downlink_list_word(word address, word *value)
{
    if (rope_words == 0u) assemble();
    if (address < ADDRESS_OF(LOWIDCOD) || address - ADDRESS_OF(LOWIDCOD) >= rope_words)
        return false;
    *value = rope[address - ADDRESS_OF(LOWIDCOD)];
    return true;
}
