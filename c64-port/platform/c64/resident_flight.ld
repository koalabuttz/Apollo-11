/* SPDX-License-Identifier: MIT
 * The ABI 2 flight resident (docs/RESIDENT-FLIGHT.md), on the
 * overlay_v2_resident.ld map: resident code and data in $0801..$6EFF, the
 * service jump table at $6F00, the canonical image at $7000, the job slot at
 * $8000. Boot-only code (.boot.text, and the boot-only runtime functions
 * when LTO keeps them out of line) links into the job slot: the PRG loads it
 * there, it runs once from main, and the first module load overwrites it.
 * The PRG loads through the image's 4 KB (zero filled). */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x66ff
    services (rx) : ORIGIN = 0x6f00, LENGTH = 0x100
    image (rw) : ORIGIN = 0x7000, LENGTH = 0x1000
    boot (rwx) : ORIGIN = 0x8000, LENGTH = 0x4000
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
    .boot : {
        __rk_boot_start = .;
        *(.boot.text .boot.text.*)
        *(.text.apollo_reu_store_open .text.apollo_reu_store_verify .text.apollo_reu_store_backend .text.apollo_ov2_init)
        __rk_boot_end = .;
    } >boot
    .text : { INCLUDE text-sections.ld } >ram
    .rodata : { INCLUDE rodata-sections.ld } >ram
    .data : { INCLUDE data-sections.ld } >ram
    INCLUDE data-symbols.ld
    .bss (NOLOAD) : { INCLUDE bss-sections.ld } >ram
    INCLUDE bss-symbols.ld
    .noinit (NOLOAD) : { INCLUDE noinit-sections.ld } >ram
    __rk_resident_end = .;
    .apollo_services : { KEEP(*(.apollo_services)) } >services
    __rk_services_end = .;
    /* rk_hardware at RK_HARDWARE, rk_flight at RK_FLIGHT_IO and, CM,
     * cm_channels at RK_CM_CHANNELS (resident_flight.h): flight modules
     * link these addresses. */
    .rk_hardware 0x6fbe : { KEEP(*(.rk_hardware)) } >services
    .rk_flight 0x6fdc : { KEEP(*(.rk_flight)) } >services
    .rk_channels 0x6ff4 : { KEEP(*(.rk_channels)) } >services
}
ASSERT(__rk_services_end<=0x6fbe,"the service table overlaps rk_hardware")
ASSERT(rk_hardware==0x6fbe && SIZEOF(.rk_hardware)<=0x1e,"rk_hardware is not at RK_HARDWARE or overlaps rk_flight")
ASSERT(rk_flight==0x6fdc && SIZEOF(.rk_flight)<=0x18,"rk_flight is not at RK_FLIGHT_IO or overlaps cm_channels")
ASSERT(!DEFINED(cm_channels) || cm_channels==0x6ff4,"cm_channels is not at RK_CM_CHANNELS")
__stack = 0xd000;
ASSERT(__rk_resident_end<=0x6f00,"resident overlaps the ABI 2 service table")
OUTPUT_FORMAT { SHORT(0x0801) FULL(ram) FULL(services) FULL(image) TRIM(boot) }
