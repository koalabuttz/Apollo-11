/* SPDX-License-Identifier: MIT */
/* The landing guidance's indexed instructions and transfers: the machine
 * steps more than one of its files writes alike.
 *
 * INDEX at, then an instruction: the AGC adds the word at holds to the
 * whole instruction word, so an index can make it another instruction.
 * Each helper here runs the indexed instruction where the index leaves it
 * the same instruction on an erasable word, and calls not_modelled()
 * otherwise (as the faithful port, c64-port/src/lm_llge.c, refuses those
 * states). An address that is a double word's (DCA, DXCH, DAS) is written
 * one more than the address. */
#ifndef READABLE_LANDING_GUIDANCE_INDEXING_H
#define READABLE_LANDING_GUIDANCE_INDEXING_H

#include "../agc/agc.h"

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* INDEX at; CA address. */
static inline void ca_indexed(unsigned at, word address)
{
    word instruction = indexed_instruction((word)(030000u + address), fetch(at));

    if (OPERATION_CODE(instruction) != 3u || ADDRESS_FIELD(instruction) >= 02000u) not_modelled();
    ca(ADDRESS_FIELD(instruction));
}

/* EXTEND; INDEX at; DCA address. Not modelled too: a DCA of 1777, whose
 * second word is fixed memory's 2000. */
static inline void dca_indexed(unsigned at, word address)
{
    word instruction = indexed_instruction((word)(030000u + address + 1u), fetch(at));

    if (OPERATION_CODE(instruction) != 3u || ADDRESS_FIELD(instruction) == 0u ||
        ADDRESS_FIELD(instruction) >= 02000u)
        not_modelled();
    dca(ADDRESS_FIELD(instruction) - 1u);
}

/* INDEX at; DXCH address (an erasable instruction: its address field is
 * ten bits). Not modelled too: a pair that takes in a bank register or Z,
 * or an address field of 0 (DXCH -1). */
static inline void dxch_indexed(unsigned at, word address)
{
    word instruction = indexed_instruction((word)(052000u + address + 1u), fetch(at));
    word field = (word)(instruction & 01777u);

    if ((instruction & 076000u) != 052000u || field == 0u || (field <= 7u && field >= 3u))
        not_modelled();
    dxch(field - 1u);
}

/* EXTEND; INDEX at; MP address. */
static inline void mp_indexed(unsigned at, word address)
{
    word instruction = indexed_instruction((word)(070000u + address), fetch(at));

    if (OPERATION_CODE(instruction) != 7u || ADDRESS_FIELD(instruction) >= 02000u) not_modelled();
    mp_at(ADDRESS_FIELD(instruction));
}

/* INDEX at; DAS address. Not modelled too: a pair that takes in a bank
 * register or Z, or a DAS of 1777 (as DCA's). */
static inline void das_indexed(unsigned at, word address)
{
    word instruction = indexed_instruction((word)(020000u + address + 1u), fetch(at));
    word field = ADDRESS_FIELD(instruction);

    if (OPERATION_CODE(instruction) != 2u || field == 0u || field >= 02000u ||
        (field <= 7u && field >= 3u))
        not_modelled();
    das(field - 1u);
}

#endif
