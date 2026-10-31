/* SPDX-License-Identifier: MIT */
/* PINBALL_NOUN_TABLES.agc: the readers -- LM pp. 304-305, CM pp. 270-271,
 * the same code in both. The faithful C is c64-port/src/noun_reader.c.
 *
 * "THE FOLLOWING ROUTINES ARE FOR READING THE NOUN TABLES AND THE SF
 * TABLES (WHICH ARE IN A SEPARATE BANK FROM THE REST OF PINBALL). THESE
 * READING ROUTINES ARE IN THE SAME BANK AS THE TABLES. THEY ARE CALLED BY
 * DXCH Z."
 *
 * DXCH Z puts the caller's return 2CADR in A and L and jumps; each reader
 * first parks it (DXCH into a pair of words it is about to fill anyway),
 * and last takes it back and leaves with DXCH Z again. The tables are read
 * as the listing reads them, INDEX on the table's name: INDEX k; CAF TAB is
 * ca(indexed(ADDRESS_OF(TAB), k)). */
#include "pinball_noun_tables.h"
#include "../agc/names.h"

static void lodmixnn(void);
static void lodnlv(void);
static void sfcom(void);

/* LODNNTAB -- LM p. 304, CM p. 270.
 * "LODNNTAB LOADS NNADTEM WITH THE NNADTAB ENTRY, NNTYPTEM WITH THE
 * NNTYPTAB ENTRY. IF THE NOUN IS MIXED, IDADITEM IS LOADED WITH THE FIRST
 * IDADDTAB ENTRY, IDAD2TEM THE SECOND IDADDTAB ENTRY, IDAD3TEM THE THIRD
 * IDADDTAB ENTRY, RUTMXTEM WITH THE RUTMXTAB ENTRY. MIXBR IS SET FOR MIXED
 * OR NORMAL NOUN."
 * The noun is NOUNREG. Nouns 40 and up are mixed: CS NOUNREG; AD MIXCON is
 * 40 less the noun, and BZMF branches on zero or less. */
void lodnntab(void)
{
    dxch(ADDRESS_OF(IDAD2TEM)); /* "SAVE RETURN INFO IN IDAD2TEM, IDAD3TEM." */
    ca(indexed(ADDRESS_OF(NNADTAB), NOUNREG));
    ts(ADDRESS_OF(NNADTEM));
    ca(indexed(ADDRESS_OF(NNTYPTAB), NOUNREG));
    ts(ADDRESS_OF(NNTYPTEM));
    cs(ADDRESS_OF(NOUNREG));
    ad(MIXCON);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF LODMIXNN */
        lodmixnn();             /* "NOUN NUMBER G/E FIRST MIXED NOUN" */
        return;
    }
    set_A(ONE);                    /* "NOUN NUMBER L/ FIRST MIXED NOUN" */
    ts(ADDRESS_OF(MIXBR));         /* "NORMAL. +1 INTO MIXBR" */
    tc_at((word)(LODNNTAB + 13u)); /* TC LODNLV */
    lodnlv();
}

/* LODMIXNN -- LM p. 304, CM p. 270.
 * A mixed noun: MIXBR +2, its routine codes from RUTMXTAB ("FIRST MIXED
 * NOUN = 40", so the table is read from 40 words before it), and its three
 * component addresses from IDADDTAB, starting where NNADTAB's low ten bits
 * say. Q holds that start for a moment ("TEMP"), and is left so. The
 * second and third go to A and L (DCA), for LODNLV's DXCH to put in
 * IDAD2TEM and IDAD3TEM. */
static void lodmixnn(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(MIXBR));                            /* "MIXED. +2 INTO MIXBR." */
    ca(indexed(ADDRESS_OF(RUTMXTAB) - 40u, NOUNREG)); /* CAF RUTMXTAB -40D */
    ts(ADDRESS_OF(RUTMXTEM));
    set_A(LOW10);
    mask_at(ADDRESS_OF(NNADTEM));
    ts(Q_ADDRESS);                               /* "TEMP" */
    ca(indexed(ADDRESS_OF(IDADDTAB), a_word())); /* INDEX A; CAF IDADDTAB */
    ts(ADDRESS_OF(IDAD1TEM));                    /* "LOAD IDAD1TEM WITH FIRST IDADDTAB ENTRY" */
    dca(indexed(ADDRESS_OF(IDADDTAB) + 1u, (word)Q)); /* EXTEND; INDEX Q; DCA IDADDTAB +1 */
    lodnlv();
}

/* LODNLV -- LM p. 304, CM p. 270.
 * "PUT RETURN INFO INTO A, L." and back to the caller. A normal noun's
 * DXCH leaves in IDAD2TEM and IDAD3TEM what A and L hold: the +1 put in
 * MIXBR, and whatever L was. */
static void lodnlv(void)
{
    dxch(ADDRESS_OF(IDAD2TEM));
    dtcb((word)(LODNLV + 1u)); /* DXCH Z */
}

/* GTSFOUT -- LM pp. 304-305, CM p. 271.
 * "GTSFOUT LOADS SFTEMP1, SFTEMP2 WITH THE DP SFOUTAB ENTRIES."
 * "2X(SFCONUM) ARRIVES IN SFTEMP1": twice the scale factor constant code,
 * the double word's index. */
void gtsfout(void)
{
    dxch(ADDRESS_OF(SFTEMP1));
    dca(indexed(ADDRESS_OF(SFOUTAB), a_word())); /* EXTEND; INDEX A; DCA SFOUTAB */
    sfcom();
}

/* SFCOM -- LM p. 305, CM p. 271.
 * The double word into SFTEMP1 and SFTEMP2, the return 2CADR out, and back
 * to the caller. */
static void sfcom(void)
{
    dxch(ADDRESS_OF(SFTEMP1));
    dtcb((word)(SFCOM + 1u)); /* DXCH Z */
}

/* GTSFIN -- LM p. 305, CM p. 271.
 * "GTSFIN LOADS SFTEMP1, SFTEMP2 WITH THE DP SFINTAB INTRIES." The same,
 * from SFINTAB; TCF SFCOM. */
void gtsfin(void)
{
    dxch(ADDRESS_OF(SFTEMP1));                   /* "2X(SFCONUM) ARIVES IN SFTEMP1." */
    dca(indexed(ADDRESS_OF(SFINTAB), a_word())); /* EXTEND; INDEX A; DCA SFINTAB */
    sfcom();
}

/* ---- Where the tables are ---- */

/* Each table's words at its place in bank 42, by the physical address of
 * its first word (the listing's symbol table). */
word noun_table_word(unsigned physical)
{
    static const struct {
        unsigned at, count;
        const word *words;
    } tables[] = {
        {P_NNADTAB, 100u, NNADTAB},        {P_NNTYPTAB, 100u, NNTYPTAB},
        {P_SFINTAB, SFTAB_WORDS, SFINTAB}, {P_SFOUTAB, SFTAB_WORDS, SFOUTAB},
        {P_IDADDTAB, 180u, IDADDTAB},      {P_RUTMXTAB, 60u, RUTMXTAB},
    };
    unsigned t;

    for (t = 0; t < sizeof tables / sizeof tables[0]; ++t)
        if (physical >= tables[t].at && physical < tables[t].at + tables[t].count)
            return tables[t].words[physical - tables[t].at];
    not_modelled();
    return 0;
}
