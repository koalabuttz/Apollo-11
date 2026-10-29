/* SPDX-License-Identifier: MIT
 * Stock KERNAL $ff48 has already saved A/X/Y before JMP ($0314).
 * __rc0/1 are the LLVM-MOS software-stack pointer, __rc2..31 imaginary registers.
 * IRQ build requires -fno-static-stack -mlto-zp=0: no shared automatic/ZP spills. */
.zeropage __rc0
.zeropage __rc1
.section .bss.c64_irq_saved,"aw",@nobits
.globl c64_irq_saved
c64_irq_saved: .space 32
.section .text.c64_irq_gate,"ax",@progbits
.globl c64_irq_gate
.type c64_irq_gate,@function
c64_irq_gate:
    lda $d019
    and $d01a
    and #1
    bne .Lowned
    jmp (c64_irq_previous)
.Lowned:
    lda #1
    sta $d019
    lda #$81
    sta $c030
    cld
    ldx #31
.Lsave:
    lda __rc0,x
    sta c64_irq_saved,x
    dex
    bpl .Lsave
    lda #<(c64_irq_stack+256)
    sta __rc0
    lda #>(c64_irq_stack+256)
    sta __rc1
    jsr c64_irq_producer
    lda c64_irq_poison
    beq .Lrestore_begin
    ldx #31
.Lpoison:
    txa
    eor #$5a
    sta __rc0,x
    dex
    bpl .Lpoison
.Lrestore_begin:
    ldx #31
.Lrestore:
    lda c64_irq_saved,x
    sta __rc0,x
    dex
    bpl .Lrestore
    lda #$82
    sta $c030
    jmp $ea81 /* KERNAL restores Y/X/A; RTI restores interrupted P/PC. */
.size c64_irq_gate,.-c64_irq_gate

/* Diagnostic assembly caller: verifies all 32 registers, A/X/Y, D and I.
 * No C call occurs while registers contain canaries. IRQ deliberately poisons
 * all 32 before its restore. Protect normal callee-saved rc20..31 on entry. */
.section .bss.c64_probe,"aw",@nobits
.Lprobe_sp: .space 2
.Lprobe_bad: .space 1
.section .text.c64_register_probe,"ax",@progbits
.globl c64_register_probe
.type c64_register_probe,@function
c64_register_probe:
    php
    sei
    lda __rc0
    sta .Lprobe_sp
    lda __rc1
    sta .Lprobe_sp+1
    ldx #20
.Lpush:
    lda __rc0,x
    pha
    inx
    cpx #32
    bne .Lpush
    lda #0
    sta .Lprobe_bad
    ldx #2
.Lseed:
    txa
    eor #$a5
    sta __rc0,x
    inx
    cpx #32
    bne .Lseed
    sed
    lda #$39
    ldx #$62
    ldy #$71
    cli
.Lwait:
    bit c64_irq_done
    bpl .Lwait
    cmp #$39
    beq .Lx
    inc .Lprobe_bad
.Lx:
    cpx #$62
    beq .Ly
    inc .Lprobe_bad
.Ly:
    cpy #$71
    beq .Lp
    inc .Lprobe_bad
.Lp:
    php
    pla
    and #$0c
    cmp #$08 /* Decimal restored, IRQ enabled by CLI. */
    beq .Lregs
    inc .Lprobe_bad
.Lregs:
    sei
    cld
    ldx #2
.Lcheck:
    txa
    eor #$a5
    cmp __rc0,x
    beq .Lnext
    inc .Lprobe_bad
.Lnext:
    inx
    cpx #32
    bne .Lcheck
    lda __rc0
    cmp .Lprobe_sp
    beq .Lsp1
    inc .Lprobe_bad
.Lsp1:
    lda __rc1
    cmp .Lprobe_sp+1
    beq .Lpop_begin
    inc .Lprobe_bad
.Lpop_begin:
    ldx #31
.Lpop:
    pla
    sta __rc0,x
    dex
    cpx #19
    bne .Lpop
    plp
    lda .Lprobe_bad
    rts
.size c64_register_probe,.-c64_register_probe
