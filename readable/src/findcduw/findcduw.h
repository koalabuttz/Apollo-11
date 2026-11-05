/* SPDX-License-Identifier: MIT */
/* FINDCDUW--GUIDAP_INTERFACE.agc (Luminary099 pp. 908-925, bank 30): the
 * interface between the powered flight guidance (P12, P40-P42, P63-P67,
 * the aborts) and the digital autopilot. "THE INPUTS TO FINDCDUW ARE THE
 * THRUST COMMAND VECTOR AND THE WINDOW COMMAND VECTOR, AND THE OUTPUTS ARE
 * THE GIMBAL ANGLE INCREMENTS, THE COMMANDED ATTITUDE ANGLE RATES, AND THE
 * COMMANDED ATTITUDE LAG ANGLES". In the order of the listing:
 *
 *   INITCDUW and FINDCDUW (findcduw.c): the start of each guided maneuver;
 *     then each pass's housekeeping, the CDUs (or, in PNGCS auto, the
 *     commanded ones, CDUXD-CDUZD), the two commands made semi-unit and the
 *     thrust direction filter.
 *   AFTRFLTR to TCQCDUW (gimbal_angles.c): the window, the direction
 *     cosine matrix and its gimbal angles (NB2CDUSP), the middle gimbal
 *     angle limited, the attitude angle changes limited, the commanded
 *     rates, and the data for the autopilot.
 *   The subroutines (subroutines.c): FLTRSUB, UNWCTEST, NB2CDUSP,
 *     DVBYCOSM, ARCTRGSP, SPARCSIN, LIMITSUB, ONESTO2S, NOATTCNT and
 *     ALARMMGA.
 *
 * The faithful C is c64-port/src/lm_findcduw.c. Each function below runs
 * from a word the program can resume at (an entry, a subroutine's return,
 * the word after an EXIT) until its code leaves the file: at a TC or TCF
 * out of it (Z the routine, Q the return), at a TC to an erasable word
 * (FLTRSUB's TC TEM2, ARCTRGSP's TC TEM4), or at a TC INTPRET, run on to
 * NEWOPS (intpret()). The faithful port stops there too: the binding,
 * src/binding/findcduw_routines.c, calls each from its key. The file's
 * own subroutines (LIMITSUB, ONESTO2S, DVBYCOSM, SPARCSIN) return to the
 * caller by TC Q, and the caller goes on. */
#ifndef READABLE_FINDCDUW_H
#define READABLE_FINDCDUW_H

#include "../agc/agc.h"

#if LUMINARY099
#include "../interpreter/notation.h"
#include "../agc/names.h"

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* Instruction words, for an INDEX before them: the operation code (CCS is
 * 1, CA 3, CS 4, TS 5 with its quarter code; after an EXTEND, MSU is 2, MP
 * 7, DV 1 and DCA 3, whose address is written one more). */
#define CCS_CODE 010000u
#define CA_CODE  030000u
#define CS_CODE  040000u
#define TS_CODE  054000u
#define MSU_CODE 020000u
#define MP_CODE  070000u
#define DV_CODE  010000u
#define DCA_CODE 030001u

/* INDEX k, then an instruction: C(k) is added to the whole instruction
 * word (indexed_instruction()), and the sum names the address. Not
 * modelled: an index that makes it another instruction (its operation
 * code changed), which the program never gives and the faithful port
 * refuses. */
static inline unsigned indexed_operand(word instruction, word index)
{
    word sum = indexed_instruction(instruction, index);

    if (OPERATION_CODE(sum) != OPERATION_CODE(instruction)) not_modelled();
    return ADDRESS_FIELD(sum);
}

/* The same for TS, which names an erasable word (bits 1-10): its quarter
 * code unchanged too, and not a bank register or Z (03-06), as the
 * faithful port has it. */
static inline unsigned indexed_store(word instruction, word index)
{
    word sum = indexed_instruction(instruction, index);
    unsigned at = sum & 01777u;

    if ((sum ^ instruction) & 076000u) not_modelled();
    if (at >= 03u && at <= 06u) not_modelled();
    return at;
}

/* INDEX NDXCDUW, then an instruction on a constant of a pair (GAINFLTR,
 * DAXMAX, DAY/2MAX): the first without the CSM docked, the second with it
 * (NDXCDUW 0 or 1). Not modelled: an index past the pair, which the
 * faithful port refuses. */
static inline word docked_constant(word instruction)
{
    unsigned at = indexed_operand(instruction, NDXCDUW);

    if (at != ADDRESS_FIELD(instruction) && at != ADDRESS_FIELD(instruction) + 1u) not_modelled();
    return fetch_fixed((word)at);
}

/* INITCDUW, FINDCDUW, PAUTNO and the thrust direction filter --
 * findcduw.c */
void findcduw_after_exit(void);   /* the word after FINDCDUW's EXIT */
void thrust_filter(void);         /* the word after PAUTNO's EXIT */
void thrust_filter_after_y(void); /* FLTRSUB's return, for Y */
void thrust_filter_after_z(void); /* FLTRSUB's return, for Z */
extern const struct interpretive_code INITCDUW_CODE, FINDCDUW_CODE, PAUTNO_CODE;

/* AFTRFLTR to TCQCDUW -- gimbal_angles.c */
void limit_mga(void);                /* the word after DCMCL's EXIT */
void limit_mga_after_limitsub(void); /* LIMITSUB's return */
void limit_attitude_after_z(void);   /* LIMITSUB's return, for Z */
void limit_attitude_after_y(void);   /* LIMITSUB's return, for Y */
void limit_attitude_after_x(void);   /* LIMITSUB's return, for X */
void cduwxfr_after_onesto2s(void);   /* ONESTO2S's return */
void cduwxfr_after_limitsub(void);   /* LIMITSUB's return */
void noattcnt(void);                 /* NOATTCNT */
void noattcnt_plus2(void);           /* NOATTCNT +2 (ALARM's return) */
void noattcnt_after_stoprate(void);  /* IBNKCALL's return */
void alarmmga_after_alarm(void);     /* ALARM's return */
void mgaret(void);                   /* MGARET */
void tcqcduw(void);                  /* TCQCDUW */
void tcqcduw_intpret(void);          /* TCQCDUW +2: TC INTPRET */
extern const struct interpretive_code AFTRFLTR_CODE, TCQCDUW_CODE;

/* The subroutines -- subroutines.c */
void fltrsub(void);                   /* FLTRSUB */
void fltrsub_after_increment(void);   /* LIMITSUB's return */
void fltrsub_after_total(void);       /* LIMITSUB's return */
void nb2cdusp_after_exit(void);       /* the word after NB2CDUSP's EXIT */
void nb2cdusp_store_cduz(void);       /* ARCTRGSP's return, CDUZ */
void nb2cdusp_cduy_second(void);      /* DVBYCOSM's return */
void nb2cdusp_cduy_angle(void);       /* DVBYCOSM's return */
void nb2cdusp_store_cduy(void);       /* ARCTRGSP's return, CDUY */
void nb2cdusp_cdux_second(void);      /* DVBYCOSM's return */
void nb2cdusp_cdux_angle(void);       /* DVBYCOSM's return */
void nb2cdusp_store_cdux(void);       /* ARCTRGSP's return, CDUX */
void arctrgsp_after_sparcsin(void);   /* SPARCSIN -1's return */
void arctrgsp_after_onesto2s(void);   /* ONESTO2S's return */
void one_to2_tcq_plus1(void);         /* ONESTO2S's return (1TO2&TCQ +1) */
void usecos_after_sparcsin(void);     /* SPARCSIN -1's return */
void sparcsin_minus1(void);           /* SPARCSIN -1 */
void limitsub(void);                  /* LIMITSUB */
void onesto2s(void);                  /* ONESTO2S */
void leave_through(unsigned address); /* TC to an erasable word */
extern const struct interpretive_code UNWCTEST_CODE, NB2CDUSP_CODE, NB2CDUSP_RVQ_CODE;
#endif

#endif
