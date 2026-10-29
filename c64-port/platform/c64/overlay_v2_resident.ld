/* SPDX-License-Identifier: MIT
 * ABI 2 resident. $0801..$6EFF code and data, $6F00..$6FFF service jump
 * table, $7000 canonical erasable image, $8000 job slot, $C000 scratch,
 * $C800 software stack, $E000 task slot, $FFFA RAM vectors. */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY { ram (rw) : ORIGIN = 0x0801, LENGTH = 0x67ff }
INCLUDE commodore.ld
INPUT(unmap-basic.o)
SECTIONS { .apollo_services 0x6f00 : { KEEP(*(.apollo_services)) } > ram }
__stack = 0xd000;
ASSERT(ADDR(.bss)+SIZEOF(.bss)<=0x6f00,"ABI 2 resident overlaps the service table")
ASSERT(SIZEOF(.apollo_services)<=0x100,"ABI 2 service table exceeds 256 bytes")
OUTPUT_FORMAT { SHORT(0x0801) TRIM(ram) }
