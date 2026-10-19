/* SPDX-License-Identifier: MIT */
/* fdz: the flight disks' compression (tools/flight_disk.py,
 * platform/c64/flight_disk_loader.c). An LZ77 bit stream in the style of
 * Einar Saukas's ZX0 (Elias gamma numbers, a repeat of the last offset
 * after literals), written for this loader, with offsets up to FDZ_WINDOW
 * (24 KB: the loader's 32 KB ring less the 8 KB part it fills).
 *
 *   fdz c IN OUT     compress
 *   fdz d IN OUT     decompress (the loader's decoder, for checking)
 *
 * The stream: bits come MSB first from bytes taken when the last is used
 * up; a literal byte and an offset's low byte come straight from the
 * stream between them. gamma(n), n >= 1: n = 1; while (!bit()) n = n << 1
 * | bit(). Then, from the start (literals first):
 *
 *   literals:  gamma(count), count bytes; then bit 0: a repeat match,
 *              gamma(length), from the last offset; bit 1: a new match.
 *   new match: gamma(hi), a byte lo: the offset is (hi - 1) * 256 + lo + 1;
 *              hi = FDZ_END_HI ends the stream; gamma(length - 1).
 *   after any match: bit 0, literals; bit 1, a new match.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FDZ_WINDOW 24576
#define FDZ_END_HI (FDZ_WINDOW / 256 + 1)
#define HASH_BITS 16
#define CHAIN 512
#define MAX_LEN 65535

static uint8_t *out;
static size_t olen, bitpos;
static int bits_left;

static void put_byte(uint8_t b) { out[olen++] = b; }
static void put_bit(int b)
{
    if (!bits_left) {
        bitpos = olen;
        put_byte(0);
        bits_left = 8;
    }
    --bits_left;
    if (b) out[bitpos] |= (uint8_t)(1u << bits_left);
}
static void put_gamma(unsigned n)
{
    int k = 0;
    while ((n >> k) > 1) ++k;
    while (k--) {
        put_bit(0);
        put_bit((int)((n >> k) & 1u));
    }
    put_bit(1);
}
static int gamma_bits(unsigned n)
{
    int k = 0;
    while ((n >> k) > 1) ++k;
    return 2 * k + 1;
}

static const uint8_t *cin;
static size_t cn;
static int32_t *head, *prev;
#define HASH(p) ((((uint32_t)cin[p] << 8 ^ (uint32_t)cin[(p) + 1] << 4 ^ cin[(p) + 2]) * 2654435761u) >> (32 - HASH_BITS))

static void insert(size_t p)
{
    if (p + 3 <= cn && prev[p] == -2) {
        uint32_t h = HASH(p);
        prev[p] = head[h];
        head[h] = (int32_t)p;
    }
}

/* The best match at i, as bits saved over literals (9 a byte, roughly). A
 * repeat of last_off counts only right after literals (rep_ok). */
static long best_match(size_t i, size_t last_off, int rep_ok, size_t *len, size_t *off)
{
    long best_gain = 0;
    *len = *off = 0;
    if (rep_ok && i >= last_off) {
        size_t rep = 0;
        while (i + rep < cn && rep < MAX_LEN && cin[i + rep] == cin[i + rep - last_off]) ++rep;
        if (rep) {
            long gain = (long)rep * 9 - (1 + gamma_bits((unsigned)rep)) - 1;
            if (gain > 0) {
                best_gain = gain;
                *len = rep;
                *off = last_off;
            }
        }
    }
    if (i + 3 <= cn) {
        int32_t j = head[HASH(i)];
        int c = 0;
        while (j >= 0 && i - (size_t)j <= FDZ_WINDOW && c++ < CHAIN) {
            size_t l = 0, o = i - (size_t)j;
            while (i + l < cn && l < MAX_LEN && cin[j + l] == cin[i + l]) ++l;
            if (l >= 2) {
                long cost = 1 + gamma_bits((unsigned)((o - 1) / 256 + 1)) + 8 + gamma_bits((unsigned)(l - 1));
                long gain = (long)l * 9 - cost - 1;
                if (gain > best_gain) {
                    best_gain = gain;
                    *len = l;
                    *off = o;
                }
            }
            j = prev[j];
        }
    }
    return best_gain;
}

static void put_literals(size_t from, size_t to, int after_match)
{
    if (after_match) put_bit(0);
    put_gamma((unsigned)(to - from));
    for (size_t k = from; k < to; ++k) put_byte(cin[k]);
}

static size_t compress(const uint8_t *in, size_t n)
{
    size_t i = 0, lit_start = 0, last_off = 1;
    int after_match = 0;
    cin = in;
    cn = n;
    head = malloc(sizeof(int32_t) << HASH_BITS);
    prev = malloc(sizeof(int32_t) * (n + 1));
    memset(head, 0xff, sizeof(int32_t) << HASH_BITS);
    for (size_t k = 0; k <= n; ++k) prev[k] = -2;
    while (i < n) {
        size_t len, off, len2, off2;
        long gain = best_match(i, last_off, i > lit_start, &len, &off);
        if (gain > 0 && i + 1 < n) {
            /* Lazy: a literal now and a better match next. */
            insert(i);
            if (best_match(i + 1, last_off, 1, &len2, &off2) > gain + 9) gain = 0;
        }
        if (gain <= 0) {
            insert(i);
            ++i;
            continue;
        }
        if (i > lit_start) {
            put_literals(lit_start, i, after_match);
            if (off == last_off) {
                put_bit(0);
                put_gamma((unsigned)len);
                goto matched;
            }
            put_bit(1);
        } else if (after_match)
            put_bit(1);
        else
            abort(); /* a match at the start: there is no history */
        put_gamma((unsigned)((off - 1) / 256 + 1));
        put_byte((uint8_t)((off - 1) & 255));
        put_gamma((unsigned)(len - 1));
        last_off = off;
    matched:
        for (size_t k = 0; k < len; ++k) insert(i + k);
        i += len;
        lit_start = i;
        after_match = 1;
    }
    if (i > lit_start) {
        put_literals(lit_start, i, after_match);
        put_bit(1);
    } else if (after_match)
        put_bit(1);
    put_gamma(FDZ_END_HI);
    free(head);
    free(prev);
    return olen;
}

/* The decoder, as the loader runs it. */
static const uint8_t *src;
static uint8_t bitbuf, bitcnt;
static uint8_t get_byte(void) { return *src++; }
static uint8_t get_bit(void)
{
    if (!bitcnt) {
        bitbuf = get_byte();
        bitcnt = 8;
    }
    --bitcnt;
    return (uint8_t)((bitbuf >> bitcnt) & 1u);
}
static uint16_t get_gamma(void)
{
    uint16_t v = 1;
    while (!get_bit()) v = (uint16_t)(v << 1 | get_bit());
    return v;
}
static size_t decompress(const uint8_t *in, uint8_t *dst)
{
    size_t o = 0;
    uint16_t off = 1, n, hi;
    src = in;
    bitcnt = 0;
    for (;;) {
        n = get_gamma();
        while (n--) dst[o++] = get_byte();
        if (!get_bit()) {
            n = get_gamma();
            while (n--) { dst[o] = dst[o - off]; ++o; }
            if (!get_bit()) continue;
        }
        for (;;) {
            hi = get_gamma();
            if (hi == FDZ_END_HI) return o;
            off = (uint16_t)((hi - 1) * 256 + get_byte() + 1);
            n = (uint16_t)(get_gamma() + 1);
            while (n--) { dst[o] = dst[o - off]; ++o; }
            if (!get_bit()) break;
        }
    }
}

int main(int argc, char **argv)
{
    FILE *f;
    uint8_t *in;
    size_t n;
    if (argc != 4) return 2;
    f = fopen(argv[2], "rb");
    if (!f) return 1;
    fseek(f, 0, SEEK_END);
    n = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    in = malloc(n + 1);
    if (fread(in, 1, n, f) != n) return 1;
    fclose(f);
    out = malloc(argv[1][0] == 'c' ? n * 2 + 1024 : (size_t)64 << 20); /* d: up to 64 MB out */
    if (argv[1][0] == 'c')
        olen = compress(in, n);
    else
        olen = decompress(in, out);
    f = fopen(argv[3], "wb");
    fwrite(out, 1, olen, f);
    fclose(f);
    fprintf(stderr, "%zu -> %zu\n", n, olen);
    return 0;
}
