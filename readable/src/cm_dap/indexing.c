/* SPDX-License-Identifier: MIT */
/* INDEX in the CM's RCS autopilot (cm_dap.h): the address an indexed order
 * names, and the sums the faithful port refuses, which are not modelled
 * here either. The faithful C reads fixed memory under INDEX only from
 * copies of each file's tables; so does this code. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* The field plus the index: INDEX adds the index word's value. */
static int index_sum(unsigned field, word index)
{
    return (int)field + value_of(index);
}

/* RCS-CSM_DIGITAL_AUTOPILOT's tables: GAIN1 and GAIN2 with the three words
 * before each, DACLIMIT with the words either side, FREETAU's four and
 * MANTABLE's nine. */
bool rcs_dap_tables(unsigned address)
{
    return (address >= ADDRESS_OF(GAIN1) - 3u && address <= ADDRESS_OF(GAIN2) + 013u) ||
           (address >= ADDRESS_OF(DACLIMIT) - 1u && address <= ADDRESS_OF(DACLIMIT) + 1u) ||
           (address >= ADDRESS_OF(FREETAU) && address <= ADDRESS_OF(FREETAU) + 3u) ||
           (address >= ADDRESS_OF(MANTABLE) && address <= ADDRESS_OF(MANTABLE) + 010u);
}

/* AUTOMATIC_MANEUVERS's: MINTAU's four, BIT13 and the two bits after it,
 * and the forced firing masks OCT01400, OCT00060 and OCT00300. */
bool maneuver_tables(unsigned address)
{
    return (address >= ADDRESS_OF(MINTAU) && address <= ADDRESS_OF(MINTAU) + 3u) ||
           (address >= ADDRESS_OF(BIT13) && address <= ADDRESS_OF(BIT13) + 2u) ||
           (address >= ADDRESS_OF(OCT01400) && address <= ADDRESS_OF(OCT01400) + 2u);
}

/* JET_SELECTION_LOGIC's: the translation biases XLNNDX and XLN1NDX, the
 * jet tables PYTABLE, RTABLE and YZTABLE, and the words about NJET and
 * DFTMAX (four either side), which the jet on-times read. */
bool jet_select_tables(unsigned address)
{
    return (address >= ADDRESS_OF(XLNNDX) && address <= ADDRESS_OF(XLNNDX) + 3u) ||
           (address >= ADDRESS_OF(XLN1NDX) && address <= ADDRESS_OF(XLN1NDX) + 3u) ||
           (address >= ADDRESS_OF(PYTABLE) && address <= ADDRESS_OF(PYTABLE) + 016u) ||
           (address >= ADDRESS_OF(RTABLE) && address <= ADDRESS_OF(RTABLE) + 016u) ||
           (address >= ADDRESS_OF(YZTABLE) && address <= ADDRESS_OF(YZTABLE) + 010u) ||
           (address + 4u >= ADDRESS_OF(NJET) && address <= ADDRESS_OF(NJET) + 4u) ||
           (address + 4u >= ADDRESS_OF(DFTMAX) && address <= ADDRESS_OF(DFTMAX) + 4u);
}

unsigned index_read_in(fixed_tables *tables, unsigned field, word index)
{
    int sum = index_sum(field, index);

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !tables((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* The double word's field (its address +1), checked as a read; in fixed
 * memory both words must be table words. A field of 0 names the low word
 * at 0, A itself, and the high word one before it, which wraps to 01777 of
 * the erasable bank (as the faithful port reads it). */
static unsigned double_field(fixed_tables *tables, unsigned address, word index)
{
    unsigned field = index_read_in(tables, address + 1u, index);

    if (field >= 02000u && !tables(field - 1u)) not_modelled();
    return field;
}

void dca_indexed_in(fixed_tables *tables, unsigned address, word index)
{
    unsigned field = double_field(tables, address, index);

    if (field == 0u) {
        set_L(overflow_corrected(A));
        A = sign_extend(fetch(01777u));
        return;
    }
    dca(field - 1u);
}

void dcs_indexed_in(fixed_tables *tables, unsigned address, word index)
{
    unsigned field = double_field(tables, address, index);

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
