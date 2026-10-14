/* SPDX-License-Identifier: MIT
 * Interpreter extension (INTERP_EXTENSION): the compact interpreter's cold
 * math bodies (section .interp_ext) in RAM under I/O, $D000-$DFFF, at most
 * 4 KB (steward ruling 2026-10-13: cold bodies only, at or above $D000;
 * the soft stack grows down from $D000). Linked after
 * overlay_v2_payload.ld; the module's PRG carries only its slot, so the
 * extension's bytes are output separately (tools/interp_runtime_size.py). */
MEMORY { interp_ext (rwx) : ORIGIN = 0xD000, LENGTH = 0x1000 }
SECTIONS { .interp_ext : { __interp_ext_start = .; KEEP(*(.interp_ext)) __interp_ext_end = .; } > interp_ext }
