/* mos-sim memory map for tools/interp_core_cycles.c: the program above
 * $8000, so the image can sit at $7000 as on the C64 (llvm-mos sim link.ld
 * otherwise). */
__rc0 = 0x00;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x001f, "Inconsistent zero page map.")
MEMORY {
    zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)
    ram (rw) : ORIGIN = 0x8000, LENGTH = 0x7ff0
}
REGION_ALIAS("c_readonly", ram)
REGION_ALIAS("c_writeable", ram)
SECTIONS { INCLUDE c.ld }
__stack = 0xFFF0;
OUTPUT_FORMAT {
    SHORT(0x8000)
    SHORT(__data_end - 0x8000)
    TRIM(ram)
    SHORT(0xFFFA)
    SHORT(6)
    SHORT(0)
    SHORT(_start)
    SHORT(0)
}
