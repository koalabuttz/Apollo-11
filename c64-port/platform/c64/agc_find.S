/* SPDX-License-Identifier: MIT
 * agc_transfer.c's find() for the C64 resident (AGC_TRANSFER_FIND_ASM): the
 * same result as the C, which stays the reference (tools/agc_find_check.c
 * compares the two in mos-sim). The first table in the list that holds the
 * key wins; a table whose first..last keys miss it is skipped; within a
 * table, a lower-bound search with one 16-bit compare per halving.
 *
 *   agc_routine agc_find(const agc_routine_table *tables, uint8_t count,
 *                        uint16_t physical);
 * In: tables in __rc2/3, count in A, physical low in X and high in __rc4.
 * Out: the routine (0 when none) in __rc2/3. Uses A, X, Y and __rc4-__rc15,
 * which the LLVM-MOS calling convention lets a callee clobber. */
.zeropage __rc2, __rc3, __rc4, __rc5, __rc6, __rc8, __rc9, __rc10, __rc11
.zeropage __rc12, __rc13, __rc14, __rc15
TP = __rc2      /* the table list, advanced a table at a time */
KH = __rc4      /* the key */
KL = __rc5
CNT = __rc6     /* tables left */
E = __rc8       /* the search's low entry */
N = __rc10      /* entries from E that may hold the key */
M = __rc12      /* a probe, or the table's last entry */
H = __rc14      /* half of N */
.section .text.agc_find,"ax",@progbits
.globl agc_find
.type agc_find,@function
agc_find:
    stx KL
    sta CNT
    lda TP
    ora TP+1
    beq .Lnone          /* no tables */
    lda KL
    and KH
    cmp #$ff
    bne .Ltable         /* not AGC_TRANSFER_NONE */
.Lnone:
    lda #0
    sta TP
    sta TP+1
    rts
.Ltable:
    lda CNT
    beq .Lnone
    dec CNT
    ldy #0
    lda (TP),y
    sta E
    iny
    lda (TP),y
    sta E+1
    iny
    lda (TP),y
    sta N
    iny
    lda (TP),y
    sta N+1
    clc                 /* the next table */
    lda TP
    adc #4
    sta TP
    bcc 1f
    inc TP+1
1:  lda N
    ora N+1
    beq .Ltable         /* empty */
    ldy #0              /* key < first: skip */
    lda KL
    cmp (E),y
    iny
    lda KH
    sbc (E),y
    bcc .Ltable
    lda N               /* M = E + 4 * (N - 1), the last entry */
    sec
    sbc #1
    sta M
    lda N+1
    sbc #0
    asl M
    rol a
    asl M
    rol a
    sta M+1
    clc
    lda M
    adc E
    sta M
    lda M+1
    adc E+1
    sta M+1
    ldy #0              /* last < key: skip */
    lda (M),y
    cmp KL
    iny
    lda (M),y
    sbc KH
    bcc .Lnext
.Lhalve:                /* E holds a key <= the key; N > 1 entries left */
    lda N+1
    bne 2f
    lda N
    cmp #2
    bcc .Lfound         /* N == 1 */
2:  lda N+1             /* H = N / 2 */
    lsr a
    sta H+1
    lda N
    ror a
    sta H
    asl a               /* M = E + 4 * H */
    sta M
    lda H+1
    rol a
    asl M
    rol a
    sta M+1
    clc
    lda M
    adc E
    sta M
    lda M+1
    adc E+1
    sta M+1
    ldy #0              /* M's key <= the key: search from M */
    lda KL
    cmp (M),y
    iny
    lda KH
    sbc (M),y
    bcc 3f
    lda M
    sta E
    lda M+1
    sta E+1
    sec                 /* N -= H */
    lda N
    sbc H
    sta N
    lda N+1
    sbc H+1
    sta N+1
    jmp .Lhalve
3:  lda H               /* N = H */
    sta N
    lda H+1
    sta N+1
    jmp .Lhalve
.Lnext:
    jmp .Ltable         /* not in this table: the next may hold it */
.Lfound:
    ldy #0
    lda (E),y
    cmp KL
    bne .Lnext
    iny
    lda (E),y
    cmp KH
    bne .Lnext
    iny
    lda (E),y
    sta TP
    iny
    lda (E),y
    sta TP+1
    rts
.size agc_find, . - agc_find
