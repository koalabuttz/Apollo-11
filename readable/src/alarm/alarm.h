/* SPDX-License-Identifier: MIT */
/* Alarms and aborts (ALARM_AND_ABORT.agc): what the program does when
 * something has gone wrong.
 *
 * An alarm is a code, an octal number like 1202: "AAANN, ALARM NO. NN IN
 * GENERAL AREA AAA". The routines here keep it for the crew and turn on
 * the PROG light. FAILREG +0 keeps the first code since the crew last
 * cleared them, FAILREG +1 the second, and FAILREG +2 the latest (its
 * sign bit set once more than three have come). ALMCADR keeps where the
 * last one came from. V05N09 shows the three codes on the DSKY.
 *
 * Then, by the routine called:
 *   - ALARM (and VARALARM, CURTAINS): carry on after the call;
 *   - PRIOLARM: also show V05N09 as a priority display;
 *   - BAILOUT and WHIMPER: a software restart (ENEMA), which restarts the
 *     jobs and tasks the restart tables say to;
 *   - POODOO: stop the mission program as well (the LM's goes to idle,
 *     GOPOODOO and SERVIDLE, unless the average-G servicer is running).
 *
 * The caller passes the code in the word after its TC (TC ALARM; OCT
 * 1202), in A (VARALARM, PRIOLARM), or as the routine's own constant
 * (CCSHOLE is 1103, CURTAINS 217).
 *
 * Each routine is entered with the machine as the caller's TC leaves it,
 * and leaves with Z at the address the original goes on to. Where the
 * original calls out (BANKCALL, SUPERSW, DOWNFLAG, POSTJUMP) or returns
 * into this file through MULTEXIT, the routine ends there, and the place
 * it comes back to is a routine of its own (its name says what it
 * follows). */
#ifndef READABLE_ALARM_H
#define READABLE_ALARM_H

#include "../agc/agc.h"

/* The alarm itself. */
void alarm(void);
void alarm2(void);

/* With the PROG light, a V05N09 priority display (bank 10). */
void priolarm(void);
void priolarm_after_alarm(void);

/* Alarms that return through ALMCADR. */
void curtains(void);
void curtains_after_alarm(void);
void varalarm(void);
void varalarm_after_alarm(void);
void almcadr_as_instruction(void);

/* Aborts. */
void bailout(void);
void whimper_minus1(void);
void whimper(void);
void poodoo(void);
void poodoo_after_alarm(void);
void ccshole(void);

#if LUMINARY099
void abort2(void);
void gopoodoo(void);
void gopoodoo_after_flags(void);
void gopoodoo_after_v37klean(void);
void gopoodoo_after_mr_klean(void);
void strtidle_after_supersw(void);
void bailout1(void);
void poodoo1(void);
void alarm1(void);
void almncadr(void);
void flags(void);
#else
void bailout_after_vac5stor(void);
void poodoo_after_vac5stor(void);
void ccshole_after_vac5stor(void);
void poodoo_after_stateflg(void);
void poodoo_after_reintflg(void);
void poodoo_after_nodoflag(void);
void poodoo_after_mr_klean(void);
#endif

#endif
