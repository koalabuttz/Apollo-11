; SPDX-License-Identifier: MIT
; The C64 side of the flight disk loader (flight_disk_loader.c; the drive
; side is flight_disk_drive.s).
;
; fd_recv(dst, n): receive n bytes (1 to 255; 0 is 256) into dst. For each
; byte it waits for the drive to release DATA, pulls CLK low for 6 cycles
; and reads the four pairs at cycles 22, 38, 54 and 70 after the strobe
; (the drive puts them out 12, 28, 44 and 60 us after it sees CLK low; a
; PAL or NTSC cycle is about 1 us). Each read takes DATA (bit 7) and CLK
; (bit 6); four reads with two shifts between them make the byte, low pair
; first, with the lines' 1 for released. Needs SEI, the screen blanked (no
; badlines) and $DD00 at $03 (VIC bank 0, ATN, CLK and DATA released).
        .text
        .globl fd_recv
fd_recv:
        tax
        ldy #0
1:      bit $dd00
        bpl 1b
        lda #$13
        sta $dd00          ; CLK low: the strobe (cycle 0)
        lda #$03
        sta $dd00          ; released at 6
        nop
        nop
        nop
        nop
        nop
        nop
        lda $dd00          ; pair 1 at 22
        lsr
        lsr
        nop
        nop
        nop
        nop
        eor $dd00          ; pair 2 at 38
        lsr
        lsr
        nop
        nop
        nop
        nop
        eor $dd00          ; pair 3 at 54
        lsr
        lsr
        nop
        nop
        nop
        nop
        eor $dd00          ; pair 4 at 70
        eor #$03           ; $DD00's bits 0-1 come through the last read
        sta (__rc2),y
        iny
        dex
        bne 1b
        rts

; The hand-off, copied to FD_TRAMP ($0340, the tape buffer) and entered
; with JMP once the store is in the REU. The REU registers already hold the
; resident's fetch (REU to $0801). It runs the fetch, overwriting the
; loader, then zeroes the REU bytes the resident came from (from the zero
; at $0334, C64 address fixed), puts the screen and $DD00 back and jumps
; to the resident's SYS address. Position independent: everything it
; reads is at a fixed address the loader fills in.
;   $0334      0
;   $0335-36   the resident's start (the SYS address)
;   $0337-39   the REU address the resident was staged at
;   $033a-3b   its length
        .section .rodata.fd_tramp,"a",@progbits
        .globl fd_tramp, fd_tramp_end
fd_tramp:
        lda #$91
        sta $df01          ; execute: REU to C64
        lda #$00
        sta $0334
        lda #$34
        sta $df02
        lda #$03
        sta $df03
        lda $0337
        sta $df04
        lda $0338
        sta $df05
        lda $0339
        sta $df06
        lda $033a
        sta $df07
        lda $033b
        sta $df08
        lda #$80
        sta $df0a          ; C64 address fixed
        lda #$90
        sta $df01          ; execute: C64 to REU
        lda #$00
        sta $df0a
        lda #$07
        sta $dd00          ; the KERNAL's: VIC bank 0, serial lines released
        lda #$1b
        sta $d011          ; screen on
        lda #$0e
        sta $d020          ; the default border
        cli
        jmp ($0335)
fd_tramp_end:
