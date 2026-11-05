/* SPDX-License-Identifier: MIT */
/* P51-P53.agc: INDEX before CA, TS and DXCH, as R52, R59 and PLANET use
 * it (parts 6 and 7). */
#include "p51_p53.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- INDEX before CA, TS and DXCH ---- */

/* INDEX adds the index to the whole instruction word after it
 * (indexed_instruction). Not modelled: an index that turns it into
 * another instruction, or that points a TS or a DXCH at the bank
 * registers or Z (EBANK, FBANK, Z, BBANK), which no program here does. */
unsigned indexed_ca(unsigned address, word index)
{
    word instruction = indexed_instruction((word)(030000u | address), index);

    if (OPERATION_CODE(instruction) != 3u) not_modelled();
    return ADDRESS_FIELD(instruction);
}

unsigned indexed_ts(unsigned address, word index)
{
    word instruction = indexed_instruction((word)(054000u | address), index);
    unsigned to = instruction & 01777u;

    if ((instruction & 076000u) != 054000u || (to >= EBANK_ADDRESS && to <= 6u)) not_modelled();
    return to;
}

/* DXCH k is written k + 1. */
unsigned indexed_dxch(unsigned address, word index)
{
    word instruction = indexed_instruction((word)(052000u | (address + 1u)), index);
    unsigned to = (instruction & 01777u) - 1u;

    if ((instruction & 076000u) != 052000u || (to <= 6u && to + 1u >= EBANK_ADDRESS))
        not_modelled();
    return to;
}

#endif
