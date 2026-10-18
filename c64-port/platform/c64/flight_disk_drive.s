; SPDX-License-Identifier: MIT
; The 1581 side of the flight disk loader (docs/RUN-ON-C64.md). The C64
; writes it to drive RAM at $0600 with M-W, puts a file's first track and
; sector in trk and sec and the number of sectors to send in max (0 for the
; whole chain), and starts it with M-E. It reads the file's sectors with
; job 0 (READ into the buffer at $0300, run at once through the ROM's $FF54)
; and sends each one: a count byte (the data bytes that follow: 254, or
; fewer in the last sector), then the data. A read error sends $FF and the
; job's error code. At the end of the chain (or after max sectors) it
; releases the bus and returns to DOS.
;
; Each byte goes out two bits at a time on DATA (the high bit of a pair) and
; CLK (the low bit), low pair first; a released line is a 1. The drive
; releases DATA when a byte is ready. The C64 then pulls CLK low for 6 of
; its cycles; 12 us after the drive sees it the first pair goes out, then one
; every 16 us (the C64 reads each in the middle of its 16 us). Between the
; pairs the drive works out the next byte's four port values, so 18 us after
; the fourth pair it is ready with the next byte (DATA released): by then
; the C64 has read the fourth pair (at 70 of its cycles after the strobe)
; and is not yet looking for the next (at 87 or later). At a sector's end it
; pulls DATA low instead (busy) while it reads the next. The 1581 runs at
; 2 MHz: the counts below are drive cycles. The C64 side is
; flight_disk_rx.s.
        .text
        .globl fd_drive_start
fd_drive_start:
        lda #$02
        sta $4001          ; busy: DATA low
next:   lda trk
        beq done
        sta $0b            ; job 0 track
        lda sec
        sta $0c            ; job 0 sector
        ldx #0
        lda #$80           ; job 0: READ into $0300
        jsr $ff54          ; run it now and wait (the ROM's BRK into the
        sei                ; controller); A: the job's status
        cmp #$02
        bcs err
        lda $0300
        sta trk
        lda $0301
        sta sec
        ldx trk
        bne full
        sec
        sbc #1             ; the last sector: bytes 2 to its link
        .byte $2c          ; bit abs: skip the next lda
full:   lda #254
        sta $0301          ; the count goes out first, from byte 1
        tax
        inx
        jsr send           ; bytes 1 to count + 1
        lda max            ; 0: the whole chain
        beq next
        dec max
        bne next
        beq done
err:    sta $0302
        lda #$ff
        sta $0301
        ldx #2
        jsr send
done:   lda #$00
        sta $4001          ; release the bus
        cli
        rts

; Send X bytes (1-255) from $0301.
send:   stx left
        lda #$01
        sta fetch+1
        lda $0301
        sta nb
        jsr split          ; the first byte's port values
        inc fetch+1
        ldx v1
        ldy v2
byte:   lda #$00
        sta $4001          ; ready: DATA released
wclk:   lda $4001          ; the read is cycle 0 when CLK is seen low
        and #$04
        beq wclk
fetch:  lda $0300          ; 5-20: the next byte (the address's low byte
        sta nb             ; steps through the sector)
        inc fetch+1
        nop
        stx $4001          ; pair 1 at cycle 24 (12 us)
        lda nb             ; 25-52: the next byte's pair 1, X free
        and #3
        tax
        lda map,x
        sta v1
        lda nb
        lsr
        lsr
        sta nb
        sty $4001          ; pair 2 at 56
        and #3             ; 57-80: its pair 2, Y free
        tax
        lda map,x
        sta v2
        lda nb
        lsr
        lsr
        sta nb
        lda v3
        sta $4001          ; pair 3 at 88
        lda nb             ; 89-112: its pair 3
        and #3
        tax
        lda map,x
        sta v3
        lda nb
        lsr
        lsr                ; its pair 4's bits, kept in A
        ldx v4
        stx $4001          ; pair 4 at 120
        tax                ; 121-152: its pair 4, then X and Y for its
        lda map,x          ; pairs 1 and 2
        sta v4
        ldx v1
        ldy v2
        lda #$00
        dec left
        beq last
        nop
        nop
        sta $4001          ; ready again at 156 (78 us)
        jmp wclk
last:   lda #$02
        sta $4001          ; busy at 155: the next sector
        rts

; v1-v4: the port values of the pairs of the byte in nb, low pair first.
split:  ldy #0
1:      lda nb
        and #3
        tax
        lda map,x
        sta v1,y
        lsr nb
        lsr nb
        iny
        cpy #4
        bne 1b
        rts
; Port B values for a pair (high bit, low bit): bit 1 pulls DATA low, bit 3
; pulls CLK low. In one page: map,x takes no extra cycle.
map:    .byte $0a, $02, $08, $00
        .globl fd_drive_trk
fd_drive_trk:
trk:    .byte 0
sec:    .byte 0
max:    .byte 0
left:   .byte 0
nb:     .byte 0
v1:     .byte 0
v2:     .byte 0
v3:     .byte 0
v4:     .byte 0
