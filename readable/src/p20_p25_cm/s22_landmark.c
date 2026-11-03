/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 10 (Comanche055 pp. 600-603): S22.1's first pass for
 * an unknown landmark (bank 30).
 *
 * With the landmark not known (KNOWNFLG clear; a known one goes on at
 * S22BOX22), S22.1 estimates where it is on the first mark and gives the
 * W matrix rows for it. The landmark
 * is put where the line of sight UM meets the earth's or moon's surface
 * (S22F2410, the "EQUATION 2.4.10", with the radius SETRE gives), at the
 * distance D along UM. The U matrix, I less (UM)(RL T)/D, carries the
 * errors of the CSM's position to the landmark's; with RHO, the sextant's
 * variance times the distance squared, and RPVAR, the variance of the
 * landmark's position, it makes the small E matrix (S22UUT), whose
 * triangular square root is the W matrix's W8 block: W72-W78, scaled to
 * meters B-19 (S22SCLW). The next part goes on at
 * S22SAVET. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* S22.1 +0145 -- pp. 600-603, KNOWNFLG clear: "UNIT ALSO PUTS ABVAL(RC) IN 36D", the CSM's
 * unit position in ALPHAV ("ALPHAV +4=SINL FOR SETRE"); LUNAFLAG as
 * CMOONFLG. S22C=I: "ERADM= R0 METERS B-29 BOTH EARTH/MOON" (SETRE), and
 * "COMPUTE RL FROM EQUATION 2.4.10" (S22F2410), "SCALE RL B-29 FOR BOTH
 * EARTH/MOON" to S22RL; S22D, "D=UM RL B-29".
 *
 * Then the push-down list's words 0-18D "= I BACKWARDS" (the identity, B-3:
 * zero vectors with HIDPHALF, shifted, in 4D, 8D and 12D's +0 words), and
 * the product "(UM)(RL T) B-30 STORED IN S22UMRL THRU S22UMRL +17D".
 * S22NXTU, row by row (X1 18, 12 and 6): "(UM)(RL T) B-32" over D,
 * "SUBTRACT FROM I B-3", the "U MATRIX B-3" in S22UMRL. S22NXTWI: "W(I+18)=
 * UW(I) B-19", the U matrix times each row of W0-W5 (W +36D on) into W6-W8
 * (W +144D on).
 *
 * S22RHO, the range (B-28, or with the moon's "MAKE RHO B-30"), normalized
 * (its shift added to X2), squared, times the sextant's variance (SCTVAR,
 * "B+16"), halved: "1/2(RHO SQ)(VARSCT)". S22NXXA: "1/2(RHO SQ)(VARSCT)(U)(U
 * T)" in S22UUT, the shift taken off ("0 -12D,2", "WITH VARRP SCALED
 * B-28"). Then "(UM)(UM T) B-2 IN S22UMRL" (S2231X13), and RPVAR, the
 * landmark's variance, times R0 over D squared ("TEMP (VARRP)(R0/D)") in
 * S22RHO; S22NXXB: the "SMALL E MATRIX", their sum in S22UUT, and "CLEAR
 * W8" (W +162D on), a row at a time.
 *
 * The W8 block from E0-E5, the small E matrix's six distinct words, each
 * a square root or a quotient skipped when its argument is negative or
 * zero, or it overflows: "W74= SQ ROOT E5", "W73= E4/W74", W72, E3/W74
 * (S22W72X); S22W76X: "W76= SQ ROOT (E2-W73 SQ)", "W75=
 * (E1-W72W73)/W76"; S22W78X: "W78= SQ RT(E0-W72 SQ-W75 SQ)". S22SCLW:
 * "SCALE W6 METERS B-19", the three vectors halved. */
static const word s22_unknown_words[] = {
    /* "UNIT ALSO PUTS ABVAL(RC) IN 36D" */
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(CSMPOS),
    /* "ALPHAV +4=SINL FOR SETRE" */
    STORE(ALPHAV),
    ORDERS(CLEAR, BOFF),
    SWITCH(CLEAR, LUNAFLAG),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(S22C_EQ_I),
    ORDER(SET),
    SWITCH(SET, LUNAFLAG),
    /* S22C=I: "ERADM= R0 METERS B-29 BOTH EARTH/MOON" */
    ORDER(CALL),
    ADDRESS(SETRE),
    /* "COMPUTE RL FROM EQUATION 2.4.10" */
    ORDER(CALL),
    /* "STORED IN X789,MPAC B-27,B-29" */
    ADDRESS(S22F2410),
    /* "SCALE RL B-29 FOR BOTH EARTH/MOON" */
    ORDERS(BOFF, VSR2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS_AT(S22C_EQ_I, 7), /* +1 */
    STORE(S22RL),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UM),
    /* "D=UM RL B-29" */
    STOVL(S22D),
    ADDRESS_PLUS_ONE(ZEROVECS),
    ORDERS(SETPD, PUSH),
    ADDRESS_PLUS_ONE_D(0),
    /* "SET 0-18D = I BACKWARDS" */
    ORDERS(PUSH, PDDL),
    /* "PD 18" */
    ADDRESS_PLUS_ONE(HIDPHALF),
    /* "B-3" */
    ORDER(SR2),
    STORE_D(4),
    STORE_D(8),
    STOVL_D(12),
    /* "B-1" */
    ADDRESS_PLUS_ONE(UM),
    STOVL(S223X1),
    /* "B-29" */
    ADDRESS_PLUS_ONE(S22RL),
    /* "(UM)(RL T) B-30 STORED IN S22UMRL THRU" */
    ORDER(CALL),
    /* "S22UMRL +17D" */
    ADDRESS(S2231X13),
    ORDERS(AXT_1, SSP),
    NUMBER(18),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    /* S22NXTU: "(UM)(RL T) B-32" */
    ORDERS(VLOAD_STAR, VSR2),
    ADDRESS_PLUS_ONE_AT_X1(S22UMRL, 18),
    ORDER(V_SC),
    /* "D B-29" */
    ADDRESS_PLUS_ONE(S22D),
    /* "SUBTRACT FROM I B-3" */
    ORDERS(BVSU, STADR),
    /* "U MATRIX B-3" */
    AFTER_STADR(STORE_AT_X1(S22UMRL, 18)),
    /* "PD 0 AFTER TIX" */
    ORDERS(TIX_1, AXT_1),
    ADDRESS(S22NXTU),
    /* "S1 STILL 6 FROM ABOVE" */
    NUMBER(36),
    /* S22NXTWI */
    ORDERS(VLOAD_STAR, MXV),
    /* "B-19" */
    ADDRESS_PLUS_ONE_AT_X1(W, 36),
    /* "B-3" */
    ADDRESS_PLUS_ONE(S22UMRL),
    ORDER(VSL3),
    /* "W(I+18)= UW(I) B-19" */
    STORE_AT_X1(W, 144),
    ORDERS(TIX_1, DLOAD),
    ADDRESS(S22NXTWI),
    /* "B-28,B-30" */
    ADDRESS_PLUS_ONE(S22RHO),
    /* "MAKE RHO B-30" */
    ORDERS(BOFF, SR2),
    SWITCH(BOF, CMOONFLG),
    ADDRESS_AT(S22NXTWI, 013), /* +1 */
    ORDERS(NORM, XAD_2),
    ADDRESS_PLUS_ONE(X2),
    ADDRESS(X2),
    ORDERS(DSQ, DMP),
    /* "B+16" */
    ADDRESS_PLUS_ONE(SCTVAR),
    /* "ACCOUNTS FOR 1/2 IN NEXT FORMULA" */
    ORDER(SR1),
    /* "1/2(RHO SQ)(VARSCT)" */
    STORE(S22RHO),
    ORDER(AXT_1),
    /* "S1 STILL 6 FROM ABOVE" */
    NUMBER(18),
    /* S22NXXA */
    ORDERS(VLOAD_STAR, MXV),
    /* "B-3" */
    ADDRESS_PLUS_ONE_AT_X1(S22UMRL, 18),
    /* "B-3" */
    ADDRESS_PLUS_ONE(S22UMRL),
    ORDERS(VXSC, VSR_STAR),
    ADDRESS_PLUS_ONE(S22RHO),
    /* "WITH VARRP SCALED B-28" */
    SR_X2(-12),
    /* "1/2(RHO SQ)(VARSCT)(U)(U T)" */
    STORE_AT_X1(S22UUT, 18),
    ORDERS(TIX_1, VLOAD),
    ADDRESS(S22NXXA),
    ADDRESS_PLUS_ONE(UM),
    /* "UM ALSO IN MPAC FOR S2231X13 SUBR." */
    STCALL(S223X1),
    /* "(UM)(UM T) B-2 IN S22UMRL,P17D" */
    ADDRESS(S2231X13),
    ORDERS(DLOAD, SR3),
    /* "B0 B-29" */
    ADDRESS_PLUS_ONE(ERADM),
    ORDERS(DDV, DSQ),
    /* "B-29" */
    ADDRESS_PLUS_ONE(S22D),
    ORDER(DMP),
    /* "***** METERS SQ" */
    ADDRESS_PLUS_ONE(RPVAR),
    /* "TEMP (VARRP)(R0/D)" */
    STORE(S22RHO),
    ORDER(AXT_1),
    /* "S1 STILL 6 FROM ABOVE" */
    NUMBER(18),
    /* S22NXXB */
    ORDERS(VLOAD_STAR, VXSC),
    /* "(UM)(UM T) B-2" */
    ADDRESS_PLUS_ONE_AT_X1(S22UMRL, 18),
    ADDRESS_PLUS_ONE(S22RHO),
    ORDER(VAD_STAR),
    ADDRESS_PLUS_ONE_AT_X1(S22UUT, 18),
    /* "SMALL E MATRIX" */
    STORE_AT_X1(S22UUT, 18),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),
    /* "CLEAR W8" */
    STORE_AT_X1(W, 162),
    ORDERS(TIX_1, BOV),
    ADDRESS(S22NXXB),
    ADDRESS_AT(S22NXXB, 014), /* +1 */
    ORDERS(DLOAD, BMN),
    /* "E5" */
    ADDRESS_PLUS_ONE_AT(S22UUT, 16),
    ADDRESS(S22W76X),
    ORDERS(SQRT, BZE),
    ADDRESS(S22W76X),
    /* "W74= SQ ROOT E5" */
    STODL_AT(W, 148),
    /* "E4" */
    ADDRESS_PLUS_ONE_AT(S22UUT, 14),
    ORDERS(DDV, BOV),
    ADDRESS_PLUS_ONE_AT(W, 148),
    ADDRESS(S22W72X),
    /* "W73= E4/W74" */
    STORE_AT(W, 146),
    /* S22W72X */
    ORDERS(DLOAD, DDV),
    /* "E3" */
    ADDRESS_PLUS_ONE_AT(S22UUT, 12),
    ADDRESS_PLUS_ONE_AT(W, 148),
    ORDER(BOV),
    ADDRESS(S22W76X),
    /* "W72= E4/W74" (E3 over W74, as the code divides) */
    STORE_AT(W, 144),
    /* S22W76X */
    ORDERS(DLOAD, DSQ),
    /* "W73" */
    ADDRESS_PLUS_ONE_AT(W, 146),
    ORDERS(BDSU, BMN),
    /* "E2" */
    ADDRESS_PLUS_ONE_AT(S22UUT, 8),
    ADDRESS(S22W78X),
    ORDERS(SQRT, BZE),
    ADDRESS(S22W78X),
    /* "W76= SQ ROOT (E2-W73 SQ)" */
    STODL_AT(W, 152),
    /* "W72" */
    ADDRESS_PLUS_ONE_AT(W, 144),
    ORDERS(DMP, BDSU),
    /* "W73" */
    ADDRESS_PLUS_ONE_AT(W, 146),
    /* "E1" */
    ADDRESS_PLUS_ONE_AT(S22UUT, 6),
    ORDERS(DDV, BOV),
    /* "W76" */
    ADDRESS_PLUS_ONE_AT(W, 152),
    ADDRESS(S22W78X),
    /* "W75= (E1-W72W73)/W76" */
    STORE_AT(W, 150),
    /* S22W78X */
    ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE_AT(W, 150),
    ORDERS(PDDL, DSQ),
    /* "W72" */
    ADDRESS_PLUS_ONE_AT(W, 144),
    ORDER(DAD),
    ORDERS(BDSU, BMN),
    /* "E0" */
    ADDRESS_PLUS_ONE(S22UUT),
    ADDRESS(S22SCLW),
    ORDER(SQRT),
    /* "W78= SQ RT(E0-W72 SQ-W75 SQ)" */
    STORE_AT(W, 156),
    /* S22SCLW: "SCALE W6 METERS B-19" */
    ORDERS(VLOAD, VSR1),
    ADDRESS_PLUS_ONE_AT(W, 144),
    STOVL_AT(W, 144),
    ADDRESS_PLUS_ONE_AT(W, 150),
    ORDER(VSR1),
    STOVL_AT(W, 150),
    ADDRESS_PLUS_ONE_AT(W, 156),
    ORDER(VSR1),
    STORE_AT(W, 156),
};

/* ---- The interpretive code ---- */

const struct interpretive_code S22_UNKNOWN_CODE = {030u, S22_1 + 0145u, COUNT(s22_unknown_words),
                                                   s22_unknown_words};

#endif
