/* SPDX-License-Identifier: MIT
 * Interpreter extension (INTERP_EXTENSION): the compact interpreter's cold
 * math bodies (section .interp_ext) in RAM under I/O, $D000-$DFFF, as ABI 2
 * module $0A02 of slot class 2 (APOLLO_OV2_SLOT_FIXED), which the resident
 * loads once at boot (steward ruling 2026-10-13: cold bodies only, no BSS,
 * no REU access). Linked with the job-slot module $0A01 after
 * overlay_v2_payload.ld, whose PRG carries only its slot: the extension's
 * PRG is cut from the same ELF (tools/interp_runtime_size.py), so the two
 * call each other at linked addresses. */
MEMORY { interp_ext (rwx) : ORIGIN = 0xD000, LENGTH = 0x1000 }
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
        *(.interp_ext_ro)
        __interp_ext_end = .;
    } > interp_ext
}
ASSERT(__interp_ext_end <= 0xE000, "interpreter extension exceeds $D000-$DFFF")
