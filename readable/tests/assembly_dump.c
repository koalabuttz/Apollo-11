/* SPDX-License-Identifier: MIT */
/* Prints one of src/assembly's tables back as the listing file's text, for
 * tests/assembly_cases.py to compare with the listing (make
 * assembly-cases). Built once per mission; the argument is the listing
 * file's name without .agc. Line breaks and spacing are free: the
 * comparison is of the words in order. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../src/assembly/assembly.h"

/* MAIN: a file as "$NAME.agc first last directory", a note as "# note". */
static void main_agc(void)
{
    unsigned f;

    for (f = 0; f < MAIN_COUNT; ++f) {
        const struct listing_file *file = &MAIN[f];

        if (file->name)
            printf("$%s.agc %u %u %s\n", file->name, file->first_page, file->last_page,
                   file->readable);
        else
            printf("# %s\n", file->note);
    }
}

/* ASSEMBLY_AND_OPERATION_INFORMATION as the listing's text: each line's
 * cells in order. */
static void assembly_and_operation_information(void)
{
    unsigned t, line, cell;

    for (t = 0; t < INFORMATION_COUNT; ++t) {
        const struct information_table *table = &INFORMATION[t];

        for (line = 0; line < table->line_count; ++line) {
            for (cell = 0; cell < table->columns; ++cell) {
                const char *text = table->cells[line * table->columns + cell];

                if (text) printf("%s ", text);
            }
            printf("\n");
        }
    }
}

/* The same, to compare line by line and column by column: each line as its
 * list's name and then a tab before each cell of the list's columns (a
 * cell with nothing in it is empty). */
static void assembly_and_operation_information_lines(void)
{
    unsigned t, line, cell;

    for (t = 0; t < INFORMATION_COUNT; ++t) {
        const struct information_table *table = &INFORMATION[t];

        for (line = 0; line < table->line_count; ++line) {
            printf("%s", table->name);
            for (cell = 0; cell < table->columns; ++cell) {
                const char *text = table->cells[line * table->columns + cell];

                printf("\t%s", text ? text : "");
            }
            printf("\n");
        }
    }
}

/* TAGS_FOR_RELATIVE_SETLOC as the listing's text: the heading; each bank's
 * comment, BLOCK or BANK card, tags and BNKSUM card; each equate; and for
 * the CM the ECADRs, each under its EBANK= card, with a BANK card where
 * the bank changes. */
static void tags_for_relative_setloc(void)
{
    unsigned n, t;

    for (n = 0; n < TAGS_HEADING_COUNT; ++n) printf("%s\n", TAGS_HEADING[n]);
    for (n = 0; n < SETLOC_BANK_COUNT; ++n) {
        const struct setloc_bank *bank = &SETLOC_BANKS[n];

        if (bank->before) printf("%s\n", bank->before);
        printf("%s %02o\n", bank->fixed_fixed ? "BLOCK" : "BANK", bank->number);
        for (t = 0; t < bank->tag_count; ++t)
            printf("%s EQUALS %s\n", bank->tags[t].name,
                   bank->tags[t].remark ? bank->tags[t].remark : "");
        printf("BNKSUM %02o\n", bank->number);
    }
    for (n = 0; n < SETLOC_EQUATE_COUNT; ++n) {
        const struct setloc_equate *equate = &SETLOC_EQUATES[n];

        printf("%s %s %s %s\n", equate->name ? equate->name : "", equate->card ? equate->card : "",
               equate->operand ? equate->operand : "", equate->remark ? equate->remark : "");
    }
#if COMANCHE055
    printf("%s\n", EBANK_ECADRS_REMARK);
    for (n = 0; n < EBANK_ECADR_COUNT; ++n) {
        const struct ebank_ecadr *ecadr = &EBANK_ECADRS[n];

        if (n == 0 || ecadr->bank != EBANK_ECADRS[n - 1].bank) printf("BANK %o\n", ecadr->bank);
        printf("EBANK= %s\n%s ECADR %s\n", ecadr->erasable, ecadr->name, ecadr->erasable);
    }
    printf("%s\n", TAGS_END);
#endif
}

/* Where TAGS_FOR_RELATIVE_SETLOC puts things, for the comparison with the
 * assembled program: each tag as "TAG name bank" (the bank as the
 * assembler numbers it: fixed-fixed 02 and 03 are banks 2 and 3), and for
 * the CM each ECADR as "WORD name bank address value". */
static void tags_for_relative_setloc_where(void)
{
    unsigned n, t;

    for (n = 0; n < SETLOC_BANK_COUNT; ++n)
        for (t = 0; t < SETLOC_BANKS[n].tag_count; ++t)
            printf("TAG %s %o\n", SETLOC_BANKS[n].tags[t].name, SETLOC_BANKS[n].number);
#if COMANCHE055
    for (n = 0; n < EBANK_ECADR_COUNT; ++n)
        printf("WORD %s %o %o %o\n", EBANK_ECADRS[n].name, EBANK_ECADRS[n].bank,
               EBANK_ECADRS[n].address, EBANK_ECADRS[n].ecadr);
#endif
}

#if LUMINARY099

static void input_output_channel_bit_descriptions(void)
{
    unsigned c, b;

    printf("%s\n", CHANNEL_DESCRIPTIONS_HEADING);
    for (c = 0; c < CHANNEL_COUNT; ++c) {
        const struct channel *channel = &CHANNELS[c];

        if (channel->before) printf("%s\n", channel->before);
        printf("CHANNEL %o %s\n", channel->number, channel->text);
        for (b = 0; b < channel->bit_count; ++b) {
            const struct channel_bit *bit = &channel->bits[b];

            printf("BIT %u %s %s\n", bit->bit, bit->text, bit->note ? bit->note : "");
        }
    }
}

/* A text of several lines as words on one line ("" for NULL). */
static void print_words(const char *text)
{
    for (; text && *text; ++text) putchar(*text == '\n' ? ' ' : *text);
    putchar('\n');
}

/* CONTROLLED_CONSTANTS as the listing's text: a group as its comment and
 * its SETLOC, BANK and COUNT* cards; a constant as its comment, its line
 * and the rest of its remark. */
static void controlled_constants(void)
{
    unsigned g, c;

    for (g = 0; g < CONSTANT_GROUP_COUNT; ++g) {
        const struct constant_group *group = &CONTROLLED_CONSTANTS[g];

        print_words(group->before);
        printf("SETLOC %s\nBANK\nCOUNT* %s\n", group->tag, group->count);
        for (c = 0; c < group->constant_count; ++c) {
            const struct controlled_constant *constant = &group->constants[c];

            print_words(constant->before);
            printf("%s %s %s ", constant->name ? constant->name : "", constant->card,
                   constant->value);
            print_words(constant->remark);
        }
    }
}

/* The words a decimal card makes of its number: one word, or two for a
 * 2DEC. A number under 1 in size is a fraction, of 2(14) for one word and
 * 2(28) for two, rounded to the nearest; any other is a whole number and
 * is itself. A negative number's words are the one's complements. */
static void words_of(double number, unsigned count, unsigned words[2])
{
    double size = fabs(number);
    unsigned long whole;

    if (size < 1.0) size = round(size * (count == 2 ? 0x1p28 : 0x1p14));
    whole = (unsigned long)size;
    words[0] = count == 2 ? (unsigned)(whole >> 14) : (unsigned)whole;
    words[1] = (unsigned)(whole & 037777u);
    if (number < 0.0) {
        words[0] ^= 077777u;
        words[1] ^= 077777u;
    }
}

/* Where CONTROLLED_CONSTANTS puts its words, for the comparison with the
 * assembled program: each constant with words as "bank address words",
 * then, for a decimal card, "=" and the words worked out from its
 * number; each = card as "NAME name operand"; and each group as "TAG tag
 * bank". */
static void controlled_constants_where(void)
{
    unsigned g, c;

    for (g = 0; g < CONSTANT_GROUP_COUNT; ++g) {
        const struct constant_group *group = &CONTROLLED_CONSTANTS[g];

        printf("TAG %s %o\n", group->tag, group->bank);
        for (c = 0; c < group->constant_count; ++c) {
            const struct controlled_constant *constant = &group->constants[c];
            unsigned count = constant->card[0] == '2' ? 2 : 1, words[2];

            if (!strcmp(constant->card, "=")) {
                printf("NAME %s %s\n", constant->name, constant->value);
                continue;
            }
            printf("%o %o %05o", group->bank, constant->address, constant->words[0]);
            if (count == 2) printf(" %05o", constant->words[1]);
            if (strcmp(constant->card, "OCT")) {
                words_of(constant->number, count, words);
                printf(" = %05o", words[0]);
                if (count == 2) printf(" %05o", words[1]);
            }
            printf("\n");
        }
    }
}

/* One line of a flag's text: prints it, and returns the rest. */
static const char *print_line(const char *text)
{
    size_t length;

    if (!text || !*text) return text;
    length = strcspn(text, "\n");
    printf("%.*s ", (int)length, text);
    return text[length] ? text + length + 1 : text + length;
}

/* FLAGWORD_ASSIGNMENTS as the listing's text, with each block's code
 * before its comment (the listing has the two side by side). The list: a
 * line each. The assignments: a flagword as its other name's line, its
 * own line and the heads of the two columns; a flag as its "BIT n FLAG m"
 * comment, its two lines of code, and its text, a line of the listing at
 * a time (SET's column, then RESET's). */
static void flagword_assignments(void)
{
    unsigned w, f;

    printf("%s\n%s\n%s\n", FLAGWORDS_INTRODUCTION, FLAG_LIST_HEADING, FLAG_LIST_COLUMNS);
    for (f = 0; f < FLAG_LIST_COUNT; ++f) {
        const struct listed_flag *listed = &FLAG_LIST[f];

        printf("%s ", listed->name);
        if (listed->number != NOT_GIVEN) printf("%03d ", listed->number);
        if (listed->bit != NOT_GIVEN) printf("BIT %d FLAG ", listed->bit);
        if (listed->flagword != NOT_GIVEN) printf("%d ", listed->flagword);
        printf("%s %s\n", listed->bit_name ? listed->bit_name : "",
               listed->note ? listed->note : "");
    }
    printf("%s\n", FLAG_ASSIGNMENTS_HEADING);
    for (w = 0; w < FLAGWORD_COUNT; ++w) {
        const struct flagword *word = &FLAGWORDS[w];

        if (word->other_name)
            printf("%s EQUALS %s %s\n", word->other_name, word->name, word->other_remark);
        printf("%s = STATE +%u%s (%03u-%03u) %s\n(SET) (RESET)\n", word->name, word->number,
               word->number >= 8 ? "D" : "", 15 * word->number, 15 * word->number + 14,
               word->remark ? word->remark : "");
        for (f = 0; f < word->flag_count; ++f) {
            const struct flag *flag = &word->flags[f];
            const char *note = flag->note, *set = flag->set, *reset = flag->reset;

            printf("BIT %u FLAG %u %s\n", flag->bit, word->number, flag->marks ? flag->marks : "");
            printf("%s = ", flag->name ? flag->name : "");
            if (flag->same_as)
                printf("%s\n", flag->same_as);
            else
                printf("%03uD\n", flag->number);
            printf("%s = BIT%u\n", flag->bit_name ? flag->bit_name : "", flag->bit);
            while (note && *note) note = print_line(note);
            while ((set && *set) || (reset && *reset)) {
                set = print_line(set);
                reset = print_line(reset);
            }
            printf("\n");
        }
    }
}

/* One column of a flag's text, a line at a time as "flag column+line
 * text", without the lines that are empty in this column. */
static void print_column(unsigned flag, char column, const char *text)
{
    unsigned line;

    for (line = 0; text && *text; ++line) {
        size_t length = strcspn(text, "\n");

        if (length) printf("%u %c%u %.*s\n", flag, column, line, (int)length, text);
        text += text[length] ? length + 1 : length;
    }
}

/* FLAGWORD_ASSIGNMENTS's flags, counted from 0 in the listing's order,
 * each with its text by column: N the note, S the SET column, R the
 * RESET column. */
static void flagword_assignments_columns(void)
{
    unsigned w, f, flag = 0;

    for (w = 0; w < FLAGWORD_COUNT; ++w)
        for (f = 0; f < FLAGWORDS[w].flag_count; ++f, ++flag) {
            print_column(flag, 'N', FLAGWORDS[w].flags[f].note);
            print_column(flag, 'S', FLAGWORDS[w].flags[f].set);
            print_column(flag, 'R', FLAGWORDS[w].flags[f].reset);
        }
}

/* What FLAGWORD_ASSIGNMENTS assigns, for the comparison with the
 * assembled program and of the list with the assignments: a flagword as
 * "WORD name number other-name", an assigned flag as "FLAG name number
 * bit-name bit", a listed one as "LISTED name number bit flagword
 * bit-name" (-1 for a number not given). */
static void flagword_assignments_where(void)
{
    unsigned w, f;

    for (w = 0; w < FLAGWORD_COUNT; ++w) {
        const struct flagword *word = &FLAGWORDS[w];

        printf("WORD %s %u %s\n", word->name, word->number,
               word->other_name ? word->other_name : "");
        for (f = 0; f < word->flag_count; ++f)
            if (word->flags[f].name)
                printf("FLAG %s %u %s %u\n", word->flags[f].name, word->flags[f].number,
                       word->flags[f].bit_name, word->flags[f].bit);
    }
    for (f = 0; f < FLAG_LIST_COUNT; ++f)
        if (FLAG_LIST[f].bit_name)
            printf("LISTED %s %d %d %d %s\n", FLAG_LIST[f].name, FLAG_LIST[f].number,
                   FLAG_LIST[f].bit, FLAG_LIST[f].flagword, FLAG_LIST[f].bit_name);
}

#else

static void contract_and_approvals(void)
{
    unsigned a;

    printf("%s\n%s\n%s\n", CONTRACT.heading, CONTRACT.name, CONTRACT.statement);
    for (a = 0; a < APPROVAL_COUNT; ++a) {
        const struct approval *approval = &APPROVALS[a];

        printf("%s: %s DATE: %s\n%s\n%s\n", approval->action, approval->name, approval->date,
               approval->title, approval->office);
    }
}

#endif

static const struct {
    const char *file;
    void (*print)(void);
} files[] = {
    {"ASSEMBLY_AND_OPERATION_INFORMATION", assembly_and_operation_information},
    {"ASSEMBLY_AND_OPERATION_INFORMATION:lines", assembly_and_operation_information_lines},
    {"MAIN", main_agc},
    {"TAGS_FOR_RELATIVE_SETLOC", tags_for_relative_setloc},
    {"TAGS_FOR_RELATIVE_SETLOC:where", tags_for_relative_setloc_where},
#if LUMINARY099
    {"INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS", input_output_channel_bit_descriptions},
    {"CONTROLLED_CONSTANTS", controlled_constants},
    {"CONTROLLED_CONSTANTS:where", controlled_constants_where},
    {"FLAGWORD_ASSIGNMENTS", flagword_assignments},
    {"FLAGWORD_ASSIGNMENTS:where", flagword_assignments_where},
    {"FLAGWORD_ASSIGNMENTS:columns", flagword_assignments_columns},
#else
    {"CONTRACT_AND_APPROVALS", contract_and_approvals},
#endif
};

int main(int argc, char **argv)
{
    unsigned f;

    for (f = 0; argc == 2 && f < COUNT(files); ++f)
        if (!strcmp(argv[1], files[f].file)) {
            files[f].print();
            return 0;
        }
    fprintf(stderr, "usage: %s <listing file's name>\n", argv[0]);
    return 2;
}
