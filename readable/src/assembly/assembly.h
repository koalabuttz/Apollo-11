/* SPDX-License-Identifier: MIT */
/* The listing's files that hold no routines: what the program is made of
 * and where its words live. Each is one C file here, named for it, that
 * carries the listing file's text as tables, word for word (make
 * assembly-cases prints the tables back and compares them with the
 * listing). The words are the listing's own: the transcriber's heading of
 * each file, down to its first page mark, is not carried. No routine is
 * in these files and no flight code uses these tables; they are the
 * listing's data and comment in a form a reader and the gate can use.
 * The C comments that name a page are there to find the place in the
 * listing; the gate does not compare them.
 *
 *   INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS.agc (LM pp. 54-60): each
 *     channel and its bits (input_output_channel_bit_descriptions.c);
 *   CONTRACT_AND_APPROVALS.agc (CM p. 1): the program's other name, its
 *     contract and the people who signed it off
 *     (contract_and_approvals.c);
 *   ENTRY_LEXICON.agc (CM pp. 837-843): the entry guidance's glossary,
 *     list by list (entry_lexicon.c);
 *   LUNAR_LANDMARK_SELECTION_FOR_CM.agc (CM p. 936): a page with one line
 *     (lunar_landmark_selection_for_cm.c);
 *   MAIN.agc (both missions): the listing's files in the order of their
 *     pages, each with the directory of its readable C (main.c);
 *   ASSEMBLY_AND_OPERATION_INFORMATION.agc (LM pp. 1-27, CM pp. 2-26):
 *     the log sections, verbs, nouns, alarm codes, checklist codes and
 *     option codes, list by list
 *     (assembly_and_operation_information.c);
 *   TAGS_FOR_RELATIVE_SETLOC.agc (LM pp. 28-37, CM pp. 27-35): the tags
 *     that say which bank each part of the program goes in
 *     (tags_for_relative_setloc.c);
 *   ERASABLE_ASSIGNMENTS.agc (LM pp. 90-152, CM pp. 37-130): the name
 *     and the place of every word of erasable memory, line by line, and
 *     the CM's flags (erasable_assignments.c);
 *   FLAGWORD_ASSIGNMENTS.agc (LM pp. 61-88): the flags, bit by bit, and
 *     what each means set and reset (flagword_assignments.c);
 *   CONTROLLED_CONSTANTS.agc (LM pp. 38-53): the constants of the
 *     spacecraft and of nature, each with its number and its words
 *     (controlled_constants.c). */
#ifndef READABLE_ASSEMBLY_H
#define READABLE_ASSEMBLY_H

#include <stdbool.h>

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

/* ---- ASSEMBLY_AND_OPERATION_INFORMATION ---- */

/* One list of a file of lists (this file's, and the CM's ENTRY_LEXICON's
 * below): its name here, the number of columns the
 * listing sets it in, the number of its lines, and the lines' cells, line
 * after line (columns cells to a line; a cell is NULL where the line has
 * nothing in that column). */
struct information_table {
    const char *name;
    unsigned columns;
    unsigned line_count;
    const char *const *cells;
};

extern const struct information_table INFORMATION[];
extern const unsigned INFORMATION_COUNT;

/* ---- TAGS_FOR_RELATIVE_SETLOC ---- */

/* One tag: its name, and the comment beside it (NULL for none). */
struct setloc_tag {
    const char *name;
    const char *remark;
};

/* One bank's cards: the comment that stands before them in the listing
 * (NULL for none), whether the bank is fixed-fixed (a BLOCK card) or
 * switched (a BANK card), its number, and its tags. The bank's BNKSUM
 * card, which closes them, has the same number. */
struct setloc_bank {
    const char *before;
    bool fixed_fixed;
    unsigned number;
    const struct setloc_tag *tags;
    unsigned tag_count;
};

#define FIXED_FIXED true
#define SWITCHED    false

/* One line after the banks: a name that stands for another (card EQUALS
 * or =, the other name the operand), with the comment beside it (NULL for
 * none); or a card with no name (name NULL); or a line of comment alone
 * (only the remark). */
struct setloc_equate {
    const char *name;
    const char *card;
    const char *operand;
    const char *remark;
};

extern const char *const TAGS_HEADING[];
extern const unsigned TAGS_HEADING_COUNT;
extern const struct setloc_bank SETLOC_BANKS[];
extern const unsigned SETLOC_BANK_COUNT;
extern const struct setloc_equate SETLOC_EQUATES[];
extern const unsigned SETLOC_EQUATE_COUNT;

/* ---- ERASABLE_ASSIGNMENTS ---- */

/* What a name is to the assembler, and its value:
 *   IN_ERASABLE  a word of erasable memory, by its address as an ECADR, 0
 *                to 03777. The erasable bank is the address / 0400; the
 *                words of banks 0 to 2 are at their own addresses, and a
 *                word of banks 3 to 7 is reached at 01400 + address % 0400
 *                with its bank switched in;
 *   NUMBER       a number. A name set equal to a bare number is a number,
 *                whatever the program uses it for: the special registers
 *                and counters at addresses 0 to 060, the channels and the
 *                flags are all numbers;
 *   IN_FIXED     a word of fixed memory, by its address. */
enum name_kind { NO_NAME, ERASABLE_ADDRESS, PLAIN_NUMBER, FIXED_ADDRESS };

struct name_value {
    enum name_kind kind;
    unsigned number;
};

/* clang-format off */
#define IN_ERASABLE(address) {ERASABLE_ADDRESS, address}
#define NUMBER(number)       {PLAIN_NUMBER, number}
#define IN_FIXED(address)    {FIXED_ADDRESS, address}
#define NO_VALUE             {NO_NAME, 0}
/* clang-format on */

/* One line of ERASABLE_ASSIGNMENTS.agc:
 *   name     the card's name (NULL for a card with none, and for a line of
 *            comment);
 *   card     ERASE, EQUALS, = or SETLOC (NULL for a line of comment);
 *   operand  the card's operand as the listing writes it, or NULL;
 *   value    the name's value in the assembled program;
 *   remark   the comment beside the card, or the line of comment;
 *   set, reset  in the lines that assign the CM's flags, the comment where
 *            it stands in two columns: what the flag means set, and what
 *            it means reset. */
struct erasable_line {
    const char *name;
    const char *card;
    const char *operand;
    struct name_value value;
    const char *remark;
    const char *set;
    const char *reset;
};

extern const struct erasable_line ERASABLE_ASSIGNMENTS[];
extern const unsigned ERASABLE_LINE_COUNT;

#if COMANCHE055

/* One of the CM's ECADRs for switching erasable banks: the bank and
 * address of its word in the rope, its name, the erasable it names (also
 * the operand of the EBANK= card above it), and the word. */
struct ebank_ecadr {
    unsigned bank;
    const char *name;
    unsigned address;
    const char *erasable;
    unsigned ecadr;
};

/* ---- ENTRY_LEXICON and LUNAR_LANDMARK_SELECTION_FOR_CM ---- */

extern const struct information_table ENTRY_LEXICON[];
extern const unsigned ENTRY_LEXICON_COUNT;

/* The page's lines. */
extern const char *const LUNAR_LANDMARK_SELECTION_FOR_CM[];
extern const unsigned LUNAR_LANDMARK_LINE_COUNT;

extern const char EBANK_ECADRS_REMARK[];
extern const struct ebank_ecadr EBANK_ECADRS[];
extern const unsigned EBANK_ECADR_COUNT;
extern const char TAGS_END[];

#endif

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

/* ---- FLAGWORD_ASSIGNMENTS ---- */

/* A number the listing leaves out. */
#define NOT_GIVEN (-1)

/* One line of the alphabetical list: a flag (its name, its number among
 * the flags, its bit, its flagword and the bit's name), with the note
 * after it (NULL for none); or a name that is no flag, and what the list
 * says of it (the note, with the numbers not given). */
struct listed_flag {
    const char *name;
    int number;
    int bit, flagword;
    const char *bit_name;
    const char *note;
};

/* One bit of a flagword, as the assignments give it:
 *   name      the flag (NULL for a bit with no flag);
 *   same_as   for a second name of a flag, the flag it is set equal to
 *             (NAME = OTHER in the listing, where the rest have NAME =
 *             nnnD), else NULL;
 *   number    the flag's number, decimal;
 *   bit_name  the name of the bit (NULL with no flag);
 *   bit       the bit, 15 to 1;
 *   marks     what the listing puts after "BIT n FLAG m" in the comment
 *             above the flag ((S), (L), (L)(R12) and the like, which the
 *             listing does not explain), or NULL;
 *   note      the flag's comment where it is not in two columns, or NULL;
 *   set, reset  what the flag means set and reset, or NULL.
 * note, set and reset keep the listing's lines, parted by a new line. */
struct flag {
    const char *name;
    const char *same_as;
    unsigned number;
    const char *bit_name;
    unsigned bit;
    const char *marks;
    const char *note;
    const char *set;
    const char *reset;
};

/* One flagword: its other name where it has one (X EQUALS name, with that
 * line's comment), its name, its number (the word is STATE + number),
 * what the listing says after its range of flags (NULL for nothing), and
 * its bits from 15 down. */
struct flagword {
    const char *other_name;
    const char *other_remark;
    const char *name;
    unsigned number;
    const char *remark;
    const struct flag *flags;
    unsigned flag_count;
};

/* ---- CONTROLLED_CONSTANTS ---- */

/* One constant:
 *   before   the comment that stands before it in the listing, or NULL;
 *   name     its label (NULL for a word with none, such as the second and
 *            third of a vector);
 *   card     DEC or 2DEC (one word or two, decimal), DEC* or 2DEC* (the
 *            same, with the operand closed by an asterisk), OCT (one
 *            word, octal), or = (another name for a word elsewhere);
 *   value    the operand, as the listing writes it;
 *   number   for a decimal card, the operand as a number;
 *   address  the address of its first word in the group's bank;
 *   words    its word or words;
 *   remark   the comment beside and under it, or NULL.
 * before and remark keep the listing's lines, parted by a new line. */
struct controlled_constant {
    const char *before;
    const char *name;
    const char *card;
    const char *value;
    double number;
    unsigned address;
    unsigned words[2];
    const char *remark;
};

/* One group of constants: the comment before it (NULL for none), the tag
 * its SETLOC names, the tag's bank (fixed-fixed memory is banks 02 and
 * 03), the operand of its COUNT* card, and the constants. */
struct constant_group {
    const char *before;
    const char *tag;
    unsigned bank;
    const char *count;
    const struct controlled_constant *constants;
    unsigned constant_count;
};

extern const struct constant_group CONTROLLED_CONSTANTS[];
extern const unsigned CONSTANT_GROUP_COUNT;

extern const char FLAGWORDS_INTRODUCTION[];
extern const char FLAG_LIST_HEADING[];
extern const char FLAG_LIST_COLUMNS[];
extern const struct listed_flag FLAG_LIST[];
extern const unsigned FLAG_LIST_COUNT;
extern const char FLAG_ASSIGNMENTS_HEADING[];
extern const struct flagword FLAGWORDS[];
extern const unsigned FLAGWORD_COUNT;

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
