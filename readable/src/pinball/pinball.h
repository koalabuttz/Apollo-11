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
 * the word display routines (LM pp. 441-445, CM pp. 359-363). Part 2b,
 * ENTER and the verb fan (LM pp. 406-412, CM pp. 323-329), with the
 * fixed-fixed subroutines they call. Part 3, the display verbs and the
 * scale factor routines for display (LM pp. 414-424, CM pp. 331-342), with
 * the scale factor code subroutines (LM p. 428, CM p. 346) and the triple
 * word shifts (TPLEFTN, TPSL1, PRSHRTMP). The entries
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

/* ---- Part 2b: ENTER and the verb fan (enter.c, subroutines.c) ---- */

void nvsubb(void);
void loadlv1(void);
void enterjmp(void);
void enter(void);
void entpas0(void);
void reqadd(void);
void reqadd_after_reqdatz(void);
void useadd(void);
void intmctbs(void);
void intmctbs_plus2_entry(void);
void intmctbs_after_dspoctwd(void);
void verbfan_minus2(void);
void verbfan(void);
void verbfan_after_reldsp(void);
void vbfandir(void);
void mixnoun(void);
void reqdatx(void);
void reqdaty(void);
void reqdatz(void);
void reqcom_after_5blank(void);
void updatnn_minus1(void);
void updatnn(void);
void updatvb_minus1(void);
void updatvb(void);
void updat1_plus2(void);
void goalmcyc(void);
void godspalm(void);

/* The subroutines ENTER and the verb fan call, in fixed-fixed but SFRUTMIX
 * (bank 41): each goes back with TC Q (or through its return word, RELRET
 * or EXITEM). DPTEST is true for a double precision routine. */
void setncadr(void);
void setncadr_plus1(void);
void setnadd(void);
void setebank(void);
void right5(void);
void left5(void);
void falton(void);
void flashon(void);
void flashoff(void);
void reldsp(void);
void reldsp1(void);
void almcycle(void);
void almcycle_after_updatvb(void);
void sfrutmix(void);

bool dptest(void);

/* ---- Part 3: the display verbs (display_verbs.c, output_scale.c,
 * subroutines.c) ---- */

void getcomp(void);
void comptest(void);
void comptst1(void);
void dectest(void);
void dcomptst(void);
void dctstcyc(void);
void nountest(void);
void dspabc(void);
void dspab(void);
void dspa(void);
void dspb(void);
void dspc(void);
void dspcom3_after_dspoctwd(void);
void chandsp(void);
void decdsp(void);
void dspdcput(void);
void dspdcend(void);
void dspdcend_plus2(void);
void dspdpdec(void);
void dspdpdec_plus13(void);
void enddpdec(void);
void degoutsf(void);
#if LUMINARY099
void three_sixty_cduo(void);
void three_sixty_cdu(void);
void three_sixty_cdue(void);
void lrposout(void);
#else
void opdegout(void);
void dpfracot(void);
#endif
void artoutsf(void);
void arout1sf(void);
void dp1outsf(void);
void dp2outsf(void);
void dp3outsf(void);
void two_intout(void);
void two_intout_plus1(void);
void two_intout_plus2(void);
void two_intout_plus4(void);
void two_intout_plus11(void);
void readlo(void);
void hmsout(void);
void hmsout_plus2(void);
void hmsout_plus4(void);
void hmsout_plus8(void);
void hmsout_plus10(void);
void hmsout_plus19(void);
void hmsout_plus21(void);
void hmsout_plus28(void);
void hmsout_plus30(void);
void m_sout(void);
void m_sout_plus2(void);
void m_snorm_plus1(void);
void m_snorm_plus5(void);
void m_snorm_plus7(void);
void m_snorm_plus15(void);
void m_snorm_plus23(void);
void m_snorm_plus25(void);

/* Part 4: the load verbs (load_verbs.c) and the scale factor routines for
 * loading (input_scale.c). */
void abcload(void);
void abcload_plus2(void);
void abcload_plus3(void);
void abcload_plus5(void);
void abcload_plus6(void);
void abcload_plus8(void);
void abcload_plus9(void);
void abcload_plus11(void);
void putxyz(void);
void putxyz_plus7(void);
void putxyz_plus11(void);
void putxyz_plus15(void);
void putxyz_plus22(void);
void bitsoff(void);
void bitsoff1(void);
void abload(void);
void abload_plus2(void);
void abload_plus3(void);
void abload_plus5(void);
void abload_plus6(void);
void abload_plus8(void);
void putxy(void);
void putxy_plus7(void);
void putxy_plus11(void);
void aload(void);
void aload_plus1(void);
void aload_plus6(void);
void bload(void);
void bload_plus2(void);
void bload_plus5(void);
void bload_plus10(void);
void cload(void);
void cload_plus2(void);
void cload_plus5(void);
void cload_plus10(void);
void loadlv(void);
void loadlv_plus5(void);
void alldc_oc(void);
void goq(void);
void sfret1(void);
void sfret(void);
void conumnor(void);
void putcom(void);
void putdpcom(void);
void putnorm(void);
void putcom2(void);
void chanload(void);
void putdecsf(void);
void putsfnor(void);
void putdcsf2(void);
void two_round(void);
void two_round_plus2(void);
void two_rndend(void);
void testofuf(void);
void deginsf(void);
void deginsf2(void);
void signfix(void);
void endscale(void);
void neg180(void);
void sgnto1(void);
#if LUMINARY099
void sfmixcal_plus2(void);
void sfnorcal_plus2(void);
#else
void optdegin(void);
void opdegin2(void);
#endif
void arthinsf(void);
void binround(void);
void artin1sf(void);
void dpinsf(void);
void dpinsf_plus2(void);
void dpinsf_plus9(void);
void dpinsf2(void);
void dpinsf4(void);
void hmsin(void);
void rnd_tst(void);
void mpactst(void);
void sizetst(void);
void all3dec(void);
void forcev25(void);

/* Called from part 4: DCTSTCYC, true when it went back (not to ALMCYCLE);
 * and the noun tables' reader, by EXTEND; DCA LODNNLOC; DXCH Z at at,
 * back at at +1 (enter.c). */
bool dctstcyc_returns(void);
void read_noun_tables(word at);
/* The scale factor codes and the triple word shifts they use. */
void sfrutnor(void);
void sfconum(void);
void tpleftn(void);
void leftncom(void);
void tpsl1(void);
void prshrtmp(void);

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

/* ---- INDEX MIXBR ---- */

/* INDEX MIXBR; TC +0 at at: the TC goes to the word after it for a normal
 * noun (MIXBR 1) and to the one after that for a mixed noun (MIXBR 2).
 * True for a mixed noun (display_verbs.c). */
bool mixed_noun(word at);

#endif
