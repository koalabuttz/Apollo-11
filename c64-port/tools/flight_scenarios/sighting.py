#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Sighting helpers for the flight scenarios (lm_p52.py, lm_p57.py; CM
P52 can use them too): AGC word and CDU angle conversions, the rope
listing's star catalog, the stable member to navigation base matrix of the
gimbal angles (inner Y, middle Z, outer X) and its inverse, and the LM
AOT's optic axis (OANB) and reticle (SURFSTAR run backward)."""
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


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def optic_axis(az, el):
    """OANB: the AOT optic axis and the X and Y mark plane vectors (NB)
    for detent azimuth az and elevation el (radians)."""
    oa = [math.sin(el), math.cos(el) * math.sin(az), math.cos(el) * math.cos(az)]
    uyp = [0.0, math.cos(az), -math.sin(az)]
    return oa, unit(cross(uyp, oa)), uyp


def reticle(s_nb, az, el, az_ref):
    """The cursor and spiral angles and the angle off the optic axis
    (radians) of a star (NB unit vector) seen in the detent at az, el:
    SURFSTAR run backward. OPTAXIS turns the mark planes by the detent's
    apparent field rotation az_ref - az (az_ref: AOTAZ+1, the forward
    detent)."""
    oa, uxp, uyp = optic_axis(az, el)
    c, s = math.cos(az_ref - az), math.sin(az_ref - az)
    xp = [c * uxp[i] + s * uyp[i] for i in range(3)]
    yp = [c * uyp[i] - s * uxp[i] for i in range(3)]
    sep = math.acos(max(-1.0, min(1.0, dot(s_nb, oa))))
    d = dot(s_nb, oa)
    upp = unit([s_nb[i] - d * oa[i] for i in range(3)])
    cursor = cross(oa, upp)             # the cursor line: cos(Y) yp - sin(Y) xp
    yrot = math.atan2(-dot(cursor, xp), dot(cursor, yp)) % (2 * math.pi)
    return yrot, (yrot + 12 * sep) % (2 * math.pi), sep
