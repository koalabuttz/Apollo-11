/* SPDX-License-Identifier: MIT
 * ABI 2 module, fixed slot. The driver passes, with -Wl,--defsym:
 *   __ov2_slot      0 (job slot $8000..$BFFF) or 1 (task slot $E000..$FFF9)
 *   __ov2_mission   1 LM, 2 CM
 *   __ov2_id        lane<<8 | number
 *   __ov2_services  service-table version the module was built against
 *   __ov2_selectors number of entry selectors
 * and includes overlay_v2_slot_job.ld or overlay_v2_slot_task.ld first for
 * MEMORY. Resident services resolve through overlay_v2_services.ld. No CRT,
 * no private zero page, no AGC rope. */
__rc0 = 0x02;
INCLUDE imag-regs.ld
INCLUDE overlay_v2_services.ld
REGION_ALIAS("c_readonly", slot)
REGION_ALIAS("c_writeable", slot)
SECTIONS {
    .overlay_header : {
        BYTE(0x4e) BYTE(0x41) BYTE(0x54) BYTE(0x56)
        BYTE(2) BYTE(24) BYTE(__ov2_slot) BYTE(__ov2_mission)
        SHORT(__ov2_id) SHORT(ORIGIN(slot)+24)
        SHORT(__ov2_bss_start) SHORT(__ov2_bss_end-__ov2_bss_start)
        SHORT(__ov2_load_end-ORIGIN(slot)) SHORT(__ov2_services)
        SHORT(__ov2_selectors)
        /* TABLES: a flight module's pointer to its table list (.ov2_tables,
         * platform/c64/flight_table_module.c), else 0. */
        __ov2_tables = .; KEEP(*(.ov2_tables)) . = __ov2_tables + 2;
    } > slot
    .overlay_entry : { KEEP(*(.overlay_entry)) } > slot
    .text : { INCLUDE text-sections.ld } > slot
    INCLUDE rodata.ld
    INCLUDE data.ld
    __ov2_load_end = .;
    __ov2_bss_start = .;
    .bss (NOLOAD) : { *(.bss .bss.*) *(COMMON) } > slot
    __ov2_bss_end = .;
    .forbidden_zp (NOLOAD) : { *(.zp .zp.*) }
    .forbidden_noinit (NOLOAD) : { *(.noinit .noinit.*) }
}
ASSERT(ADDR(.overlay_entry)==ORIGIN(slot)+24,"ABI 2 entry must follow the 24-byte header")
ASSERT(SIZEOF(.forbidden_zp)==0,"ABI 2 module allocated private zero page")
ASSERT(SIZEOF(.forbidden_noinit)==0,"ABI 2 module no-init storage is unsupported")
ASSERT(__ov2_bss_end<=ORIGIN(slot)+LENGTH(slot),"ABI 2 module exceeds its slot")
ASSERT(__ov2_id>=0x100 && __ov2_id<0x1000,"ABI 2 module id outside lane ranges")
OUTPUT_FORMAT { SHORT(ORIGIN(slot)) TRIM(slot) }
