/* SPDX-License-Identifier: MIT */
#include "lanec_case.h"

uint32_t lanec_rand(uint32_t *state)
{
    uint32_t x = *state ? *state : 0x9e3779b9u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return *state = x;
}

void lanec_fill(apollo_overlay_state *s, lanec_io *io, uint32_t seed)
{
    uint32_t r = seed * 2654435761u + 0x6a09e667u;
    uint16_t i, eb;
    for (i = 0; i < APOLLO_SHARED_WORDS; ++i)
        s->words[i] = (uint16_t)(lanec_rand(&r) & (i <= LANEC_Q ? 0177777u : 077777u));
    /* L reads back sign-extended in Virtual AGC; keep it in that form. */
    s->words[LANEC_L] = (uint16_t)((s->words[LANEC_L] & 077777u) | ((s->words[LANEC_L] << 1) & 0100000u));
    /* OPTY/OPTX and the GYROCMD..OPTXCMD drive cells are serviced by Virtual
     * AGC's simulated hardware between instructions; leave them idle. */
    s->words[035] = s->words[036] = 0;
    for (i = 047; i <= 054; ++i) s->words[i] = 0;
    eb = (uint16_t)((lanec_rand(&r) & 7u) << 8);
    s->words[LANEC_EB] = eb;
    s->words[7] = 0;
    for (i = 0; i < LANEC_CHANNELS; ++i) io->channel[i] = (uint16_t)(lanec_rand(&r) & 077777u);
    io->interrupt_enabled = (uint16_t)(lanec_rand(&r) & 1u);
    io->superbank = (uint16_t)(lanec_rand(&r) & 0160u);
}

void lanec_enter(apollo_overlay_state *s, lanec_io *io, uint16_t bank, uint16_t z)
{
    s->words[LANEC_FB] = lanec_fb(bank);
    if (bank >= 040u) io->superbank = (uint16_t)(io->superbank | 0100u);
    else if (bank >= 030u) io->superbank = (uint16_t)(io->superbank & ~0100u);
    s->words[LANEC_BB] = (uint16_t)(s->words[LANEC_FB] | (s->words[LANEC_EB] >> 8));
    s->words[LANEC_Z] = z;
}

static uint32_t crc_table[256];

static uint32_t crc_byte(uint32_t crc, uint8_t b)
{
    uint16_t k;
    if (!crc_table[1])
        for (k = 0; k < 256u; ++k) {
            uint32_t c = k;
            uint8_t j;
            for (j = 0; j < 8u; ++j) c = (c & 1u) ? 0xedb88320u ^ (c >> 1) : c >> 1;
            crc_table[k] = c;
        }
    return crc_table[(uint8_t)(crc ^ b)] ^ (crc >> 8);
}

uint32_t lanec_hash(const apollo_overlay_state *s, const lanec_io *io, enum lanec_status status)
{
    uint32_t crc = 0xffffffffu;
    uint16_t i, v;
    for (i = 0; i < APOLLO_SHARED_WORDS + LANEC_CHANNELS + 3u; ++i) {
        if (i < APOLLO_SHARED_WORDS) v = s->words[i];
        else if (i < APOLLO_SHARED_WORDS + LANEC_CHANNELS) v = io->channel[i - APOLLO_SHARED_WORDS];
        else if (i == APOLLO_SHARED_WORDS + LANEC_CHANNELS) v = io->superbank;
        else if (i == APOLLO_SHARED_WORDS + LANEC_CHANNELS + 1u) v = io->interrupt_enabled;
        else v = (uint16_t)status;
        crc = crc_byte(crc_byte(crc, (uint8_t)v), (uint8_t)(v >> 8));
    }
    return crc ^ 0xffffffffu;
}
