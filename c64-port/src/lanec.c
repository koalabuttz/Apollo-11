/* SPDX-License-Identifier: MIT */
#include "lanec.h"

uint16_t lanec_fb(uint16_t bank)
{
    return (uint16_t)((bank >= 040u ? bank - 010u : bank) << 10);
}

uint16_t lanec_current_bank(const apollo_overlay_state *s, const lanec_io *io)
{
    uint16_t bank = (uint16_t)((s->words[LANEC_FB] >> 10) & 037u);
    return (uint16_t)(bank >= 030u && (io->superbank & 0100u) ? bank + 010u : bank);
}

uint8_t lanec_registers_valid(const apollo_overlay_state *s)
{
    const uint16_t *w = s->words;
    return (uint8_t)((w[LANEC_EB] & ~03400u) == 0 && (w[LANEC_FB] & ~076000u) == 0 &&
                     w[LANEC_BB] == (uint16_t)(w[LANEC_FB] | (w[LANEC_EB] >> 8)) &&
                     w[LANEC_Z] <= 07777u && w[7] == 0);
}

void lanec_tc(apollo_overlay_state *s, uint16_t target, uint16_t return_address)
{
    s->words[LANEC_Q] = return_address;
    s->words[LANEC_Z] = target;
}

uint16_t lanec_a16(uint16_t word)
{
    return (uint16_t)((word & 077777u) | ((word << 1) & 0100000u));
}

uint8_t lanec_ccs(apollo_overlay_state *s, uint16_t word)
{
    uint16_t m = (uint16_t)(word & 077777u);
    uint16_t mag = (m & 040000u) ? (uint16_t)(037777u & ~m) : m;
    s->words[LANEC_A] = mag > 1u ? (uint16_t)(mag - 1u) : 0u;
    if (m == 0) return 1;
    if (m == 077777u) return 3;
    return (m & 040000u) ? 2u : 0u;
}

uint8_t lanec_ccs_a(apollo_overlay_state *s)
{
    uint16_t a = s->words[LANEC_A], mag = (a & 0100000u) ? (uint16_t)(0177777u & ~a) : a;
    uint16_t corrected = (uint16_t)((a & 037777u) | ((a >> 1) & 040000u));
    s->words[LANEC_A] = mag > 1u ? (uint16_t)(mag - 1u) : 0u;
    if ((a & 0140000u) == 0040000u) return 0;
    if ((a & 0140000u) == 0100000u) return 2;
    if (corrected == 0) return 1;
    if (corrected == 077777u) return 3;
    return (corrected & 040000u) ? 2u : 0u;
}

void lanec_mask(apollo_overlay_state *s, uint16_t word)
{
    uint16_t a = s->words[LANEC_A];
    uint16_t corrected = (uint16_t)((a & 037777u) | ((a >> 1) & 040000u));
    s->words[LANEC_A] = lanec_a16((uint16_t)(corrected & word));
}

uint8_t lanec_ts(apollo_overlay_state *s, uint16_t k)
{
    uint16_t a = s->words[LANEC_A];
    s->words[k] = (uint16_t)((a & 037777u) | ((a >> 1) & 040000u));
    if ((a & 0140000u) == 0040000u) { s->words[LANEC_A] = 1u; return 1; }
    if ((a & 0140000u) == 0100000u) { s->words[LANEC_A] = 0177776u; return 1; }
    return 0;
}

void lanec_ad(apollo_overlay_state *s, uint16_t word)
{
    uint32_t sum = (uint32_t)s->words[LANEC_A] + lanec_a16(word);
    if (sum & 0200000u) sum = (sum + 1u) & 0177777u;
    s->words[LANEC_A] = (uint16_t)sum;
}

void lanec_rewrite(apollo_overlay_state *s, uint16_t k)
{
    uint16_t v = s->words[k];
    switch (k) {
    case 020u: v = (uint16_t)((v >> 1) | ((v & 1u) << 14)); break;            /* CYR */
    case 021u: v = (uint16_t)((v >> 1) | (v & 040000u)); break;               /* SR */
    case 022u: v = (uint16_t)(((v << 1) | (v >> 14)) & 077777u); break;       /* CYL */
    case 023u: v = (uint16_t)((v >> 7) & 0177u); break;                       /* EDOP */
    default: return;
    }
    s->words[k] = v;
}

uint16_t lanec_erasable(const apollo_overlay_state *s, uint16_t address)
{
    if (address < 01400u) return address;
    return (uint16_t)(((s->words[LANEC_EB] >> 8) & 7u) * 0400u + (address & 0377u));
}

void lanec_write(apollo_overlay_state *s, lanec_io *io, uint16_t channel)
{
    uint16_t a = s->words[LANEC_A];
    uint16_t v = (uint16_t)((a & 037777u) | ((a >> 1) & 040000u));
    if (channel == 1u || channel == 2u) { s->words[channel] = a; return; }
    if (channel == 013u) v &= 043777u;
    if (channel == 033u) v = (uint16_t)(io->channel[033] | 076000u);
    if (channel == 077u) v = 0;
    io->channel[channel] = v;
}
