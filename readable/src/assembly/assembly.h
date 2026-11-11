/* SPDX-License-Identifier: MIT */
/* The listing's files that hold no routines: what the program is made of
 * and where its words live. Each is one C file here, named for it, that
 * carries the listing file's text as tables, word for word (make
 * assembly-cases prints the tables back and compares them with the
 * listing).
 *
 *   INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS.agc (LM pp. 54-60): each
 *     channel and its bits (input_output_channel_bit_descriptions.c);
 *   CONTRACT_AND_APPROVALS.agc (CM p. 1): the program's other name, its
 *     contract and the people who signed it off
 *     (contract_and_approvals.c);
 *   MAIN.agc (both missions): the listing's files in the order of their
 *     pages, each with the directory of its readable C (main.c). */
#ifndef READABLE_ASSEMBLY_H
#define READABLE_ASSEMBLY_H

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- MAIN ---- */

/* One line of MAIN.agc: a file of the listing (its name without .agc, its
 * first and last pages, and the directory of readable/ that holds its
 * readable C), or else a note, the line's comment where it names no file. */
struct listing_file {
    const char *name;
    unsigned first_page, last_page;
    const char *readable;
    const char *note;
};

extern const struct listing_file MAIN[];
extern const unsigned MAIN_COUNT;

#if LUMINARY099

/* ---- INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS ---- */

/* One bit of a channel: its number (1 the low bit), what the listing says
 * it is, and the note in the listing's second column, NULL for none. */
struct channel_bit {
    unsigned bit;
    const char *text;
    const char *note;
};

/* clang-format off */
#define BIT(bit, text)                {bit, text, NULL}
#define BIT_AND_NOTE(bit, text, note) {bit, text, note}
/* clang-format on */

/* One channel: the note that stands before it in the listing (NULL for
 * none), its number, its description, and its bits where the listing
 * tells them apart. */
struct channel {
    const char *before;
    unsigned number;
    const char *text;
    const struct channel_bit *bits;
    unsigned bit_count;
};

extern const char CHANNEL_DESCRIPTIONS_HEADING[];
extern const struct channel CHANNELS[];
extern const unsigned CHANNEL_COUNT;

#else

/* ---- CONTRACT_AND_APPROVALS ---- */

/* The page's box: its heading, the program's other name, and the
 * contract statement. */
struct contract {
    const char *heading;
    const char *name;
    const char *statement;
};

/* One signature: SUBMITTED or APPROVED, by whom and when, and the two
 * lines under the name. */
struct approval {
    const char *action;
    const char *name;
    const char *date;
    const char *title;
    const char *office;
};

extern const struct contract CONTRACT;
extern const struct approval APPROVALS[];
extern const unsigned APPROVAL_COUNT;

#endif

#endif
