/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_H
#define CM_RESTART_H
#include "cm_executive.h"
#include "cm_waitlist.h"
/* Six CM restart records; transient view of the canonical image, not a second
 * persistent flight image. Word maps are source-specific. */
typedef struct {
 agc_word complement[6],phase[6];
 exec_address address[6];
 agc_word priority[6];
} cm_restart_phases;
enum cm_rs_word {
 CM_RS_TIME4,CM_RS_TIME5,CM_RS_DNTMGOTO,CM_RS_FLAGWRD2,CM_RS_DSRUPTSW,CM_RS_DELAYLOC,CM_RS_DELAYLOC1,CM_RS_DELAYLOC2,CM_RS_DELAYLOC3,CM_RS_R1SAVE,CM_RS_INLINK,CM_RS_DSPCNT,CM_RS_CADRSTOR,CM_RS_REQRET,CM_RS_CLPASS,CM_RS_DSPLOCK,CM_RS_MONSAVE,CM_RS_MONSAVE1,CM_RS_VERBREG,CM_RS_NOUNREG,CM_RS_DSPLIST,CM_RS_MARKSTAT,CM_RS_IMUCADR,CM_RS_OPTCADR,CM_RS_RADCADR,CM_RS_ATTCADR,CM_RS_LGYRO,CM_RS_FLAGWRD4,CM_RS_NOUT,CM_RS_EXTVBACT,CM_RS_SELFRET,CM_RS_DSPCOUNT,CM_RS_FLAGWRD6,CM_RS_RASFLAG,CM_RS_WORD_COUNT
};
typedef struct {
 agc_word failreg[3],almcadr[2],dsptab[12];
 agc_word word[CM_RS_WORD_COUNT];
 agc_word channel11,channel12,channel13,channel14;
 agc_word trap31a,trap31b,trap32;
} cm_restart_state;
enum cm_rs_status { CM_RS_OK=0,CM_RS_OUT_OF_SCOPE=1,CM_RS_RESOURCE_ALARM=2,CM_RS_PHASE_BAD=01107 };
/* Source-excluded STARTSB2 preserves TIME3/4/5 and protected restart records;
 * STARTSUB additionally resets those counters and DNTMGOTO. Payloads remain.
 * Native scheduler alarm markers clear at cleanup; no C stack resumes. */
void cm_rs_cleanup(cm_restart_state *,cm_executive *,cm_waitlist *);
void cm_rs_startsub(cm_restart_state *,cm_executive *,cm_waitlist *);
enum cm_rs_status cm_rs_validate(cm_executive *,const cm_restart_phases *);
/* Zero-based group, variable Type C job through original admission boundary.
 * Exact source scratch effects retained, including alarm mutations. */
enum exec_status cm_rs_job(cm_executive *,const cm_restart_phases *,uint8_t);
#endif
