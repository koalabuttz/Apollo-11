/* SPDX-License-Identifier: MIT */
/* Inter-bank communication (INTER-BANK_COMMUNICATION.agc): calling and
 * jumping between banks of fixed memory.
 *
 * The AGC addresses 02000-03777 of fixed memory through FBANK, one bank of
 * 02000 words at a time. Code in one bank cannot simply TC to code in
 * another: the call must change FBANK and transfer in one step, and the
 * return must put the caller's bank back. These routines, in fixed-fixed
 * memory (always addressable), do that for the whole program. A routine's
 * full address is a CADR: its bank in the upper bits, its address within
 * the bank in the low ten.
 *
 * Each entry is called with the machine as the original's TC leaves it and
 * returns with Z at the address the original continues at. A CADR that
 * follows the TC (TC BANKCALL; CADR X) is read from memory with
 * read_word(), which reaches the rope through the binding. */
#ifndef READABLE_INTERBANK_H
#define READABLE_INTERBANK_H

#include "../agc/agc.h"

/* Calls that return: the CADR inline, or in A. */
void bankcall(void);
void swcall_minus1(void);
void swcall(void);
void swreturn(void);

/* Jumps that do not return. */
void postjump(void);
void bankjump(void);
void q_plus_10000(void);

/* The return address of a BANKCALL, as a CADR. */
void makecadr(void);

/* One word of data from another bank. */
void supdacal(void);

/* BANKCALL, SWCALL and SWRETURN for interrupt routines. */
void ibnkcall(void);
void iswcall(void);
void iswretrn(void);

/* Entering the interpreter from basic code in another bank. */
void usprcadr(void);

/* Setting the superbank bits (channel 7). */
void supersw(void);

#endif
