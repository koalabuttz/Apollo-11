/* SPDX-License-Identifier: MIT */
/* Prints one of src/assembly's tables back as the listing file's text, for
 * tests/assembly_cases.py to compare with the listing (make
 * assembly-cases). Built once per mission; the argument is the listing
 * file's name without .agc. Line breaks and spacing are free: the
 * comparison is of the words in order. */
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
    {"MAIN", main_agc},
#if LUMINARY099
    {"INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS", input_output_channel_bit_descriptions},
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
