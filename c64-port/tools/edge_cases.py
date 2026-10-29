# SPDX-License-Identifier: MIT
"""Flight-typical edge values for case generators (15-bit ones' complement).

A seeded fill almost never yields +-0 high words, equal times, zero or
full-scale vectors, and tests/interp_case.c clears 010-061 (TIME1, TIME2,
the CDUs) before the case's edits, so only edits put values there. Each
function returns (address, word) edits to append after a case's own; a
generator opts in (interp specs: 'edge_cases': True) so frozen vectors of
specs without the flag stay byte-identical.

  edits += edge_cases.edits(n, rng, operand_addresses)
"""
PZERO, MZERO = 0o00000, 0o77777
PMAX, MMAX = 0o37777, 0o40000
TIME2, TIME1 = 0o24, 0o25
CDUX, CDUY, CDUZ, OPTY, OPTX = 0o32, 0o33, 0o34, 0o35, 0o36
COUNTERS = (TIME2, TIME1, CDUX, CDUY, CDUZ, OPTY, OPTX)


def neg(w):
    return ~w & 0o77777


def small(rng):
    w = rng.randrange(1, 0o40)
    return w if rng.random() < 0.5 else neg(w)


def signed_zero_high(a, rng):
    """A double word +-0 high, small low (either sign)."""
    return [(a, rng.choice((PZERO, MZERO))), (a + 1, small(rng))]


def max_magnitude(a, rng, n=6):
    """n words of full scale, each sign."""
    return [(a + k, rng.choice((PMAX, MMAX))) for k in range(n)]


def zero_vector(a, rng, n=6):
    """A vector of +-0 words."""
    return [(a + k, rng.choice((PZERO, MZERO))) for k in range(n)]


def equal_time(a, b, rng):
    """The same double-precision time at a and b (b = TIME2 compares to the clock)."""
    hi, lo = rng.randrange(0o40000), rng.randrange(0o40000)
    return [(a, hi), (a + 1, lo), (b, hi), (b + 1, lo)]


def counters(rng):
    """Non-zero TIME2, TIME1, CDU and optics counters (TIME1 near its overflow)."""
    out = [(c, rng.randrange(1, 0o77777)) for c in COUNTERS]
    out[1] = (TIME1, rng.choice((0o37777, 0o37776, rng.randrange(1, 0o40000))))
    return out


FAMILIES = (signed_zero_high, max_magnitude, zero_vector,
            lambda a, rng: equal_time(a, TIME2, rng))


def edits(n, rng, addrs):
    """Case n's edge edits: non-zero counters, then family n % 4 at every
    address of addrs (erasable, 0-03777)."""
    fam = FAMILIES[n % len(FAMILIES)]
    out = counters(rng)
    for a in addrs:
        # (A family running past 03777, from an operand at the top of E7,
        # stops there rather than wrapping into A and L.)
        out += [(x, w) for x, w in fam(a, rng) if x <= 0o3777]
    return out
