/* SPDX-License-Identifier: MIT */
/* STAR_TABLES.agc of the CM (Comanche055 pp. 1389-1393): the star
 * catalog, CATLOG (star_tables.c). The LM's catalog is in its own files
 * and is not written here. */
#ifndef READABLE_STAR_TABLES_H
#define READABLE_STAR_TABLES_H

#include <stdbool.h>
#include "../agc/agc.h"

/* One star of the catalog: its number and its unit vector, X, Y and Z, as
 * the listing writes them (each a 2DEC, scaled B-1). */
struct star {
    unsigned number;
    double x, y, z;
};

/* The catalog's stars in the listing's order, from star 37 down to star
 * 1, and the word after them, at CATLOG. */
#define STAR_COUNT 37u
extern const struct star STARS[STAR_COUNT];
#define CATLOG_WORD 6970u /* CATLOG DEC 6970 */

/* The catalog's word at a rope address (the bank * 02000 plus the word's
 * offset in the bank, as I_ names give it), as the assembler lays it out,
 * false off the catalog. */
bool star_table_word(word address, word *value);

#endif
