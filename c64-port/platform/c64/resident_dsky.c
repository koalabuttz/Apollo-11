/* SPDX-License-Identifier: MIT */
/* The flight resident's DSKY (docs/RUN-ON-C64.md): the display on the text
 * screen and the C64 keyboard as the DSKY's keys, for the flight program
 * booted from disk (resident_flight_main.c). It costs the resident no
 * bytes. Its code runs from $0200-$03FF, its tables and state sit in the
 * screen's last rows ($06A8-$07E7, drawn in the background colour, so
 * they do not show) and its scratch in zero page $E0-$F0: nothing else
 * uses them in flight. They load with
 * the boot-only code, and rk_dsky_install (boot-only too) copies them.
 *
 * Display. The relays latch what T4RUPT last sent on channel 10, and
 * T4RUPT sends exactly the display table: DSPTAB+0 to +10 for relay words
 * 1 to 11 (a negative entry is waiting to go out, a positive one has gone
 * out, its low 11 bits the relay bits) and DSPTAB+11 for row 12's lamps
 * (src/t4rupt_core.c, dsplay and CDRVE). So the positive entries are the
 * relays' state, and the DSKY reads them instead of catching each channel
 * 10 write. The other lamps are channel 11's (COMP ACTY, UPLINK ACTY,
 * TEMP, KEY REL, OPR ERR, and the verb and noun flash, which the DSKY's
 * own clock times) and the RESTART flip-flop (rk_restart_lamp). It redraws
 * only what changed, from the AGC's idle loop (rk_dsky_idle, from the idle
 * memo in resident_flight.c), at most 20 times a second. The run-time part
 * is assembly, resident_dsky.S, to fit; this file puts it in place.
 *
 * Keys. The CIA1 interrupt (rk_dsky_irq) scans the
 * keyboard 25 times a second and queues each change of key. The idle loop
 * takes them one at a time: a DSKY key goes to channel 15 with KEYRUPT1
 * (on the CM to rk_device's copy too, which the CM modules write back over
 * channel 15), and PRO clears channel 32's bit 14, for 255 idle passes
 * (more than two T4RUPT periods). */
#include "resident_flight.h"
#include <string.h>

#define SCREEN ((volatile uint8_t *)0x0400)
#define COLOUR ((volatile uint8_t *)0xd800)
#define DARK   11u /* dark grey: the lamps when out, and the text */
#define GREEN  13u /* the digits */

void rk_dsky_irq(void);
extern uint8_t __rk_dsky_start[], __rk_dsky_end[], __rk_dsky_load[];
extern uint8_t __rk_dsky_data_start[], __rk_dsky_data_end[], __rk_dsky_data_load[];
extern uint8_t __rk_dsky_bss_start[], __rk_dsky_bss_end[];
/* resident_dsky.S: where relay words 1-11's digits and R1-R3's signs go in
 * the digits' page ($0500), and the digits as drawn. */
extern const uint8_t rk_dsky_at_left[11], rk_dsky_at_right[11], rk_dsky_signs[9];
extern uint8_t rk_dsky_left[11];

/* The panel's fixed text, drawn once: the screen offset and the text, a 0
 * ending each and an offset of 0 the list. */
/* clang-format off */
static const uint8_t panel[] __attribute__((section(".boot.text.panel"))) =
#if RK_LM
    "\x01\x00" "APOLLO 11 LM DSKY\0"
#else
    "\x01\x00" "APOLLO 11 CM DSKY\0"
#endif
    "\x51\x00" "UPLINK ACTY\0"
    "\x5e\x00" "TEMP\0"
    "\x6b\x00" "COMP ACTY\0"
    "\x79\x00" "NO ATT\0"
    "\x86\x00" "GIMBAL LOCK\0"
    "\x93\x00" "PROG\0"
    "\xa1\x00" "KEY REL\0"
    "\xae\x00" "RESTART\0"
    "\xbb\x00" "TRACKER\0"
    "\xc9\x00" "OPR ERR\0"
    "\xd6\x00" "ALT\0"
    "\xe3\x00" "VEL\0"
    "\x20\x01" "VERB    NOUN    PROG\0"
    "\x9c\x01" "R1\0"
    "\xc4\x01" "R2\0"
    "\xec\x01" "R3\0"
    "\x31\x02" "V VERB  N NOUN  RETURN ENTR  C CLR\0"
    "\x59\x02" "R RSET  K KEY REL  P PRO  0-9 + -\0"
    "\0";
/* clang-format on */

/* Boot-only: copy the DSKY to $0200 and the screen's last rows, clear its
 * state, draw the panel and take the CIA1 interrupt (rk_dsky_irq goes on to
 * ov2_irq_entry). From the end of rk_boot, before the first module load overwrites
 * the boot-only code. */
__attribute__((noinline, section(".boot.text"))) void rk_dsky_install(void)
{
    const uint8_t *t = panel;
    uint16_t at, i;
    for (i = 0; i < 1000; ++i) {
        SCREEN[i] = ' ';
        COLOUR[i] = i < 0x2a8u ? DARK : 0; /* the last rows hold the tables */
    }
    memcpy(__rk_dsky_start, __rk_dsky_load, (size_t)(__rk_dsky_end - __rk_dsky_start));
    memcpy(__rk_dsky_data_start, __rk_dsky_data_load,
           (size_t)(__rk_dsky_data_end - __rk_dsky_data_start));
    memset(__rk_dsky_bss_start, 0, (size_t)(__rk_dsky_bss_end - __rk_dsky_bss_start));
    memset(rk_dsky_left, 0xff, sizeof rk_dsky_left);
    memset((void *)0xe0, 0, 0x11); /* resident_dsky.S's zero page */
    *(volatile uint8_t *)0xd020 = 0;
    *(volatile uint8_t *)0xd021 = 0;
    *(volatile uint8_t *)0xd018 = 0x15; /* the upper case characters */
    while ((at = (uint16_t)(t[0] | (uint16_t)t[1] << 8)) != 0) {
        for (t += 2; *t; ++t, ++at) SCREEN[at] = (uint8_t)(*t & 077u);
        ++t;
    }
    for (i = 0; i < 11; ++i) {
        COLOUR[0x100u + rk_dsky_at_left[i]] = GREEN;
        COLOUR[0x100u + rk_dsky_at_right[i]] = GREEN;
    }
    for (i = 0; i < 9; i += 3) COLOUR[0x100u + rk_dsky_signs[i]] = GREEN;
    COLOUR[0x100u] = 0; /* relay word 8's left digit, which the DSKY has not */
    __asm__ volatile("sei");
    *(volatile uint16_t *)0xfffe = (uint16_t)(uintptr_t)rk_dsky_irq;
    __asm__ volatile("cli");
}
