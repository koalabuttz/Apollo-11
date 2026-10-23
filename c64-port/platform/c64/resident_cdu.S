/* SPDX-License-Identifier: MIT
 * The output drives for the C64 resident (resident_cdu.h), in 6502: the
 * same results as src/resident_cdu.c's rk_cdu_drive, which stays the host
 * and yaAGC-vector version (tools/resident_cdu_asm_check.c runs both).
 *
 *   void rk_cdu_drive6502(uint8_t hi);
 * One 10 ms period. hi: channel 14's high byte (bit 15 is 0100). The
 * counters, from CDUXCMD (050) at the fixed image (APOLLO_SHARED_ADDRESS),
 * one per channel 14 bit from 15 down: 3 in the LM, 5 in the CM (053 and
 * 054 on bits 12 and 11). A period with none of their bits only counts
 * its 4 time units and returns.
 * In: hi in A. Uses A, X, Y and __rc2-__rc5, which a callee may clobber
 * (the LLVM-MOS calling convention). */
#if defined(RK_CM)
#define N 5
#define MASK 0x7c
#else
#define N 3
#define MASK 0x70
#endif
IMG = 0x7000 + 2 * 050  /* CDUXCMD's word in the fixed image */

.zeropage __rc2, __rc3, __rc4, __rc5
HI = __rc2     /* channel 14's high byte */
BIT = __rc3    /* this counter's bit in it */
CAP = __rc4    /* pulses a burst */
BURST = __rc5  /* a counter still owes pulses */

/* rk_cdu_state rk_cdu: int16_t owed[5]; uint16_t elapsed; uint8_t active. */
OWED = rk_cdu
ELAPSED = rk_cdu + 10
ACTIVE = rk_cdu + 12

/* The replay and measurement harness (platform/c64/dap_pass_check.c, DP_SNAP)
 * runs the drive from the text screen, not from the resident's RAM below the
 * service table: from .dp_screen, or for the hardware check, whose text
 * fills the screen's top rows, from the last rows (.dsky_data), which it
 * colours black. Its load_snapshot copies both. */
#if defined(DP_SNAP) && defined(DP_HW_CHECK)
.section .dsky.rodata.rk_cdu_drive6502,"ax",@progbits
#elif defined(DP_SNAP)
.section .dp.screen.rk_cdu_drive6502,"ax",@progbits
#else
.section .text.rk_cdu_drive6502,"ax",@progbits
#endif
.globl rk_cdu_drive6502
.type rk_cdu_drive6502,@function
rk_cdu_drive6502:
    sta HI
    /* elapsed += 4, short of 0xfff0. */
    ldx ELAPSED+1
    inx
    bne .Lcount         /* high byte not 0xff */
    ldx ELAPSED
    cpx #0xf0
    bcs .Lcounted
.Lcount:
    lda ELAPSED
    clc
    adc #4
    sta ELAPSED
    bcc .Lcounted
    inc ELAPSED+1
.Lcounted:
    lda HI
    and #MASK
    bne .Lon
    rts                 /* none of the counters' bits */
.Lon:
    /* Idle (nothing owed): the burst comes at once. */
    lda ACTIVE
    bne .Lwait
    lda #34
    sta ELAPSED
    lda #0
    sta ELAPSED+1
.Lwait:
    /* while (elapsed > 30) */
    lda ELAPSED+1
    bne .Lburst
    lda ELAPSED
    cmp #31
    bcs .Lburst
    rts
.Lburst:
    lda ELAPSED
    sec
    sbc #30
    sta ELAPSED
    bcs 1f
    dec ELAPSED+1
1:
    lda #0
    sta BURST
    lda #0x40
    sta BIT
    ldx #0
.Lcounter:
    lda HI
    and BIT
    beq .Lowed
    /* owed += the count, a one's complement word (-0 is 0); count = 0. */
    clc
    lda OWED,x
    adc IMG,x
    sta OWED,x
    lda OWED+1,x
    adc IMG+1,x
    sta OWED+1,x
    lda IMG+1,x
    and #0x40
    beq 2f
    clc                 /* negative: less 077777, so + 0x8001 */
    lda OWED,x
    adc #1
    sta OWED,x
    lda OWED+1,x
    adc #0x80
    sta OWED+1,x
2:
    lda #0
    sta IMG,x
    sta IMG+1,x
.Lowed:
    /* At most 24 IMU pulses a burst (050-052), 240 optics pulses: the
     * magnitude less the cap, never past 0. */
    ldy #24
    cpx #6
    bcc 3f
    ldy #240
3:
    sty CAP
    lda OWED+1,x
    bmi .Lneg
    sec
    lda OWED,x
    sbc CAP
    sta OWED,x
    lda OWED+1,x
    sbc #0
    sta OWED+1,x
    bpl .Lleft          /* 0 or more */
    bmi .Lzero
.Lneg:
    clc
    lda OWED,x
    adc CAP
    sta OWED,x
    lda OWED+1,x
    adc #0
    sta OWED+1,x
    bmi .Lowes          /* still owes */
.Lzero:
    lda #0
    sta OWED,x
    sta OWED+1,x
    beq .Lnext
.Lleft:
    ora OWED,x
    beq .Lnext
.Lowes:
    lda #1
    sta BURST
.Lnext:
    inx
    inx
    lsr BIT
    cpx #2 * N
    beq 4f
    jmp .Lcounter
4:
    lda BURST
    sta ACTIVE
    beq 5f
    jmp .Lwait
5:
    rts
