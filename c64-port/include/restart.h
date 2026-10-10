/* SPDX-License-Identifier: MIT */
#ifndef RESTART_H
#define RESTART_H
#include "waitlist.h"
enum rs_word {
    RS_TIME4,
    RS_TIME5,
    RS_RCSFLAGS,
    RS_T6NEXT,
    RS_NXT6ADR,
    RS_NEXTP,
    RS_DAPBOOLS,
    RS_T5ADR,
    RS_T5ADR1,
    RS_FLAGWRD3,
    RS_RADMODES,
    RS_FLAGWRD5,
    RS_DSRUPTSW,
    RS_DELAYLOC,
    RS_DELAYLOC1,
    RS_DELAYLOC2,
    RS_R1SAVE,
    RS_INLINK,
    RS_DSPCNT,
    RS_CADRSTOR,
    RS_REQRET,
    RS_CLPASS,
    RS_DSPLOCK,
    RS_MONSAVE,
    RS_MONSAVE1,
    RS_VERBREG,
    RS_NOUNREG,
    RS_DSPLIST,
    RS_MARKSTAT,
    RS_EXTVBACT,
    RS_IMUCADR,
    RS_OPTCADR,
    RS_RADCADR,
    RS_ATTCADR,
    RS_LGYRO,
    RS_FLAGWRD4,
    RS_NOUT,
    RS_SAMPLIM,
    RS_IMODES33,
    RS_SELFRET,
    RS_DSPCOUNT,
    RS_WORD_COUNT
};
enum rs_status { RS_OK=0, RS_OUT_OF_SCOPE=1, RS_RESOURCE_ALARM=2, RS_PHASE_BAD=01107 };
enum rs_alarm_entry { RS_ALARM1, RS_BAILOUT1, RS_POODOO1 };
typedef struct {
    agc_word failreg[3], almcadr[2], dsptab[12];
    agc_word word[RS_WORD_COUNT];
    agc_word channel11, channel12, channel13, channel14;
    agc_word trap31a, trap31b, trap32;
} restart_state;
typedef struct {
    agc_word complement[6], phase[6];
    exec_address address[6];
    agc_word priority[6];
} restart_phases;
/* Raw stages require caller exclusion and stopped counter/IRQ publication.
 * The entry pair is the explicit A/L input of ALARM1/BAILOUT1/POODOO1.
 * Alarm recording stops at return, WHIMPER, or OCT77770 respectively. */
enum rs_status rs_record(restart_state *, enum rs_alarm_entry, agc_word code,
                         exec_address caller);
/* STARTSB1 through its TC Q. Retains core/VAC payloads, RUPTAGN, alarm history
 * and protected phase records; clears native scheduler alarm markers only. */
void rs_cleanup(restart_state *, executive *, waitlist *);
/* GOPROG3 phase-pair agreement loop, stopping before MMDSPLAY or PTBAD.
 * Failure returns 1107 BEFORE the original PTBAD alarm/fresh-start body. */
enum rs_status rs_validate(executive *, const restart_phases *);
/* RESTARTS Type C variable JOB only, through ENDFIND/BAILOUT1.
 * group is zero-based, unlike the source's user-facing group numbers 1..6.
 * No table, display, timed task, longcall, or Type B route is translated. */
enum exec_status rs_job(executive *, const restart_phases *, uint8_t group);
/* For pending 1201/1202/1203 only (1204 takes POODOO1).
 * Preflight all demands, then bounded BAILOUT1 -> cleanup -> validation ->
 * Type C job admissions in descending group order. Does not execute jobs or
 * original WHIMPER/RESUME, LIGHTSET, MMDSPLAY or ENDRSTRT glue. Native runtimes
 * must be reinitialized explicitly after success; C stacks are not resumed.
 * Unsupported input makes no changes. A failure after cleanup is retained. */
enum rs_status rs_recover(restart_state *, executive *, waitlist *,
                          const restart_phases *, exec_address caller);
#endif
