/* SPDX-License-Identifier: MIT
 * tools/interp_core_vice.c: llvm-mos's C64 PRG layout (link.ld) with the
 * program below the image at $7000, and the interpreter extension
 * (.interp_ext with its ABI 2 header, as platform/c64/interp_ext.ld) at
 * $D000, outside the PRG: the test puts the module in the REU and main
 * loads it as the resident's boot does. The driver passes __ov2_mission
 * and __ov2_services. */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY { ram (rw) : ORIGIN = 0x0801, LENGTH = 0x7000 - 0x0801 }
MEMORY { interp_ext (rwx) : ORIGIN = 0xD000, LENGTH = 0x1000 }
INCLUDE commodore.ld
INPUT(unmap-basic.o)
SECTIONS {
    .interp_ext : {
        __interp_ext_start = .;
        /* ABI 2 header, slot class 2 (tools/overlay_v2_pack.py): module
         * $0A02, no BSS, one selector whose entry (an RTS) nothing calls:
         * boot only loads the module (resident_flight.c rk_load_fixed). */
        BYTE(0x4e) BYTE(0x41) BYTE(0x54) BYTE(0x56) BYTE(2) BYTE(24) BYTE(2) BYTE(__ov2_mission)
        SHORT(0x0A02) SHORT(0xD018) SHORT(__interp_ext_end) SHORT(0)
        SHORT(__interp_ext_end - 0xD000) SHORT(__ov2_services) SHORT(1) SHORT(0)
        BYTE(0x60)
        KEEP(*(.interp_ext))
        __interp_ext_end = .;
    } > interp_ext
}
__stack = 0xC000;

OUTPUT_FORMAT { SHORT(ORIGIN(ram)) TRIM(ram) }
