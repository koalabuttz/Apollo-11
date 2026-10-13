/* SPDX-License-Identifier: MIT
 * resident-kernel diagnostics: the kernel runs on the canonical erasable
 * image at $7000 (APOLLO_SHARED_ADDRESS), so the program leaves it free:
 * code in $0801..$6FFF; the case generators, read-only data, data and bss
 * in $8000..$BFFF; the $c000 mailbox; the soft stack below $D000. The PRG loads straight through
 * the image's 4 KB (zero filled; each case writes its own image). */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x67ff
    image (rw) : ORIGIN = 0x7000, LENGTH = 0x1000
    hi (rw) : ORIGIN = 0x8000, LENGTH = 0x4000
}
__rc0 = __basic_zp_start;
INCLUDE imag-regs.ld
__basic_zp_size = __basic_zp_end - __basic_zp_start;
MEMORY { zp : ORIGIN = __rc31 + 1, LENGTH = __basic_zp_end - (__rc31 + 1) }
INPUT(basic-header.o)
INPUT(unmap-basic.o)
REGION_ALIAS("c_readonly", hi)
REGION_ALIAS("c_writeable", hi)
SECTIONS {
    .basic_header : { *(.basic_header) } >ram
    INCLUDE zp.ld
    /* main and the case generators (test inputs, not the kernel) go high too. */
    .text.inputs : { *(.text.main .text.*_input .text.*_input.*) } >hi
    .text : { INCLUDE text-sections.ld } >ram
    .rodata : { INCLUDE rodata-sections.ld } >hi
    .data : { INCLUDE data-sections.ld } >hi
    INCLUDE data-symbols.ld
    .bss (NOLOAD) : { INCLUDE bss-sections.ld } >hi
    INCLUDE bss-symbols.ld
    .noinit (NOLOAD) : { INCLUDE noinit-sections.ld } >hi
}
__stack = 0xd000;
OUTPUT_FORMAT { SHORT(0x0801) FULL(ram) FULL(image) TRIM(hi) }
