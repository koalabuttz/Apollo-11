/* SPDX-License-Identifier: MIT */
/* A live DSKY in the terminal for the native x86_64 flight program
 * (docs/X86-FLIGHT.md, X86_DSKY=1).
 *
 * Display: the relay words the flight program writes to channel 10 and the
 * lamps of channel 11 go through the port's own DSKY device adapter
 * (src/dsky_display.c, the one the C64 build draws with), rendered into a
 * 40x25 character buffer and drawn with ANSI escapes whenever it changes.
 * The flight program writes one relay word per T4RUPT, so reading channel
 * 10 after every routine sees each of them.
 *
 * Keys: typed keys are DSKY keys, sent as channel 15 codes with a KEYRUPT1
 * request at the next idle ADVAN sample point, as scenario keys are:
 *   0-9  digits      v VERB   n NOUN   e or Enter ENTER   + -
 *   c CLR            r RSET   k KEY REL
 *   p PRO (channel 32 bit 14 held low for half a second)
 *   m / j  the CM's MARK / MARK REJECT (the LM has no such keys)
 *   q or Ctrl-C  quit
 * Keys are read without waiting, from a terminal or a pipe, so the AGC clock
 * never stops for input. The terminal goes back to its own mode on exit.
 *
 * Lamps: the adapter draws the channel 11 lamps and the relay row 12
 * annunciators. Channel 13 bit 10 (TEST DSKY, the V35E lamp test) also
 * lights RESTART and STBY here, and the LM's PRIO DISP and NO DAP (row 12
 * bits 1 and 2), which the adapter's 40x25 screen has no room for, are drawn
 * on the status line below it. */
#ifndef TERMINAL_DSKY_H
#define TERMINAL_DSKY_H
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include "dsky_display.h"
#include "flight_scenario.h"

#ifdef RK_LM
#define TD_MISSION "LM"
#else
#define TD_MISSION "CM"
#endif

/* Typed keys go in at least TD_KEY_CS apart (scenario keys: 50 cs). */
enum { TD_QUEUE = 256, TD_PRO_CS = 50, TD_KEY_CS = 20 };

typedef struct {
    dsky_display_state display;
    agc_word channel10, channel11, channel13;
    char queue[TD_QUEUE];
    unsigned head, tail;
    unsigned long pro_until, drawn_cs, key_cs;
    uint8_t screen[1000], color[1000], shown[1000], shown_color[1000];
    int on, quit;
    int eof;         /* stdin ended (a pipe that sent everything) */
    int quit_at_eof; /* a run with no CS argument ends once a pipe is done */
} terminal_dsky;

static struct termios td_saved;
static int td_raw, td_screen;
static volatile sig_atomic_t td_interrupted;

/* The terminal as it was: called at the end of the run and at exit. */
static inline void td_restore(void)
{
    if (td_raw) tcsetattr(STDIN_FILENO, TCSANOW, &td_saved);
    td_raw = 0;
    if (!td_screen) return;
    td_screen = 0;
    fputs("\033[0m\033[?25h\033[?1049l", stdout);
    fflush(stdout);
}

static inline void td_on_signal(int sig)
{
    (void)sig;
    td_interrupted = 1;
}

/* The DSKY with an empty key queue and no terminal (the lockstep check). */
static inline void td_init(terminal_dsky *d)
{
    memset(d, 0, sizeof *d);
    dsky_display_reset(&d->display);
    d->on = 1;
    memset(d->shown, 0xff, sizeof d->shown);
}

/* Raw keys and the alternate screen. When stdin is a terminal it goes into
 * raw mode; a pipe is read as it is. Either way td_read_keys never waits. */
static inline void td_start(terminal_dsky *d)
{
    struct termios raw;
    td_init(d);
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &td_saved) == 0) {
        raw = td_saved;
        raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        td_raw = tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
    }
    td_screen = 1;
    atexit(td_restore);
    signal(SIGINT, td_on_signal);
    signal(SIGTERM, td_on_signal);
    fputs("\033[?1049h\033[?25l\033[2J", stdout);
    fflush(stdout);
}

/* A typed character as a scenario key letter (fs_keycode, fs_markcode),
 * 'P' for PRO, or 0. */
static inline char td_key(int c)
{
    if (c >= '0' && c <= '9') return (char)c;
    switch (c) {
    case 'v':
    case 'V': return 'V';
    case 'n':
    case 'N': return 'N';
    case 'e':
    case 'E':
    case '\r':
    case '\n': return 'E';
    case '+':
    case '=': return '+';
    case '-':
    case '_': return '-';
    case 'c':
    case 'C': return 'C';
    case 'r':
    case 'R': return 'R';
    case 'k':
    case 'K': return 'K';
    case 'p':
    case 'P': return 'P';
#ifdef RK_CM
    case 'm': return 'M';
    case 'j': return 'J';
#endif
    default: return 0;
    }
}

/* One typed character: queued as a key, or q / Ctrl-C to quit. */
static inline void td_type(terminal_dsky *d, int c)
{
    char k;
    if (c == 'q' || c == 'Q' || c == 3) {
        d->quit = 1;
        return;
    }
    if ((k = td_key(c)) != 0 && (d->tail + 1u) % TD_QUEUE != d->head) {
        d->queue[d->tail] = k;
        d->tail = (d->tail + 1u) % TD_QUEUE;
    }
}

/* The characters waiting on stdin, without blocking (poll with no wait). */
static inline void td_read_keys(terminal_dsky *d)
{
    struct pollfd in;
    unsigned char c;
    if (td_interrupted) d->quit = 1;
    in.fd = STDIN_FILENO;
    in.events = POLLIN;
    while (!d->quit && !d->eof) {
        in.revents = 0;
        if (poll(&in, 1, 0) != 1 || !(in.revents & (POLLIN | POLLHUP))) return;
        if (read(STDIN_FILENO, &c, 1) != 1) {
            d->eof = 1;
            return;
        }
        td_type(d, c);
    }
}

/* At an idle ADVAN sample point: release PRO when due, then the next typed
 * key (1 when one went in, as fs_inputs_due's FS_KEY). */
static inline int td_inputs_due(terminal_dsky *d, fd_state *st)
{
    unsigned long t = fd_centiseconds(st);
    char k;
    agc_word mark;
    if (d->pro_until && t >= d->pro_until) {
        st->hw.channel32 |= 020000u;
        d->pro_until = 0;
    }
    if (d->head == d->tail) {
        if (d->eof && d->quit_at_eof && !d->pro_until) d->quit = 1;
        return 0;
    }
    if (d->key_cs && t < d->key_cs + TD_KEY_CS) return 0;
    d->key_cs = t;
    k = d->queue[d->head];
    d->head = (d->head + 1u) % TD_QUEUE;
    if (k == 'P') {
        st->hw.channel32 &= (agc_word)~020000u;
        d->pro_until = t + TD_PRO_CS;
        return 0;
    }
    if ((mark = fs_markcode(k)) != 0) {
        st->io.channel16 = mark;
        st->pending |= RK_RUPT_BIT(RK_RUPT_KEY2);
    } else {
        st->hw.channel15 = fs_keycode(k);
        st->pending |= RK_RUPT_BIT(RK_RUPT_KEY1);
    }
    return 1;
}

/* The adapter's RESTART and STBY lamp bits, lit by TEST DSKY (channel 13
 * bit 10) as the real DSKY's lamp test lights them. */
#define TD_TEST_DSKY    01000u
#define TD_RESTART_STBY 0600u

/* After each routine: a new relay word, lamp word or lamp test. */
static inline void td_after_routine(terminal_dsky *d, const fd_state *st)
{
    if (st->hw.channel10 != d->channel10) {
        d->channel10 = st->hw.channel10;
        dsky_display_output(&d->display, 010, d->channel10);
    }
    if (st->hw.channel11 != d->channel11 || st->hw.channel13 != d->channel13) {
        d->channel11 = st->hw.channel11;
        d->channel13 = st->hw.channel13;
        dsky_display_output(&d->display, 011,
                            (uint16_t)(d->channel11 & ~TD_RESTART_STBY) |
                                (d->channel13 & TD_TEST_DSKY ? TD_RESTART_STBY : 0u));
    }
}

/* A C64 screen code as ASCII: letters 1-26, the rest as they are. */
static inline char td_ascii(uint8_t code)
{
    if (code >= 1u && code <= 26u) return (char)('A' + code - 1u);
    return code >= 32u && code < 64u ? (char)code : ' ';
}

/* Redraw the rows that changed, at most every 5 cs of AGC time. Lamps lit
 * (adapter colour 7) show in bold yellow; everything else in green. */
static inline void td_draw(terminal_dsky *d, const fd_state *st)
{
    unsigned long t = fd_centiseconds(st);
    unsigned row, col;
    if (t < d->drawn_cs + 5u && d->drawn_cs) return;
    dsky_display_advance(&d->display, (uint16_t)(t - d->drawn_cs));
    d->drawn_cs = t;
    dsky_display_render(&d->display, d->screen, d->color, dsky_display_flash_visible(&d->display));
    for (row = 0; row < 25u; ++row) {
        const unsigned p = row * 40u;
        if (!memcmp(d->screen + p, d->shown + p, 40) &&
            !memcmp(d->color + p, d->shown_color + p, 40))
            continue;
        printf("\033[%u;1H", row + 1u);
        for (col = 0; col < 40u; ++col)
            printf("%s%c", d->color[p + col] == 7u ? "\033[1;33m" : "\033[0;32m",
                   td_ascii(d->screen[p + col]));
        fputs("\033[0m", stdout);
    }
    memcpy(d->shown, d->screen, sizeof d->shown);
    memcpy(d->shown_color, d->color, sizeof d->shown_color);
    printf("\033[26;1H\033[0m %s  %lu.%02lu s  major mode %02o", TD_MISSION, t / 100u, t % 100u,
           (unsigned)st->s.words[01011]);
#ifdef RK_LM
    /* Row 12 bits 1 and 2, kept by the adapter while row 12 is valid. */
    {
        const unsigned row12 = d->display.valid & (1u << 11) ? d->display.relay[11] : 0u;
        printf("  %sPRIO DISP\033[0m  %sNO DAP\033[0m", row12 & 1u ? "\033[1;33m" : "\033[2m",
               row12 & 2u ? "\033[1;33m" : "\033[2m");
    }
#endif
    fputs("   (q quits)\033[K", stdout);
    fflush(stdout);
}
#endif
