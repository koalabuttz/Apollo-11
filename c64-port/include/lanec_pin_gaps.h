/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_GAPS_H
#define LANEC_PIN_GAPS_H
#include "lanec.h"
/* PINBALL_GAME_BUTTONS_AND_LIGHTS paths not covered elsewhere (both missions).
 *   dspdpdec  DSPDPDEC (verb 07), MIXBR 1 or 2 -> DSPALARM or TPAGREE;
 *             DSPDPDEC +13 (TPAGREE returns) -> DSP2DEC;
 *             ENDDPDEC (DSP2DEC returns) -> ENTEXIT (the saved ENTRET word).
 *             NOUNADD names any erasable word pair below 01377 or in the
 *             current EB bank.
 *   chanload  CHANLOAD (bank 41), NOUNCADR = channel; channel 7 or a write
 *             to channel 1-077 -> LOADLV.
 *   dsp2dec   DSP2DEC (bank 40) -> 11DSPIN; its returns +6 -> 11DSPIN,
 *             +8 -> DSPSIGN, +9 and END2DEC -> DSPDCWD1.
 *   dspdcwd1  DSPDCWD1 -1 and +0 (bank 40): one digit -> DSPIN, or the next
 *             digit; +16 (DSPIN returns) -> the next digit or WDRET.
 *   leadin    NVSUBB -> NVSUB1, LOADLV1 -> LOADLV, GOALMCYC -> ALMCYCLE (bank 41).
 *   small     LODSAMPT, FLASHON, FLASHOFF (fixed-fixed) -> the caller in Q;
 *             the lamp test's WAITLIST and NOVAC returns -> ENDOFJOB, TASKOVER.
 *   jumps     the POSTJUMP and BANKJUMP targets and their lead-ins: GOVNUPDT
 *             -> DSPDECVN, +1 -> POSTJUMP; DSPOCTIN -> DSPIN, +1 -> BANKJUMP
 *             (bank 40); UPDAT1 +2 -> UPDATRET; OCTBACK -> the next octal
 *             character (WDAGAIN, inline) and POSTJUMP, or WDRET (bank 41).
 *   dspsign   DSPSIGN (bank 40): +0, +010 after -ON and +014 after +ON;
 *             TC DSPWDRET runs the saved return as a TC to fixed memory.
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_pin_dspdpdec(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_chanload(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_dsp2dec(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_dspdcwd1(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_leadin(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_small(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_jumps(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_dspsign(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum lanec_status lanec_pin_gaps_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
