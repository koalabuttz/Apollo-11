/* SPDX-License-Identifier: MIT */
__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;
MEMORY { ram (rw) : ORIGIN = 0x0801, LENGTH = 0x67ff }
INCLUDE commodore.ld
INPUT(unmap-basic.o)
/* Canonical source RAM is $7000-$7fff; overlay is $8000-$bfff.
 * $c000-$c7ff is reserved transient scratch; $c800-$cfff is software stack. */
__stack = 0xd000;
ASSERT(ADDR(.bss)+SIZEOF(.bss)<=0x7000,"resident overlaps canonical source RAM")
OUTPUT_FORMAT { SHORT(0x0801) TRIM(ram) }
