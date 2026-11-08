/* SPDX-License-Identifier: MIT */
/* INDEX in the RCS DAP (cm_dap.h): the address an indexed order names,
 * and the sums the faithful port refuses, which are not modelled here
 * either. The faithful C reads fixed memory under INDEX only from copies
 * of the file's tables; so does this code. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* The field plus the index: INDEX adds the index word's value. */
static int index_sum(unsigned field, word index)
{
    return (int)field + value_of(index);
}

/* The words of the file's tables that INDEX reads: GAIN1 and GAIN2 with
 * the three words before each, DACLIMIT with the words either side,
 * FREETAU's four and MANTABLE's nine. */
static bool a_table_word(unsigned address)
{
    return (address >= ADDRESS_OF(GAIN1) - 3u && address <= ADDRESS_OF(GAIN2) + 013u) ||
           (address >= ADDRESS_OF(DACLIMIT) - 1u && address <= ADDRESS_OF(DACLIMIT) + 1u) ||
           (address >= ADDRESS_OF(FREETAU) && address <= ADDRESS_OF(FREETAU) + 3u) ||
           (address >= ADDRESS_OF(MANTABLE) && address <= ADDRESS_OF(MANTABLE) + 010u);
}

unsigned index_read(unsigned field, word index)
{
    int sum = index_sum(field, index);

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !a_table_word((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* The double word's field (its address +1), checked as a read; in fixed
 * memory both words must be table words. A field of 0 names the low word
 * at 0, A itself, and the high word one before it, which wraps to 01777 of
 * the erasable bank (as the faithful port reads it). */
static unsigned double_field(unsigned address, word index)
{
    unsigned field = index_read(address + 1u, index);

    if (field >= 02000u && !a_table_word(field - 1u)) not_modelled();
    return field;
}

void dca_indexed(unsigned address, word index)
{
    unsigned field = double_field(address, index);

    if (field == 0u) {
        set_L(overflow_corrected(A));
        A = sign_extend(fetch(01777u));
        return;
    }
    dca(field - 1u);
}

void dcs_indexed(unsigned address, word index)
{
    unsigned field = double_field(address, index);

    if (field == 0u) {
        set_L(negate(overflow_corrected(A)));
        A = sign_extend(negate(fetch(01777u)));
        return;
    }
    dcs(field - 1u);
}

unsigned index_store(unsigned field, word index)
{
    int sum = index_sum(field, index);

    if (sum < 010 || sum > 01777) not_modelled();
    return (unsigned)sum;
}

/* DAS and DXCH: their field is the double word's address +1. */
void das_indexed(unsigned address, word index)
{
    das(index_store(address + 1u, index) - 1u);
}

void dxch_indexed(unsigned address, word index)
{
    dxch(index_store(address + 1u, index) - 1u);
}

unsigned index_ts(unsigned field, word index)
{
    int sum = index_sum(field, index);

    if (sum < 0 || sum > 01777 || (sum >= 4 && sum <= 6)) not_modelled();
    return (unsigned)sum;
}

#endif
