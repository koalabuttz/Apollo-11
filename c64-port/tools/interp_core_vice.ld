/* SPDX-License-Identifier: MIT
 * tools/interp_core_vice.c: llvm-mos's C64 PRG layout (link.ld) with the
 * program below the image at $7000, and the interpreter extension
 * (.interp_ext, platform/c64/interp_ext.ld) run at $D000 and loaded after
 * the program, from where main copies it with I/O banked out. */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY { ram (rw) : ORIGIN = 0x0801, LENGTH = 0x7000 - 0x0801 }
MEMORY { interp_ext (rwx) : ORIGIN = 0xD000, LENGTH = 0x1000 }
INCLUDE commodore.ld
INPUT(unmap-basic.o)
SECTIONS {
    .interp_ext : { __interp_ext_start = .; KEEP(*(.interp_ext)) __interp_ext_end = .; } > interp_ext AT> ram
}
__interp_ext_load = LOADADDR(.interp_ext);
__stack = 0xC000;
OUTPUT_FORMAT { SHORT(ORIGIN(ram)) TRIM(ram) }
