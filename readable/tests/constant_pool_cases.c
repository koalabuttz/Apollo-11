/* SPDX-License-Identifier: MIT */
/* The readable constant pools against the assembled rope and the listings
 * (make constant-pool-cases).
 *
 * Built once per mission. For FIXED_FIXED_CONSTANT_POOL (laid out from
 * DPOSMAX and from PRIO22, src/constant_pool/fixed_fixed_constant_pool.c)
 * and the interpretive constants (from DP1/4TH and from ZUNIT,
 * src/constant_pool/interpretive_constants.c), every word laid out must be
 * the rope's word at that address, and the labels, word by word, must be
 * the listing's: each OCT, DEC or VN line one word, each 2DEC or 2OCT two
 * (the second with no label), with none left out. The listing's
 * commented-out lines and its equates take no word. */
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

/* A listing, read word by word. */
struct listing {
    FILE *file;
    char label[64];
    unsigned words_left; /* of the line last read */
};

/* The listing's next word's label ("" for none), or false at the end. */
static bool next_listing_word(struct listing *listing, char *label, size_t size)
{
    char line[256];
    char first[64];
    char second[16];

    if (listing->words_left > 0u) {
        listing->words_left--;
        snprintf(label, size, "%s", "");
        return true;
    }
    while (fgets(line, sizeof line, listing->file)) {
        bool labeled = line[0] != ' ' && line[0] != '\t';
        const char *op = second;

        if (line[0] == '#' || line[0] == '\n') continue;
        if (sscanf(line, "%63s %15s", first, second) < 1) continue;
        if (!labeled) op = first;
        if (strcmp(op, "OCT") != 0 && strcmp(op, "DEC") != 0 && strcmp(op, "VN") != 0 &&
            strcmp(op, "2DEC") != 0 && strcmp(op, "2OCT") != 0)
            continue;
        listing->words_left = op[0] == '2' ? 1u : 0u;
        snprintf(label, size, "%s", labeled ? first : "");
        return true;
    }
    return false;
}

/* One block's words, from first: each the rope's, each label the
 * listing's next. */
static unsigned check_block(struct listing *listing, word first, bool (*value_at)(word, word *),
                            const char *(*label_at)(word), unsigned *words)
{
    unsigned failures = 0;
    char expected[64];

    for (word address = first; label_at(address); address++) {
        const char *label = label_at(address);
        agc_word rope = 0;
        word value = 0;

        if (!value_at(address, &value) || !apollo_rope_read(MISSION, address, &rope) ||
            value != (word)rope) {
            printf("%s at %05o: %05o, rope %05o\n", label, (unsigned)address, (unsigned)value,
                   (unsigned)rope);
            failures++;
        }
        if (!next_listing_word(listing, expected, sizeof expected) ||
            strcmp(expected, label) != 0) {
            printf("%05o: \"%s\", listing \"%s\"\n", (unsigned)address, label, expected);
            failures++;
        }
        (*words)++;
    }
    return failures;
}

/* A pool of two blocks against its listing. */
static unsigned check_pool(const char *name, const char *path, word first, word second,
                           bool (*value_at)(word, word *), const char *(*label_at)(word))
{
    struct listing listing = {0};
    unsigned failures = 0;
    unsigned words = 0;
    char extra[64];
    word value = 0;

    if (!(listing.file = fopen(path, "r"))) {
        printf("no %s\n", path);
        return 1;
    }
    failures += check_block(&listing, first, value_at, label_at, &words);
    failures += check_block(&listing, second, value_at, label_at, &words);
    if (next_listing_word(&listing, extra, sizeof extra)) {
        printf("%s: listing word \"%s\" not laid out\n", name, extra);
        failures++;
    }
    fclose(listing.file);
    if (value_at((word)(first - 1u), &value) || value_at((word)(second - 1u), &value)) {
        printf("%s: a word before a block\n", name);
        failures++;
    }
    printf("%s %s: %u words, %u failures\n", MISSION ? "CM" : "LM", name, words, failures);
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0;

    if (argc < 4 || !apollo_rope_load(MISSION, argv[1])) {
        printf("usage: %s ROPE FIXED_FIXED_CONSTANT_POOL.agc INTERPRETIVE_CONSTANT(S).agc\n",
               argv[0]);
        return 2;
    }
    failures += check_pool("fixed-fixed constant pool", argv[2], ADDRESS_OF(DPOSMAX),
                           ADDRESS_OF(PRIO22), fixed_fixed_constant, fixed_fixed_constant_label);
    failures += check_pool("interpretive constants", argv[3], I_DP1_4TH, I_ZUNIT,
                           interpretive_constant, interpretive_constant_label);
    return failures ? 1 : 0;
}
