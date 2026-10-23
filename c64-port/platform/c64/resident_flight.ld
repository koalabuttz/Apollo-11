/* SPDX-License-Identifier: MIT
 * The ABI 2 flight resident (docs/RESIDENT-FLIGHT.md), on the
 * overlay_v2_resident.ld map: resident code and data in $0801..$6EFF, the
 * service jump table at $6F00, the canonical image at $7000, the job slot at
 * $8000. Boot-only code (.boot.text, and the boot-only runtime functions
 * when LTO keeps them out of line) links into the job slot: the PRG loads it
 * there, it runs once from main, and the first module load overwrites it.
 * The PRG loads through the image's 4 KB (zero filled). The flight
 * program's DSKY (platform/c64/resident_dsky.c) runs from $0200-$03FF,
 * with its tables and state in the screen's last rows ($06A8-$07F7, not
 * shown), all free in flight: it loads after the boot-only code, and boot
 * copies it. */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x66ff
    services (rx) : ORIGIN = 0x6f00, LENGTH = 0x100
    image (rw) : ORIGIN = 0x7000, LENGTH = 0x1000
    boot (rwx) : ORIGIN = 0x8000, LENGTH = 0x4000
    low (rwx) : ORIGIN = 0x0200, LENGTH = 0x200
    screen_end (rw) : ORIGIN = 0x06a8, LENGTH = 0x150
    screen (rwx) : ORIGIN = 0x0400, LENGTH = 0x3f8
}
__rc0 = __basic_zp_start;
INCLUDE imag-regs.ld
__basic_zp_size = __basic_zp_end - __basic_zp_start;
MEMORY { zp : ORIGIN = __rc31 + 1, LENGTH = __basic_zp_end - (__rc31 + 1) }
INPUT(basic-header.o)
INPUT(unmap-basic.o)
REGION_ALIAS("c_readonly", ram)
REGION_ALIAS("c_writeable", ram)
SECTIONS {
    .basic_header : { *(.basic_header) } >ram
    INCLUDE zp.ld
    /* rk_dilation at RK_DILATION and ov2_irq_count at RK_IRQ_COUNT
     * (resident_flight.h), below rk_hardware: flight modules link
     * ov2_irq_count's address. Before the .bss rule, which would take them
     * otherwise. */
    .rk_dilation 0x6fb8 : { KEEP(*(.bss.rk_dilation)) } >services
    .rk_irq_count 0x6fbc : { KEEP(*(.bss.ov2_irq)) } >services
    .boot : {
        __rk_boot_start = .;
        *(.boot.text .boot.text.*)
        *(.text.apollo_reu_store_open .text.apollo_reu_store_verify .text.apollo_reu_store_backend .text.apollo_ov2_init)
        __rk_boot_end = .;
    } >boot
    .dsky : {
        __rk_dsky_start = .;
        *(.dsky.text .dsky.text.* .dsky.data .dsky.data.*)
        __rk_dsky_end = .;
    } >low AT>boot
    __rk_dsky_load = LOADADDR(.dsky);
    .dsky_data : {
        __rk_dsky_data_start = .;
        *(.dsky.rodata .dsky.rodata.*)
        __rk_dsky_data_end = .;
    } >screen_end AT>boot
    __rk_dsky_data_load = LOADADDR(.dsky_data);
    .dsky_bss (NOLOAD) : {
        __rk_dsky_bss_start = .;
        *(.dsky.bss .dsky.bss.*)
        __rk_dsky_bss_end = .;
    } >screen_end
    /* The measurement and replay harness's own code (platform/c64/
     * dap_pass_check.c DP_SCREEN_TEXT) in the text screen ($0400-$07F7),
     * which nothing shows there; its load_snapshot copies it. Builds with
     * the flight DSKY have none (that DSKY uses the last rows, .dsky_data). */
    .dp_screen : {
        __dp_screen_start = .;
        *(.dp.screen .dp.screen.*)
        __dp_screen_end = .;
    } >screen AT>boot
    __dp_screen_load = LOADADDR(.dp_screen);
    ASSERT(__dp_screen_end <= 0x07e8, "harness code past the text screen's last character")
    ASSERT(__dp_screen_end == __dp_screen_start || __dp_screen_end <= 0x06a8 ||
           (__rk_dsky_data_end == __rk_dsky_data_start && __rk_dsky_bss_end == __rk_dsky_bss_start),
           "harness code in the text screen overlaps the DSKY's last rows")
    .text : { INCLUDE text-sections.ld } >ram
    .rodata : { INCLUDE rodata-sections.ld } >ram
    .data : { INCLUDE data-sections.ld } >ram
    INCLUDE data-symbols.ld
    .bss (NOLOAD) : { INCLUDE bss-sections.ld } >ram
    INCLUDE bss-symbols.ld
    .noinit (NOLOAD) : { INCLUDE noinit-sections.ld } >ram
    __rk_resident_end = .;
    /* Zeroed state of the measurement harnesses (dap_pass_check.c
     * CASSETTE) in the cassette buffer, above the step profile ($033C-
     * $0381) and below the replay row ($03F2). Flight builds have none. */
    .cassette 0x0384 (NOLOAD) : {
        __dp_cassette_start = .;
        *(.cassette .cassette.*)
        __dp_cassette_end = .;
    }
    .apollo_services 0x6f00 : { KEEP(*(.apollo_services)) } >services
    __rk_services_end = .;
    /* rk_hardware at RK_HARDWARE and rk_flight at RK_FLIGHT_IO
     * (resident_flight.h): flight modules link these addresses. */
    .rk_hardware 0x6fbe : { KEEP(*(.rk_hardware)) } >services
    .rk_flight 0x6fdd : { KEEP(*(.rk_flight)) } >services
    .rk_restart_lamp 0x6ff5 : { KEEP(*(.rk_restart_lamp)) } >services
    /* rk_part_cont and rk_part_prev (resident_flight.h RK_PART_AT): the part
     * modules reach them by address. */
    .rk_part 0x6ff6 : { KEEP(*(.rk_part)) } >services
}
ASSERT(__rk_services_end<=0x6fb8,"the service table overlaps rk_dilation")
ASSERT(rk_dilation==0x6fb8 && SIZEOF(.rk_dilation)==4,"rk_dilation is not at RK_DILATION")
ASSERT(ov2_irq_count==0x6fbc && SIZEOF(.rk_irq_count)==2,"ov2_irq_count is not at RK_IRQ_COUNT")
ASSERT(rk_hardware==0x6fbe && SIZEOF(.rk_hardware)<=0x1f,"rk_hardware is not at RK_HARDWARE or overlaps rk_flight")
ASSERT(rk_flight==0x6fdd && SIZEOF(.rk_flight)<=0x18,"rk_flight is not at RK_FLIGHT_IO")
ASSERT(rk_restart_lamp==0x6ff5 && SIZEOF(.rk_restart_lamp)==1,"rk_restart_lamp is not at RK_RESTART_LAMP")
ASSERT(rk_part==0x6ff6 && SIZEOF(.rk_part)==2,"rk_part is not at RK_PART_AT")
__stack = 0xd000;
ASSERT(__rk_resident_end<=0x6f00,"resident overlaps the ABI 2 service table")
ASSERT(__dp_cassette_end<=0x03f2,"harness state overlaps the replay row at $03F2")
OUTPUT_FORMAT { SHORT(0x0801) FULL(ram) FULL(services) FULL(image) TRIM(boot) }
