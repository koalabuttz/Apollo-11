/* SPDX-License-Identifier: MIT */
/* The readable star catalog against the assembled rope and the listing
 * (make star-tables-cases).
 *
 * Built for the CM. Each of the catalog's words, from star 37's X to
 * CATLOG, made from the decimal stars of src/star_tables/star_tables.c as
 * the assembler would, must be the rope's word at that address, with no
 * word before or after; and the stars must be the listing's: each 2DEC
 * line's number exactly the table's, for the star and axis its comment
 * names, in order, with CATLOG's DEC 6970 after them. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rope_image.h"
#include "../src/star_tables/star_tables.h"
#include "../src/agc/names.h"

#define MISSION 1u /* Comanche055 */

static unsigned check_rope(void)
{
    unsigned failures = 0, words = 0;
    word first = (word)(I_CATLOG - 6u * STAR_COUNT);
    word value = 0;

    for (word address = first; address <= I_CATLOG; address++) {
        agc_word rope = 0;

        if (!star_table_word(address, &value) || !apollo_rope_read(MISSION, address, &rope) ||
            value != (word)rope) {
            printf("%05o: %05o, rope %05o\n", (unsigned)address, (unsigned)value, (unsigned)rope);
            failures++;
        }
        words++;
    }
    if (star_table_word((word)(first - 1u), &value) ||
        star_table_word((word)(I_CATLOG + 1u), &value)) {
        printf("a word outside the catalog\n");
        failures++;
    }
    printf("CM star catalog: %u words against the rope, %u failures\n", words, failures);
    return failures;
}

static unsigned check_listing(const char *path)
{
    unsigned failures = 0, components = 0;
    bool catlog = false;
    char line[256];
    FILE *file = fopen(path, "r");

    if (!file) {
        printf("no %s\n", path);
        return 1;
    }
    while (fgets(line, sizeof line, file)) {
        char number[32], axis = 0;
        unsigned star = 0, value = 0;

        if (sscanf(line, " 2DEC %31s B-1 # STAR %u %c", number, &star, &axis) == 3) {
            const struct star *s = &STARS[components / 3u];
            double x = components % 3u == 0u ? s->x : components % 3u == 1u ? s->y : s->z;

            if (components >= 3u * STAR_COUNT || star != s->number ||
                axis != "XYZ"[components % 3u] || strtod(number, NULL) != x) {
                printf("listing star %u %c %s, table's %u\n", star, axis, number, s->number);
                failures++;
            }
            components++;
        } else if (sscanf(line, "CATLOG DEC %u", &value) == 1) {
            catlog = true;
            if (value != CATLOG_WORD || components != 3u * STAR_COUNT) {
                printf("CATLOG DEC %u after %u components\n", value, components);
                failures++;
            }
        }
    }
    fclose(file);
    if (!catlog || components != 3u * STAR_COUNT) {
        printf("listing: %u components, CATLOG %s\n", components, catlog ? "read" : "missing");
        failures++;
    }
    printf("CM star catalog: %u stars against the listing, %u failures\n", components / 3u,
           failures);
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0;

    if (argc < 3 || !apollo_rope_load(MISSION, argv[1])) {
        printf("usage: %s ROPE STAR_TABLES.agc\n", argv[0]);
        return 2;
    }
    failures += check_rope();
    failures += check_listing(argv[2]);
    return failures ? 1 : 0;
}
