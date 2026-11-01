/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc: the DSKY's keys, verbs, nouns and
 * displays (Luminary099 pages 390-471, Comanche055 pages 308-389). The
 * faithful C is the port's c64-port/src/lanec_pin_*.c and lanec_pw_*.c.
 *
 * KEYRUPT puts each key's code in MPAC and starts CHARIN as a job. CHARIN
 * sends the key to its routine: a digit is assembled into the verb, the
 * noun or one of the three data registers (NUM), VERB and NOUN start a new
 * verb or noun, + and - give a decimal load its sign, CLEAR blanks the
 * register being loaded, ENTER executes the verb or loads the data, KEY
 * RELEASE gives the DSKY back to a flashing display and ERROR LIGHT RESET
 * puts out the lights.
 *
 * This header grows with the parts written. Part 1, the keys (LM pp.
 * 398-405, CM pp. 315-322): CHARIN, the number, verb, noun, sign and clear
 * keys and the blanking routines they use; with them DSPIN and 11DSPIN,
 * which put a character into the display table (LM pp. 444-445, CM pp.
 * 362-363), and SLEFT5 and RELDSPON (LM p. 461, CM p. 379). Part 2a,
 * the word display routines (LM pp. 441-445, CM pp. 359-363). The entries
 * are the routines' labels and the words a call out of the part returns
 * to. */
#ifndef READABLE_PINBALL_H
#define READABLE_PINBALL_H

#include <stdbool.h>
#include "../agc/agc.h"

/* ---- Part 1: the keys (keys.c, dspin.c, subroutines.c) ---- */

void charin(void);
void eighty_nine_test(void);
void num_minus2(void);
void num(void);
void verb(void);
void noun(void);
void negsgn(void);
void posgn(void);
void plus_on(void);
void minus_on(void);
void clear(void);
void clpashi_after_updatvb(void);
void clr5(void);
void five_blank_minus1(void);
void five_blank(void);
void five_blank_plus2(void);
void two_blank(void);
void dspin(void);
void eleven_dspin(void);
void sleft5(void);
void reldspon(void);

/* ---- Part 2a: the word display routines (display_words.c) ---- */

void dspsign(void);
void dspsign_after_minus_on(void);
void dspsign_after_plus_on(void);
void dsprnd(void);
void dspdecwd(void);
void dspdecwd_after_dspsign(void);
void dspdcwd1_minus1(void);
void dspdcwd1(void);
void dspdcwd1_after_dspin(void);
void dspdecnr(void);
void dspdecnr_after_dspsign(void);
void dspdc2nr(void);
void dspdc2nr_plus3(void);
void dsp2dec(void);
void dsp2dec_after_minus_r2(void);
void dsp2dec_after_plus_r2(void);
void dsp2dec_after_dspsign(void);
void end2dec(void);
void dspdecvn(void);
void govnupdt(void);
void govnupdt_after_dspdecvn(void);
void dspoctwd(void);
void wdagain(void);
void wdagain_plus5(void);
void octback(void);
void dspoctin(void);
void dspoctin_after_dspin(void);
void dsp2bit(void);

/* Part 1's routines called from within the part: each returns true when
 * it went back to its caller (TC Q, TC L or TC DSEXIT), false when it left
 * for another routine (ENDOFJOB, CCSHOLE). */
bool dspin_returns(void);
bool eleven_dspin_returns(void);
bool two_blank_returns(void);

/* ---- Leaving a routine ---- */

/* TC at fixed address at to routine: Q gets the return, at +1. */
static inline void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q, TC L or TC DSEXIT: back to the return address the word holds. The
 * AGC runs the word as an instruction, a TC to that address, so Q is left
 * the word's address +1. A word that holds no such TC (a register's
 * address, or not a TC at all) the readable port does not model. */
static inline void tc_return(unsigned address)
{
    word to = (word)agc.memory[erasable_index(address)];

    if (to < 010u || to > 07777u) not_modelled();
    tc_through(address);
}

/* ---- INDEX ---- */

/* INDEX x before an instruction on address: the address x words on, as
 * the AGC adds x to the instruction. An x that would carry out of the
 * instruction's address field (and change the instruction) the readable
 * port does not model. */
static inline word indexed_by(word address, word x)
{
    word at = indexed_instruction(address, x);

    if (at > 07777u) not_modelled();
    return at;
}

/* The same for an instruction that writes the word (TS, XCH): it must be
 * an ordinary erasable word. */
static inline word indexed_erasable(word address, word x)
{
    word at = indexed_by(address, x);

    if (at < 010u || at >= 02000u) not_modelled();
    return at;
}

#endif
