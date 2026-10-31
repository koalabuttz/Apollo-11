/* SPDX-License-Identifier: MIT */
/* PINBALL_NOUN_TABLES.agc: the noun tables and the scale factor tables,
 * and the three routines that read them (LM pp. 301-319, CM pp. 268-284).
 * The faithful C is c64-port/src/noun_reader.c, with the tables in
 * c64-port/src/fixed_data.c.
 *
 * PINBALL (the DSKY's verb and noun program) needs, for each noun the crew
 * keys, where its data is and how to show it. The noun tables say so:
 *
 *   NNADTAB   the noun's address. A normal noun (00-39) has one: the ECADR
 *             of its first word. A mixed noun (40-99), of up to three
 *             components each at its own address, has instead its
 *             components' codes and where its three addresses start in
 *             IDADDTAB.
 *   NNTYPTAB  the noun's type: how many components, and for each the scale
 *             factor routine (how to convert, in RUTMXTAB for a mixed
 *             noun) and the scale factor constant (by how much, a code
 *             into SFINTAB and SFOUTAB).
 *   SFINTAB   the scale factors for loading (decimal keyed to the machine's
 *             units) and SFOUTAB for display (the machine's units to
 *             decimal), a double word per constant code.
 *   IDADDTAB  three ECADRs per mixed noun.
 *   RUTMXTAB  three scale factor routine codes per mixed noun.
 *
 * The tables are in bank 42, apart from the rest of PINBALL, so PINBALL
 * reads them through three routines in the same bank, which it calls with
 * DXCH Z (A and L hold the return 2CADR): LODNNTAB, a noun's entries;
 * GTSFOUT and GTSFIN, a scale factor. Here the tables are C arrays holding
 * the listing's words in the listing's notation (tables.c), and the
 * routines (readers.c) read them where the AGC did: at their addresses in
 * bank 42, through the machine's fixed memory (noun_table_word). */
#ifndef READABLE_PINBALL_NOUN_TABLES_H
#define READABLE_PINBALL_NOUN_TABLES_H

#include "../agc/agc.h"

/* The tables (tables.c), each as many words as the listing has. */
#if LUMINARY099
#define SFTAB_WORDS 58u /* SFINTAB and SFOUTAB: 29 double words each */
#else
#define SFTAB_WORDS 40u /* 20 double words each */
#endif
extern const word NNADTAB[100], NNTYPTAB[100], SFINTAB[SFTAB_WORDS], SFOUTAB[SFTAB_WORDS],
    IDADDTAB[180], RUTMXTAB[60];

/* The word at a physical address in the tables (bank * 02000 + offset), for
 * the machine's fixed memory. Any other address is not modelled. */
word noun_table_word(unsigned physical);

/* The readers, each entered by DXCH Z with the return 2CADR in A and L,
 * each leaving by DXCH Z back to it. */
void lodnntab(void);
void gtsfout(void);
void gtsfin(void);

#endif
