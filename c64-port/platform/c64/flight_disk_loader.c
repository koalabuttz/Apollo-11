/* SPDX-License-Identifier: MIT */
/* The flight disk loader (docs/RUN-ON-C64.md, tools/flight_disk.py): the
 * first file on a mission's disk 1. It checks for a 2 MB REU, clears it,
 * and reads every disk of the set into it with the 1581 fast loader
 * (flight_disk_drive.s in the drive, fd_recv here), then fetches the
 * flight resident from the REU over itself and starts it (fd_tramp). The
 * resident's boot verifies the store (apollo_reu_store_verify).
 *
 * Each disk starts with an id sector at track 1, sector 0 (a one-sector
 * file): "APOLLO11", the mission letter, the disk's number, the set's disk
 * count and the track and sector of the disk's data file. The data file is
 * records: 'Z', a 2-byte count of REU ranges, each range's 3-byte REU
 * address and 3-byte length, then the ranges' bytes, one after the other,
 * as one fdz stream (tools/fdz.c: LZ77 with offsets up to 24 KB, decoded
 * here through a 32 KB ring at $4000-$BFFF and sent to the REU 8 KB at a
 * time); 'R', the REU address of the resident (already in a range), its
 * 2-byte length and its 2-byte start address (the last disk only); 'E',
 * the disk's end. Numbers are low byte first.
 *
 * The fast loader needs a 1581 (it runs 6502 code in the drive, timed to
 * the 1581's 2 MHz). For any other drive on device 8 (an SD2IEC, an
 * emulated drive that does not run drive code) the loader reads the same
 * files by name with the KERNAL instead: "LM DISK n" (the id sector) and
 * "LM DATA n". That is about 12 times slower, but boots.
 *
 * $03F0-$03FF is a mailbox for tools/flight_disk.py's VICE run: the state
 * (1 loading, 2 waiting for the disk in $03F1, 3 handed off to the
 * resident, $FF stopped with the reason in $03F1), the cycles spent reading
 * disks at $03F2 (CIA 2's timers, stopped while the loader waits for a key)
 * the cycles spent clearing the REU at $03F6, and at $03FA 1 if it used
 * the fast loader, 0 the KERNAL. */
#include <cbm.h>
#include <stdint.h>
#include <string.h>
#include "flight_disk_gen.h"

void fd_recv(uint8_t *dst, uint8_t n);
extern const uint8_t fd_tramp[], fd_tramp_end[];

#define REU        ((volatile uint8_t *)0xDF00)
#define CIA        ((volatile uint8_t *)0xDD00)
#define BOX        ((volatile uint8_t *)0x03F0)
#define TRAMP      ((uint8_t *)0x0340)
#define POKE(a, v) (*(volatile uint8_t *)(a) = (v))
#define PEEK(a)    (*(volatile uint8_t *)(a))

enum { BOX_LOADING = 1, BOX_DISK = 2, BOX_STARTED = 3, BOX_STOPPED = 0xff };
enum { STOP_NO_REU = 1, STOP_BAD_RECORD = 2, STOP_NO_START = 3 };

/* Shared with flight_disk_unz.s. */
uint8_t buf[256];
uint8_t pos, len;
static uint8_t err;
static uint8_t zero;
static uint8_t fast; /* a 1581: the fast loader */
static uint8_t eof;  /* the KERNAL path: the file's last byte is read */

static void say(const char *s)
{
    while (*s) cbm_k_chrout((uint8_t)*s++);
}

static void say_u(uint32_t v)
{
    char d[11];
    uint8_t i = sizeof d - 1;
    d[i] = 0;
    do {
        d[--i] = (char)('0' + v % 10u);
        v /= 10u;
    } while (v);
    say(d + i);
}

static void stop(uint8_t why, const char *s)
{
    BOX[1] = why;
    BOX[0] = BOX_STOPPED;
    say(s);
    for (;;) {
    }
}

/* Set up an REU transfer of n bytes (0 is 65536) between c64 and reu. */
static void dma_setup(const void *c64, uint32_t reu, uint16_t n, uint8_t fix_c64)
{
    REU[2] = (uint8_t)(uintptr_t)c64;
    REU[3] = (uint8_t)((uintptr_t)c64 >> 8);
    REU[4] = (uint8_t)reu;
    REU[5] = (uint8_t)(reu >> 8);
    REU[6] = (uint8_t)(reu >> 16);
    REU[7] = (uint8_t)n;
    REU[8] = (uint8_t)(n >> 8);
    REU[0xA] = fix_c64 ? 0x80 : 0;
}

/* Run one: cmd is TO_REU or FROM_REU. */
static void dma(uint8_t cmd, const void *c64, uint32_t reu, uint16_t n, uint8_t fix_c64)
{
    dma_setup(c64, reu, n, fix_c64);
    REU[1] = cmd;
}
#define TO_REU   0x90
#define FROM_REU 0x91

/* A 2 MB REU: each 64 KB bank's first byte holds its own number after
 * writing them from the top down (a smaller REU wraps, and a later write
 * replaces an earlier one). */
static uint8_t reu_2mb(void)
{
    uint8_t b, v;
    REU[0] = REU[0];                                                  /* clear the status */
    if (REU[2] == 0xff && REU[3] == 0xff && REU[4] == 0xff) return 0; /* no REU: open bus */
    for (b = 32; b--;) {
        v = b ^ 0xa5;
        dma(TO_REU, &v, (uint32_t)b << 16, 1, 0);
    }
    for (b = 0; b < 32; ++b) {
        dma(FROM_REU, &v, (uint32_t)b << 16, 1, 0);
        if (v != (b ^ 0xa5)) return 0;
    }
    return 1;
}

static void timers_start(void)
{
    CIA[0x0e] = 0;
    CIA[0x0f] = 0;
    CIA[4] = CIA[5] = CIA[6] = CIA[7] = 0xff;
    CIA[0x0f] = 0x51; /* timer B counts timer A's underflows */
    CIA[0x0e] = 0x11;
}

static uint32_t timers_stop(void)
{
    uint32_t b;
    CIA[0x0e] = 0;
    CIA[0x0f] = 0;
    b = (uint32_t)CIA[6] | (uint32_t)CIA[7] << 8;
    return 0xffffffffu - (b << 16 | (uint32_t)CIA[4] | (uint32_t)CIA[5] << 8);
}

static void add32(uint8_t at, uint32_t v)
{
    uint32_t t;
    memcpy(&t, (const void *)(0x03F0 + at), 4);
    t += v;
    memcpy((void *)(0x03F0 + at), &t, 4);
}

static void command(const uint8_t *p, uint8_t n)
{
    cbm_k_listen(8);
    cbm_k_second(0x6f);
    while (n--) cbm_k_ciout(*p++);
    cbm_k_unlsn();
}

static void mw(uint16_t a, const uint8_t *d, uint8_t n)
{
    uint8_t c[38];
    c[0] = 'M';
    c[1] = '-';
    c[2] = 'W';
    c[3] = (uint8_t)a;
    c[4] = (uint8_t)(a >> 8);
    c[5] = n;
    memcpy(c + 6, d, n);
    command(c, (uint8_t)(6 + n));
}

/* Start the drive code on the chain from track t, sector s, and take the
 * bus: from here to fd_end only the fast loader talks on it. */
static void fd_start(uint8_t t, uint8_t s, uint8_t max)
{
    static const uint8_t me[5] = {'M', '-', 'E', (uint8_t)FD_DRIVE_START,
                                  (uint8_t)(FD_DRIVE_START >> 8)};
    uint8_t ts[3];
    uint16_t i;
    ts[0] = t;
    ts[1] = s;
    ts[2] = max;
    mw(FD_DRIVE_TRK, ts, 3);
    command(me, 5);
    __asm__ volatile("sei");
    POKE(0xD011, PEEK(0xD011) & 0xEF);
    CIA[0] = 0x03;
    for (i = 0; i < 20000; ++i) __asm__ volatile("nop");
    pos = len = 0;
    err = 0;
}

/* The drive's status on its command channel: 0 for "00, OK", else the
 * error number (a 1541 always answers on channel 15). */
static uint8_t drive_status(void)
{
    uint8_t a, b;
    cbm_k_talk(8);
    cbm_k_tksa(0x6f);
    a = cbm_k_acptr();
    b = cbm_k_acptr();
    while (!(cbm_k_readst() & 0xc2)) cbm_k_acptr(); /* to EOI, or a timeout */
    cbm_k_untlk();
    if (a < '0' || a > '9' || b < '0' || b > '9') return 0xfc;
    return (uint8_t)((a - '0') * 10 + (b - '0'));
}

/* The KERNAL path: open file "<mission> DISK d" (data 0) or "<mission>
 * DATA d" (data 1) for reading. */
static void kernal_start(uint8_t data, uint8_t d)
{
    static char name[] = FD_NAME " DISK 0";
    if (data)
        memcpy(name + sizeof FD_NAME, "DATA", 4);
    else
        memcpy(name + sizeof FD_NAME, "DISK", 4);
    name[sizeof name - 2] = (char)('0' + d);
    cbm_k_setnam(name);
    cbm_k_setlfs(2, 8, 2);
    cbm_k_open();
    pos = len = 0;
    err = drive_status();
    eof = err != 0;
    /* Only an open file is read: a 1541 that failed the open (no disk yet
     * after a swap, 74 DRIVE NOT READY) may never take the bus as talker
     * for it, and the KERNAL's TKSA waits for that with no timeout. */
    if (!err) cbm_k_chkin(2);
}

static void fd_end(void)
{
    if (!fast) {
        cbm_k_clrch();
        cbm_k_close(2);
        return;
    }
    CIA[0] = 0x07;
    POKE(0xD011, PEEK(0xD011) | 0x10);
    __asm__ volatile("cli");
}

/* Up to 254 bytes into buf with the KERNAL; 0 past the end or at an error
 * (err holds the KERNAL's status, or $FD for reading past the end). */
static uint8_t kernal_sector(void)
{
    uint8_t n = 0, b, st;
    if (eof) {
        if (!err) err = 0xfd;
        return 0;
    }
    do {
        b = cbm_k_chrin();
        st = cbm_k_readst();
        if (st & 0xbf) {
            err = st;
            return 0;
        }
        buf[n++] = b;
    } while (!st && n != 254);
    eof = st;
    pos = 0;
    len = n;
    return 1;
}

/* The next sector into buf; 0 at a read error (err holds the drive's code). */
uint8_t fd_sector(void);
uint8_t fd_sector(void)
{
    uint8_t n;
    if (!fast) return kernal_sector();
    fd_recv(&n, 1);
    if (n == 0xff) {
        fd_recv(&err, 1);
        return 0;
    }
    fd_recv(buf, n);
    pos = 0;
    len = n;
    return 1;
}

static uint8_t fd_byte(void)
{
    if (pos == len && !fd_sector()) return 0;
    return buf[pos++];
}

static uint32_t fd_word(uint8_t n)
{
    uint32_t v = 0;
    uint8_t i;
    for (i = 0; i < n; ++i) v |= (uint32_t)fd_byte() << (8 * i);
    return v;
}

/* About n/60 s on the KERNAL's jiffy clock (its IRQ runs on this path). */
static void wait_jiffies(uint8_t n)
{
    uint8_t t0 = PEEK(0xA2);
    while ((uint8_t)(PEEK(0xA2) - t0) < n) {
    }
}

static void wait_return(void)
{
    while (cbm_k_getin() != 13) {
    }
}

/* A 1581: its DOS ROM's version string, "...DOS V10 1581" with the last
 * character's top bit set, read from the drive with M-R. */
static uint8_t drive_1581(void)
{
    static const uint8_t mr[6] = {'M', '-', 'R', 0xe7, 0xa6, 4};
    static const uint8_t want[4] = {'1', '5', '8', '1' | 0x80};
    uint8_t i, ok;
    POKE(0x90, 0); /* the KERNAL's status: the LOAD left EOF in it */
    command(mr, 6);
    if (cbm_k_readst()) return 0; /* no drive 8 */
    cbm_k_talk(8);
    cbm_k_tksa(0x6f);
    ok = 1;
    for (i = 0; i < 4; ++i)
        if (cbm_k_acptr() != want[i]) ok = 0;
    cbm_k_untlk();
    return ok;
}

static void upload_drive_code(void)
{
    uint16_t i;
    for (i = 0; i < sizeof fd_drive_code; i += 32)
        mw((uint16_t)(FD_DRIVE_START + i), fd_drive_code + i,
           (uint8_t)(sizeof fd_drive_code - i < 32 ? sizeof fd_drive_code - i : 32));
}

/* The fdz decoder (tools/fdz.c has the format; flight_disk_unz.s runs
 * it). Output goes round the ring; each 8 KB part, once whole, goes to the REU over the ranges in
 * order, and the last part when the stream ends. A transfer stays in one
 * 64 KB bank: past a bank's end the REU's address does not carry into the
 * bank register reliably (VICE's 2 MB REU put the bytes after 0x100000
 * elsewhere). */
#define RING       ((uint8_t *)0x4000)
#define RING_MASK  0x7fffu
#define MAX_RANGES 512u
static uint8_t range_at[MAX_RANGES][3], range_len[MAX_RANGES][3];
static uint16_t ranges, range_i;
static uint32_t to_at, to_left; /* the current range: where, and bytes left */
uint16_t w;                     /* the ring's next byte to write */
static uint16_t sent;           /* the ring's next byte for the REU */
static uint32_t done;

static uint32_t get24(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16;
}

/* Send ring bytes sent..w (not wrapping: w is at a part's end or the
 * stream's) to the ranges. 0 if the stream holds more than they do. */
static uint8_t send(void)
{
    uint16_t n = (uint16_t)((w - sent) & RING_MASK), k;
    while (n) {
        while (!to_left) {
            if (range_i == ranges) return 0;
            to_at = get24(range_at[range_i]);
            to_left = get24(range_len[range_i]);
            ++range_i;
        }
        k = n;
        if (to_left < k) k = (uint16_t)to_left;
        if ((uint16_t)((uint16_t)to_at + k) < (uint16_t)to_at) k = (uint16_t)-(uint16_t)to_at;
        dma(TO_REU, RING + sent, to_at, k, 0);
        to_at += k;
        to_left -= k;
        sent = (uint16_t)((sent + k) & RING_MASK);
        n = (uint16_t)(n - k);
        done += k;
        /* The border steps through the colours every 16 KB. */
        POKE(0xD020, (uint8_t)(done >> 14));
    }
    return 1;
}

/* flight_disk_unz.s: decode a stream into the ring, z_send at each part. */
uint8_t fd_unz(void);
uint8_t z_send(void);
uint8_t z_send(void)
{
    return send();
}

/* A 'Z' record: its ranges, then its stream. */
static uint8_t unpack(void)
{
    uint16_t i, n;
    ranges = (uint16_t)fd_word(2);
    if (ranges > MAX_RANGES) return 0;
    for (i = 0; i < ranges; ++i) {
        for (n = 0; n < 3; ++n) range_at[i][n] = fd_byte();
        for (n = 0; n < 3; ++n) range_len[i][n] = fd_byte();
    }
    range_i = 0;
    to_left = 0;
    w = sent = 0;
    return fd_unz() && !err && send() && !to_left && range_i == ranges;
}

static uint32_t start_reu;
static uint16_t start_len, start_at;

/* Read disk d of the set. 0: the disk in the drive is another one (or
 * none); 1: read. */
static uint8_t read_disk(uint8_t d)
{
    uint8_t t, s, tag, ok, tries;
    if (fast) {
        upload_drive_code();
        fd_start(1, 0, 1);
    } else
        for (tries = 0;;) {
            kernal_start(0, d);
            /* 74 DRIVE NOT READY: a 1541 just after a swap has not yet
             * seen the new disk. Give it a second before asking again. */
            if (err != 74 || ++tries == 5) break;
            fd_end();
            wait_jiffies(60);
        }
    ok = fd_sector() && len >= 13 && !memcmp(buf, "APOLLO11", 8) && buf[8] == FD_MISSION &&
         buf[9] == d && buf[10] == FD_DISKS;
    /* The drive sent the one sector and let go of the bus (the KERNAL
     * path: the id file is one sector). */
    fd_end();
    if (!ok) return 0;
    t = buf[11];
    s = buf[12];
    say("READING DISK ");
    say_u(d);
    say("\r");
    timers_start();
    if (fast)
        fd_start(t, s, 0);
    else
        kernal_start(1, d);
    for (;;) {
        tag = fd_byte();
        if (err) break;
        if (tag == 'E') break;
        if (tag == 'R') {
            start_reu = fd_word(3);
            start_len = (uint16_t)fd_word(2);
            start_at = (uint16_t)fd_word(2);
            continue;
        }
        if (tag != 'Z' || !unpack()) {
            if (!err) err = 0xfe;
            break;
        }
    }
    /* 'E' is the data file's last byte: the drive has let go of the bus.
     * After a read error it has too. */
    fd_end();
    add32(2, timers_stop());
    if (err) {
        say("READ ERROR ");
        say_u(err);
        say(", PRESS RETURN TO READ THE DISK AGAIN\r");
        return 2;
    }
    return 1;
}

int main(void)
{
    uint8_t d, r;
    uint32_t c;
    memset((void *)0x03F0, 0, 16);
    BOX[0] = BOX_LOADING;
    POKE(0xD020, 0);
    POKE(0xD021, 0);
    say("\x93\x05" FD_TITLE "\r\r");
    if (!reu_2mb())
        stop(STOP_NO_REU,
             "THIS NEEDS A 2 MB REU (1764 OR 1750 WITH\rA 2 MB EXPANSION, OR AN EMULATED ONE).\r");
    say("CLEARING THE REU\r");
    timers_start();
    zero = 0;
    for (d = 0; d < 32; ++d) dma(TO_REU, &zero, (uint32_t)d << 16, 0, 1);
    c = timers_stop();
    memcpy((void *)0x03F6, &c, 4);
    fast = drive_1581();
    BOX[10] = fast;
    if (!fast) say("NO 1581 ON DEVICE 8: READING WITH THE KERNAL\r(SLOW)\r");
    for (d = 1; d <= FD_DISKS; ++d)
        for (;;) {
            r = read_disk(d);
            if (r == 1) break;
            if (r == 0) {
                say("INSERT DISK ");
                say_u(d);
                say(" OF ");
                say_u(FD_DISKS);
                say(" AND PRESS RETURN\r");
            }
            BOX[1] = d;
            BOX[0] = BOX_DISK;
            wait_return();
            BOX[0] = BOX_LOADING;
        }
    if (!start_len) stop(STOP_NO_START, "THE LAST DISK HAS NO RESIDENT\r");
    memcpy(&c, (const void *)0x03F2, 4);
    say("LOADED IN ");
    say_u(c / (PEEK(0x02A6) ? 985248u : 1022727u));
    say(" S. STARTING THE AGC\r");
    memcpy(TRAMP, fd_tramp, (size_t)(fd_tramp_end - fd_tramp));
    POKE(0x0335, (uint8_t)start_at);
    POKE(0x0336, (uint8_t)(start_at >> 8));
    POKE(0x0337, (uint8_t)start_reu);
    POKE(0x0338, (uint8_t)(start_reu >> 8));
    POKE(0x0339, (uint8_t)(start_reu >> 16));
    POKE(0x033A, (uint8_t)start_len);
    POKE(0x033B, (uint8_t)(start_len >> 8));
    BOX[0] = BOX_STARTED;
    __asm__ volatile("sei");
    /* The hand-off runs the fetch from $0340, outside the bytes it
     * overwrites. */
    dma_setup((const void *)0x0801, start_reu, start_len, 0);
    ((void (*)(void))0x0340)();
    return 0;
}
