/* SPDX-License-Identifier: MIT
 * The work clock's per-step checks for the C64 resident
 * (platform/c64/resident_flight.c), in 6502: the two the step pays every
 * time, with the same results as the C they replace.
 *
 *   void rk_rupts(void);
 * Most steps end no timer step and find no request pending (rk_pending &
 * 03776): rk_rupt_take would take nothing and rk_rupts_due's loop stop at
 * once, so only its last two stores are left (rk_idle, and with
 * RK_RUPT_TRACE rk_rupt_last, to 0); else rk_rupts_due.
 *
 *   uint16_t rk_limit(uint8_t mct);
 * rk_module_limit(rk_budget(), mct): 4 for the idle weight (RK_MCT_IDLE,
 * 0), else 1 + budget / mct, by shift and subtract in 16 passes; the C's
 * loop subtracts once per routine allowed. A weight is 0 or at least 4
 * (tools/resident_flight_build.py mct_weight) and a budget at most 854,
 * so the count is at most 214 and fits the low byte returned (as it
 * fits the C's uint8_t count).
 * In: mct in A. Out: A low, X high. Uses Y, __rc4-__rc6, which a callee
 * may clobber (the LLVM-MOS calling convention).
 *
 *   uint16_t rk_budget(void);
 * Below. */
#ifndef RK_WALL_CLOCK
.zeropage __rc4, __rc5, __rc6
BL = __rc4      /* the budget, shifted out as the quotient shifts in */
BH = __rc5
D = __rc6       /* mct */

.section .text.rk_rupts,"ax",@progbits
.globl rk_rupts
.type rk_rupts,@function
rk_rupts:
    lda rk_pending
    and #$fe
    bne .Ldue
    lda rk_pending+1
    and #$07
    bne .Ldue
    lda rk_tick_phase
    asl a
    tay
    lda rk_work+1
    cmp rk_phase_mct+1,y
    bcc .Lnone          /* high byte below: rk_work < the step's MCT */
    bne .Ldue
    lda rk_work
    cmp rk_phase_mct,y
    bcs .Ldue
.Lnone:
    lda #0
    sta rk_idle
#ifdef RK_RUPT_TRACE
    sta rk_rupt_last
#endif
    rts
.Ldue:
    jmp rk_rupts_due

/*   uint16_t rk_budget(void);
 * The MCT left before the next timer step: the step's MCT less rk_work.
 * With a request waiting (rk_pending: interrupts inhibited, or inside an
 * interrupt) it is 0: the host takes the request at the first routine
 * boundary it can, so a run goes one routine at a time (RELINT, RESUME),
 * and rk_limit's count of 1 makes that run a cut one (rk_cut): if the
 * request is not taken, the next step goes on in the same module, whose
 * table the host goes on charging (tools/flight_host_run.c, charged).
 * Out: A low, X high. Uses Y. */
.section .text.rk_budget,"ax",@progbits
.globl rk_budget
.type rk_budget,@function
rk_budget:
    lda rk_pending
    ora rk_pending+1
    beq .Lfree
    lda #0
    tax
    rts
.Lfree:
#ifdef DP_REPLAY
    jmp dp_budget       /* platform/c64/dap_pass_check.c: the replay's cap */
.globl rk_phase_budget
.type rk_phase_budget,@function
rk_phase_budget:
#endif
    lda rk_tick_phase
    asl a
    tay
    sec
    lda rk_phase_mct,y
    sbc rk_work
    pha
    lda rk_phase_mct+1,y
    sbc rk_work+1
    tax
    pla
    rts

.section .text.rk_limit,"ax",@progbits
.globl rk_limit
.type rk_limit,@function
rk_limit:
    cmp #0
    bne .Lweigh
    lda #4              /* RK_MCT_IDLE: one pass */
    sta rk_run_limit    /* for rk_charge: a run of them all was cut */
    ldx #0
    rts
.Lweigh:
#ifdef DP_REPLAY
    pha                 /* dp_budget is C: it may clobber __rc4-__rc6 */
    jsr rk_budget
    sta BL
    stx BH
    pla
    sta D
#else
    sta D
    jsr rk_budget
    sta BL
    stx BH
#endif
    ldy #16
    lda #0              /* the remainder; carry its ninth bit */
.Lbit:
    asl BL
    rol BH
    rol a
    bcs .Lsub           /* over 255: at least mct (carry set) */
    cmp D
    bcc .Lnext
.Lsub:
    sbc D
    inc BL
.Lnext:
    dey
    bne .Lbit
    ldx #0
    lda BL              /* the quotient's low byte, plus 1 */
    clc
    adc #1
    sta rk_run_limit
    rts
#endif
