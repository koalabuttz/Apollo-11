/* SPDX-License-Identifier: MIT */
/* The interpreter (INTERPRETER.agc): the program that runs the AGC's
 * interpretive language, the compact code in which most of the guidance
 * equations are written (VLOAD, DOT, UNIT, STORE ...).
 *
 * An interpretive program is a list of words in fixed memory. Each word is
 * an order-code pair (two seven-bit order codes, stored complemented so the
 * word reads negative), a store code (positive), or an address that an
 * order before it asked for. LOC holds the address of the word being
 * worked on, BANKSET the bank it is in, and the multi-purpose accumulator
 * MPAC (MPAC to MPAC +6) the scalar or vector the orders work on; MODE
 * says which (+1 triple precision, +0 double precision, -1 vector).
 *
 * The dispatcher (DANZIG, NEWOPS, OPJUMP) fetches each pair, splits it,
 * forms the address an order needs (ADDRESS, INDEX, PUSHUP), and jumps
 * through one of three tables (INDJUMP, MISCJUMP, UNAJUMP) to the order.
 * Every order ends at DANZIG, which runs the pair's other order or fetches
 * the next pair.
 *
 * How the C maps the listing:
 *   - each routine is a function named after its label (STYLE.md);
 *   - an order ends by setting Z, to DANZIG nearly always. The dispatcher's
 *     loop (DANZIG to the order and back) is not a C loop: each pass ends
 *     at DANZIG, and the binding (src/binding/interpreter_core.c), as the
 *     faithful port's compact interpreter, runs the next pass when the
 *     flight program wants it;
 *   - a subroutine the listing calls with TC is a C call after tc_at(site)
 *     (Q gets the return address, as the TC sets it), and it returns with
 *     the listing's TC Q, which leaves Z at the return and Q = 3. A routine
 *     that can leave the interpreter instead of returning (SQRTSUB's abort)
 *     says so in its result. */
#ifndef READABLE_INTERPRETER_H
#define READABLE_INTERPRETER_H

#include "../agc/agc.h"
#include "../agc/names.h"

/* The MPAC, BUF, BUF2 and VBUF words. */
#define MPAC_AT(n) (ADDRESS_OF(MPAC) + (n))
#define BUF_AT(n)  (ADDRESS_OF(BUF) + (n))
#define BUF2_AT(n) (ADDRESS_OF(BUF2) + (n))
#define VBUF_AT(n) (ADDRESS_OF(VBUF) + (n))

/* The words the push-down list holds below PUSHLOC: INDEX PUSHLOC on the
 * listing's 0 -4, 0 -2 and 0 -1 (the assembler leaves the address field
 * one less, and INDEX's add makes it DXCH or TS PUSHLOC -n). */
#define PUSHED(n) ((unsigned)(PUSHLOC - (n)))

/* ---- The dispatcher (dispatcher.c, LM pp. 1002-1013) ---- */

void intpret(void);
void intpret_plus2(void);
void intpret_plus3(void);
void intrsm(void);
void dload(void);
void sload2(void);
void sload2_plus2(void);
void newmode(void);
void danzig(void);
void noibnksw(void);
void newops(void);
void diradres(void);
void dodload_star(void);
void dodload_star_plus1(void);
void fifteen_bitadr(void);

/* The jump tables: the orders by code (INDJUMP 00-37, MISCJUMP 00-17,
 * UNAJUMP by the unary code less one). */
void indjump(unsigned order);
void miscjump(unsigned order);
void unajump(unsigned order);

/* ---- Loading and storing (load_store.c, LM pp. 1014-1022) ---- */

void stadr(void);
void dostore(void);
void store_(void);
void dodload(void);
void dovload(void);
void dovload_star(void);
void tload(void);
void tload_plus6(void);
void tmode(void);
void sload(void);
void vload(void);
void vmode(void);
void pddl(void);
void endtpush_plus2(void);
void pdvl(void);
void ssp(void);
void store1(void);

/* ---- Sequence changing (sequence.c, LM pp. 1023-1025) ---- */

void ccall(void);
void call(void);
void goto_(void);
void goto_plus1(void);
void goto_plus4(void);
void cgoto(void);
void swbranch(void);

/* BRANCH: which way MPAC's triple-precision sign sent the caller, which
 * TC BRANCH continues at L +1, L +2 or L +3 (Z and Q as the original
 * leaves them). */
enum branch { BRANCH_POSITIVE, BRANCH_ZERO, BRANCH_NEGATIVE };
enum branch branch(void);
void branch_plus4(void);
void q_plus_1(void);
void neg(void);
void neg_plus1(void);
void exit_(void);

/* ---- Add, subtract, multiply, dot, polynomials (arithmetic.c, LM pp.
 * 1026-1037) ---- */

void vsu(void);
void vad(void);
void vad_plus1(void);
void dad(void);
void endvxv(void);
void setovf(void);
void dsu(void);
void overflwz(void);
void overflwy(void);
void overflow(void);
void bvsu(void);
void bdsu(void);
void tad(void);
void dmp(void);
void dmpsub_minus1(void);
void dmpsub(void);
void dmpsub2(void);
void roundsub(void);
void roundsub_plus1(void);
void vround(void);
void setovf2(void);
void predot(void);
void dotsub(void);
void enddot(void);
void powrsers(void);
void poly(void);
void dpagree(void);
void tpagree(void);
void shortmp(void);
void shortmp_plus1(void);
void shortmp2(void);
void dmpnsub(void);

/* ---- Vector orders, SIGN, COMP (vectors.c, LM pp. 1038-1046) ---- */

void dot(void);
void dmode(void);
void mxv(void);
void vxm(void);
void vxsc(void);
void vrotatex(void);
void vproj(void);
void vxv(void);
void mpacvbuf(void);
void alsignag(void);
void dmp1(void);
void dmpr(void);
void ddv(void);
void bddv(void);
void setpd(void);
void tslc(void);
void gshift(void);
void v_sc(void);
void sign(void);
void comp(void);
void dcomp(void);

/* ---- Shifts (shifts.c, LM pp. 1047-1056) ---- */

void shortt(void);
void mpacshr_plus3(void);
void vshrrnd(void);
void tssl_plus1(void);
void tssl_plus2(void);
void round_(void);
void shortv(void);
void vssl(void);
void tslc2(void);
void genshift(void);
void genscr_plus4(void);

/* ---- Division (divide.c, LM pp. 1057-1069) ---- */

void ddv_bddv(void);
void dvovf(void);
void ovf_plus_minus1(void);
void into_maxtest(word base, unsigned k);
/* MAXTEST -n: the prologues that shift the dividend before the divide
 * (n = 1 to 14, and 21: MAXTEST -21D, the special UNIT prologue). */
void maxtest_minus(unsigned n);
void maxtest(void);
void finaldv_plus3(void);
void v_sc2(void);
void vecagree(void);

/* ---- UNIT, unary orders, squaring, SQRT (unit_sqrt.c, LM pp.
 * 1070-1081) ---- */

void unit(void);
void unit_plus1(void);
void unit_plus16(void);
void dsq(void);
void abvalabs(void);
void sqrt_(void);
void abs_(void);
void vdef(void);
void vsq(void);
void push(void);
void rvq(void);
void dsqsub(void);
void vsqsub(void);
/* SQRTSUB: true when it aborted (alarm 1302) instead of returning. */
bool sqrtsub(void);

/* ---- Trigonometry (trig.c, LM pp. 1082-1086) ---- */

void cosine(void);
void sine(void);
void sine_plus1(void);
void sn1_plus16(void);
void arcsin(void);
void arccos(void);
void arccos_plus1(void);

/* ---- Index registers and the miscellaneous orders (index_orders.c, LM
 * pp. 1087-1091) ---- */

void axt(void);
void axc(void);
void lxa(void);
void lxc(void);
void sxa(void);
void xchx(void);
void xad(void);
void incr_(void);
void xsu(void);
void tix(void);
void fifteen_adrers(void);
void tagsub(void);
void rtb_bhiz(void);
void rtb(void);
void bhiz_plus1(void);
void bov_b(void);
void bze_goto(void);
void bpl_bmn(void);
void call_ita(void);

/* ---- Switches (switches.c, LM pp. 1092-1094) ---- */

void switches(void);

#endif
