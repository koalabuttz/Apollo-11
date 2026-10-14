/* SPDX-License-Identifier: MIT
 * Minimal resident RAM vectors for the ABI 2 proof ($01=$35, KERNAL out):
 * acknowledge CIA1, count, return. Production IRQ entry belongs to Lane A. */
.section .text.ov2_irq,"ax",@progbits
.globl ov2_irq_entry
.globl ov2_nmi_entry
.globl ov2_irq_count
ov2_irq_entry:
    pha
    lda $01             /* code may run with I/O banked out ($34/$30, the */
    pha                 /* $D000 extension): see I/O to acknowledge CIA1 */
    lda #$35
    sta $01
    lda $dc0d
    inc ov2_irq_count
    bne 1f
    inc ov2_irq_count+1
1:  pla
    sta $01
    pla
ov2_nmi_entry:
    rti
.section .bss.ov2_irq,"aw",@nobits
ov2_irq_count:
    .zero 2
