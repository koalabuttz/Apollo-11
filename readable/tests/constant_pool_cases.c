/* SPDX-License-Identifier: MIT */
/* The readable FIXED_FIXED_CONSTANT_POOL against the assembled rope and the
 * listing (make constant-pool-cases).
 *
 * Built once per mission. Every word laid out from DPOSMAX and from PRIO22
 * (src/constant_pool/fixed_fixed_constant_pool.c) must be the rope's word
 * at that address (fixed-fixed: the address is the rope's physical
 * address), and the labels, in order, must be the listing's labeled words
 * (OCT, DEC, VN), with none left out. The listing's commented-out lines and
 * its equates take no word, so they are not counted. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "../src/constant_pool/constant_pool.h"
#include "../src/agc/names.h"

#if LUMINARY099
#define MISSION 0u
#else
#define MISSION 1u
#endif

/* The next labeled word of the listing: its label, or false at the end. */
static bool next_listing_label(FILE *listing, char *label, size_t size)
{
    char line[256];
    char name[64];
    char op[16];

    while (fgets(line, sizeof line, listing)) {
        if (line[0] == '#' || line[0] == ' ' || line[0] == '\t' || line[0] == '\n') continue;
        if (sscanf(line, "%63s %15s", name, op) != 2) continue;
        if (strcmp(op, "OCT") != 0 && strcmp(op, "DEC") != 0 && strcmp(op, "VN") != 0) continue;
        snprintf(label, size, "%s", name);
        return true;
    }
    return false;
}

/* One block's words, from first: each the rope's, each label the
 * listing's next. */
static unsigned check_block(FILE *listing, word first, unsigned *words)
{
    unsigned failures = 0;
    char expected[64];

    for (word address = first; fixed_fixed_constant_label(address); address++) {
        const char *label = fixed_fixed_constant_label(address);
        agc_word rope = 0;
        word value = 0;

        if (!fixed_fixed_constant(address, &value) || !apollo_rope_read(MISSION, address, &rope) ||
            value != (word)rope) {
            printf("%s at %05o: %05o, rope %05o\n", label, (unsigned)address, (unsigned)value,
                   (unsigned)rope);
            failures++;
        }
        if (!next_listing_label(listing, expected, sizeof expected) ||
            strcmp(expected, label) != 0) {
            printf("%05o: %s, listing %s\n", (unsigned)address, label, expected);
            failures++;
        }
        (*words)++;
    }
    return failures;
}

int main(int argc, char **argv)
{
    FILE *listing;
    unsigned failures = 0;
    unsigned words = 0;
    char extra[64];
    word value = 0;

    if (argc < 3 || !apollo_rope_load(MISSION, argv[1]) || !(listing = fopen(argv[2], "r"))) {
        printf("usage: %s ROPE LISTING\n", argv[0]);
        return 2;
    }
    failures += check_block(listing, ADDRESS_OF(DPOSMAX), &words);
    failures += check_block(listing, ADDRESS_OF(PRIO22), &words);
    if (next_listing_label(listing, extra, sizeof extra)) {
        printf("listing word %s not laid out\n", extra);
        failures++;
    }
    fclose(listing);
    if (fixed_fixed_constant(ADDRESS_OF(DPOSMAX) - 1u, &value) ||
        fixed_fixed_constant(ADDRESS_OF(PRIO22) - 1u, &value)) {
        printf("a word before a block\n");
        failures++;
    }
    printf("%s constant pool: %u words, %u failures\n", MISSION ? "CM" : "LM", words, failures);
    return failures ? 1 : 0;
}
