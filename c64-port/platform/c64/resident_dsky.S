/* SPDX-License-Identifier: MIT
 * The flight resident's DSKY at run time (platform/c64/resident_dsky.c
 * says what it shows and why; rk_dsky_install there puts it in place):
 * the keyboard scan in the CIA1 interrupt and the key and display work
 * from the AGC's idle loop. Assembly, to fit $0200-$03FF; its tables and
 * state go in the screen's last rows (.dsky.rodata, .dsky.bss). */
#if RK_LM
#define CHANNEL11 rk_hardware       /* agc_hardware.channel11 */
#else
#define CHANNEL11 rk_device         /* cm_native_io.channel11 */
#define MAIN_KEYS rk_device+8       /* cm_native_io.main_keys */
#endif
#define CHANNEL15 rk_hardware+10    /* agc_hardware.channel15 */
#define CHANNEL32 rk_hardware+15    /* agc_hardware.channel32, high byte */
#define KEY1      $20               /* RK_RUPT_BIT(RK_RUPT_KEY1), low byte */
#define DSPTAB    $7426             /* the image's word 01023 */
#define NUMBERS   $0500             /* the digits' screen page */
#define COLOUR    $d800
#define PRO       $40               /* rk_dsky_keys' code for PRO */
#define LIT       7                 /* yellow */
#define DARK      11                /* dark grey */
/* Scratch and small state in zero page $E0-$F1: the KERNAL's, free with
 * it banked out (the C code's zero page ends at $8F). */
#define lamps      $e0              /* 2: the lamp bytes as drawn */
#define now        $e2              /* 2 */
#define tail       $e4              /* the queue's next to take */
#define hold       $e5              /* idle passes left after PRO */
#define drawn_at   $e6
#define dark       $e7
#define blank      $e8
#define new_left   $e9
#define new_right  $ea
#define new_sign   $eb
#define save_x     $ec
#define count      $ed
#define scan_count $ee
#define last_key   $ef
#define rk_dsky_head $f0            /* the queue's next to fill */

.section .dsky.text,"ax",@progbits
/* The CIA1 interrupt's entry, going on to ov2_irq_entry. Every fourth
 * interrupt (25 a second) it looks at the keyboard: with no key down (all
 * columns selected, no row low) that is all; with one, it takes the first
 * key of rk_dsky_keys that is down. A change from the last key goes into
 * the queue (0 for a release; a full queue loses the oldest). */
.globl rk_dsky_irq
rk_dsky_irq:
    dec scan_count
    bne 6f
    pha
    lda #4
    sta scan_count
    lda $01             /* the class-2 slot runs with I/O out */
    pha
    lda #$35
    sta $01
    txa
    pha
    lda #0
    sta $dc00
    lda $dc01
    eor #$ff            /* no key: A = 0 */
    beq 3f
    ldx #0
2:  lda rk_dsky_keys,x
    beq 3f              /* none of the DSKY's keys: A = 0 */
    sta $dc00
    lda $dc01
    and rk_dsky_keys+1,x
    beq 4f
    inx
    inx
    inx
    bne 2b
4:  lda rk_dsky_keys+2,x
3:  cmp last_key
    beq 5f
    sta last_key
    ldx rk_dsky_head
    sta rk_dsky_queue,x
    inx
    txa
    and #7
    sta rk_dsky_head
5:  lda #$ff
    sta $dc00
    pla
    tax
    pla
    sta $01
    pla
6:  jmp ov2_irq_entry

/* From the idle loop: the next key, then the display if 5 interrupts (50
 * ms) have gone by since the last look. A key waits while KEYRUPT1 is
 * still pending; after PRO, 255 idle passes go by before the next. */
.globl rk_dsky_idle
rk_dsky_idle:
    lda hold
    beq 1f
    dec hold
    bne look
1:  ldx tail
    cpx rk_dsky_head
    beq look
    lda rk_pending
    and #KEY1
    bne look
    ldy rk_dsky_queue,x
    inx
    txa
    and #7
    sta tail
    lda CHANNEL32
    cpy #PRO
    bne 2f
    and #$bf            /* PRO down: channel 32 bit 14 low */
    sta CHANNEL32
    lda #255
    sta hold
    bne look
2:  ora #$40
    sta CHANNEL32
    tya
    beq look            /* a release */
    sta CHANNEL15       /* the high bytes stay 0 */
    cmp #$12            /* RSET (022) resets the RESTART flip-flop, in */
    bne 3f              /* the AGC's hardware as here */
    lsr rk_restart_lamp
3:
#if !RK_LM
    sta MAIN_KEYS
#endif
    lda rk_pending
    ora #KEY1
    sta rk_pending
look:
    lda ov2_irq_count
    sec
    sbc drawn_at
    cmp #5
    bcs show
    rts

/* The display. The DSKY's flash, on the C64's clock: dark 0.32 s in 1.28
 * s; the verb and noun go blank in it while channel 11's 040 is set. */
show:
    lda ov2_irq_count
    sta drawn_at
    ldy #0
    and #$60
    bne 1f
    iny                 /* dark */
1:  sty dark
    tya
    beq 2f
    lda CHANNEL11
    and #$20
2:  cmp blank
    beq 3f
    sta blank
    ldx #8
    jsr draw
    inx
    jsr draw
    /* Relay words 1-11 from DSPTAB+0 to +10: a negative entry has not
     * gone out, so the relays hold what they had. */
3:  ldx #0
4:  txa
    asl
    tay
    lda DSPTAB+1,y
    and #$40
    bne 5f
    lda DSPTAB,y
    and #$1f
    sta new_right
    lda DSPTAB+1,y
    and #$04            /* bit 11: the sign */
    sta new_sign
    lda DSPTAB+1,y
    asl
    asl
    asl
    sta new_left
    lda DSPTAB,y
    lsr
    lsr
    lsr
    lsr
    lsr
    ora new_left
    and #$1f
    sta new_left
    cmp left,x
    bne 6f
    lda new_right
    cmp right,x
    bne 6f
    lda new_sign
    cmp sign,x
    beq 5f
6:  lda new_left
    sta left,x
    lda new_right
    sta right,x
    lda new_sign
    sta sign,x
    jsr draw
    jsr draw_signs
5:  inx
    cpx #11
    bne 4b
    /* The lamps: channel 11's 02-0100 (KEY REL and OPR ERR flash), row
     * 12's 04-0400 from DSPTAB+11 (when it has gone out) and RESTART. */
    lda #$5e            /* 0136 */
    ldy dark
    beq 1f
    lda #$0e            /* 016 */
1:  and CHANNEL11
    sta now
    lda DSPTAB+23
    and #$40
    beq 2f
    lda lamps+1
    jmp 3f
2:  lda DSPTAB+23       /* (word >> 2) & 0177 */
    lsr
    lda DSPTAB+22
    ror
    lsr
3:  and #$7f
    ldy rk_restart_lamp
    beq 4f
    ora #$80
4:  sta now+1
    ldx #0
5:  lda rk_dsky_lamps+2,x
    beq 9f
    sta count
    ldy rk_dsky_lamps,x
    lda now,y
    eor lamps,y
    and rk_dsky_lamps+1,x
    beq 8f
    lda now,y
    and rk_dsky_lamps+1,x
    beq 6f
    lda #LIT
    .byte $2c           /* bit abs: skip the next lda */
6:  lda #DARK
    ldy rk_dsky_lamps+3,x
7:  sta COLOUR,y
    iny
    dec count
    bne 7b
8:  inx
    inx
    inx
    inx
    bne 5b
9:  lda now
    sta lamps
    lda now+1
    sta lamps+1
    rts

/* Relay word X+1's two digits (blank for the verb and noun in the
 * flash's dark phase); X kept. */
draw:
    lda left,x
    cmp #$ff
    beq 2f              /* not drawn yet */
    tay
    lda rk_dsky_digit,y
    sta new_left
    ldy right,x
    lda rk_dsky_digit,y
    sta new_right
    lda blank
    beq 1f
    cpx #8
    bcc 1f
    cpx #10
    bcs 1f
    lda #' '
    sta new_left
    sta new_right
1:  ldy rk_dsky_at_left,x
    lda new_left
    sta NUMBERS,y
    ldy rk_dsky_at_right,x
    lda new_right
    sta NUMBERS,y
2:  rts

/* R1-R3's signs: + for the plus word's bit 11, - for the minus word's. X
 * kept. */
draw_signs:
    stx save_x
    ldx #6
1:  ldy rk_dsky_signs+1,x
    lda sign,y
    bne 2f
    ldy rk_dsky_signs+2,x
    lda sign,y
    bne 3f
    lda #' '
    .byte $2c
2:  lda #'+'
    .byte $2c
3:  lda #'-'
    ldy rk_dsky_signs,x
    sta NUMBERS,y
    dex
    dex
    dex
    bpl 1b
    ldx save_x
    rts

.section .dsky.rodata,"a",@progbits
/* The key table: the $DC00 column select, the $DC01 row bit and the
 * channel 15 code; a 0 ends it. */
.globl rk_dsky_keys
rk_dsky_keys:
    .byte $7f, $01, 001     /* 1 */
    .byte $7f, $08, 002     /* 2 */
    .byte $fd, $01, 003     /* 3 */
    .byte $fd, $08, 004     /* 4 */
    .byte $fb, $01, 005     /* 5 */
    .byte $fb, $08, 006     /* 6 */
    .byte $f7, $01, 007     /* 7 */
    .byte $f7, $08, 010     /* 8 */
    .byte $ef, $01, 011     /* 9 */
    .byte $ef, $08, 020     /* 0 */
    .byte $f7, $80, 021     /* V: VERB */
    .byte $ef, $80, 037     /* N: NOUN */
    .byte $df, $01, 032     /* + */
    .byte $df, $08, 033     /* - */
    .byte $fe, $02, 034     /* RETURN: ENTR */
    .byte $fb, $10, 036     /* C: CLR */
    .byte $fe, $01, 036     /* DEL: CLR */
    .byte $fb, $02, 022     /* R: RSET */
    .byte $ef, $20, 031     /* K: KEY REL */
    .byte $df, $02, PRO     /* P: PRO */
    .byte 0
/* A relay digit code's screen code (Luminary099 PINBALL: blank 0, 0-9 as
 * 21, 3, 25, 27, 15, 30, 28, 19, 29, 31); ? for no digit. */
rk_dsky_digit:
    .ascii " ??1???????????4???7?0???2?36859"
/* Relay words 1-11: where their left and right digits go in NUMBERS
 * (word 8 has no left digit: it goes to a blank cell). */
.globl rk_dsky_at_left, rk_dsky_at_right, rk_dsky_signs
rk_dsky_at_left:
    .byte 243, 241, 204, 202, 200, 163, 161, 0, 81, 73, 89
rk_dsky_at_right:
    .byte 244, 242, 240, 203, 201, 164, 162, 160, 82, 74, 90
/* R1-R3's signs: where, and the indexes of their plus and minus relay
 * words (7 and 6, 5 and 4, 2 and 1). */
rk_dsky_signs:
    .byte 159, 6, 5,  199, 4, 3,  239, 1, 0
/* The lamps: which lamp byte (0: channel 11; 1: row 12's 04-0400 shifted
 * right two, RESTART as 0200), the bit, the label's length and where it is
 * (in the screen's first page); a 0 length ends it. */
rk_dsky_lamps:
    .byte 0, 004, 11, 81    /* UPLINK ACTY */
    .byte 0, 010, 4, 94     /* TEMP */
    .byte 0, 002, 9, 107    /* COMP ACTY */
    .byte 0, 020, 7, 161    /* KEY REL */
    .byte 0, 0100, 7, 201   /* OPR ERR */
    .byte 1, 002, 6, 121    /* NO ATT (row 12's 010) */
    .byte 1, 010, 11, 134   /* GIMBAL LOCK (040) */
    .byte 1, 0100, 4, 147   /* PROG (0400) */
    .byte 1, 040, 7, 187    /* TRACKER (0200) */
    .byte 1, 004, 3, 214    /* ALT (020) */
    .byte 1, 001, 3, 227    /* VEL (04) */
    .byte 1, 0200, 7, 174   /* RESTART */
    .byte 0, 0, 0

.section .dsky.bss,"aw",@nobits
.globl rk_dsky_queue, rk_dsky_left
rk_dsky_queue: .zero 8
rk_dsky_left:
left:          .zero 11     /* the relay words as drawn: digit codes */
right:         .zero 11
sign:          .zero 11     /* bit 11 */
