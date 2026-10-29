; SPDX-License-Identifier: MIT
; The flight disk loader's fdz decoder (flight_disk_loader.c; the format
; is in tools/fdz.c). fd_unz() decodes one stream from the loader's sector
; buffer (buf, pos, len; fd_sector refills it) into the 32 KB ring at
; $4000-$BFFF, calling z_send (with w, the ring offset written to) each
; time an 8 KB part is whole. It returns 1 at the stream's end, 0 when a
; sector could not be read or z_send failed (the hardware stack is unwound
; to the call). The last offset starts at 1, as tools/fdz.c's does.
;
; The two ring pointers are at $FB-$FE; Y is 0 between bytes. At the
; end w is where the stream stopped.
        .set dst, 0xfb               ; $FB-$FE: free under the KERNAL and
        .set src, 0xfd               ; outside llvm-mos's zero page
        .set END_HI, 97              ; 24576 / 256 + 1

        .bss
unz_sp:     .zero 1
bitbuf:     .zero 1
val:        .zero 2
off:        .zero 2
cnt_hi:     .zero 1
save_x:     .zero 1

; The next bit into C: shift it out of bitbuf, whose lowest 1 marks the
; end of its bits.
        .macro getbit
        asl bitbuf
        bne 9f
        jsr refill
9:
        .endm

        .text
        .globl fd_unz
fd_unz:
        tsx
        stx unz_sp
        ldy #0
        sty dst
        lda #0x40
        sta dst+1
        lda #0x80                    ; empty: only the marker bit
        sta bitbuf
        lda #1
        sta off
        sty off+1
literals:
        jsr gamma
        jsr count
1:      ldy pos                      ; get_byte, in line
        cpy len
        beq 7f
        lda buf,y
        inc pos
        ldy #0
8:      sta (dst),y
        inc dst
        bne 2f
        jsr dst_page
2:      dex
        bne 1b
        dec cnt_hi
        bne 1b
        getbit
        bcs new_match
        jsr gamma                    ; a repeat of the last offset
        jsr copy
        getbit
        bcc literals
        jmp new_match
7:      jsr get_byte
        jmp 8b
new_match:
        jsr gamma
        lda val
        cmp #END_HI
        beq done
        sec                          ; off = (hi - 1) * 256 + lo + 1
        sbc #1
        sta off+1
        jsr get_byte
        clc
        adc #1
        sta off
        bcc 1f
        inc off+1
1:      jsr gamma
        inc val                      ; the length less 1
        bne 2f
        inc val+1
2:      jsr copy
        getbit
        bcs new_match
        jmp literals
done:
        lda dst
        sta w
        lda dst+1
        sec
        sbc #0x40
        sta w+1
        lda #1
        rts

; val bytes from off back in the ring.
copy:
        lda dst
        sec
        sbc off
        sta src
        lda dst+1
        sbc off+1
        bcc 1f
        cmp #0x40
        bcs 5f
1:      adc #0x80                    ; carry clear: back round the ring
5:      sta src+1
        lda val+1                    ; most copies stay in dst's page and
        bne 6f                       ; clear of the ring's end: Y indexes
        lda src+1
        cmp #0xbf
        beq 6f
        lda dst
        clc
        adc val
        bcs 6f
        pha
        ldx val
1:      lda (src),y
        sta (dst),y
        iny
        dex
        bne 1b
        pla
        sta dst
        ldy #0
        rts
6:      jsr count
2:      lda (src),y
        sta (dst),y
        inc src
        bne 3f
        inc src+1
        lda src+1
        cmp #0xc0
        bne 3f
        lda #0x40
        sta src+1
3:      inc dst
        bne 4f
        jsr dst_page
4:      dex
        bne 2b
        dec cnt_hi
        bne 2b
        rts

; X and cnt_hi count val down (val >= 1).
count:
        ldx val
        lda val+1
        cpx #0
        beq 1f
        clc
        adc #1
1:      sta cnt_hi
        rts

; dst's low byte wrapped: the next page, round the ring, and at an 8 KB
; part's end z_send. Keeps X; Y 0.
dst_page:
        inc dst+1
        lda dst+1
        cmp #0xc0
        bne 1f
        lda #0x40
        sta dst+1
1:      and #0x1f
        bne 2f
        stx save_x
        lda dst+1                    ; w = dst - $4000, a part's start
        sec
        sbc #0x40
        sta w+1
        lda #0
        sta w
        jsr z_send
        ldx save_x
        ldy #0
        cmp #0
        beq fail
2:      rts

; bitbuf ran out: the next byte's top bit into C, the marker below.
refill:
        jsr get_byte
        sec
        rol a
        sta bitbuf
        rts

; Elias gamma into val: 1, then while a 0 bit, another bit in.
gamma:
        lda #1
        sta val
        lda #0
        sta val+1
1:      getbit
        bcs 2f
        getbit
        rol val
        rol val+1
        jmp 1b
2:      rts

; The next stream byte into A. Keeps X; Y 0.
get_byte:
        ldy pos
        cpy len
        beq 1f
        lda buf,y
        inc pos
        ldy #0
        rts
1:      stx save_x
        jsr fd_sector
        ldx save_x
        cmp #0
        bne get_byte
fail:
        ldx unz_sp
        txs
        lda #0
        rts
