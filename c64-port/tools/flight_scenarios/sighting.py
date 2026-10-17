#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Sighting helpers for the P52 flight scenarios (lm_p52.py; CM P52 can
use them too): AGC word and CDU angle conversions, the rope listing's star
catalog, and the stable member to navigation base matrix of the gimbal
angles (inner Y, middle Z, outer X) and its inverse."""
import math
import re


def word(w):
    """AGC ones' complement word -> integer."""
    return w - 0o77777 if w & 0o40000 else w


def dp(hi, lo, scale=0):
    return (word(hi) * 2**14 + word(lo)) / 2**28 * 2**scale


def cdu_angle(w):
    """A CDU or AOT word (two's complement, 2^15 per revolution) in radians."""
    return (w - 0o100000 if w & 0o40000 else w) / 2**15 * 2 * math.pi


def cdu_word(a):
    return round(a / (2 * math.pi) * 2**15) % 2**15


def catalog(lst):
    """Star code -> unit vector in reference coordinates (CATLOG's 2DECs
    in the assembly listing lst)."""
    vec = {}
    for line in open(lst, errors='replace'):
        m = re.search(r'2DEC\s+([+-][.\d]+)\s+B-1\s+#\s+STAR\s+(\d+)', line)
        if m:
            vec.setdefault(int(m.group(2)), []).append(float(m.group(1)) * 2)
    return {k: v for k, v in vec.items() if len(v) == 3}


def rx(a):
    c, s = math.cos(a), math.sin(a)
    return [[1, 0, 0], [0, c, s], [0, -s, c]]


def ry(a):
    c, s = math.cos(a), math.sin(a)
    return [[c, 0, -s], [0, 1, 0], [s, 0, c]]


def rz(a):
    c, s = math.cos(a), math.sin(a)
    return [[c, s, 0], [-s, c, 0], [0, 0, 1]]


def mm(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def mv(a, v):
    return [sum(a[i][k] * v[k] for k in range(3)) for i in range(3)]


def smnb(cdux, cduy, cduz):
    """Stable member to navigation base: inner (Y), middle (Z), outer (X)."""
    return mm(rx(cdux), mm(rz(cduz), ry(cduy)))


def gimbals(m):
    """(CDUX, CDUY, CDUZ) of an SMNB matrix (smnb's inverse)."""
    return (math.atan2(-m[2][1], m[1][1]), math.atan2(-m[0][2], m[0][0]), math.asin(max(-1.0, min(1.0, m[0][1]))))


def rotation(u, v):
    """The smallest rotation taking unit u onto unit v (Rodrigues)."""
    k = [u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0]]
    s, c = math.sqrt(sum(x * x for x in k)), sum(a * b for a, b in zip(u, v))
    if s < 1e-12:
        return [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
    k = [x / s for x in k]
    K = [[0, -k[2], k[1]], [k[2], 0, -k[0]], [-k[1], k[0], 0]]
    K2 = mm(K, K)
    return [[(i == j) + s * K[i][j] + (1 - c) * K2[i][j] for j in range(3)] for i in range(3)]


def unit(v):
    n = math.sqrt(sum(x * x for x in v))
    return [x / n for x in v]
