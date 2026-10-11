/* SPDX-License-Identifier: MIT
 * Independent native module, fixed overlay slot. No CRT startup or AGC rope.
 * Driver supplies __overlay_module (1..254); imaginary-register ABI is shared
 * with the resident. Static/automatic ZP allocations are forbidden. */
__rc0 = 0x02;
INCLUDE imag-regs.ld
MEMORY { overlay (rw) : ORIGIN = 0x8000, LENGTH = 0x4000 }
REGION_ALIAS("c_readonly", overlay)
REGION_ALIAS("c_writeable", overlay)
PROVIDE(__overlay_code_origin=0x8013);
SECTIONS {
    .overlay_header : {
        BYTE(0x4e) BYTE(0x41) BYTE(0x54) BYTE(0x56)
        BYTE(1) BYTE(__overlay_module) BYTE(16) BYTE(0)
        SHORT(0x8010) SHORT(__overlay_bss_start)
        SHORT(__overlay_bss_end-__overlay_bss_start)
        SHORT(__overlay_load_end-0x8000)
    } > overlay
    .overlay_entry : { KEEP(*(.overlay_entry)) } > overlay
    .text __overlay_code_origin : { INCLUDE text-sections.ld } > overlay
    INCLUDE rodata.ld
    INCLUDE data.ld
    __overlay_load_end = .;
    __overlay_bss_start = .;
    .bss (NOLOAD) : { *(.bss .bss.*) *(COMMON) } > overlay
    __overlay_bss_end = .;
    .forbidden_zp (NOLOAD) : { *(.zp .zp.*) }
    .forbidden_noinit (NOLOAD) : { *(.noinit .noinit.*) }
}
ASSERT(ADDR(.overlay_entry)==0x8010,"native overlay entry moved")
ASSERT(SIZEOF(.forbidden_zp)==0,"native overlay allocated private zero page")
ASSERT(SIZEOF(.forbidden_noinit)==0,"native overlay no-init storage is unsupported")
ASSERT(__overlay_bss_end<=0xc000,"native overlay exceeds 16KB slot")
OUTPUT_FORMAT { SHORT(0x8000) TRIM(overlay) }
