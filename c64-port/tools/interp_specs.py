# SPDX-License-Identifier: MIT
"""Milestone specs for tools/interp_compile.py: the file, its entries, the
outside basic routines it calls (with their inline word counts) and the
native routines its RTBs reach."""

# TVCMASSPROP breakpoints and slopes (VARST0-VARST8): magnitudes to 07777.
VARST = [0, 1, 0o77776, 0o17, 0o77760, 0o400, 0o77377, 0o1234, 0o76543,
         0o3777, 0o74000, 0o7777, 0o70000]


# CSMMASS (E6,1474, B+16 kg) within the CSM's mass (0 to 30000 kg), either
# side of TVCMASSPROP's 33956 lb breakpoint (NEGBPW).
CSMMASS = [0, 0o2000, 0o7000, 0o7412, 0o7413, 0o7414, 0o10000, 0o13000, 0o16514]

def ones(x):
    """x as a ones'-complement word."""
    return x if x >= 0 else ~(-x) & 0o77777


def kalcmanu_joint(cm):
    """KALCMANU steering states that only mean something together:
    TIMECHK's TM - TIME (TTEMP) small, of both signs and exactly 0, around
    ONESEK (100); CONTMANU's NEXTIME at, near and wrapped past TIME1;
    INCRDCDU's BCDU - NCDU small enough that MP DT/TAU leaves +-0; RATEBIAS's
    BRATE +-0; and (Comanche) HOLDFLAG +-0 and each RATEINDX."""
    out = []
    for t2 in (0, 0o1234, 0o37777):
        for t1 in (0o200, 0o37000):
            for dh in (0, 1):
                for d in (-101, -100, -99, -1, 0, 1, 99, 100, 101):
                    out.append({'0024': t2, '0025': t1, 'TM': t2 + dh, 'TM+1': t1 + d})
    for t1 in (0o200, 0o37770, 0o37777):
        for d in (-1, 0, 1, 50, 100):
            out.append({'0025': t1, 'NEXTIME': (t1 + d) % 0o40000})
    for e in (-12, -5, -1, 0, 1, 5, 12):
        st = {}
        for i in range(3):
            st['BCDU+%d' % i] = ones(0o100 + e * (i - 1))
            st['NCDU+%d' % i] = 0o100
        out.append(st)
    for hi in (0, 0o77777, 3, 0o77774):
        out.append({'BRATE+0': hi, 'BRATE+1': 0, 'BRATE+2': 0o77777 ^ hi, 'BRATE+3': 0, 'BRATE+4': hi, 'BRATE+5': 0})
    if cm:
        for v in (0, 0o77777, 1, 0o77776):
            out.append({'HOLDFLAG': v})
        for v in (0, 2, 4, 5, 6, 7):
            out.append({'RATEINDX': v})
    return out


def anglfind_joint():
    """MFI at identity and axis rotations (AM = ARCCOS(1), the ACOSST .5
    edge), and COF with each ordering of its components, ties included
    (COFMAXGO's BMN arms)."""
    out = []
    for one in (0o10000, 0o20000):
        for signs in ((1, 1, 1), (1, -1, -1), (-1, 1, -1), (-1, -1, 1)):
            st = {'MFI+%d' % k: 0 for k in range(18)}
            for i, sg in enumerate(signs):
                st['MFI+%d' % (8 * i)] = one if sg > 0 else ones(-one)
                st['MFI+%d' % (8 * i + 1)] = 0 if sg > 0 else 0o77777
            out.append(st)
    for a, b, c in ((1, 2, 3), (1, 3, 2), (2, 1, 3), (2, 3, 1), (3, 1, 2), (3, 2, 1),
                    (2, 2, 1), (1, 2, 2), (2, 1, 2), (1, 1, 1)):
        out.append({'COF': a * 0o2000, 'COF+1': 0, 'COF+2': b * 0o2000, 'COF+3': 0,
                    'COF+4': c * 0o2000, 'COF+5': 0})
    return out


def axisgen_states():
    """AXISGEN's loops entered at their heads or at any resume point in them
    (AXISGEN+1..AXISGEN1+28 with AXISGEN1's, the AXISGEN2 and AXISGEN3
    loops with theirs): the index registers and steps as the loops hold
    them."""
    one = {'vac+046': ['AXISGEN+1', 0o14], 'vac+050': ['AXISGEN+3'], '0166': ['vac+0']}
    two = {'vac+046': [0o22, 0o14, 0o6], 'vac+050': [0o6], 'vac+047': [0o6, 0o4, 0o2],
           'vac+051': [0o2], 'vac+036': [0o77771], '0166': ['vac+0']}
    # Past AXISGEN1's loop SXA,1 has put -6 in 30D. Between the AXISGEN2
    # loop's two XCHX,1 30D, X1 holds -6 and 30D the loop count.
    after = dict(one, **{'vac+036': [0o77771]})
    inside = dict(two, **{'vac+046': [0o77771], 'vac+036': [0o22, 0o14, 0o6]})
    out = {'AXISGEN+%d' % n: one for n in range(4, 6)}
    out.update({'AXISGEN1+%d' % n: one for n in range(0, 0o14)})
    out.update({'AXISGEN1+%d' % n: after for n in range(0o14, 0o27)})
    out['AXISGEN2'] = two
    out.update({'AXISGEN2+%d' % n: inside for n in range(1, 0o17)})
    out.update({'AXISGEN2+%d' % n: two for n in range(0o17, 0o23)})
    out.update({'AXISGEN3+%d' % n: two for n in range(0, 0o20)})
    return out



def p51_p53_states():
    """(states, joint_states) for P51-P53's star-catalog indexing. The pair search (PICAPAR) steps X1
    and X2 by S1 = S2 = 6 through the 37 stars (228 = 37 x 6 + 6); BESTI and
    BESTJ hold the best pair's indices; STARIND picks BESTI or BESTJ (0 or
    1); LM POSCODE is the AOT detent position, 1-6. LM GRAVEL steps X1 18,
    12, 6 (S1 6) with X2 -6, -4, -2 (and its complement after the first LXC)
    over the backward unit vectors below XUNIT."""
    cat = list(range(6, 229, 6))
    # (and the pushdown list low, as SETPD 0 leaves it, so STRAT's pushes
    # stay below X1)
    pic = {'vac+046': cat, 'vac+047': cat, 'vac+050': [6], 'vac+051': [6],
           '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10']}
    # (from AXT,1 228D to NEWPAR's GOTO; X1 enters CATLOG reads after R59's
    # and NOTPLAN's TS X1 too)
    lm = {'PIC1%+d' % n: pic for n in range(-12, 100)}
    cm = {'PIC1%+d' % n: pic for n in range(-12, 61)}
    # STOVL BESTI stores the pair as the search starts (DPZERO, or a pair).
    for d in (lm, cm):
        d['PIC1-17'] = {'0154': [0] + cat, '0155': [0] + cat, '0163': [0]}
    lm.update({'R59A+%d' % n: {'vac+046': cat} for n in range(16, 18)})
    # CALSAM(1) runs for a planet (index 228 on, its vector at STARAD +0, +6
    # or +12): LM X1 -STARIND for DLOAD* BESTI,1, then -BESTI for STARAD
    # -228D,1; CM LXC,1 MPAC with MPAC that index.
    planet = [228, 234, 240]
    lm.update({'CALSAM+%d' % n: {'vac+046': [0o77777, 0o77776], 'BESTI': planet, 'BESTJ': planet}
               for n in range(-1, 12)})
    lm.update({'CALSAM+%d' % n: {'vac+046': [~v & 0o77777 for v in planet]} for n in range(12, 16)})
    cm.update({'CALSAM1+%d' % n: {'BESTI': planet, 'BESTJ': planet} for n in range(0, 4)})
    cm['CALSAM1+4'] = {'0154': planet}
    # LM GRAVEL's head: LXC,2 S1 with X1 18 and S1 6.
    lm.update({'GRAVEL%+d' % n: {'vac+046': [18], 'vac+050': [6]} for n in (-2, -1)})
    # The small loops: S52.2A's XNB rows (X1 18, 12, 6 by S1 6), CALOOP's
    # three CDU differences (X1 3, 2, 1 by S1 1), LM WRTDESIR's two
    # vectors (X2 12, 6 by S2 6).
    for d in (lm, cm):
        d.update({'S52.2A+%d' % n: {'vac+046': [18, 12, 6], 'vac+050': [6]} for n in range(0, 7)})
        d.update({'CALOOP+%d' % n: {'vac+046': [3, 2, 1], 'vac+050': [1]} for n in range(0, 0o30)})
        d.update({'CALOOP1+%d' % n: {'vac+046': [3, 2, 1], 'vac+050': [1]} for n in range(0, 2)})
        # (S1 1 from the SSP before AXT,1 SETPD; the loop's pushes from 4D)
        d.update({'CALOOP-%d' % n: {'vac+050': [1]} for n in (1, 2, 3)})
        for n in list(range(0, 0o30)):
            d['CALOOP+%d' % n] = dict(d['CALOOP+%d' % n], **{'0166': ['vac+4', 'vac+6', 'vac+10', 'vac+12']})
    # CM CALCSMSC pushes five DP terms and reads 26D: its pushdown list
    # starts low, as its callers leave it, so the pushes stay below X1 and QPRET.
    cm.update({'CALCSMSC+%d' % n: {'0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6']} for n in range(0, 0o47)})
    # CM ADVTRACK (SETPD 0) and ROTA push up to three vectors.
    cm.update({'ADVTRACK+%d' % n: {'0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']}
               for n in range(0, 0o104)})
    lm.update({'WRTDESIR+%d' % n: {'vac+047': [12, 6], 'vac+051': [6]} for n in range(0, 0o11)})
    # CM SXTSM: LXC,2 MPAC with MPAC the mark downlink ECADR (SLOAD* MKDNCDR,2).
    cm['SXTSM+9'] = {'0154': ['MKDNCDR+0', 'MKDNCDR+1']}
    # CM R53's STODL SAC and STORE PAC store double precision (PAC, at E5
    # 1775, is three words from the bank's end: a vector STORE would leave it).
    cm.update({'SXTSM+%d' % n: {'0163': [0]} for n in range(49, 53)})
    for d in (lm, cm):
        d.update({'NOTPLAN+%d' % n: {'vac+046': cat} for n in range(9, 11)})
    neg = lambda v: ~v & 0o77777
    loop = [(18, 6), (12, 4), (6, 2)]
    joint = {}
    for n in range(0, 0o24):
        # X2 positive after the first LXC,2 X2 (GRAVEL+6) up to VSU*
        # (GRAVEL+0o13); 2 further on after INCR,2 2D (GRAVEL+0o16).
        x2 = (lambda x, k: k) if 7 <= n < 0o13 else (lambda x, k: neg(k - 2)) if n >= 0o16 else (lambda x, k: neg(k))
        lm['GRAVEL%+d' % n] = {'vac+050': [6]}
        joint['GRAVEL%+d' % n] = [{'vac+046': x, 'vac+047': x2(x, k)} for x, k in loop]
    return {'lm': lm, 'cm': cm}, {'lm': joint, 'cm': {}}

def cdutodcm_states():
    """CDUTODCM's LOOPSIN loop: X1 3, 2 or 1 with S1 1 and the pushdown list
    at 0D at the loop head; at its resume points (CDUTODCM+4 on, inside the
    loop) the pushdown list anywhere the pushes take it. SUHALFA and CALCPHI
    run with PSI and THETA pushed (4D)."""
    head = {'vac+046': [3, 2, 1], 'vac+050': [1], '0166': ['vac+0']}
    body = dict(head, **{'0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12', 'vac+14']})
    out = {'CDUTODCM+%d' % n: dict(head, **{'vac+046': [3]}) for n in (4, 5)}
    out['LOOPSIN'] = head
    out.update({'LOOPSIN+%d' % n: body for n in range(1, 0o12)})
    out.update({'SUHALFA': {'0166': ['vac+4']}, 'CALCPHI': {'0166': ['vac+4']}})
    # DCMTOCDU's SL1 and STORE S1 run on the double-precision sine of
    # PSI: in another mode the STORE would reach QPRET (S1 +2).
    out.update({'DCMTOCDU+%d' % n: {'0163': [0]} for n in (3, 5)})
    return out


def integration_joint():
    """Flight values of the integration erasables that only mean something
    together or at one value (review #165): TDEC = TET (the last pass of an
    integration, DT/2COMP +0), UPSVFLAG +0 (UPDATE_PROGRAM's normal
    INTWAKEU) and +-2, MODREG +0 and 27 (P27 running), and a small ALPHAV
    after RECTIFY. One set per case, in half of every routine's cases."""
    out = []
    for hi, lo in ((0, 0), (0o1234, 0o4567), (0o37777, 0o37777), (ones(-0o1234), ones(-0o4567))):
        out.append({'TDEC': hi, 'TDEC+1': lo, 'TET': hi, 'TET+1': lo})
    out += [{'UPSVFLAG': v} for v in (0, 2, ones(-2))]
    out += [{'MODREG': v} for v in (0, 0o27)]
    for v in ((0, 3, 0, ones(-5), 0, 1), (0, 0o40, ones(-0o20), 0, 0, 0o100), (0, 1, 0, 0, 0, 0)):
        out.append({'ALPHAV+%d' % k: x for k, x in enumerate(v)})
    return out


def n49_wait():
    """R22LEM96's wait for the N49 display (SLOAD N49FLAG, BZE -3): a zero
    flag loops until another job is waiting (NEWJOB positive)."""
    return [{'N49FLAG': 0, '0067': 1}, {'N49FLAG': 1}, {'N49FLAG': 0o77776}, {'N49FLAG': 2}]


def v67_loop():
    """V67's W-matrix loop state: X1 a pass (6 to 54), S1 6."""
    return [{'vac+46': x, 'vac+50': 6} for x in range(6, 55, 6)]


# Lat-long ARCTAN's quadrant edges (reviewer 1, #175): SINTH and COSTH at
# +-0 together (BZE: theta 0/0), SINTH +-0 (ASIN +-0; with COSTH negative,
# NEGCOS's BPL on -0: LONG 180 degrees) and COSTH +-0 (ASIN +-1), as
# (hi, lo) doubles; the empty sets keep half the cases random.
def lat_long_arctan_joint():
    z, mz, h, mh = (0, 0), (0o77777, 0o77777), (0o20000, 0), (0o57777, 0o77777)
    sets = []
    for s_, c_ in ((z, z), (mz, mz), (z, mz), (mz, z), (z, h), (mz, h), (z, mh), (mz, mh),
                   (h, z), (mh, z), (h, mz), (mh, mz)):
        # SINTH and COSTH are 18D and 16D of the work area.
        sets.append({'vac+22': s_[0], 'vac+23': s_[1], 'vac+20': c_[0], 'vac+21': c_[1]})
    return sets + [{}] * len(sets)


# The planetary vector LAT-LONG unitizes after R-TO-RP returns: on an axis,
# so SINTH or COSTH of the latitude and longitude come out exact (+-0, 1/2).
def lat_long_axis_joint():
    sets = []
    for axis in range(3):
        for hi, lo in ((0o20000, 0), (0o57777, 0o77777)):
            v = {'MPAC+%d' % k if k else 'MPAC': 0 for k in range(6)}
            v['MPAC+%d' % (2 * axis) if axis else 'MPAC'] = hi
            v['MPAC+%d' % (2 * axis + 1)] = lo
            sets.append(v)
    return sets + [{}] * len(sets)

# AOTMARK: MARKSTAT (zero, or the marks' VAC area +1 under flag bits 10-14)
# and the surface mark slots (that area plus 6 words a mark).
AOT_MARKSTAT = [0] + [(f << 9) | a for f in range(32) for a in (0o401, 0o455, 0o531, 0o605, 0o661)]
AOT_SLOTS = [a + 6 * n for a in (0o401, 0o455, 0o531, 0o605, 0o661) for n in range(6)]

# LLGE's ROOTPSRS work words (MPAC +3 to +6, PWRPTR, DERPTR, RETROOT)
# mid-routine.
ROOTPS_STATES = {'0157': [0o1565], '0160': [1, 2], '0161': [0o147], '0162': [0, 1],
                 '0117': [0o1565, 0o1563, 0o1561], '0141': [0o147, 0o145, 0o143], '0132': [0o3113],
                 'WCHPHASE': [0o77776, 0, 1], '0166': ['vac+0']}

SPECS = {
    # Lane D1: Luminary099 AOTMARK (probe).
    'lm-aotmark': {
        'missions': ('lm',),
        'file': {'lm': 'AOTMARK.agc'},
        'base': {'lm': 'lm_aotmark'},
        'symbol': {'lm': 'lm_aotmark'},
        'entries': ['@external'],
        'externals': {'POSTJUMP': 'auto', 'NOVAC': 'auto', 'RESUME': 'auto', 'ALARM': 'auto', 'ENDMARK': 'auto', 'WAITLIST': 'auto', 'GOTOPOOH': 'auto', 'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'FINDVAC': 'auto', 'BAILOUT1': 'auto', 'POODOO': 'auto', },
        'rtb': {'CDULOGIC': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_CDU_LOGIC)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'basic_fixed_index': {'MKVB54': [0, 1, 2, 3]},
        # MKVACFND's INDEX MARKSTAT; TS 0 -1 assembles as DXCH 1777 and
        # becomes TS VACnUSE once the index is added.
        'index_carry': {'07,2035': 0o54000},
        # MKVACFND is reached from CCS VACnUSE: A holds the free area's address -1.
        # MKDEX is the mark verb index (0-3) in flight, the mark's VAC slot
        # (area +1, 6 words a mark) on the surface (FLAGWRD8 bit 8).
        'states': dict({'MKVACFND': {'0': [0o377, 0o453, 0o527, 0o603, 0o657]},
                        '07,2633': {'MKDEX': AOT_SLOTS}, '07,2634': {'MKDEX': AOT_SLOTS}},
                       # The star vector code indexes a mark's slot with X1 = -slot
                       # (AVESTAR), XYMARK holding the marks' VAC area (MARKCHEX);
                       # AVESTAR empties the pushdown list, which stays shallow
                       # (a deeper PUSHLOC would push over X1).
                       **{'%02o,%04o' % (b, a): {'vac+046': [0o77777 - x for x in AOT_SLOTS],
                                                 'XYMARK': [0o401, 0o455, 0o531, 0o605, 0o661],
                                                 '0166': ['vac+%o' % n for n in range(0, 0o16, 2)]}
                          for b, lo, hi in ((7, 0o2171, 0o2202), (7, 0o2255, 0o2316), (0o15, 0o2000, 0o2046))
                          for a in range(lo, hi)},
                       # Past MARKRUPT's CCS MARKSTAT, marks are being accepted.
                       **{'07,%04o' % a: {'MARKSTAT': AOT_MARKSTAT[1:]} for a in range(0o2365, 0o2430)}),
        'joint_states': {
                         # 15,2041 VSL1 VAD from the state the original reaches it
                         # in after AVEIT and SURFSTAR (the oracle's, at that NEWOPS):
                         # VAD's overflow there carries into a word that does not
                         # overflow, which random states almost never give.
                         'JUSTOA-1': [{}, {}, {}, {
                             '0120': 0o605, '0121': 0o1, '0154': 0o17573, '0155': 0o76534, '0156': 0o0,
                             '0157': 0o70311, '0160': 0o70337, '0161': 0o10371, '0162': 0o14166,
                             '0163': 0o77776, '0166': 0o613, '0605': 0o603, '0606': 0o2356, '0607': 0o1471,
                             '0610': 0o21246, '0611': 0o25, '0612': 0o36514}],
                         'CHANGEVB': [{'FLAGWRD8': f | o, 'MKDEX': m}
                                      for f, ms in ((0, range(4)), (0o200, AOT_SLOTS))
                                      for o in (0, 0o12345 & ~0o200, 0o77577) for m in ms]},
        'ebank': 7,
        # FRESH_START leaves a free VAC area holding its own address
        # (VAC1ADRC ADRES VAC1USE); a busy one holds zero.
        'all_states': {'VAC1USE': [0, 0o400], 'VAC2USE': [0, 0o454], 'VAC3USE': [0, 0o530],
                       'VAC4USE': [0, 0o604], 'VAC5USE': [0, 0o660],
                       # MARKSTAT: the VAC area's address +1 in the low 9
                       # bits, mark flags in bits 10-14 (or zero: no marks).
                       'MARKSTAT': AOT_MARKSTAT,
                       # MARKCNTR: marks wanted or taken, 0 to 5.
                       'MARKCNTR': list(range(6)),
                       # AOTCODE: detent 0-7 in bits 7-9, star code below.
                       'AOTCODE': [(d << 6) | c for d in range(8) for c in (0, 1, 0o45, 0o77)]},
        'resume_keys': True,
        'resume_orphans': 'all',
        # Mid-file interpretive points start from the PUSHLOC, MODE and
        # index registers the file's own runs reach them with.
        'flow_states': True,
        'resume_flow': True,
        'channel_inputs': True,
        'c64_flags': ['-fno-inline', '-DIRC_INPUTS'],
        'target': 'tests/lm_aotmark_target.c',
        # Host vectors and C64 rows chosen to reach every branch outcome
        # (gcov arcs and clang edges, the shared bodies' edges per call
        # site); TIME1/TIME2 and the CDUs vary; a quarter of the cases put
        # edge values in MPAC and the operands.
        'c64_rows': 'cover',
        'cover_stride': 8,
        'site_edges': True,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 32,
    },
    # Lane D1: Comanche055 SXTMARK, the whole file (basic code) as routines
    # at its original addresses.
    'cm-sxtmark': {
        'missions': ('cm',),
        'file': {'cm': 'SXTMARK.agc'},
        'base': {'cm': 'cm_sxtmark'},
        'symbol': {'cm': 'cm_sxtmark'},
        'basic': 'full',
        # And the returns the outside calls can make past their first:
        # CHECKMM's in-mode return (+2 past its MM), GOMARK4's verb 33 and
        # enter returns (+1, +2 past the CADR).
        'entries': ['@external', 'MKVACFND+10', 'MKVACFND+14', 'TERMSXT-2', 'TERMSXT-1', 'TERMP03-3'],
        'externals': {'BAILOUT': 'auto', 'CHECKMM': 'auto', 'SWRETURN': 'auto', 'NOVAC': 'auto', 'GENTRAN': 'auto', 'TASKOVER': 'auto', 'ALARM': 'auto', 'POSTJUMP': 'auto', 'RESUME': 'auto', 'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'UPFLAG': 'auto', 'WAITLIST': 'auto', 'ENDMARK': 'auto', 'CLEARMRK': 'auto'},
        # Channel 12 (WAND CHAN12 in MKRLEES) in the io record; NAVKEYIN
        # (channel 16) is a latched input ('channel_inputs').
        'channels': {0o12: 'channel12'},
        'io_channels': 'auto',
        # MKVACFND's INDEX MARKSTAT; TS 0 -1 assembles as DXCH 1777 and
        # becomes TS VACnUSE once MARKSTAT (a VAC area) is added.
        'index_words': {'07,2037': [0o401, 0o455, 0o531, 0o605, 0o661]},
        'ebank': 7,
        'all_states': {'VAC1USE': [0, 0o400], 'VAC2USE': [0, 0o454], 'VAC3USE': [0, 0o530],
                       'VAC4USE': [0, 0o604], 'VAC5USE': [0, 0o660],
                       # MARKSTAT: the VAC area's address in the low 9 bits,
                       # marks wanted in bits 12-14 and the reject enable in
                       # bit 10 (or zero: no marks).
                       'MARKSTAT': AOT_MARKSTAT,
                       # QPRET (each area's word 42D): the next mark slot,
                       # the area plus 7 words a mark taken.
                       **{'%04o' % (a + 0o52): [a + 7 * n for n in range(6)]
                          for a in (0o401, 0o455, 0o531, 0o605, 0o661)}},
        # DIFCHK is entered with MKNDX -1, 0 or 1 (it indexes CDUX-CDUZ).
        'states': {'DIFCHK': {'MKNDX': [0o77776, 0, 1]},
                   'MKVACFND': {'0': [0o377, 0o453, 0o527, 0o603, 0o657]}},
        'channel_inputs': True,
        'c64_flags': ['-fno-inline', '-DIRC_INPUTS', '-DIRC_CHANNELS'],
        'target': 'tests/cm_sxtmark_target.c',
        'c64_rows': 'cover',
        'cover_stride': 8,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 32,
    },
    'inflight-align': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'INFLIGHT_ALIGNMENT_ROUTINES.agc', 'cm': 'INFLIGHT_ALIGNMENT_ROUTINES.agc'},
        'base': {'lm': 'lm_inflight_align', 'cm': 'cm_inflight_align'},
        'symbol': {'lm': 'lm_inflight_align', 'cm': 'cm_inflight_align'},
        'entries': ['CALCGTA', 'ARCTRIG', 'CALCGA', 'AXISGEN'],
        'externals': {'ALARM': 1, 'UPFLAG': 1},
        'rtb': {'V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_VECTOR_ONE_TO_TWO)',
                            'cm': 'rtb_angles(s,io,RTB_CM,RTB_VECTOR_ONE_TO_TWO)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/inflight_align_target.c',
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        # Job code: a DANZIG that finds NEWJOB positive stops at CHANG2, and
        # the word after it is the routine the job resumes at.
        'resume_keys': True,
        # Every 128th case (with the cover) keeps the vectors under the 1 MB fast-checkout limit.
        # KALCMANU's matrix loops run past 200 routines from some resume points.
        'max_steps': 1000,
        'freeze_stride': 128,
        # AXISGEN's loops entered directly: index registers and steps as
        # the loops hold them (X1 and S1 at VAC+046/050, X2 and S2 at
        # VAC+047/051, the saved X1 in 30D at VAC+036). 'LABEL+n' is the
        # word at that address (AXT,1 STARAD +6 and SSP S1 STARAD -6);
        # 'vac+n' the VAC address (PUSHLOC 0166 at 0D, AXISGEN's SETPD).
        # The job's resume points inside the loops (resume_keys) take the
        # states of the loop they are in.
        'states': axisgen_states(),
    },
    'geometry': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'LEM_GEOMETRY.agc', 'cm': 'CSM_GEOMETRY.agc'},
        'base': {'lm': 'lm_geometry', 'cm': 'cm_geometry'},
        'symbol': {'lm': 'lm_geometry', 'cm': 'cm_geometry'},
        'entries': {'lm': ['SVDWN2', 'SVDWN1', 'RRANGLES', 'RRNB', 'RRNBMPAC'],
                    'cm': ['SVDWN1', 'SVDWN2', 'LOG', 'EARROT1', 'EARROT2', 'SXTNB', 'CALCSXA', 'SXTANG']},
        'externals': {'POLY': 9},
        # SXTNB's RTB SXTLOGIC runs natively (src/geometry_rtb.c), and
        # Comanche's TC SHORTMP (LOG, SXTLOGIC) runs natively in place.
        'native_ranges': {'SXTLOGIC': [5, 'geometry_rtb']},
        'native_calls': {'SHORTMP': {'cm': 'geometry_shortmp(s)'}},
        # CALCSXA and SXTANG keep their caller's return CADR in 28D (ITA 28D)
        # and leave through GOTO 28D: a CADR outside the file. X1 and X2
        # name what the next indexed operand reads (SXTNB's 5,1 and 3,1).
        'all_states': {'lm': {}, 'cm': {'vac+034': ['outside']}},
        # The DP stores of SAC and PAC (after RTB CDULOGIC, 1STO2S or a DLOAD)
        # and SXTLOGIC's RTB run in DP mode.
        'states': {'cm': {lab: {'0163': [0]} for lab in ('23,2003', '23,2005', '23,2006', '23,2120', '23,2136', '23,2147', '23,2151')}},
        'index_states': True,
        'rtb': {'CDULOGIC': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_CDU_LOGIC)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_CDU_LOGIC)'},
                '2V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_TWO_VECTOR_ONE_TO_TWO)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_TWO_VECTOR_ONE_TO_TWO)'},
                '1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_ONE_TO_TWO)',
                           'cm': 'rtb_angles(s,io,RTB_CM,RTB_ONE_TO_TWO)'},
                'SXTLOGIC': {'cm': 'geometry_rtb(s,io)'},
                'TRANSP1': {'cm': 'rtb_agreement(s,io,RTB_CM,RTB_TRANSPOSE1)'},
                'TRANSP2': {'cm': 'rtb_agreement(s,io,RTB_CM,RTB_TRANSPOSE2)'}},
        'includes': ['rtb_angles.h', 'rtb_agreement.h', 'geometry_rtb.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_agreement.c', 'src/geometry_rtb.c'],
        'target': 'tests/geometry_target.c',
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        # Job code: a DANZIG that finds NEWJOB positive stops at CHANG2, and
        # the word after it is the routine the job resumes at.
        'resume_keys': True,
        # KALCMANU's matrix loops run past 200 routines from some resume points.
        'max_steps': 1000,
        'freeze_stride': 128,
    },
    'kalcmanu-steering': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'KALCMANU_STEERING.agc', 'cm': 'KALCMANU_STEERING.agc'},
        'base': {'lm': 'lm_kalcmanu_steering', 'cm': 'cm_kalcmanu_steering'},
        'symbol': {'lm': 'lm_kalcmanu_steering', 'cm': 'cm_kalcmanu_steering'},
        # Entries from other files, and the Waitlist/Executive/POSTJUMP
        # targets the file names itself.
        'entries': {'lm': ['NEWDELHI', 'NEWANGL', 'GOODMANU', 'MANUSTOP', 'UPDTCALL'],
                    'cm': ['NEWDELHI', 'NEWANGL', 'MANUSTOP', 'UPDTCALL', 'CONTMANU', 'INCRDCDU-1',
                           'STOPRATE', 'LOADCDUD', 'ZEROEROR', 'ENDMANU', 'LOADYZ']},
        'externals': {'BANKCALL': 1, 'IBNKCALL': 1, 'POSTJUMP': 1, 'WAITLIST': 2, 'FINDVAC': 2,
                      'SPVAC': 0, 'TASKOVER': None, 'ENDOFJOB': None},
        # Calls that take inline words but never return past them.
        'noreturn': ['POSTJUMP'],
        'rtb': {'V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_VECTOR_ONE_TO_TWO)',
                            'cm': 'rtb_angles(s,io,RTB_CM,RTB_VECTOR_ONE_TO_TWO)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/kalcmanu_steering_target.c',
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        # Job code: a DANZIG that finds NEWJOB positive stops at CHANG2, and
        # the word after it is the routine the job resumes at.
        'resume_keys': True,
        # KALCMANU's matrix loops run past 200 routines from some resume points.
        'max_steps': 1000,
        'freeze_stride': 32,
        # ISITAUTO leaves CHAN31 & BIT14 in A: +0 or 020000.
        'states': {'NEWDELHI+2': {'0': [0, 0o20000]}},
        # The joint states reach TIMECHK's end of maneuver, CONTMANU's wrap
        # and the +-0 arms.
        'joint_states': {'*': {'lm': kalcmanu_joint(False), 'cm': kalcmanu_joint(True)}},
        # The file's EBANK= TTEMP (E6, as its 2CADR BBCONs carry): its
        # switched names (KSPNDX, BRATE...) mean nothing under another EB.
        'ebank': 6,
        # STOPRATE (and the other basic routines that never name FIXLOC) is
        # called from tasks and NOVAC jobs (V37): any FIXLOC.
        'any_fixloc': True,
    },
    # Lane A: Comanche055 T4RUPT_PROGRAM.agc optics monitor and drive
    # (OPTMON 06,2765-3262, OPTDRIVE 10,2000-2202), basic only. The IMU
    # monitor and T4RUPT itself are cm_t4rupt_imu and t4rupt_core.
    'cm-t4rupt-optics': {
        'missions': ('cm',),
        'file': {'cm': 'T4RUPT_PROGRAM.agc'},
        'base': {'cm': 'cm_t4rupt_optics'},
        'symbol': {'cm': 'cm_t4rupt_optics'},
        'basic': 'full',
        # T4JUMP's TCF OPTMON, OPTTEST's IBNKCALL OPTDRIVE, and ENDZOPT
        # (SETZOEND's WAITLIST task).
        'entries': ['OPTMON', 'OPTDRIVE', 'ENDZOPT'],
        # T4RUPT through 90SECS (06,2000-2764) is t4rupt_core and
        # cm_t4rupt_imu, with the IMU monitor's own task CADRs.
        'outside': {'T4RUPT': 501},
        'partial': {'cm': 'native in t4rupt_core and cm_t4rupt_imu (T4RUPT through 90SECS, 06,2000-2764)'},
        # CDUT and CDUS (counters, 0 otherwise): around 0, the 45-degree and
        # 90-degree tests of OPTDRIVE and its shaft-stop avoidance, and +-180.
        'cells': {a: [0, 1, 0o7777, 0o10000, 0o10001, 0o17777, 0o20000, 0o20001, 0o37777,
                      0o40000, 0o57777, 0o60000, 0o67777, 0o70000, 0o77776, 0o77777] for a in (0o35, 0o36)},
        # SETOFF's BZF TCQ (no lamp change) returns from OCDUFTST through
        # the interpreter's TC Q word (TCQ = Q+2 +1, 06711).
        'tcq_words': (0o6711,),
        # One body per bank, so OPTMON (06) and OPTDRIVE (10) are separate
        # flight-store modules.
        'basic_split': True,
        'cover': True,
        'channels': 'all',
        'io_channels': 'auto',
        'engine': 'flight-device',
        'ebank': 'listing',
        'target': 'tests/cm_t4rupt_optics_target.c',
    },
    # Lane G: Comanche055 SERVICER207 (bank 37), basic and interpretive.
    'cm-servicer': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'SERVICER207.agc'},
        'base': {'cm': 'cm_servicer'},
        'symbol': {'cm': 'cm_servicer'},
        'basic': 'full',
        # Entered from outside the file: restart tables (CHEKAVEG, NORMLIZE,
        # PREREAD, REDO5.5, REREADAC, SERVICER), P11 (PREREAD1 BANKCALL,
        # READACCS task), P40-P47/P61-P67 (PREREAD, SERVEXIT, CALCGRAV),
        # REENTRY_CONTROL (SERVEXIT). The file's own CADRs (task, job,
        # restart and AVGEXIT words) are added by the compiler.
        'entries': ['@external', 'PREREAD', 'PREREAD1', 'READACCS', 'REDO5.5', 'CHEKAVEG', 'SERVICER', 'NORMLIZE',
                    'REREADAC', 'SERVEXIT', 'CALCGRAV'],
        'channels': 'all',
        'io_channels': 'auto',
        'externals': {},
        # CALCGRAV's BDDV* -MUDT(E),1 with X1 = -RTX2 (0 Earth, -2 Moon):
        # -MUDT(E) or -MUDT(M).
        'indexed_fixed': {'-MUDT(E)': (0, 2)},
        # RTX2 (E7,1746): 0 Earth or 2 Moon, as the program sets it.
        # PIPAGE (1230): 0 or 1, the values the file stores.
        # AVGEXIT (1222-1223): AVOUTCAD's 2CADR AVGEND, as AVEGOUT stores it.
        'cells': {0o3746: [0, 2], 0o1230: [0, 1], 0o1222: [0o3165], 0o1223: [0o76067]},
        # ITISMOON (CALCGRAV's BMN target) holds X1 = -RTX2 from LXC,1.
        # The pushdown list as the jobs that reach SERVICER's interpretive
        # code hold it: AVERAGEG's and NORMLIZE's INTPRET start a fresh VAC
        # (PUSHLOC at its start); CALCRVG and CALCGRAV push up to 28 words.
        'states': {'ITISMOON': {'vac+046': [0o77777, 0o77775], '0166': ['vac+0', 'vac+2']},
                   '37,3145': {'0166': ['vac+0', 'vac+2']}, '37,3146': {'0166': ['vac+0', 'vac+2']},
                   '37,3242': {'0166': ['vac+0', 'vac+2']}, '37,3244': {'0166': ['vac+0', 'vac+2']},
                   'CALCRVG': {'0166': ['vac+0', 'vac+2']}, 'CALCGRAV': {'0166': ['vac+0', 'vac+2']},
                   # CHANG2 resume keys: the job resumes with the pushdown list
                   # as above, and after CALCGRAV's LXC,1 with X1 = -RTX2.
                   **{'37,%o' % a: {'0166': ['vac+0', 'vac+2']}
                      for a in (0o3136, 0o3251, 0o3252, 0o3354, 0o3355, 0o3417, 0o3423, 0o3425, 0o3426,
                                0o3427, 0o3431, 0o3434, 0o3437, 0o3442, 0o3444)},
                   **{'37,%o' % a: {'vac+046': [0o77777, 0o77775], '0166': ['vac+0', 'vac+2']}
                      for a in (0o3360, 0o3362, 0o3364, 0o3366, 0o3371, 0o3372, 0o3374, 0o3376, 0o3401,
                                0o3403, 0o3405, 0o3412, 0o3415)},
                   # PREREAD1 is entered by IBNKCALL (P40's CADR PREREAD1):
                   # Q = ISWRETRN, which QXCH RUPTREG1 saves and TC RUPTREG1
                   # runs.
                   '37,2722': {'2': [0o4753]},
                   # REREADAC after a restart mid-PIPASR: PIPAGE 0, and the
                   # DELVZ/DELVY/TEMX states PIPASR leaves (+-0 or a count).
                   'REREADAC': {'1230': [0], '1166': [0, 0o77777, 5], '1164': [0, 0o77777, 5],
                                '1224': [0, 0o77777, 7, 0o77770], '1226': [0, 0o77777, 7, 0o77770]}},
        'target': 'tests/cm_servicer_target.c',
        # Every 128th case keeps the vectors under the 1 MB fast-checkout limit.
        'freeze_stride': 128,
        # CALCGRAV's routines with their bodies fit below $c000 only when
        # LTO keeps the bodies out of line.
        'c64_flags': ['-fno-inline'],
    },
    # Lane A2: Comanche055 P11 (earth orbit insertion monitor), basic and
    # interpretive.
    'cm-p11': {
        'missions': ('cm',),
        'file': {'cm': 'P11.agc'},
        'base': {'cm': 'cm_p11'},
        'symbol': {'cm': 'cm_p11'},
        'basic': 'full',
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        'engine': 'flight-device',
        'counters': True,
        'ebank': 'listing',
        'resume_keys': True,
        # STICKCHK's rate indexes (RMANNDX, PMANNDX, YMANNDX): 0 no
        # rotation, 1 plus, 2 minus. SATSW (E6,1725), the boost-takeover
        # switch the file keeps: +1 (display), +0 (needles initialized), -1
        # (steer).
        'cells': {'E6,1656': [0, 1, 2], 'E6,1657': [0, 1, 2], 'E6,1660': [0, 1, 2],
                  'E6,1725': [1, 0, 0o77776]},
        # RESCALES can return with overflow in A: TS AK2 then skips into
        # SATOUT's CADR NEEDLER, which the AGC runs as CS 2420 (on to
        # ATERSET), and so does the port.
        # TIME2/TIME1 (024/025, counters) as P11 finds them: half of every routine's
        # cases hold TIME2 at +0, as it is for the first 163.84 s after
        # liftoff (P11's DXCH TIME2 zeroes it), with TIME1 at 0, small, at
        # 2SECS (200: REP11A's CS TIME1, AD 2SECS gives -0), past it, or
        # just below its wrap.
        'joint_states': {'*': [{'24': 0, '25': t} for t in (0, 1, 100, 199, 200, 201, 3000, 0o37777)]},
        'execute_data': ('34,2472',),
        # CA SATSW; CCS A: the -0 exit lands on STEERSAT's TLOAD TAD (an
        # interpretive word), which no SATSW value takes; as basic code its
        # TC returns would shadow the interpretive STORE AK at 34,2456.
        'interp_landings': ('34,2453',),
        # NOPOLYM's RTB V1STO2S is the first order of its pair (RTB RTB): the
        # native V1STO2S, so the second RTB (DELSTOR, in this file) runs on.
        'rtb': {'V1STO2S': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_VECTOR_ONE_TO_TWO)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/cm_p11_target.c',
        'freeze_stride': 64,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 CM_BODY_ATTITUDE (bank 37), basic and interpretive.
    'cm-body-attitude': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'CM_BODY_ATTITUDE.agc'},
        'base': {'cm': 'cm_body_attitude'},
        'symbol': {'cm': 'cm_body_attitude'},
        'basic': 'full',
        # CM/POSE: the entry job P61-P67 starts (2CADR at 26,2431); REDOPOSE
        # (REPOSADR) is the file's own restart point.
        'entries': ['@external', 'CM/POSE'],
        'rtb': {'CDULOGIC': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_CDU_LOGIC)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # CORANGOV's INDEX A; CA LIMITS (LIMITS = 05000) with A an overflow
        # sign (-1, 0, +1).
        'fixed_tables': {'5000': (-1, 1)},
        'target': 'tests/cm_body_attitude_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 REENTRY_CONTROL (entry guidance), basic and
    # interpretive.
    'cm-reentry': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'REENTRY_CONTROL.agc'},
        'base': {'cm': 'cm_reentry'},
        'symbol': {'cm': 'cm_reentry'},
        'basic': 'full',
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        'engine': 'flight-device',
        'ebank': 'listing',
        # PREDICT3's INDEX JJ; AD VREFER (+1), CS VREFER: JJ counts 12 down
        # to 0 through the reference-velocity table, then BACK2 adds 13 per
        # pass (five passes) through the 13-word tables after it.
        'fixed_tables': {'VREFER': (0, 91)},
        # KEP's RTB P66 is the first order of its pair: P66 returns through
        # TC DANZIG with SSP still in EDOP, so the interpreter runs SSP
        # GOTOADDR KEP2 from OPJUMP, not at NEWOPS where the transfer ABI
        # keys interpretive routines.
        'partial': {'cm': 'SSP GOTOADDR KEP2 (25,3315-3316), the order KEP leaves in EDOP across RTB P66: it '
                          'runs from the interpreter\'s DANZIG/OPJUMP when P66 returns, a point the transfer ABI '
                          'does not key yet (interpretive routines are keyed by LOC at NEWOPS)'},
        # LEWD (E7,1724), the lift-over-drag ratio the loop halves toward:
        # positive in both words, as STARTENT and LEWDOVFL leave it.
        # Negative (a negative low word under a +0 high word too),
        # LWDSTORE's BMN LEWDPTR loop never ends (on the AGC too).
        'cells': {'E7,1724': list(range(0, 0o40000, 0o1001)),
                  'E7,1725': list(range(0, 0o40000, 0o1001))},
        'target': 'tests/cm_reentry_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 P40-P47 (the SPS and RCS thrusting programs),
    # basic and interpretive.
    # Lane G: Comanche055 TIME_OF_FREE_FALL, the whole file as routines at
    # its original addresses (Lane B's LM form is lm-time-free-fall).
    'cm-time-free-fall': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TIME_OF_FREE_FALL.agc'},
        'base': {'cm': 'cm_time_free_fall'},
        'symbol': {'cm': 'cm_time_free_fall'},
        'basic': 'full',
        'entries': ['@external'],
        'target': 'tests/cm_time_free_fall_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES, the whole
    # file as routines at its original addresses (issue #58: LSPOS, LUNPOS,
    # LUNVEL and SOLPOS are routine keys of the transfer ABI).
    'cm-ephemeris': {
        'rope_fallback': True,
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc'},
        'base': {'cm': 'cm_ephemeris'},
        'symbol': {'cm': 'cm_ephemeris'},
        # SOLPOS has no caller in Comanche055: a public entry of its own.
        # RES, RESA, REM and VEM are where LSTIME's GOTO X1 (and RES's
        # GOTO X2) go: the AXT,1/AXT,2 operands of the four entries.
        'entries': ['@external', 'SOLPOS', 'RES', 'RESA', 'REM', 'VEM'],
        # (the file's EBANK= END-E7: VECOEM and the other ephemeris cells)
        'ebank': 'listing',
        # Mid-sequence resumes (REMA/VEMA's TIX loops, RES's pushdown chain)
        # start from the registers the file's own runs reach them with, and
        # a CHANG2 resume from the state the routine before it reaches.
        'flow_states': True,
        'resume_flow': True,
        # S1 (vac+050) is the TIX step: LSTIME's SSP S1 6D sets it before
        # the GOTO X1 into RES, REM or VEM (S1 = 0 would loop forever).
        # The pushdown list stays low in the VAC area (LSTIME's SETPD 0D;
        # RES pushes to 16D): a push near X1 would overwrite the index.
        'all_states': {'vac+050': [6], '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        # C64 rows: the cases that take every branch outcome the host cases take.
        'c64_rows': 'cover',
        # (INDEX's end-around carry kept within 16 bits for the C64)
        'index_carry16': True,
        'target': 'tests/cm_ephemeris_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 ORBITAL_INTEGRATION, the whole file as routines at its
    # original addresses.
    'cm-orbital-integration': {
        'rope_fallback': True,
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'ORBITAL_INTEGRATION.agc'},
        'base': {'cm': 'cm_orbital_integration'},
        'symbol': {'cm': 'cm_orbital_integration'},
        'basic': 'full',
        'entries': ['@external'],
        'noreturn': ['POODOO'],
        'cgoto': {'DIFEQTAB': 3},
        # Bank 13's constants indexed by the body (X2 = PBODY: 0 Earth, 2
        # Moon) or, for ASCALE,1, by X1 = PBODY plus GETRPSV's INCR,1 4s.
        # (MUEARTH,2 and MUEARTH -2,2 also run with X2 at -2, 4 and the
        # DIFEQ stages -12D and -24D, as resumes in ACCOMP and GAMCOMP
        # find it.)
        'fixed_index': {'MUEARTH': [2, 0, -2, -4, 12, 24], 'J2REQSQ': [0, -2], 'J4REQ/J3': [0, -2], '2J3RE/J2': [0, -2],
                        'RDE': [0, -2], 'ASCALE': list(range(0, -50, -2))},
        # (an index that takes the address below 02000 reads erasable)
        'fixed_index_erasable': True,
        # PBODY (E4,1550) is 0 or 2; X2 holds it, and S1/S2 index steps
        # (LXA,1 S2 before the J-terms). DIFEQCNT (1500) is the
        # DIFEQ stage, 0, -12D or -24D (NBRANCH's DMP CGOTO); COLREG (1102)
        # a W column, 0 to -42D by -6.
        'all_states': {'2150': [0, 2], 'vac+046': [0, 2, 4, 6], 'vac+047': [0, 2], 'vac+050': [0, 2, 4, 6], 'vac+051': [0, 2, 4, 6], '1500': [0, 0o77763, 0o77747],
                       '1102': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717],
                       # The pushdown list stays below 14D (as on the LM).
                       '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        # Mid-sequence resumes start from the registers the file's own runs
        # reach them with.
        'flow_states': True,
        # A CHANG2 resume starts from the state the routine before it
        # reaches it in.
        'resume_flow': True,
        # NBRANCH's DMP CGOTO resumed: MPAC holds the stage SLOAD DIFEQCNT
        # left (DP).
        'states': {'11,3223': {'0154': [0, 0o77763, 0o77747], '0155': [0], '0163': [0]},
                   # (resumes no run reaches: OBLATE after SETPD 0 and DSU*; the
                   # W column store after VSL3 BOV)
                   '11,3022': {'0166': ['vac+0'], '0163': [0]},
                   # R-TO-RP returns a vector with PUSHLOC at 0.
                   '11,3030': {'0166': ['vac+0'], '0163': [0o77776]},
                   '11,3522': {'0163': [0o77776],
                               'vac+047': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717]}},
        'max_steps': 1000,
        # A DIFEQ or iteration run that does not fit one C64 diagnostic
        # drops every case whose run reaches at least its routines.
        'c64_drop_superset': True,
        'target': 'tests/cm_orbital_integration_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
    },
    # Lane G: Comanche055 INTEGRATION_INITIALIZATION, the whole file as routines at its
    # original addresses.
    'cm-integration-init': {
        'rope_fallback': True,
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'INTEGRATION_INITIALIZATION.agc'},
        'base': {'cm': 'cm_integration_init'},
        'symbol': {'cm': 'cm_integration_init'},
        'basic': 'full',
        # ERASTAL1/2 and ERASWAK1/2: the erasable-integration stall and wake
        # entries; nothing in Comanche055 calls them (WAKESTAL's CADRs
        # name ERASTAL1+1 and ERASTAL2+1), so they are listed here.
        'entries': ['@external', 'ERASTAL1', 'ERASTAL2', 'ERASWAK1', 'ERASWAK2'],
        # (TC DANZIG goes on in interpretive code: the word after it is no
        # return point.)
        'noreturn': ['POODOO', 'BANKJUMP', 'JOBSLEEP', 'DANZIG'],
        # TBASE2 holds INTSTALL's saved QPRET, a return outside the file.
        'return_words': ['1055'],
        # Bank 13's constants indexed by the body (X2 = PBODY: 0 Earth, 2
        # Moon).
        'fixed_index': {'MUEARTH': [2, 0, -2, -4, 12, 24], 'J2REQSQ': [0, -2], 'J4REQ/J3': [0, -2],
                        '2J3RE/J2': [0, -2], 'RDE': [0, -2], 'RME': [0, -2],
                        'ASCALE': list(range(0, -50, -2))},
        'fixed_index_erasable': True,
        # As ORBITAL_INTEGRATION's: PBODY 0 or 2, X1/X2 small steps,
        # DIFEQCNT a DIFEQ stage, COLREG a W column, the pushdown list below
        # 14D. (S1 and S2 hold saved returns here: STQ S2.)
        'all_states': {'2150': [0, 2], 'vac+046': [0, 2, 4, 6], 'vac+047': [0, 2], '1500': [0, 0o77763, 0o77747],
                       '1102': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717],
                       '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        'flow_states': True,
        'resume_flow': True,
        # The stall areas (L, STALTEM: 0 to 2) index INTBITAB, WAKESTAL and
        # INTFLBIT; ALLSTALL and WAKE take them in A.
        'fixed_tables': {'INTBITAB': (0, 2), 'WAKESTAL': (0, 2), '5002': (0, 2)},
        'states': {'13,3444': {'0': [0, 1, 2]}, '13,3500': {'0': [0, 1, 2]}, '13,3505': {'0154': [0, 1, 2],
                              # JOBWAKE's LOCCTR: a core set it woke (12D on) or -1, none
                              '0064': [0o14, 0o30, 0o44, 0o60, 0o74, 0o110, 0o77776]},
                   # INTWAKUQ holds INTWAKEU's caller's Q (QXCH), a fixed
                   # return (TC INTWAKEU at 27,3703) that TC INTWAKUQ runs.
                   '27,2745': {'0330': [0o3704]}},
        # CCS DIFEQCNT's negative exits (the MOVE* loops count down to +0)
        # land on interpretive words, never run as basic code.
        'interp_not_basic': True,
        'max_steps': 1000,
        'target': 'tests/cm_integration_init_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
    },
    # Lane G: Comanche055 STABLE_ORBIT, the whole file as routines at its
    # original addresses.
    'cm-stable-orbit': {
        'rope_fallback': True,
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'STABLE_ORBIT_-_P38-P39.agc'},   # (the listing's name for STABLE_ORBIT.agc)
        'source_path': {'cm': 'STABLE_ORBIT.agc'},
        'base': {'cm': 'cm_stable_orbit'},
        'symbol': {'cm': 'cm_stable_orbit'},
        'basic': 'full',
        'entries': ['@external'],
        # DSPLY81 (in the file) goes on in interpretive code after its
        # displays; TC DSPLY81 never returns.
        'noreturn': ['DSPLY81'],
        # GOFLASH/GOFLASHR returns (terminate, proceed, recycle, enter
        # after a BANKCALL) land on words only the display reaches.
        'resume_orphans': True,
        # VNDSPLY's QXCH RTRN saves its caller's return, and TC RTRN runs
        # that word (TC <return>): RTRN (E4,1767) holds one of the four
        # TC VNDSPLY returns, which the erasable continuation accepts.
        'erasable_points': {'E4,1767': [0o2750, 0o3041, 0o3152, 0o3154]},
        'states': {'04,3272': {'RTRN': [0o2750, 0o3041, 0o3152, 0o3154]}},
        # The file runs under EBANK=SUBEXIT (E4).
        'ebank': 'listing',
        'flow_states': True,
        'resume_flow': True,
        'max_steps': 1000,
        'target': 'tests/cm_stable_orbit_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
    },
    # Lane G: Comanche055 CONIC_SUBROUTINES, the whole file as routines at
    # its original addresses.
    'cm-conics': {
        'rope_fallback': True,
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'CONIC_SUBROUTINES.agc'},
        'base': {'cm': 'cm_conics'},
        'symbol': {'cm': 'cm_conics'},
        'basic': 'full',
        'entries': ['@external'],
        # (TC DANZIG goes on in interpretive code: the word after it is no
        # return point.)
        'noreturn': ['DANZIG'],
        # MUTABLE,1 (and +-n,1) with X1 -2 (Earth) or -10D (Moon), the
        # callers' AXT,1; the file never sets X1 itself.
        'fixed_index': {'MUTABLE': [2, 10]},
        'all_states': {'vac+046': [0o77775, 0o77765]},
        # KEPLER's and the time-theta iterations run hundreds of orders.
        'max_steps': 2000,
        # KEPLERN to PERIODCH (12,2045-12,2142): the pushdown list as
        # SETPD 0 leaves it (vac+2 between the two PDDLs and after
        # PERIODCH's), a positive period (MPAC at PERIODCH, 0D after it) and
        # a small TAU. (E4,1711), so PERIODCH's subtraction loop ends as
        # the flight code's does; MODE as the orders before leave it
        # (vector after VLOAD*, UNIT, VSQ's operand load, DP otherwise).
        # 04,3650 (GEOM's return) is in DP mode.
        'states': {
            **{'12,%o' % a: {'0166': ['vac+2' if a in (0o2122, 0o2124) or a >= 0o2133 else 'vac+0'],
                             '02311': [0, 0o77777], '02312': [1, 0o40, 0o77, 0o77700, 0o77776],
                             '0163': [0o77776 if a in (0o2047, 0o2051, 0o2054, 0o2060, 0o2066) else 0],
                             **({'0154': [0o100, 0o1000, 0o10000]} if a == 0o2131 else {}),
                             **({'vac+0': [0o100, 0o1000, 0o10000]} if a >= 0o2133 else {})}
               for a in range(0o2045, 0o2143)},
            '04,3650': {'0163': [0]},
            # STOREs into E7's last words (EPSILONL, COGA) after a DP
            # order: a vector store there would run past erasable.
            '12,3241': {'0163': [0]}, '12,3506': {'0163': [0]},
            # APSIDES, TIMETHET and TIMERAD after their SETPD 0: PDVL's
            # push stays below X1.
            '12,3706': {'0166': ['vac+0']}, '12,3710': {'0166': ['vac+0']}, '12,3713': {'0166': ['vac+6']},
            '12,2747': {'0166': ['vac+0']}, '12,2751': {'0166': ['vac+0']}, '12,2754': {'0166': ['vac+6']},
            '12,3561': {'0166': ['vac+0']}, '12,3563': {'0166': ['vac+0']}, '12,3566': {'0166': ['vac+6']},
        },
        # Half of GETX's cases take CSTH and SNTH so that its DDV (12,3016)
        # divides SNTH/64 by 1/4 - CSTH/2 with equal majors (both exactly
        # or a few low-word units apart), so DDV's MAXDV/MAXTEST paths run
        # in a case short enough for one C64 diagnostic.
        'joint_states': {'GETX': [{'CSTH': c, 'CSTH+1': cl, 'SNTH': sn, 'SNTH+1': sl}
                                  for sn, c in ((0o20000, 0o17400), (0o10000, 0o17600), (0o30000, 0o17200))
                                  for sl, cl in ((0, 0), (0o200, 0), (0, 0o10))]
                                 + [{}] * 9},
        # A DIFEQ or iteration run that does not fit one C64 diagnostic
        # drops every case whose run reaches at least its routines.
        'c64_drop_superset': True,
        'target': 'tests/cm_conics_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
    },
    'cm-p40-p47': {
        # GOPERF1/GOFLASH returns and other words only the display or a
        # restart enters (terminate, proceed, enter after a BANKCALL).
        'resume_orphans': True,
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'P40-P47.agc'},
        'base': {'cm': 'cm_p40_p47'},
        # Its basic code (banks 16, 20, 21, 24, 27, 42) does not fit one C64
        # diagnostic with interpretive bodies: one function per bank.
        'basic_split': True,
        # RCSDAPON ends TS FLAGWRD6 / TC Q: the TS skip past the bank's
        # last word needs an overflowing A, which CS OCT60000 / MASK / AD
        # BIT14 never makes.
        'fall_out': True,
        # S40.6's CNTR (E6,1447): +0 or -0 (pitch), -1 (yaw), +1 (restart
        # entry); OUTPUT's INDEX CNTR / CAF BIT11 reads BIT12 at -1.
        # (Physical E6 words, held under every EBANK: V97ETASK reaches
        # CSMMASS through TCR E6SETTER from E7.)
        'cells': {0o3047: [0, 0o77777, 0o77776, 1],
                  # CSMMASS (E6,1474) within the CSM's mass: V97TTASK's AD
                  # 3MDOT never overflows into the PHASCHNG operand.
                  0o3074: CSMMASS,
                  # NBRCYCLS (E6,1742), S40.13's cycle count, short of
                  # 037777: the count +1 never overflows into the OCT after
                  # TC PHASCHNG.
                  0o3342: list(range(0, 0o37777, 0o111))},
        'fixed_tables': {'BIT11': (-1, 1)},
        # P41/P40 returns to the call +1 (P41) or, INCR Q, +2 (P40).
        'call_skips': {'P41/P40': 1},
        # OUTPUT's return after FIXDELAY: TEMPR60 (1146) holds the Q its
        # QXCH saved, one of GIMDTEST's three TC OUTPUT returns.
        'states': {'20,2327': {'1146': [0o2303, 0o2305, 0o2307]}},
        'index_targets': {'42,3616': 3},
        # PRECHECK's CCS PHASE5 with PHASE5 negative runs CADR PREREAD as MASK.
        'execute_data': ('24,2573',),
        'symbol': {'cm': 'cm_p40_p47'},
        'basic': 'full',
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        'engine': 'flight-device',
        'ebank': 'listing',
        'target': 'tests/cm_p40_p47_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 P61-P67 (entry programs), basic and interpretive.
    'cm-p61-p67': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'P61-P67.agc'},
        'base': {'cm': 'cm_p61_p67'},
        'symbol': {'cm': 'cm_p61_p67'},
        'basic': 'full',
        'entries': ['@external'],
        # P61.2's RTB LOADTIME is the first order of its pair (RTB DSU):
        # the native LOADTIME, so the DSU runs on in the same routine.
        'rtb': {'LOADTIME': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'channels': 'all',
        'io_channels': 'auto',
        'engine': 'flight-device',
        # S61.1 saves its caller's return (P61's TC S61.1 at 26,2223 or
        # P62's at 26,2320) in 60GENRET (E6,1773) and RETRN1 runs it by
        # TC 60GENRET: the erasable continuation accepts those two words.
        'erasable_points': {'E6,1773': [0o2224, 0o2321]},
        # Each routine under the EBANK= in effect at it (the file switches
        # between AOG, E6, and RTINIT/DVCNTR, E7); under E6, 60GENRET holds
        # what S61.1 leaves there.
        'ebank': 'listing',
        'cells': {'E6,1773': [0o2224, 0o2321]},
        'target': 'tests/cm_p61_p67_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCMASSPROP, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-massprop': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCMASSPROP.agc'},
        'base': {'cm': 'cm_tvc_massprop'},
        'symbol': {'cm': 'cm_tvc_massprop'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        # EBANK=BZERO (E6): every erasable the file names is in E6.
        'ebank': 6,
        # LEMMASS (E6,1473) at B+16 kg within the LM's mass (0 to 15100 kg):
        # outside it the MP/DDOUBL breakpoints overflow, TS VARST0 skips
        # to TCF MASSPROP +1 with A = 1 and the original loops for ever.
        'cells': {0o3073: [0, 0o400, 0o1000, 0o1777, 0o2000, 0o3000,
                           0o4000, 0o5000, 0o6000, 0o7000, 0o7277],
                 # CSMMASS (E6,1474) within the CSM's mass (0 to 30000 kg),
                 # either side of the 33956 lb breakpoint (NEGBPW).
                 0o3074: CSMMASS,
                 # VARST0-VARST8 (E6,1511-1522) as FIXCW reads them, within
                 # +-07777 so the slope sums never overflow: on overflow
                 # TS IXX skips to BOKKEP2 with A = 1 and the original
                 # loops for ever.
                 **{0o3111 + i: VARST for i in range(9)}},
        # The constant tables MASSPROP indexes by PHI333 (9 down to 0).
        'fixed_tables': {'NOLEMVAL': (0, 9), 'SLOPEVAL': (0, 9), 'INTVALUE': (0, 9)},
        'target': 'tests/cm_tvc_massprop_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCEXECUTIVE, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-exec': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCEXECUTIVE.agc'},
        'base': {'cm': 'cm_tvc_exec'},
        'symbol': {'cm': 'cm_tvc_exec'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # VCNTR (E6,1643) counts the variable-gain passes down from 19
        # (TVCINITIALIZE); a negative count reaches TCF +0.
        'cells': {0o3243: list(range(20)) + [0o77777]},
        'target': 'tests/cm_tvc_exec_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCRESTARTS, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-restarts': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCRESTARTS.agc'},
        'base': {'cm': 'cm_tvc_restarts'},
        'symbol': {'cm': 'cm_tvc_restarts'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # EXRSTRT: TVCEXPHS (E6,1651) holds the TVCEXECUTIVE phase (1 to 6)
        # that indexes TVCEXADR's restart points.
        # TVCPHASE (E6,1644) one of the DAP phases TVCINITIALIZE and the
        # DAPs set (-2, -1, 0, 1, 2, 3): FINCOPY indexes TVCCADR by it.
        'cells': {0o3251: [1, 2, 3, 4, 5, 6],
                  0o3244: [0o77775, 0o77776, 0, 1, 2, 3]},
        'fixed_tables': {'TVCEXADR': (1, 6), 'TVCCADR': (0, 3)},
        'target': 'tests/cm_tvc_restarts_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCSTROKETEST, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-stroke': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCSTROKETEST.agc'},
        'base': {'cm': 'cm_tvc_stroke'},
        'symbol': {'cm': 'cm_tvc_stroke'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        # HACK's exits: BUNKER holds the DAP's return (QXCH BUNKER: TVCDAPS's
        # TC HACK at 20,2540, 2542, 2742, 2744) or TCTSKOVR (TC TASKOVER).
        'erasable_points': {'E6,1657': [0o2541, 0o2543, 0o2743, 0o2745, 0o5324]},
        # CARD (E6,1661) counts -4 (CS FCARD) up to -1 between sets: INDEX
        # CARD reads FCARD +1 .. +6. STROKILL leaves CARD -0 with one
        # HACKWLST still pending, which INCRs it to +1 and reads FCARD +5
        # and FCARD +8 (HACK's own words; issue #219); one more from +1
        # reads FCARD +6 and +9.
        'cells': {0o3257: [0o2541, 0o2543, 0o2743, 0o2745, 0o5324],
                  0o3261: [0o77773, 0o77774, 0o77775, 0o77776, 0o77777, 1]},
        'fixed_tables': {'FCARD': (0, 9)},
        'ebank': 6,
        # HACK is entered by the DAPs' TC HACK: Q one of their returns.
        'states': {'HACK': {'2': [0o2541, 0o2543, 0o2743, 0o2745]}},
        'externals': {},
        'target': 'tests/cm_tvc_stroke_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCINITIALIZE, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-init': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCINITIALIZE.agc'},
        'base': {'cm': 'cm_tvc_init'},
        'symbol': {'cm': 'cm_tvc_init'},
        'basic': 'full',
        # Part functions for the flight store's slots; each hand-off is a
        # routine step, so a pass runs more of them.
        'basic_parts': {'budget': 6000},
        'max_steps': 2000,
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        # EBANK=BZERO (E6) for every erasable the file names.
        'ebank': 6,
        # COEFFADR (E6,1722) holds one of the coefficient-table addresses the
        # file stores there (CSMCFADR, HBCFADR, LBCFADR) when LOADCOEF runs.
        'cells': {0o3322: [0o2500, 0o1425, 0o2512],
                  # CSMMASS (E6,1474) and EMDOT (0110, the SPS flow rate at
                  # B+3 kg/cs, about 01136) within the vehicle's range, as
                  # for TVCMASSPROP: the 10-second mass decrement then
                  # never overflows TS MASSTMP.
                  0o3074: CSMMASS, 0o110: [0, 0o400, 0o1000, 0o1136, 0o1400],
                  # The gains S40.15 multiplies (EKTLX/I E6,1416-1420,
                  # KTLX/I E6,1636, IXX/IAVG/IAVG-TLX E6,1470-1472) within
                  # +-07777, as MASSPROP and the pad load leave them: then
                  # TS 1/CONACC and TS VARK never overflow into the
                  # coefficient tables that follow TC Q.
                  **{a: VARST for a in (0o3016, 0o3017, 0o3020, 0o3236, 0o3070, 0o3071, 0o3072)}},
        # TC RTRNLOC (E6,1613) runs the Q that SWICHOVR +5 saved there: the
        # return into PRESWTCH (TC 2352) or IBNKCALL's ISWRETRN (TC 4753),
        # the only caller of SWICHOVR +5 (TVCRESTARTS).
        'erasable_points': {'E6,1613': [0o2352, 0o4753]},
        'states': {'17,2361': {'2': [0o4753]}},
        # The fixed tables INDEX CNTR (0 or 1, -CNTR for BIT2) and INDEX
        # COEFFADR (DCA 0 to CA 14D) read.
        'fixed_tables': {'CSMCFADR': (0, 1), 'TCORR': (0, 1), 'BIT2': (-1, 0),
                         'CSMN10': (0, 15), 'LBN10': (0, 15)},
        'target': 'tests/cm_tvc_init_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCROLLDAP, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-roll': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCROLLDAP.agc'},
        'base': {'cm': 'cm_tvc_roll'},
        'symbol': {'cm': 'cm_tvc_roll'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        # INDEX I (E6,1670), 0 SGNRT and the like: I is PRIO30 or BIT15,
        # so each such word runs as CA or CS.
        'index_cells': {0o1670: [0o30000, 0o40000]},
        'ebank': 6,
        'cells': {0o3270: [0o30000, 0o40000]},
        # TS TEMREG overflow protection: INDEX A (+-1), CS LIMITS (5000).
        'fixed_tables': {'5000': (-1, 1)},
        'target': 'tests/cm_tvc_roll_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 TVCDAPS, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-tvc-daps': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCDAPS.agc'},
        'base': {'cm': 'cm_tvc_daps'},
        'symbol': {'cm': 'cm_tvc_daps'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'target': 'tests/cm_tvc_daps_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 RCS-CSM_DIGITAL_AUTOPILOT, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-rcs-dap': {
        # Part functions for the flight store's 8 KB task slot; each
        # hand-off is a routine step, so a DAP pass runs more of them.
        'basic_parts': {'budget': 6000},
        'max_steps': 2000,
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'RCS-CSM_DIGITAL_AUTOPILOT.agc'},
        'base': {'cm': 'cm_rcs_dap'},
        'symbol': {'cm': 'cm_rcs_dap'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # The axis loops' SPNDX (E6,1506: 0-2) and DPNDX (E6,1507: 0, 2, 4),
        # and ATTKALMN (E6,1617) from -3 (JETSLECT's translation gains) to
        # 11 (the Kalman initialisation count, CAF ELEVEN at 21,2571).
        'cells': {0o3106: [0, 1, 2], 0o3107: [0, 2, 4],
                  0o3217: [0o77774, 0o77775, 0o77776] + list(range(12)),
                  # RMANNDX/PMANNDX/YMANNDX (E6,1656-1660) as STICKCHK
                  # leaves them (MASK THREE).
                  0o3256: [0, 1, 2, 3], 0o3257: [0, 1, 2, 3], 0o3260: [0, 1, 2, 3],
                  # RATEINDX (1130), the manual rate index into MANTABLE:
                  # 0, 2, 4 or 6, as FRESH START and P40-P47 set it.
                  0o1130: [0, 2, 4, 6]},
        'fixed_tables': {'GAIN1': (-3, 11), 'GAIN2': (-3, 11), 'DACLIMIT': (-1, 1),
                         'FREETAU': (0, 3),
                         # MANTABLE -1 indexed by RATEINDX + RMANNDX (0-6 + 0-3):
                         # 3 (both stick directions) reads =+14MS after it.
                         'MANTABLE': (0, 8)},
        'target': 'tests/cm_rcs_dap_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 AUTOMATIC_MANEUVERS, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-auto-man': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'AUTOMATIC_MANEUVERS.agc'},
        'base': {'cm': 'cm_auto_man'},
        'symbol': {'cm': 'cm_auto_man'},
        'basic': 'full',
        # Part functions for the flight store's slots; each hand-off is a
        # routine step, so a pass runs more of them.
        'basic_parts': {'budget': 6000},
        'max_steps': 2000,
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # SPNDX (E6,1506: the axis, 0-2) and RMANNDX/PMANNDX/YMANNDX
        # (E6,1656-1660: STICKCHK's MASK THREE indices), as the RCS DAP
        # leaves them.
        'cells': {0o3106: [0, 1, 2], 0o3256: [0, 1, 2, 3], 0o3257: [0, 1, 2, 3],
                  0o3260: [0, 1, 2, 3]},
        'fixed_tables': {'MINTAU': (0, 3), 'BIT13': (0, 2), 'OCT01400': (0, 2)},
        'target': 'tests/cm_auto_man_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    # Lane G: Comanche055 JET_SELECTION_LOGIC, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-jet-select': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'JET_SELECTION_LOGIC.agc'},
        'base': {'cm': 'cm_jet_select'},
        'symbol': {'cm': 'cm_jet_select'},
        'basic': 'full',
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # The jet tables INDEX XNDX/YNDX/ZNDX (0-3), PINDEX/YINDEX (0-2) and
        # NPJETS/NYJETS/NRJETS (0-3) read.
        'fixed_tables': {'XLNNDX': (0, 3), 'XLN1NDX': (0, 3), 'PYTABLE': (0, 14),
                         'RTABLE': (0, 14), 'YZTABLE': (0, 8),
                         # (negative counts after PTAUNEG/YTAUNEG: CS NPJETS)
                         'NJET': (-4, 4), 'DFTMAX': (-4, 4)},
        # BLAST/BLAST1/BLAST2 +1 (E6,1460/1462/1464) hold T6SETUP's branch
        # indices 0, 4 and 11 (REPLACER, REPLACEP, REPLACEY): INDEX them
        # before TC REPLACER.
        'index_cells': {0o1460: [0, 4, 11], 0o1462: [0, 4, 11], 0o1464: [0, 4, 11]},
        # XTRANS (E6,1631) as P40's ullage setup leaves it: +0, +1 or -1.
        'cells': {0o3060: [0, 4, 11], 0o3062: [0, 4, 11], 0o3064: [0, 4, 11],
                  0o3231: [0, 1, 0o77776]},
        'target': 'tests/cm_jet_select_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
        # Part functions for the flight store's 8 KB task slot; each
        # hand-off is a routine step, so a pass runs more of them.
        'basic_parts': {'budget': 6000},
        'max_steps': 2000,
    },
    # Lane G: Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-entry-dap': {
        'rope_fallback': True,
        'seed_counters': True,
        'missions': ('cm',),
        'file': {'cm': 'CM_ENTRY_DIGITAL_AUTOPILOT.agc'},
        'base': {'cm': 'cm_entry_dap'},
        # CM/DAPIC's TS CM/GYMDT overflows on some TIME1/PIPTIME states and
        # skips into WAITLIST's 2CADR READGYMB: the AGC runs that word as TC
        # READGYMB (its BBCON then runs as the return), and so does the port.
        'execute_data': ('20,3714', '20,3715'),
        'symbol': {'cm': 'cm_entry_dap'},
        'basic': 'full',
        # Part functions for the flight store's slots; each hand-off is a
        # routine step, so a pass runs more of them.
        'basic_parts': {'budget': 6000},
        'max_steps': 2000,
        # Every word another file names, and the file's own CADRs.
        'entries': ['@external'],
        'channels': 'all',
        'io_channels': 'auto',
        # yaAGC without its IMU and optics device models: the file loads the
        # CDU output counters and sets channel 14's drive bits itself.
        'engine': 'flight-device',
        'externals': {},
        'ebank': 6,
        # CMDAPMOD (E6,1700) is +1, +0, -0 or -1 (the DAP modes); SW/NDX
        # (0305) alternates the needles (+1, +4 .. +13) with the telemetry
        # file index (-13, -10 .. -1) in steps of three.
        'cells': {0o3300: [1, 0, 0o77777, 0o77776],
                  0o305: [1, 4, 7, 10, 13, 0o77776, 0o77773, 0o77770, 0o77765, 0o77762]},
        # The jet codes INDEX the dead-zone tags (-1, +0, +1) read, and
        # LIMITS (5000) after TS overflow.
        'fixed_tables': {'YJETCODE': (-1, 1), 'P/RJCODE': (-1, 1), '5000': (-1, 1),
                         '5001': (-1, 1)},  # HALFPR (NEG1/2 +1)
        # DOBRATE?: INDEX A (CS CM/FLAGS MASK THREE), TC +1 reaches the
        # next four words.
        'index_targets': {'15,2471': 3},
        'target': 'tests/cm_entry_dap_target.c',
        'freeze_stride': 32,
        'c64_flags': ['-fno-inline'],
    },
    'r60-r62': {
        'missions': ('lm', 'cm'),
        # The listing names the file R60,R62.agc; the repository R60_62.agc.
        'file': {'lm': 'R60,R62.agc', 'cm': 'R60,R62.agc'},
        'source_path': {'lm': 'R60_62.agc', 'cm': 'R60_62.agc'},
        'base': {'lm': 'lm_r60_r62', 'cm': 'cm_r60_r62'},
        'symbol': {'lm': 'lm_r60_r62', 'cm': 'cm_r60_r62'},
        'entries': {'lm': ['R60LEM', 'R62FLASH', 'VECPOINT', 'VECPNT1', 'BALLANGS', 'RELINUS', 'ISITAUTO', 'G+N,AUTO'],
                    'cm': ['R60CSM', 'R62DISP', 'VECPOINT', 'RELINUS']},
        # 'auto': the call's inline words are the data words after the TC.
        'externals': {'ALARM': 'auto', 'BANKCALL': 'auto', 'BLANKET': 'auto', 'CURTAINS': 'auto',
                      'DOWNFLAG': 'auto', 'UPFLAG': 'auto', 'FINDVAC': 'auto', 'IBNKCALL': 'auto',
                      'MAKECADR': 'auto', 'PHASCHNG': 'auto', 'LINUS': 'auto', 'PRIOCHNG': 'auto',
                      'CHECKMM': 'auto', 'ENDOFJOB': None, 'GOTOPOOH': None, 'GOTOV56': None,
                      'BANKJUMP': None, 'TASKOVER': None},
        # Display and restart returns (GOPERF +1/+2 and the like) land on
        # words nothing in the file reaches.
        'resume_orphans': True,
        'rtb': {'V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_VECTOR_ONE_TO_TWO)',
                            'cm': 'rtb_angles(s,io,RTB_CM,RTB_VECTOR_ONE_TO_TWO)'},
                'READCDUK': None},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/r60_r62_target.c',
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        # Job code: a DANZIG that finds NEWJOB positive stops at CHANG2, and
        # the word after it is the routine the job resumes at.
        'resume_keys': True,
        # BUF2 holds the last display's return (a fixed address), as
        # CHKLINUS's restart point (BUF2 -3) assumes; 'NNNN' is erasable.
        'all_states': {'0133': [0o2000, 0o2513, 0o3266, 0o3777]},
        'partial': {'lm': 'G+N,AUTO and ISITAUTO read channels 30 and 31, which wait for an I/O channel model',
                    'cm': 'TOBALLC reads channel 31 (RXOR CHAN31), which waits for an I/O channel model'},
        # KALCMANU's matrix loops run past 200 routines from some resume points.
        'max_steps': 1000,
        'freeze_stride': 128,
    },
    'p51-p53': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'P51-P53.agc', 'cm': 'P51-P53.agc'},
        'base': {'lm': 'lm_p51_p53', 'cm': 'cm_p51_p53'},
        'symbol': {'lm': 'lm_p51_p53', 'cm': 'cm_p51_p53'},
        # P52, P53, P54 and the R5x options are reached through P51/P57's
        # option tables. Comanche's P20-P25 CALLs ADVORB.
        'entries': {'lm': ['P51', 'P57', 'R51', 'R52', 'R55', 'R59'],
                    'cm': ['P51', 'R51', 'R52', 'R53', 'R55', 'R56', 'S50', 'ADVORB',
                           # SXTMARK POSTJUMPs to TERM52; R53 BANKCALLs R53C1.
                           'TERM52', 'R53C1']},
        'externals': {'ALARM': 'auto', 'BANKCALL': 'auto', 'BLANKET': 'auto', 'CURTAINS': 'auto',
                      'DOWNFLAG': 'auto', 'UPFLAG': 'auto', 'FINDVAC': 'auto', 'IBNKCALL': 'auto',
                      'MAKECADR': 'auto', 'PHASCHNG': 'auto', 'LINUS': 'auto', 'PRIOCHNG': 'auto',
                      'CHECKMM': 'auto', 'SWCALL': 'auto', 'SWRETURN': 'auto', 'TWIDDLE': 'auto',
                      'CLEARMRK': 'auto', 'FALTON': 'auto', 'ENDOFJOB': None, 'GOTOPOOH': None,
                      'GOTOV56': None, 'BANKJUMP': None, 'TASKOVER': None},
        # SWCALL/SWRETURN leave through the display system's saved return.
        'noreturn': ['SWCALL', 'SWRETURN', 'DANZIG'],
        'resume_orphans': True,
        # CHKSCODE returns to the caller's +1, +2 or +3.
        'returns': {'CHKSCODE': 3},
        # CATLOG is the star catalog, indexed by star code times six.
        'indexed_fixed': {'CATLOG': (-228, 0)},
        # (offsets from the operand: XUNIT -6 with X2 -6, -4 or -2; CM
        # MKDNCDR, the mark downlink ECADRs, with X2 -STARIND)
        'fixed_index': {'XUNIT': [2, 4, 6], 'MKDNCDR': [0, 1]},
        'tc_tables': {'P57JUMP+7': 4, 'P52E-4': 4, 'P52T-5': 4, 'ALIGNOPT+3': 4},
        # Basic-code RTB targets in the file, run by src/p51_p53_rtb.c.
        'native_ranges': {'RDCDUS': (12, 'p51_p53_rtb'), 'SET1/PDT': (3, 'p51_p53_rtb')},
        'rtb': {'V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_VECTOR_ONE_TO_TWO)',
                            'cm': 'rtb_angles(s,io,RTB_CM,RTB_VECTOR_ONE_TO_TWO)'},
                '1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_ONE_TO_TWO)',
                           'cm': 'rtb_angles(s,io,RTB_CM,RTB_ONE_TO_TWO)'},
                'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'},
                'SGNAGREE': {'lm': 'rtb_agreement(s,io,RTB_LM,RTB_SGN_AGREE)',
                             'cm': 'rtb_agreement(s,io,RTB_CM,RTB_SGN_AGREE)'},
                'RDCDUS': {'lm': 'p51_p53_rtb(s,io,P51_LM_RDCDUS)',
                           'cm': 'p51_p53_rtb(s,io,P51_CM_RDCDUS)'},
                'SET1/PDT': {'cm': 'p51_p53_rtb(s,io,P51_CM_SET1PDT)'},
                'READCDUK': None},
        'includes': ['rtb_angles.h', 'rtb_agreement.h', 'p51_p53_rtb.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_agreement.c', 'src/p51_p53_rtb.c'],
        'target': 'tests/p51_p53_target.c',
        'cover': True,
        'counters': True,
        'edge_values': True,
        'resume_keys': True,
        'all_states': {m: dict({'0133': [0o2000, 0o2513, 0o3266, 0o3777],
                                'BESTI': list(range(6, 229, 6)), 'BESTJ': list(range(6, 229, 6)),
                                'STARIND': [0, 1]}, **({'POSCODE': [1, 2, 3, 4, 5, 6],
                                    # (bits 1-6 the star code, 7-9 the AOT detent)
                                    'STARCODE': [c + 0o100 * d for c in range(41) for d in range(8)]} if m == 'lm' else
                                   # (the VAC area holding the marks)
                                   {'MARKSTAT': [0o401 + 0o54 * k for k in range(5)],
                                    # (star codes 1-37 and the planets 38-40 in bits 1-6)
                                    'STARCODE': [c + 0o100 * h for c in range(41) for h in (0, 1, 0o177)]}))
                       for m in ('lm', 'cm')},
        # X1/X2 land indexed erasable operands in erasable (matrix moves,
        # mark buffers, the unit-vector loops).
        # EBANK= BESTI, XSM, STARAD (LM) and SAC (CM): E5 (the AOT code sets
        # EBANK7 itself around XYMARK).
        'ebank': 5,
        # CM CATLOG ends its bank: CATLOG +1 on are the bank's checksum and
        # fillers, which only the rope holds.
        'rope_bin': True,
        'index_states': True,
        'states': p51_p53_states()[0],
        'joint_states': p51_p53_states()[1],
        # The star-pair search (up to 666 pairs of the 37 catalog stars) runs
        # past 4,000 routines.
        'max_steps': 10000,
        'freeze_stride': 192,
    },
    'attitude-maneuver': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'ATTITUDE_MANEUVER_ROUTINE.agc', 'cm': 'ANGLFIND.agc'},
        'base': {'lm': 'lm_attitude_maneuver', 'cm': 'cm_attitude_maneuver'},
        'symbol': {'lm': 'lm_attitude_maneuver', 'cm': 'cm_attitude_maneuver'},
        # Comanche's KALCMANU ends with BZMF NOGO -2 (NOGOM2's TC BANKCALL,
        # after its INHINT).
        'entries': {m: ['KALCMAN3', 'MXM3', 'TRANSPOS', 'READCDUK', 'CDUTODCM', 'DELCOMP', 'DCMTOCDU',
                        'VECOFANG', 'NOGO'] + (['NOGO-2'] if m == 'cm' else []) for m in ('lm', 'cm')},
        'externals': {'ALARM': 'auto', 'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'PHASCHNG': 'auto',
                      'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'MAKECADR': 'auto', 'CURTAINS': 'auto',
                      'WAITLIST': 'auto', 'STOPRATE': 'auto', 'LOADCDUD': 'auto', 'POSTJUMP': 1,
                      'ENDOFJOB': None, 'GOTOPOOH': None, 'BANKJUMP': None, 'TASKOVER': None},
        'noreturn': ['POSTJUMP'],
        'resume_orphans': True,
        # READCDUK is the file's own basic code; CDULOGIC is native.
        'rtb': {'CDULOGIC': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_CDU_LOGIC)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_CDU_LOGIC)'},
                'READCDUK': None},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/attitude_maneuver_target.c',
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        # Job code: a DANZIG that finds NEWJOB positive stops at CHANG2, and
        # the word after it is the routine the job resumes at.
        'resume_keys': True,
        # KALCMAN3's run reaches most of the file, more than one C64
        # diagnostic holds: the outcomes only it takes before a CHANG2
        # resume get segment rows.
        'c64_segments': True,
        # EBANK= BCDU (E6). X1 and X2 (VAC+046/047) hold the complemented
        # matrix addresses the callers' AXC orders load (MIS, KEL, MFS...).
        'ebank': 6,
        # LOOPSIN is CDUTODCM's TIX loop: X1 3, 2 or 1 with S1 1, and the
        # pushdown list at 0D (its SETPD 0), as the loop's pushes assume.
        # SUHALFA and CALCPHI (DCMTOCDU's THETA quadrant fix) run with PSI
        # and THETA pushed (pushdown at 4D).
        # The job's resume points in CDUTODCM before and inside the loop
        # take the same index registers, with the pushdown list where the
        # loop's pushes leave it (four words an iteration).
        'states': cdutodcm_states(),
        # PUSHLOC low enough that the pushes a resumed MXM3 or DCMTOCDU
        # still makes stay below X1 and X2 (VAC+046).
        'all_states': {'0166': ['vac+%o' % n for n in range(0, 0o24, 2)],
                       'vac+046': {'lm': [0o77777 ^ a for a in (0o3246, 0o2230, 0o3303)],
                                   'cm': [0o77777 ^ a for a in (0o3266, 0o3320, 0o3425)]},
                       'vac+047': {'lm': [0o77777 ^ a for a in (0o3246, 0o2230, 0o3303)],
                                   'cm': [0o77777 ^ a for a in (0o3266, 0o3320, 0o3425)]}},
        # Half the cases start from one MFI or COF set (anglfind_joint).
        'joint_states': {'*': anglfind_joint()},
        # -fno-inline keeps KALCMAN3's whole path in one diagnostic.
        'c64_flags': ['-fno-inline'],
        # KALCMANU's matrix loops run past 200 routines from some resume points.
        'max_steps': 1000,
        'freeze_stride': 256,
    },
    'gimbal-lock': {
        # (-DIRC_INPUTS: the input-latch code its VICE PRGs were built with)
        'c64_flags': ['-DIRC_INPUTS'],
        'missions': ('lm', 'cm'),
        'file': {'lm': 'GIMBAL_LOCK_AVOIDANCE.agc', 'cm': 'GIMBAL_LOCK_AVOIDANCE.agc'},
        'base': {'lm': 'lm_gimbal_lock', 'cm': 'cm_gimbal_lock'},
        'symbol': {'lm': 'lm_gimbal_lock', 'cm': 'cm_gimbal_lock'},
        # LOCSKIRT (LM NOGIMLOC, CM WCALC): KALCMANU's GOTO and BPL targets.
        'entries': {'lm': ['NOGIMLOC', 'WCALC'], 'cm': ['WCALC']},
        'externals': {},
        'target': 'tests/gimbal_lock_target.c',
        # KALCMANU runs as a job: any DANZIG can find a job waiting.
        'resume_keys': True,
        # EBANK= BCDU (E6).
        'ebank': 6,
        # ARATE,1 reads one of four rates: the KX window keeps X1 in it.
        'index_guard': True,
        # RATEINDX (LM 01325, CM 01130) picks the rate (0, 2, 4 or 6);
        # after LXC,1 X1 is its complement.
        'all_states': {'01325': [0, 2, 4, 6], '01130': [0, 2, 4, 6],
                       'vac+046': [0o77777 ^ r for r in (0, 2, 4, 6)]},
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        # Rows by gcov arc as well (the ?: outcomes clang folds).
        'c64_rows': 'cover',
        'cover_stride': 8,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 32,
    },
    'lm-orbital-integration': {
        'missions': ('lm',),
        'file': {'lm': 'ORBITAL_INTEGRATION.agc'},
        'base': {'lm': 'lm_orbital_integration'},
        'symbol': {'lm': 'lm_orbital_integration'},
        # Entered from INTEGRATION_INITIALIZATION (RVCON's CALL KEPPREP,
        # RECTIFY, MINIRECT, POOHCHK's GOTO TIMESTEP) and INITVEL (SETITCTR).
        'entries': ['KEPPREP', 'RECTIFY', 'MINIRECT', 'TIMESTEP', 'SETITCTR', 'AMOVED'],
        'noreturn': ['POODOO'],
        'externals': {'POODOO': 1, 'PHASCHNG': 1, 'UPFLAG': 1, 'ALARM': 1},
        'cgoto': {'DIFEQTAB': 3},
        # Indexed operands into bank 13's constants: the index is the body
        # (X2 = PBODY, 0 earth or 2 moon) or ASCALE's NORM count.
        'indexed_fixed': {'MUEARTH': (-4, 0), 'MUM': (-4, 4), 'J2REQSQ': (-4, 0), 'J4REQ/J3': (-4, 0),
                          '2J3RE/J2': (-4, 0), 'RDE': (-4, 0), 'ASCALE': (-16, 2), 'LODPMAX1': (-4, 0)},
        # CHANG2 resumes mid-sequence keep the sequence's own invariants:
        # between ACCOMP's XCHX,2 DIFEQCNT pairs X2 is the DIFEQ stage and
        # DIFEQCNT holds PBODY; between GAMCOMP's XCHX,2 S1 pairs S1 holds it;
        # NBRANCH's DMP CGOTO has DIFEQCNT SLOADed (DP; stage 0 here, NBRANCH
        # itself runs every stage); TIMESTEP's W
        # stores index by COLREG (1117), after a VLOAD.
        'states': {**{'ACCOMP+%d' % n: {'1500': [0, 2], 'vac+047': [0, 0o77763, 0o77747]} for n in (15, 16, 64, 65, 66)},
                   **{'GAMCOMP+%d' % n: {'vac+050': [0, 2]} for n in (24, 27)},
                   'NBRANCH+3': {'1500': [0], '0154': [0], '0155': [0], '0163': [0], 'vac+046': [0]},
                   **{'TIMESTEP+%d' % n: {'vac+047': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717],
                                          '0163': [0o77776]} for n in (178, 179, 180, 182, 184)}},
        'all_states': {'2030': [0, 2], '1117': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717], '1500': [0, 0o77763, 0o77747], 'vac+046': [0, 2], 'vac+047': [0, 2],
                       'vac+050': [0, 2, 4, 6], 'vac+051': [0, 2, 4, 6],
                       '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        # BOVB TCDANZIG: TCDANZIG is ENDT(X) in TIME_OF_FREE_FALL, TC DANZIG.
        'rtb': {'TCDANZIG': {'lm': 'lm_integration_tcdanzig(s)'}, 'ENDT(X)': {'lm': 'lm_integration_tcdanzig(s)'}},
        'includes': ['rtb_angles.h', 'lm_integration_moves.h'],
        'native_sources': ['src/lm_integration_moves.c'],
        'target': 'tests/lm_orbital_integration_target.c',
        # (with resume keys every pair is a routine: a DIFEQ step runs hundreds)
        'max_steps': 1000,
        # Integration runs as a job: any DANZIG can find a job waiting.
        'resume_keys': True,
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        # (bank checksums the listing omits, such as OCT27's DSU second word)
        # Flight edge values (review #165): tools/edge_cases.py's shape,
        # TIME2/TIME1 and the CDUs, and integration_joint's sets.
        'edge_cases': True,
        'counters': True,
        'joint_states': {'*': integration_joint()},
        # A CHANG2 resume starts from the state the program reaches it in
        # (MODE DP after a DLOAD; review #165).
        'resume_flow': True,
        # The C64 rows cover every branch arc of the host cases.
        'c64_rows': 'cover',
        # Arcs only runs too long for one diagnostic take come from
        # truncated rows ('c64_cuts').
        'c64_cuts': True,
        'rope_bin': True,
    },
    'lm-integration-init': {
        'missions': ('lm',),
        'file': {'lm': 'INTEGRATION_INITIALIZATION.agc'},
        'base': {'lm': 'lm_integration_init'},
        'symbol': {'lm': 'lm_integration_init'},
        'entries': ['INTEGRV', 'INTEGRVS', 'LEMPREC', 'CSMPREC', 'LEMCONIC', 'CSMCONIC', 'INTSTALL',
                    'SETIFLGS', 'INTWAKE', 'INTWAKE0', 'INTWAKE1', 'TESTLOOP', 'INTEGRV2', 'ATOPCSM',
                    'ATOPLEM', 'CLRMOON', 'SETMOON', 'CKMID2', 'GRP2PC', 'A-PCHK',
                    'STATEINT', 'STATINT1', 'AVETOMID', 'MIDTOAV1', 'MIDTOAV2', 'INTWAKEU',
                    'ENDINT', 'PHEXIT'],
        # The MOVE* routines and their SETBANK (with FORTYONE, the A it
        # returns, and INTBANK) run as lm_integration_move behind RTB.
        'native_ranges': {'MOVEACSM': (9, 'lm_integration_move'), 'MOVEPCSM': (9, 'lm_integration_move'),
                          'MOVEALEM': (9, 'lm_integration_move'), 'MOVEPLEM': (9, 'lm_integration_move'),
                          'SETBANK': (5, 'lm_integration_move'), 'FORTYONE': (1, 'lm_integration_move')},
        # MEASUREMENT_INCORPORATION's RTBs reach the MOVE* routines directly.
        'native_entries': {'MOVEACSM': {'lm': 'lm_integration_move(s,io,LM_MOVE_ACSM)'},
                           'MOVEPCSM': {'lm': 'lm_integration_move(s,io,LM_MOVE_PCSM)'},
                           'MOVEALEM': {'lm': 'lm_integration_move(s,io,LM_MOVE_ALEM)'},
                           'MOVEPLEM': {'lm': 'lm_integration_move(s,io,LM_MOVE_PLEM)'}},
        # Their DANZIG resumes the RTB that called them as its pair's right
        # order: LOC its address word (this file's own RTBs; MOVEPCSM's are
        # left orders, so a word of bank 14), BANKSET its bank, INTBIT15 0.
        # EDOP: +0, or the right order a left-hand RTB leaves (MEASUREMENT_
        # INCORPORATION's RTB GOTO and RTB CALL: 0127, 0153), which DANZIG
        # takes at OPJUMP.
        'states': {lab: {'0164': [loc], '0165': [bank | e for e in range(8)], '0115': [0],
                         '0023': [0, 0, 0o127, 0o153]}
                   for lab, loc, bank in (('MOVEACSM', 0o2663, 0o26000), ('MOVEPCSM', 0o2500, 0o30000),
                                          ('MOVEALEM', 0o2736, 0o26000), ('MOVEPLEM', 0o2763, 0o26000))},
        'noreturn': ['BANKJUMP', 'ENDOFJOB', 'TASKOVER', 'JOBSLEEP'],
        'externals': {'PHASCHNG': 1, 'FINDVAC': 2, 'TASKOVER': 0, 'LONGCALL': 2, 'ENDOFJOB': 0,
                      'DOWNFLAG': 1, 'UPFLAG': 1, 'JOBSLEEP': 0, 'JOBWAKE': 0, 'TPAGREE': 0,
                      'BANKJUMP': 0, 'ALARM': 1},
        # Indexed operands into bank 13's constants: the index is the body
        # (X2 = PBODY, 0 earth or 2 moon) or ASCALE's NORM count.
        'indexed_fixed': {'MUEARTH': (-4, 0), 'MUM': (-4, 4), 'J2REQSQ': (-4, 0), 'J4REQ/J3': (-4, 0),
                          '2J3RE/J2': (-4, 0), 'RDE': (-4, 0), 'ASCALE': (-16, 2), 'LODPMAX1': (-4, 0)},
        # STATEINT (from P00 and V37) runs INTSTALL under a NOVAC job: its
        # EXIT, the stall test and GOBAC's RVQ back to the caller.
        'novac': ['INTSTALL', 'INTSTALL+1', 'GOBAC+1'],
        'all_states': {'1167': [0o5155], '2030': [0, 2], '1500': [0, 0o77763, 0o77747], 'vac+046': [0, 2], 'vac+047': [0, 2],
                       '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        # TBASE2 holds INTWAKE's saved QPRET, which RVQ at GOBAC returns to.
        'return_words': ['1055'],
        'rtb': {'TCDANZIG': {'lm': 'lm_integration_tcdanzig(s)'}, 'ENDT(X)': {'lm': 'lm_integration_tcdanzig(s)'},
                'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'},
                'SGNAGREE': {'lm': 'rtb_agreement(s,io,RTB_LM,RTB_SGN_AGREE)'},
                'SIGNMPAC': {'lm': 'rtb_agreement(s,io,RTB_LM,RTB_SIGN_MPAC)'},
                'MOVEACSM': {'lm': 'lm_integration_move(s,io,LM_MOVE_ACSM)'},
                'MOVEALEM': {'lm': 'lm_integration_move(s,io,LM_MOVE_ALEM)'},
                'MOVEPCSM': {'lm': 'lm_integration_move(s,io,LM_MOVE_PCSM)'},
                'MOVEPLEM': {'lm': 'lm_integration_move(s,io,LM_MOVE_PLEM)'}},
        'includes': ['rtb_angles.h', 'rtb_agreement.h', 'lm_integration_moves.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_agreement.c', 'src/lm_integration_moves.c'],
        'target': 'tests/lm_integration_init_target.c',
        # (with resume keys every pair is a routine: a DIFEQ step runs hundreds)
        'max_steps': 1000,
        # Integration runs as a job: any DANZIG can find a job waiting.
        'resume_keys': True,
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        # (bank checksums the listing omits, such as OCT27's DSU second word)
        # Flight edge values (review #165): tools/edge_cases.py's shape,
        # TIME2/TIME1 and the CDUs, and integration_joint's sets.
        'edge_cases': True,
        'counters': True,
        'joint_states': {'*': integration_joint()},
        # A CHANG2 resume starts from the state the program reaches it in
        # (MODE DP after a DLOAD; review #165).
        'resume_flow': True,
        # The C64 rows cover every branch arc of the host cases.
        'c64_rows': 'cover',
        'rope_bin': True,
    },
    'rtb-op-codes': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'RTB_OP_CODES.agc', 'cm': 'RTB_OP_CODES.agc'},
        'base': {'lm': 'lm_rtb_op_codes', 'cm': 'cm_rtb_op_codes'},
        'symbol': {'lm': 'lm_rtb_op_codes', 'cm': 'cm_rtb_op_codes'},
        'entries': {'lm': ['LOADTIME', 'CDULOGIC', 'SGNAGREE', '1STO2S', 'V1STO2S', 'TPMODE',
                           '2V1STO2S', 'CDUINC', 'PULSEIMU', 'SIGNMPAC', 'DPMODE',
                           'NORMUNX1', 'NORMUNIT', 'VECSGNAG'],
                    'cm': ['LOADTIME', 'CDULOGIC', 'READPIPS', 'VECMODE', 'SGNAGREE', '1STO2S',
                           'V1STO2S', 'TPMODE', '2V1STO2S', 'INCRCDUS', 'CDUINC', 'PULSEIMU',
                           'TRANSP1', 'TRANSP2', 'SIGNMPAC', 'DPMODE', 'NORMUNX1', 'NORMUNIT',
                           'VECSGNAG', 'QUALITY1']},
        'externals': {'BANKCALL': 'auto', 'POSTJUMP': 'auto', 'TPAGREE': 0, 'DANZIG': 0},
        'noreturn': ['DANZIG', 'POSTJUMP'],
        # 1TO2SUB's and CDUINC's INDEX A; CAF LIMITS after a TS overflow skip:
        # A is +1 or -1.
        'basic_fixed_index': {'LIMITS': [-2, -1, 1]},
        'target': 'tests/rtb_op_codes_target.c',
        # RTB runs in NOVAC jobs too: the routines that never name FIXLOC
        # take any FIXLOC.
        # (INDEX LIMITS -2/-1 sums carry end-around on the C64's 16-bit
        # unsigned too)
        'index_carry16': True,
        'any_fixloc': True,
        # CDUINC updates the angle BUF names: THETAD, +1 or +2 (INCRCDUS
        # steps BUF through them; nothing in Luminary calls CDUINC).
        'all_states': {'BUF': {'lm': [0o321, 0o322, 0o323], 'cm': [0o1155, 0o1156, 0o1157]}},
        # After VECAGREE, NORMUNIT's MPAC +2 is FIXLOC or FIXLOC +1 (NORMUNX1):
        # INDEX MPAC +2; TS 37D stores the shift count in X1 or X2's word.
        'states': {'NORMUNIT+5': {'0156': ['vac+0', 'vac+1']}},
        'resume_keys': True,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 32,
    },
    'sband-antenna': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'S-BAND_ANTENNA_FOR_LM.agc', 'cm': 'S-BAND_ANTENNA_FOR_CM.agc'},
        'base': {'lm': 'lm_sband_antenna', 'cm': 'cm_sband_antenna'},
        'symbol': {'lm': 'lm_sband_antenna', 'cm': 'cm_sband_antenna'},
        'entries': ['SBANDANT'],
        'externals': {'BANKCALL': 'auto', 'PRIOCHNG': 'auto', 'BLANKET': 'auto', 'B5OFF': None,
                      'ENDOFJOB': None, 'ENDEXT': None},
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # Without inlining the runs that take every branch outcome fit one
        # diagnostic (the C64 cover misses none).
        'c64_flags': ['-fno-inline', '-DIRC_INPUTS'],
        'target': 'tests/sband_antenna_target.c',
        # V64 runs SBANDANT as a job: any DANZIG can find a job waiting.
        'resume_keys': True,
        # GOMARKFR's returns (terminate, proceed, recycle, immediate) land on
        # words nothing in the file reaches.
        'resume_orphans': True,
        # X2 (VAC+39D) picks the sphere for SLOAD BHIZ X2: 0 Earth, 2 Moon,
        # and -0.
        'all_states': {'vac+047': [0, 2, 0o77777]},
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        # Rows by gcov arc (the ?: outcomes clang folds) and per call site:
        # each UNIT/ASIN/ACOS/BPL site takes its own outcomes of the shared
        # bodies (tools/irc_edges.c IRC_SITES).
        'c64_rows': 'cover',
        'cover_stride': 8,
        'site_edges': True,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 32,
    },
    'cm-imu-calibration': {
        'missions': ('cm',),
        'file': {'cm': 'IMU_CALIBRATION_AND_ALIGNMENT.agc'},
        'base': {'cm': 'cm_imu_calibration'},
        'symbol': {'cm': 'cm_imu_calibration'},
        # ALLOOP: the task TWIDDLE starts (TC TWIDDLE; ADRES ALLOOP).
        'entries': ['IMUTEST', 'FINETIME', 'EARTHR', 'PROUT', 'P11OUT', 'GCOMPVER', 'RESTEST1', 'ALLOOP'],
        'externals': {'PRIOCHNG': 'auto', '2PHSCHNG': 'auto', 'ALARM': 'auto', 'BANKCALL': 'auto', 'CHANG1': 'auto', 'CHECKMM': 'auto', 'DOWNFLAG': 'auto', 'FINDVAC': 'auto', 'JOBWAKE': 'auto', 'LONGCALL': 'auto', 'NEWMODEA': 'auto', 'NEWMODEX': 'auto', 'NEWPHASE': 'auto', 'PHASCHNG': 'auto', 'POSTJUMP': 1, 'TWIDDLE': 'auto', 'ENDEXT': None, 'POODOO': None, 'ENDMARK': None, 'ENDOFJOB': None, 'JOBSLEEP': None, 'TASKOVER': None},
        'noreturn': ['POSTJUMP'],
        'resume_keys': True,
        'resume_orphans': 'all',
        'rtb': {'LOADTIME': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'},
                '1STO2S': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_ONE_TO_TWO)'},
                'CDULOGIC': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_CDU_LOGIC)'},
                'PULSEIMU': None, 'SGNAGREE': None},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # Without inlining the runs that take every branch outcome fit one
        # diagnostic (the C64 cover misses none).
        'c64_flags': ['-fno-inline', '-DIRC_INPUTS'],
        'target': 'tests/cm_imu_calibration_target.c',
        # EBANK= POSITON and XSM: the file's erasables are in E5.
        'ebank': 5,
        # CHECKG1 spins until a PIPA pulse moves PIPAX, FINETIME +1 while
        # LOSCALAR reads 37777 (the scalers are latched for a run).
        'idle': ['CHECKG1', 'FINETIME+1'],
        # FINETIME reads the scalers and CHKCOMED channel 30 (liftoff).
        'channel_inputs': True,
        # INDEX POSITON and the like must leave a 12-bit operand erasable.
        'index_guard': True,
        # PIPACHK's and PIPJOBB's INDEX NDXCTR; TC +1 (NDXCTR 0 or 1).
        'tc_tables': {'PIPACHK+1': 2, 'PIPJOBB+1': 2},
        # ONCEMORE's TC LAT(SPL) (E7) calls the erasable program the
        # performance test loads.
        'erasable_calls': [0o1400],
        # The states the test's own code leaves (E5 words at 02400 + their
        # offset from 01400): small table indexes, CCS classes, and small
        # index registers and steps for the ALFLT loops.
        'all_states': {
            '02423': list(range(8)),                                  # POSITON
            '02422': [0, 1, 2],                                       # PIPINDEX
            '02547': list(range(6)),                                  # RESULTCT
            '02421': [0, 1],                                          # NDXCTR: 1 in gimbal lock
            '02632': [0, 1, 2],                                       # PREMTRXC
            '02412': [0, 1, 7, 0o77777, 0o77770],                     # LENGTHOT
            '02450': [0, 1, 0o37777],                                 # TORQNDX: +0 or POSMAX
            '02540': [0, 1, 7, 0o77777, 0o77770],                     # ALTIM
            '02643': [0, 1],                                          # GEOCOMP1: 1 in compass
            '02454': [12 * n for n in range(13)],                     # ALX1S: 144 down by 12
            '02455': [1, 0o77776],                                    # CMPX1: +-1 alternately
            '01304': [0, 1, 0o4000],                                  # LGYRO: free or a job
            '02427': [0, 2],                                          # RUN: target 1 or 2
            '02431': [0, 6],                                          # TARG1/2
            'vac+046': [n for n in range(0o20)] + [0o77777 ^ n for n in range(0o20)],
            'vac+047': [n for n in range(0o20)] + [0o77777 ^ n for n in range(0o20)],
            'vac+050': [1, 2, 3, 6], 'vac+051': [1, 2, 3, 6],
            '01330': [0, 0o401, 0o455, 0o531, 0o605, 0o661],            # MARKSTAT: a mark VAC or none
            # The saved returns: the file's own call sites.
            '02425': [0o2012, 0o2033, 0o2102, 0o2107, 0o2131, 0o2146, 0o2202, 0o2226, 0o2227,
                      0o2267, 0o2426, 0o3070, 0o3167, 0o3356],        # QPLACE
            '02426': [0o2134, 0o2173, 0o2251, 0o2527, 0o3320],        # QPLACES
            '02424': [0o3545, 0o3572]},                               # QPLAC
        # The ALFLT filter's loops (33,2645-3033) run with S1 4 and S2 2
        # (INTVAL) and their own index values; a wider index stores over
        # CMPX1 or ALX1S and the next LXC/LXA loads data as an index.
        'range_states': [
            ('NORMLOP', 'PERFERAS', {'vac+050': [4], 'vac+051': [2], '00166': ['vac+0', 'vac+2']}),
            ('ALKCG2', 'ALFLT3', {'vac+046': list(range(2, 146, 2)), 'vac+047': list(range(2, 14, 2))}),
            ('DELMLP', 'ALILP', {'vac+046': [4, 8]}),
            ('ALILP', 'ALKLP', {'vac+047': [2, 4]}),
            ('ALKLP', 'LOOSE', {'vac+046': [1, 0o77776], 'vac+047': [2, 4, 6, 8]}),
            ('LOOSE', 'BOOP', {'vac+046': [4, 8]}),
            ('BOOP', 'PERFERAS', {'vac+046': [2, 4, 8], 'vac+047': [2, 4, 6]}),
            # The compass and erection loop: X1 8 by 4 from SETPD 0.
            ('ALWAYSG', 'COMPGS', {'vac+046': [4, 8], 'vac+050': [4], '00166': ['vac+0', 'vac+2']}),
            # TAR1: X1 2 and X2 12, then 0 and 6, S2 6.
            ('TAR1', 'CONTIN33', {'vac+046': [0, 2], 'vac+047': [6, 12], 'vac+051': [6], '00166': ['vac+0', 'vac+2']}),
            # EARTHR keeps its CALL return in S2 (ITA S2) for GOTO S2.
            ('EARTHR', 'EARTHR*', {'vac+051': ['outside']}),
            # LITTLSUB keeps its CALL return (a CADR) in QPLAC (STQ) for GOTO QPLAC.
            ('LITTLSUB', 'AZMTHCG1', {'02424': [0o67562, 0o67575]})],
        # STOVL S1 stores INTVAL's 4 and 2; TARGDRVE's callers pass 0 or 6.
        'states': {'NORMLOP+3': {'00154': [4], '00155': [2]}, 'TARGDRVE': {'00000': [0, 6]}},
        # CHECKG1's spin: PIPAX has not moved since TS ZERONDX, so C(PIPAX +
        # PIPINDEX) + ZERONDX is +-0 and BZF CHECKG1 is taken (the idle
        # point). Half the sets hold that (with the +0/-0 pairs), half miss
        # by one; points before TS ZERONDX overwrite it.
        'joint_states': {('CHECKG1+%d' % k if k else 'CHECKG1'): [
            {'02422': i, '%05o' % (0o37 + i): v, '02655': (~v - miss) & 0o77777}
            for i in (0, 1, 2) for v in (0, 0o77777, 1, 0o77776, 0o1234, 0o54321) for miss in (0, 1)]
            for k in range(15)},
        # One case per routine: the cases carry many edits, and the header
        # must stay under 1 MB.
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        # Rows by gcov arc as well (the ?: outcomes clang folds, as DIM's
        # negative operand in ai_dim).
        'c64_rows': 'cover',
        'cover_stride': 64,
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 640,
    },
    'lm-p40-p47': {
        'missions': ('lm',),
        'file': {'lm': 'P40-P47.agc'},
        'base': {'lm': 'lm_p40_p47'},
        'symbol': {'lm': 'lm_p40_p47'},
        'entries': ['P40LM', 'P41LM', 'P42LM', 'P47LM', 'POSTBURN', 'TIGNOW', 'REDO6.7', 'CALCN85',
                    'P42STAGE', 'IMPLBURN', 'ENGOFTSK', 'ENGINOF1', 'ENGINOF2', 'ENGINOF3', 'ENGINOF4',
                    'S40.8', 'S40.13', 'TRIMGIMB', 'S40.1', 'S40.2,3', 'S40.9', 'S41.1', 'UPDATEVG',
                    # dead code after a GOTO, kept as its own routine
                    'S40.133'],
        'externals': {'ALARM': 'auto', 'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'PHASCHNG': 'auto',
                      '2PHSCHNG': 'auto', 'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'FINDVAC': 'auto',
                      'NOVAC': 'auto', 'WAITLIST': 'auto', 'TWIDDLE': 'auto', 'FIXDELAY': 'auto',
                      'VARDELAY': 'auto', 'PRIOCHNG': 'auto', 'TPAGREE': 'auto', 'NOULLAGE': 'auto',
                      'POSTJUMP': 1, 'TASKOVER': None, 'ENDOFJOB': None, 'GOTOPOOH': None,
                      'P40ALM': None, 'REDO4.2': None, 'BURNBABY': None, 'B*RNB*B*': None, 'ISWRETRN': None},
        # P40-P47 run as jobs: any DANZIG can find a job waiting.
        'resume_keys': True,
        'noreturn': ['POSTJUMP'],
        'resume_orphans': True,
        # WAND/WOR CHAN12, RAND/WRITE DSALMOUT and WOR CHAN14 wait for the
        # output-channel model.
        'partial': {'lm': 'WAND/WOR CHAN12, RAND/WRITE DSALMOUT and WOR CHAN14 wait for an output-channel model'},
        # RTB routines without a native body: the run ends at them.
        'rtb': {'NORMUNX1': None},
        'target': 'tests/lm_p40_p47_target.c',
        'freeze_stride': 640,
    },
    'lm-r63': {
        'missions': ('lm',),
        'file': {'lm': 'R63.agc'},
        'base': {'lm': 'lm_r63'},
        'symbol': {'lm': 'lm_r63'},
        # V89CALL (the extended verb) and the recycle point V89RECL.
        'entries': ['V89CALL', 'V89RECL'],
        'externals': {'BANKCALL': 'auto', 'DOWNFLAG': 'auto', 'ENDEXT': None, 'ENDMARK': None},
        'returns': {'GOFLASH': 3},
        'resume_keys': True,
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'}, 'NORMUNIT': None},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # Flight edge values, and C64 rows chosen by branch outcome over
        # every linked source.
        'edge_cases': True,
        'c64_rows': 'cover',
        'target': 'tests/lm_r63_target.c',
        'freeze_stride': 16,
    },
    'lm-p12': {
        'missions': ('lm',),
        'file': {'lm': 'P12.agc'},
        'base': {'lm': 'lm_p12'},
        'symbol': {'lm': 'lm_p12'},
        # P12LM (the major mode); P12RET (ASCENT's return); P12INIT,
        # COMMINIT and GUIDINIT, which P70 and P71 call.
        'entries': ['P12LM', 'P12LMB', 'P12RET', 'P12INIT', 'COMMINIT', 'GUIDINIT'],
        'externals': {'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'PHASCHNG': 'auto', 'UPFLAG': 'auto',
                      'DOWNFLAG': 'auto', 'CLRADMOD': 'auto', 'POSTJUMP': 1, 'GOTOPOOH': None},
        'noreturn': ['POSTJUMP'],
        'returns': {'GOFLASH': 3},
        'resume_keys': True,
        'resume_orphans': True,
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'edge_cases': True,
        'c64_rows': 'cover',
        'target': 'tests/lm_p12_target.c',
        'freeze_stride': 32,
    },
    'lm-p70-p71': {
        'missions': ('lm',),
        'file': {'lm': 'P70-P71.agc'},
        'base': {'lm': 'lm_p70_p71'},
        'symbol': {'lm': 'lm_p70_p71'},
        # R10,R11 (the Waitlist task), P70 and P71 (the major modes),
        # GOABORT (4.3SPOT's restart point), P70INIT, P71RET, and TGOCOMP,
        # THROTUP and LEGAL? (subroutines other files call).
        'entries': ['R10,R11', 'P70', 'P71', 'P70A', 'P71A', 'CONTABRT', 'ABRTJASK', 'GOABORT', 'P70INIT',
                    'P71RET', 'INJTARG', 'TGOCOMP', 'THROTUP', 'LEGAL?'],
        'externals': {'TASKOVER': None, 'LRHTASK': None, 'TWIDDLE': 'auto', 'FLIP': 'auto', 'LANDISP': None,
                      'CLRADMOD': 'auto', 'POSTJUMP': 1, 'CHECKMM': 1, 'DOWNFLAG': 'auto', 'UPFLAG': 'auto',
                      'PHASCHNG': 'auto', 'BANKCALL': 'auto', 'ENDOFJOB': None, 'FALTON': 'auto',
                      'RELDSP': 'auto'},
        'noreturn': ['POSTJUMP'],
        'resume_keys': True,
        'resume_orphans': True,
        # ABRTJASK's INDEX Q; CAF MODE70: Q is -0 (P70) or 2 (P71).
        'basic_fixed_index': {'MODE70': [0, 2]},
        # BOTHPOLY's ABTCOF,1 with X1 -0 (P70, AXC,1 0D) or -8 (P71, AXC,1
        # 8D): the DPS or the APS coefficients, eight words on.
        'states': {lab: {'vac+046': [0, 0o77767]} for lab in ('BOTHPOLY', 'BOTHPOLY+3', 'BOTHPOLY+6', 'BOTHPOLY+9')},
        'partial': {'lm': 'READ CHAN30, RAND/WRITE DSALMOUT and WOR CHAN14 wait for the channel model'},
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'edge_cases': True,
        'c64_rows': 'cover',
        'target': 'tests/lm_p70_p71_target.c',
        'freeze_stride': 32,
    },
    'lm-llge': {
        'missions': ('lm',),
        'file': {'lm': 'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'},
        'base': {'lm': 'lm_llge'},
        'symbol': {'lm': 'lm_llge'},
        # LUNLAND (BURNBABY's AVEGEXIT), ?GUIDSUB (P63's guidance passes),
        # PITFALL (RUPT10), DESCBITS (AOTMARK's CADR), P63DISPS -2 (the
        # display restart), INTPRETX and ROOTPSRS (subroutines).
        'entries': ['LUNLAND', '?GUIDSUB', 'GUILDEN', 'PITFALL', 'DESCBITS', 'P63DISPS-2', 'INTPRETX', 'ROOTPSRS',
                    'FASTCHNG', 'RODTASK', 'STARTP66', 'STARTP67', 'P65START', 'STARTP64', 'REDESMON'],
        'externals': {'PHASCHNG': 'auto', 'BANKCALL': 'auto', 'POSTJUMP': 1, 'CHECKMM': 1, 'NEWMODEX': 'auto',
                      'DOWNFLAG': 'auto', 'UPFLAG': 'auto', 'ENDOFJOB': None, 'TASKOVER': None, 'ALARM': 'auto',
                      'POODOO': None, 'THROTTLE': 'auto', 'WAITLIST': 'auto', 'NOVAC': 'auto', 'FINDVAC': 'auto',
                      'BANKJUMP': None, 'RESUME': None, 'SHORTMP': 'auto', 'USPRCADR': 'auto', 'DMPNSUB': 'auto', 'POWRSERS': 'auto', 'TWIDDLE': 'auto', 'VARDELAY': 'auto', 'LOW7+2K': 'auto'},
        'noreturn': ['POSTJUMP'],
        'resume_keys': True,
        'resume_orphans': True,
        # A resume LOC starts in the MODE, PUSHLOC and index registers the
        # file's own runs bring there (VSU and VXV keep the MODE they find).
        'flow_states': True,
        # INDEX WCHPHASE (or WCHPHOLD); TCF table: WCHPHASE is -1 (IGNALG),
        # 0 (BRAKQUAD), 1 (APPRQUAD) or 2 (VERTICAL), as each table holds.
        # WHATEXIT +2 and WHATDISP -1 are the neighbouring table's words.
        'index_tcf': {'NEWPHASE': [-1, 0, 1, 2], 'PREGUIDE': [-1, 0, 1, 2], 'WHATGUID': [-1, 0, 1, 2],
                      'AFTRGUID': [-1, 0, 1, 2], 'WHATEXIT': [-1, 0, 1, 2], 'WHATDISP': [-1, 0, 1, 2],
                      'WHATALM': [-1, 0, 1]},
        # The file runs under EBANK= E2DPS (E7); WCHPHASE and WCHPHOLD hold a
        # phase (-1 to 2) in every case.
        'ebank': 7,
        # X1, set only by INTPRETX, is -TARGTDEX: -0, -034 or +1.
        'all_states': {'WCHPHASE': [0o77776, 0, 1, 2], 'WCHPHOLD': [0o77776, 0, 1, 2],
                       'vac+046': [0o77777, 0o77743, 1],
                       # PUSHLOC below 32D (the file's own pushdown words), so
                       # a push never reaches X1.
                       '0166': ['vac+%o' % n for n in range(0, 0o40, 2)],
                       # WCHVERT (P67 positive, P66 +0, P65 negative), which
                       # is also NGUIDSUB (passes left: 2 to 0).
                       'WCHVERT': [2, 1, 0, 0o77776],
                       # NIGNLOOP, the ignition loops left (THE_LUNAR_LANDING
                       # counts it down from 40).
                       'NIGNLOOP': [0, 1, 0o50]},
        # QUADGUID sets MODE to 0 before its TC INTPRETX; the resume LOCs
        # before a NORMUNIT with no load of their own are in vector mode.
        # RGVGCALC resets PUSHLOC to FIXLOC, and TTF/8CL pushes nothing, so
        # QUADGUID's interpretive code starts with an empty pushdown list.
        'states': {'QUADGUID+32': {'0163': [0], '0166': ['vac+0']}, 'CGCALC+24': {'0163': [0o77776]},
                   'CGCALC+26': {'0163': [0o77776]}, 'CGCALC+29': {'0163': [0o77776]},
                   'EXNORM+4': {'0163': [0o77776]},
                   # ROOTPSRS takes TABLTTFL (TABLTTF +3) in A and N-1 in L;
                   # inside it, MPAC +3 to +6, PWRPTR and DERPTR walk the
                   # power table and DERTABLL's derivative table.
                   # (TTF/8CL's TC ROOTPSRS is the one caller: Q = 3113.)
                   'ROOTPSRS': {'0': [0o1565], '1': [1, 2], '2': [0o3113], 'WCHPHASE': [0o77776, 0, 1]},
                   # ROOTPSRS's returns run in IGNALG to APPRQUAD (WHATALM).
                   'TTF/8CL+26': {'WCHPHASE': [0o77776, 0, 1], '0166': ['vac+0']},
                   'TTF/8CL+28': {'0166': ['vac+0']}, 'QUADGUID': {'0166': ['vac+0']},
                   **{k: ROOTPS_STATES for k in ('ROOTPSRS+17', 'ROOTPSRS+19', 'DERCLOOP+3', 'ROOTLOOP+6',
                                                 'ROOTLOOP+13', 'ROOTLOOP+15', 'ROOTLOOP+20')}},
        # VHORCOMP's BANKCALL of DISPEXIT +3 ends the job (ENDOFJOB).
        'returns': {'DISPEXIT +3': 0},
        # ROOTPSRS returns through INDEX RETROOT; TCF 2 (two words past
        # its caller's TC).
        'index_returns': True,
        'in_file_returns': {'ROOTPSRS': [2]},
        # ROOTPSRS's INDEX MPAC +3; CA 5 reads an erasable power table.
        'index_erasable': True,
        # A guidance pass (QUADGUID through EXGSUB, with ?GUIDSUB's three
        # passes) runs a few hundred routines.
        'max_steps': 2000,
        # TC INTPRETX starts the interpretive code after it (TCF INTPRET).
        'intpret_via': ['INTPRETX'],
        # INTPRETX's INDEX WCHPHASE; CS TARGTDEX.
        'basic_fixed_index': {'TARGTDEX': [-1, 0, 1, 2]},
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'},
                'NORMUNIT': {'lm': ('lm_normunit(s,io)', 'stops')}},
        'includes': ['rtb_angles.h', 'lm_normunit.h'],
        'native_sources': ['src/rtb_angles.c', 'src/lm_normunit.c', 'src/rtb_vector.c', 'src/rtb_unit.c',
                           'src/guidance_unit_phased.c', 'src/guidance_length_phased.c', 'src/guidance_norm.c',
                           'src/guidance_product.c'],
        'edge_cases': True,
        'c64_rows': 'cover',
        # Routines whose whole runs reach more code than one C64 program
        # holds stay host-checked; the proof lists the arcs only they take.
        'c64_cover_partial': True,
        # INDEX WCHPHASE with WCHPHASE -1: the end-around carry the C64
        # build needs (unsigned is 16 bits there).
        'index_carry16': True,
        # DISPEXIT's INDEX WCHPHOLD / TCF WHATDISP with WCHPHOLD = -1 lands on
        # WHATEXIT's TCF EXNORM, which loops back to DISPEXIT for ever.  The
        # program only runs IGNALG (-1) with FLUNDISP set, so DISPEXIT never
        # gets there; cases on the paths into it keep WCHPHOLD on 0..2.
        'range_states': (('CGCALC', 'P63DISPS', {'WCHPHOLD': [0, 1, 2]}),
                         ('1406P00', 'FASTCHNG', {'WCHPHOLD': [0, 1, 2]})),
        # READ CHAN30 (STERN) and RAND/READ CHAN31: latched inputs.
        'channel_inputs': True,
        'c64_flags': ['-DIRC_INPUTS'],
        'partial': {'lm': 'WOR CHAN13 (STARTP64) waits for an output-channel model'},
        'target': 'tests/lm_llge_target.c',
        'freeze_stride': 32,
        'cover_stride': 8,
    },
    # Lane F: Luminary099 RCS_FAILURE_MONITOR (RCSMON, the T4RUPT
    # channel-32 valve check), the whole file (basic code).
    'lm-rcsmon': {
        'missions': ('lm',),
        'file': {'lm': 'RCS_FAILURE_MONITOR.agc'},
        'base': {'lm': 'lm_rcsmon'},
        'symbol': {'lm': 'lm_rcsmon'},
        'entries': ['RCSMON'],
        'externals': {'RESUME': None, 'NOVAC': 'auto'},
        # INDEX L; CA BIT8 -1 / MASK 5FAILTAB / MASK 6FAILTAB: L is the
        # changed channel-32 bit's number, 1 to 8 (the DOUBLE loop's count).
        'basic_fixed_index': {'BIT8': list(range(1, 9)), '5FAILTAB': list(range(1, 9)), '6FAILTAB': list(range(1, 9))},
        # PVALVEST holds the valve-state bits RCSMON itself records (low 8);
        # with a higher bit the DOUBLE loop never overflows (the original
        # runs forever too).
        'all_states': {'PVALVEST': list(range(0o400))},
        # T4RUPT runs it whatever job is waiting: NEWJOB +0 or positive.
        'resume_keys': True,
        'channel_inputs': True,
        'c64_flags': ['-DIRC_INPUTS'],
        'target': 'tests/lm_rcsmon_target.c',
        'c64_rows': 'cover',
        'cover': True,
        'freeze_stride': 16,
    },
    'cm-r31': {
        'missions': ('cm',),
        'file': {'cm': 'R31.agc'},
        'base': {'cm': 'cm_r31'},
        'symbol': {'cm': 'cm_r31'},
        # V83 (the extended verb), R31CALL and V83CALL (the job FINDVAC
        # starts) enter from outside the file.
        'entries': ['V83', 'R31CALL', 'V83CALL'],
        'externals': {'FINDVAC': 2, 'BANKCALL': 1, 'B5OFF': 0, 'ENDEXT': None},
        # V83's job: a DANZIG that finds a job waiting stops at CHANG2, and
        # the word after it is where the job resumes.
        'edge_cases': True,
        'resume_keys': True,
        # C64 rows picked by branch outcome (tools/c64_row_cover.py).
        'c64_rows': 'cover',
        'rtb': {'LOADTIME': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/cm_r31_target.c',
        'freeze_stride': 128,
    },
    'r30': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'R30.agc', 'cm': 'R30.agc'},
        'base': {'lm': 'lm_r30', 'cm': 'cm_r30'},
        'symbol': {'lm': 'lm_r30', 'cm': 'cm_r30'},
        # V82 (the extended verb) and the Executive/Waitlist targets the
        # file names itself.
        # CANDEL (CM): CHECKMM's in-mode return, past its DEC 11.
        'entries': {'lm': ['V82CALL', 'V82GON', 'V82GOFF1', 'TICKTEST', 'V82GON1'],
                    'cm': ['V82CALL', 'V82GON', 'V82GOFF1', 'TICKTEST', 'V82GON1', 'CANDEL']},
        'externals': {'BANKCALL': 1, 'WAITLIST': 2, 'FINDVAC': 2, 'NOVAC': 2, 'B5OFF': 0, 'CHANG1': 0, 'CHECKMM': 1,
                      'ENDEXT': None, 'ENDMARK': None, 'TASKOVER': None, 'ENDOFJOB': None},
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # GOXDSPF returns to the first, second or third word after its CADR
        # (terminate, proceed, data in or recycle).
        'returns': {'GOXDSPF': 3},
        # TICKTEST's INDEX A; TC +1 on V82FLAGS bits 1-2 (A = 0 to 3).
        'tc_tables': {'DOTICK+7': 4},
        # BOTHSHIP's DLOAD* 1/RTMUE,2 and MINPERE,2: X2 is 0 (Earth) or
        # 2 (Moon, the pair's first word) as THISPREC/OTHPREC leave it.
        'fixed_index': {'1/RTMUE': [0, -2], 'MINPERE': [0, -2]},
        # The resume LOCs after BOTHSHIP's VLOAD run the same DLOAD*s with X2
        # as BOTHSHIP left it.
        # STORHAPO's DSU and STORE MPAC +4 run in DP mode (MODE +0) after its STODL.
        'states': {**{k: {'vac+047': [0, 2]} for k in ('BOTHSHIP', 'THISSHIP+3', 'BOTHSHIP+2', 'BOTHSHIP+4',
                                                       'BOTHSHIP+5', 'BOTHSHIP+7', 'BOTHSHIP+8')},
                   'STORHAPO+2': {'0163': [0]}, 'STORHAPO+4': {'0163': [0]}},
        'target': 'tests/r30_target.c',
        # Every 128th case keeps the vectors under the 1 MB fast-checkout limit.
        'edge_cases': True,
        'resume_keys': True,
        'c64_rows': 'cover',
        'cover_stride': 8,
        'freeze_stride': 128,
    },
    'cm-tpi-search': {
        'c64_cuts': True,
        'missions': ('cm',),
        'file': {'cm': 'TPI_SEARCH.agc'},
        'base': {'cm': 'cm_tpi_search'},
        'symbol': {'cm': 'cm_tpi_search'},
        # P17 and P77 (the major modes) and TRANSANG (called from P34).
        'entries': ['P17', 'P77', 'TRANSANG'],
        'externals': {'BANKCALL': 1, 'AVFLAGA': 0, 'AVFLAGP': 0, 'P20FLGON': 0, 'VNPOOH': 0,
                      'BLANKET': 0, 'ALARM': 1, 'GOTOPOOH': None, 'ENDOFJOB': None},
        # GOFLASH returns to the first, second or third word after its CADR
        # (terminate, proceed, enter); GOFLASHR also to the fourth at once.
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        # MUTABLE,1 and MUTABLE +2,1 with X1 -2 or -10 (the conics' AXT,1).
        'fixed_index': {'MUTABLE': [2, 10]},
        # The search's EBANK= RACT3 (E7): XRS (X1, X2 as the conics leave
        # them, saved by S17.1) at E7,1745.
        'ebank': 7,
        'all_states': {'03745': [0o77765, 0o77775], '03746': [0, 2]},
        # VNCOMP17's TC QSAVED (01766 under E7): one of its callers' returns.
        'states': {'VNCOMP17+7': {'03766': [0o3176, 0o3226, 0o3262]},
                   # TRANSANG's resume LOCs after LXA,1 LXA,2 (the XRS above), with
                   # the push list at 00D (02D after its first PDVL) from SETPD 0.
                   **{'TRANSANG+%d' % n: {'vac+046': [0o77765, 0o77775], 'vac+047': [0, 2],
                                          '0166': ['vac+%o' % pd]}
                      for n, pd in ((6, 0), (9, 0), (11, 0), (14, 2))}},
        'target': 'tests/cm_tpi_search_target.c',
        'edge_cases': True,
        'resume_keys': True,
        'c64_rows': 'cover',
        'cover_stride': 8,
        'freeze_stride': 128,
    },
    'planetary': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'PLANETARY_INERTIAL_ORIENTATION.agc', 'cm': 'PLANETARY_INERTIAL_ORIENTATION.agc'},
        'base': {'lm': 'lm_planetary', 'cm': 'cm_planetary'},
        'symbol': {'lm': 'lm_planetary', 'cm': 'cm_planetary'},
        # R-TO-RP and RP-TO-R (CALLed from the integration, landing,
        # P20-P25, P51-P53 and LAT-LONG).
        'entries': ['R-TO-RP', 'RP-TO-R'],
        'externals': {},
        'resume_keys': True,
        # PUSHLOC and MODE at each point a run reaches past R-TO-RP or
        # RP-TO-R, as their SETPDs and loads leave them: a resume point
        # entered with a deeper pushdown list overwrites S1/S2 (VAC+050/051)
        # and the return through them loops on the AGC as well.
        'states': {
            'EARTHL+2': {'163': [0o0]},
            'EARTHL+4': {'163': [0o0]},
            'EARTHL+6': {'163': [0o0]},
            'EARTHL+8': {'163': [0o77776]},
            'EARTHMX+3': {'0166': ['vac+10']},
            'EARTHMX+5': {'0166': ['vac+10']},
            'EARTHMX+8': {'0166': ['vac+12'], '163': [0o0]},
            'EARTHMX+10': {'0166': ['vac+10'], '163': [0o0]},
            'EARTHMX+12': {'0166': ['vac+24'], '163': [0o0]},
            'EARTHMX+14': {'0166': ['vac+26'], '163': [0o0]},
            'EARTHMX+16': {'0166': ['vac+30'], '163': [0o0]},
            'EARTHMX+18': {'0166': ['vac+32'], '163': [0o0]},
            'EARTHMX+20': {'0166': ['vac+34'], '163': [0o0]},
            'EARTHMX+22': {'0166': ['vac+36'], '163': [0o77776]},
            'EARTHMX+24': {'0166': ['vac+46'], '163': [0o0]},
            'MOONMX+3': {'0166': ['vac+10']},
            'MOONMX+5': {'0166': ['vac+10']},
            'MOONMX+8': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+10': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+11': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+12': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+13': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+15': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+17': {'0166': ['vac+14'], '163': [0o0]},
            'MOONMX+20': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+22': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+24': {'0166': ['vac+14'], '163': [0o0]},
            'MOONMX+27': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+28': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+29': {'0166': ['vac+14'], '163': [0o0]},
            'MOONMX+30': {'0166': ['vac+14'], '163': [0o0]},
            'MOONMX+32': {'0166': ['vac+14'], '163': [0o0]},
            'MOONMX+33': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+35': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+36': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+37': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+38': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+40': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+41': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+43': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+45': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+47': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+49': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMX+51': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMX+54': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+57': {'0166': ['vac+24'], '163': [0o77776]},
            'MOONMX+59': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+60': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+61': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMX+64': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+66': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMX+68': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMX+70': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+72': {'0166': ['vac+24'], '163': [0o0]},
            'MOONMX+74': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+75': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMX+77': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMX+78': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMX+80': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMX+82': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMX+83': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMXA+2': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMXA+4': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMXA+6': {'0166': ['vac+10'], '163': [0o0]},
            'MOONMXA+8': {'0166': ['vac+12'], '163': [0o0]},
            'MOONMXA+11': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+14': {'0166': ['vac+24'], '163': [0o77776]},
            'MOONMXA+16': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+17': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+18': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMXA+21': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+23': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMXA+25': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMXA+27': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+29': {'0166': ['vac+24'], '163': [0o0]},
            'MOONMXA+31': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+32': {'0166': ['vac+16'], '163': [0o77776]},
            'MOONMXA+34': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMXA+35': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMXA+37': {'0166': ['vac+16'], '163': [0o0]},
            'MOONMXA+39': {'0166': ['vac+10'], '163': [0o77776]},
            'MOONMXA+40': {'0166': ['vac+10'], '163': [0o77776]},
            'NEWANGLE+3': {'0166': ['vac+14'], '163': [0o0]},
            'NEWANGLE+6': {'0166': ['vac+14'], '163': [0o1]},
            'NEWANGLE+8': {'0166': ['vac+14'], '163': [0o0]},
            'NEWANGLE+9': {'0166': ['vac+12'], '163': [0o0]},
            'NEWANGLE+11': {'0166': ['vac+10'], '163': [0o0]},
            'NEWANGLE+13': {'0166': ['vac+12'], '163': [0o0]},
            'NEWANGLE+16': {'0166': ['vac+12'], '163': [0o0]},
            'NEWANGLE+18': {'0166': ['vac+10'], '163': [0o0]},
            'NEWANGLE+20': {'0166': ['vac+10'], '163': [0o0]},
            'R-TO-RP+5': {'0166': ['vac+10'], '163': [0o77776]},
            'R-TO-RP+8': {'0166': ['vac+10'], '163': [0o77776]},
            'RP-TO-R+5': {'0166': ['vac+10'], '163': [0o77776]},
            'RPRPXXXX': {'163': [0o77776]},
            'RPRPXXXX+2': {'0166': ['vac+0'], '163': [0o77776]},
            'RPTORA+2': {'0166': ['vac+46'], '163': [0o0]},
            'RPTORA+4': {'0166': ['vac+46'], '163': [0o77776]},
            'RPTORA+6': {'0166': ['vac+46'], '163': [0o77776]},
            'RPTORB': {'163': [0o77776]},
            'RPTORB+3': {'163': [0o77776]},
            'RTORPA+2': {'0166': ['vac+46'], '163': [0o0]},
            'RTORPA+4': {'0166': ['vac+46'], '163': [0o77776]},
            'RTORPB': {'163': [0o77776]},
            'RTORPB+3': {'163': [0o77776]},
        },
        'edge_values': True,
        # BHIZ (CCS MPAC) at RP-TO-R and R-TO-RP: MPAC +-0 is every Earth
        # call (LAT-LONG loads ZEROVEC when LUNAFLAG is 0), COS(0) the Moon.
        'joint_states': {lab: [{'MPAC': 0}, {'MPAC': 0o77777}, {'MPAC': 0o20000}, {}]
                         for lab in ('RP-TO-R', 'R-TO-RP')},
        'c64_rows': 'cover',
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        'target': 'tests/planetary_target.c',
        'freeze_stride': 64,
    },
    'lat-long': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'LATITUDE_LONGITUDE_SUBROUTINES.agc', 'cm': 'LATITUDE_LONGITUDE_SUBROUTINES.agc'},
        'base': {'lm': 'lm_lat_long', 'cm': 'cm_lat_long'},
        'symbol': {'lm': 'lm_lat_long', 'cm': 'cm_lat_long'},
        # The labels other files CALL or STCALL.
        'entries': {'lm': ['LAT-LONG', 'LALOTORV', 'ARCTAN'],
                    'cm': ['LAT-LONG', 'LALOTORV', 'GETERAD', 'SETRE']},
        'externals': {},
        'resume_keys': True,
        # PUSHLOC and MODE at each point a run reaches from the entries and
        # the file's CALL targets, as their SETPDs and loads leave them.
        'states': {
            'ARCTANXX': {'163': [0o0]},
            'ARCTANXX+1': {'163': [0o0]},
            'ATAN=90': {'163': [0o0]},
            'ATAN=90+3': {'163': [0o0]},
            'ATAN=90+4': {'163': [0o0]},
            'CLROVFLW+2': {'163': [0o0]},
            'CLROVFLW+4': {'163': [0o0]},
            'CLROVFLW+5': {'163': [0o0]},
            'CLROVFLW+7': {'163': [0o0]},
            'CLROVFLW+10': {'163': [0o0]},
            'CLROVFLW+11': {'163': [0o0]},
            'CLROVFLW+12': {'163': [0o0]},
            'CLROVFLW+15': {'163': [0o0]},
            'GETERAD+2': {'163': [0o0]},
            'GETERAD+4': {'163': [0o0]},
            'GETERAD+7': {'163': [0o0]},
            'GETERAD+9': {'163': [0o0]},
            'GETERAD+10': {'163': [0o0]},
            'GETERAD+11': {'163': [0o0]},
            'LALOTORV+3': {'0166': ['vac+0']},
            'LALOTORV+5': {'0166': ['vac+0'], '163': [0o0]},
            'LALOTORV+7': {'0166': ['vac+0'], '163': [0o0]},
            'LALOTORV+10': {'0166': ['vac+2'], '163': [0o0]},
            'LALOTORV+12': {'0166': ['vac+4'], '163': [0o0]},
            'LALOTORV+13': {'0166': ['vac+2'], '163': [0o0]},
            'LALOTORV+15': {'0166': ['vac+4'], '163': [0o0]},
            'LALOTORV+17': {'0166': ['vac+6'], '163': [0o0]},
            'LALOTORV+18': {'0166': ['vac+0'], '163': [0o77776]},
            'LALOTORV+19': {'0166': ['vac+6'], '163': [0o77776]},
            'LALOTORV+21': {'0166': ['vac+6'], '163': [0o0]},
            'LALOTORV+25': {'0166': ['vac+6'], '163': [0o0]},
            'LAT-LONG+3': {'0166': ['vac+0']},
            'LAT-LONG+5': {'0166': ['vac+0'], '163': [0o77776]},
            'LAT-LONG+6': {'0166': ['vac+6'], '163': [0o0]},
            'LAT-LONG+8': {'0166': ['vac+6'], '163': [0o0]},
            'NEGCOS': {'163': [0o0]},
            'NEGCOS+1': {'163': [0o0]},
            'NEGOUT': {'163': [0o0]},
            'SETGAMMA+4': {'163': [0o0]},
            'SETGMEX': {'163': [0o0]},
            'SETGMEX+1': {'163': [0o0]},
            'SETRE+3': {'163': [0o0]},
            'SETRE+7': {'163': [0o0]},
            'TSTRLSRM+4': {'163': [0o77776]},
            'TSTRLSRM+5': {'163': [0o0]},
        },
        'edge_values': True,
        'joint_states': {'ARCTAN': lat_long_arctan_joint(), 'CLROVFLW': lat_long_arctan_joint(),
                         'CALLRTRP+2': lat_long_axis_joint()},
        'c64_rows': 'cover',
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        'target': 'tests/lat_long_target.c',
        'freeze_stride': 64,
    },
    'cm-p30-p37': {
        'missions': ('cm',),
        'file': {'cm': 'P30,P37.agc'},
        'source_path': {'cm': 'P30-P37.agc'},
        'base': {'cm': 'cm_p30_p37'},
        'symbol': {'cm': 'cm_p30_p37'},
        # P30 and P31 (the major modes), their displays, S30.1 and S31.1
        # (called from P30, P31 and the targeting programs), DELRSPL (the
        # splash error, from P37) and the flag subroutines.
        'entries': ['P30', 'P31', 'DISPMGA', 'P30PHSI', 'COMPTGO', 'S30.1', 'S31.1', 'DELRSPL',
                    'AUGEKUGL', 'AVFLAGA', 'AVFLAGP', 'P20FLGON'],
        'externals': {'BANKCALL': 'auto', 'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'PHASCHNG': 'auto',
                      '2PHSCHNG': 'auto', 'WAITLIST': 'auto', 'GOTOPOOH': None, 'ENDOFJOB': None},
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        # K1K2LOOP's tables, indexed by X1 (AUGEKUGL's altitude band, 0-12).
        'fixed_index': {lab: [-x for x in range(13)] for lab in ('V(32K)', 'YK1K2', 'CK1K2')},
        # AVFLAGA, AVFLAGP and P20FLGON save Q in SUBEXIT (E4,1770) and
        # return through it with EB 4 (EBANK= SUBEXIT).
        'ebank': 4,
        # RGEXIT (= SUBEXIT) holds a fixed return where END45's GOTO
        # RGEXIT runs: a seeded caller return (03103) reads as an erasable
        # CADR, and GOTO's erasable fetch can chain without end.
        # P30/31RT (01664 under EB 4) holds a return where FLASHMGA's
        # EXIT goes on to TC P30/31RT: GOTOPOOH or ENDOFJOB.
        # K1K2LOOP's X1 is odd (the band index plus LOOPSET's INCR,1),
        # LOOPSET's even; S1 is TIX's step of 2.
        'states': {'DISP45+4': {'02370': [0o4100, 0o5000]}, 'END45+1': {'02370': [0o4100, 0o5000]},
                   'FLASHMGA+2': {'02264': [0o4106, 0o5217]},
                   'LOOPSET': {'vac+046': [0, 2, 4, 6, 8, 10], 'vac+050': [2]},
                   'K2CALC': {'vac+046': [1, 3, 5, 7, 9, 11], 'vac+050': [2]},
                   'K1K2LOOP': {'vac+046': [1, 3, 5, 7, 9, 11], 'vac+050': [2]}},
        'target': 'tests/cm_p30_p37_target.c',
        'edge_cases': True,
        'resume_keys': True,
        'resume_flow': True,
        'c64_rows': 'cover',
        'cover_stride': 8,
        'freeze_stride': 128,
    },
    'cm-p32-p33': {
        'c64_cuts': True,
        'missions': ('cm',),
        'file': {'cm': 'P32-P33,_P72-P73.agc'},
        'source_path': {'cm': 'P32-P33_P72-P73.agc'},
        'base': {'cm': 'cm_p32_p33'},
        'symbol': {'cm': 'cm_p32_p33'},
        # P32/P72 (CSI) and P33/P73 (CDH), their displays and the CSI/CDH
        # subroutines the other targeting programs call.
        'entries': ['P32', 'P72', 'P33', 'P73', 'ALMXIT', 'ALMXITA', 'DISDVLVC', 'CSI/A', 'CDHMVR',
                    'S32/33.1', 'S32/33.X', 'ROTATE', 'INTINT2C', 'INTINT3P'],
        'externals': {'BANKCALL': 'auto', 'ALARM': 'auto', 'VARALARM': 'auto', 'AVFLAGA': 'auto',
                      'AVFLAGP': 'auto', 'P20FLGON': 'auto', 'VNPOOH': 'auto', 'UPFLAG': 'auto',
                      'DOWNFLAG': 'auto', 'GOTOPOOH': None, 'ENDOFJOB': None},
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        # ALMXIT's alarm table, indexed by -CSIALRM (alarms 1-7).
        # PMINE -2,1 with X1 = RTX1 (-2 earth, -10 moon) reads PMINE or
        # PMINM, eight words on.
        'fixed_index': {'ALARM/TB': list(range(1, 8)), 'PMINE': [2, 10]},
        # CSIALRM (TITER) is E7,1664: EB 7, an alarm number 1-7, which
        # ALMXITA takes from X2.
        'ebank': 7,
        'all_states': {'03664': list(range(1, 8)), '03745': [0o77775, 0o77765]},
        'states': {'ALMXITA': {'vac+047': list(range(1, 8))},
                   'CSINEXT4': {'vac+046': [0o77775, 0o77765]},
                   # ROTATE and ADVANCE's returns push vectors: PUSHLOC low in the
                   # VAC area, so the pushes don't run over X1-QPRET.
                   'ROTATE': {'0166': ['vac+0', 'vac+6']},
                   'ADVANCE+5': {'0166': ['vac+0', 'vac+6']},
                   'ADVANCE+20': {'0166': ['vac+0', 'vac+6', 'vac+14']}},
        # P32/P72F subtracts 60MIN until negative: hundreds of passes (and
        # with resume_keys the loop runs over two routines).
        'self_loops': True,
        'max_steps': 10000,
        'target': 'tests/cm_p32_p33_target.c',
        'edge_cases': True,
        'resume_keys': True,
        'resume_flow': True,
        'c64_rows': 'cover',
        'cover_stride': 16,
        'freeze_stride': 256,
    },
    'cm-p34-p35': {
        'c64_cuts': True,
        'missions': ('cm',),
        'file': {'cm': 'P34-P35,_P74-P75.agc'},
        'source_path': {'cm': 'P34-35_P74-75.agc'},
        'base': {'cm': 'cm_p34_p35'},
        'symbol': {'cm': 'cm_p34_p35'},
        # P34/P74 (TPI) and P35/P75 (midcourse), the S33/34.1 and S34/35.x
        # subroutines, VN1645 and its clock, INITVEL, MIDGIM, the conic
        # setup routines (SELECTMU, PERIAPO, PRECSET...) and R36.
        'entries': ['P34', 'P74', 'P35', 'P75', 'S33/34.1', 'S34/35.1', 'S34/35.2', 'S34/35.3',
                    'S34/35.4', 'S34/35.5', 'LOMAT', 'INTINT', 'VN1645', 'KILCLOCK', 'N45PROC',
                    'CLUPDATE', 'DISPLAYE', 'P3XORP7X', 'INITVEL', 'MIDGIM', 'GET+MGA',
                    'MIDGIM1', 'GET.LVC', 'SELECTMU', 'PERIAPO1', 'PERIAPO', 'PRECSET', 'LEMSTORE',
                    'CSMSTORE', 'VECSHIFT', 'SHIFTR1', 'R36', 'LREGCHK'],
        'externals': {'BANKCALL': 'auto', 'ALARM': 'auto', 'AVFLAGA': 'auto', 'AVFLAGP': 'auto',
                      'P20FLGON': 'auto', 'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'PHASCHNG': 'auto',
                      'COMPTGO': 'auto', 'POSTJUMP': 1, 'GOTOPOOH': None, 'ENDOFJOB': None,
                      'ENDEXT': None, 'ENDMARK': None,
                      'VNPOOH': 0},
        # VNPOOH switches FBANK in basic code (XCH FBANK), which the compiled
        # basic code does not model yet: its 15 words stay with the original.
        'outside': {'VNPOOH': 15},
        'noreturn': ['POSTJUMP'],
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        'partial': {'cm': 'VNPOOH (P30SUBS, fixed-fixed) switches FBANK with XCH FBANK and TS FBANK, which the compiled basic code does not model yet'},
        # SGNAGREE (RT8_OP_CODES' TPAGREE) has no native routine yet: the
        # RTB ends the routine there. DPMODE is src/rtb_modes.c.
        'rtb': {'LOADTIME': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}, 'SGNAGREE': None,
                'DPMODE': {'cm': 'rtb_dpmode(s,io,RTB_CM)'}},
        'includes': ['rtb_angles.h', 'rtb_modes.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_modes.c'],
        # SETMUER reads MUTABLE +4,1 and -2,1 with X1 -2 (earth) or -10
        # (moon), as SELECTMU's AXC,1 sets it.
        'fixed_index': {'MUTABLE': [2, 10]},
        'states': {'SETMUER': {'vac+046': [0o77775, 0o77765]}},
        'target': 'tests/cm_p34_p35_target.c',
        'edge_cases': True,
        'resume_keys': True,
        'resume_flow': True,
        'c64_rows': 'cover',
        'cover_stride': 16,
        'freeze_stride': 512,
    },
    'cm-p37-p70': {
        'missions': ('cm',),
        'file': {'cm': 'P37,P70.agc'},
        'source_path': {'cm': 'P37_P70.agc'},
        'base': {'cm': 'cm_p37_p70'},
        'symbol': {'cm': 'cm_p37_p70'},
        # P37/P70 (return to earth): the program, its displays and the
        # RTE subroutines (TMRAD, RTEDISP, INVC, PREC, RTENCK, V2T, GAMDV,
        # DVCALC, XT1LIM).
        'entries': ['P37', 'P370ALRM', 'TMRAD100', 'RTEDISP', 'INVC100', 'PREC100', 'RTENCK1',
                    'RTENCK2', 'RTENCK3', 'V2T100', 'GAMDV10', 'DVCALC', 'XT1LIM'],
        'externals': {'BANKCALL': 'auto', 'PHASCHNG': 'auto', 'GOTOPOOH': None, 'ENDOFJOB': None, 'POLY': 'auto', 'BLANKET': 'auto', 'VARALARM': 'auto', 'POODOO': None},
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        'rtb': {'DPMODE': {'cm': 'rtb_dpmode(s,io,RTB_CM)'}, 'TPMODE': {'cm': 'rtb_tpmode(s,io,RTB_CM)'}},
        'includes': ['rtb_angles.h', 'rtb_modes.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_modes.c'],
        # GAMDV's iteration (GAMDV25 through GAMDV65, with DVCALC) runs up to
        # 144 passes of about eight routines each, and V2T's own iteration
        # calls it again on each of its passes.
        'max_steps': 10000,
        # P37PROC and P370GOFR return through INDEX SPRTEX (RTENCKEX); TCF 0 +n.
        'index_returns': True,
        # EBANK= RTEDVD: the RTE temporaries are E7 words.
        'ebank': 7,
        # NN2 (E7,1731), GAMDV's pass count, from -144 up to 0: small
        # negatives (and +0 high words), so a case runs a few passes.
        # QPRET holds a CADR outside the file where a routine's STQ saves
        # it (a random one can name the middle of a routine here).
        'all_states': {'03731': [0o77777, 0], '03732': [0o77776, 0o77700, 0o77557],
                       '0117': [0o4100, 0o5000],
                       # PUSHLOC low: pushes stay below 20D and 31D-33D, the saved
                       # returns of XT1LIM, GAMDV10, DVCALC and V2T100.
                       '0166': ['vac+0', 'vac+6']},
        # P370ALRM's GOTO SPRTEX takes it as a CADR (STQ saved it), where
        # basic code's TC SPRTEX takes it as an address.
        # GAMDV30 stores MPAC (NN2 +1) as the pass count: small too.
        'states': {'P370ALRM+9': {'03762': [0o4100, 0o5000]}, 'P370ALRM+10': {'03762': [0o4100, 0o5000]},
                   'GAMDV30': {'0154': [0o77777, 0], '0155': [0o77776, 0o77700, 0o77557]},
                   # (CALL XT1LIM's return) pushes four scalars: in vector mode
                   # they would run over XT1LIM's 20D.
                   'V2T101+51': {'0163': [0, 1]}},
        'target': 'tests/cm_p37_p70_target.c',
        'freeze_stride': 256,
    },
    'lm-p34-p35': {
        'missions': ('lm',),
        'file': {'lm': 'P34-P35,_P74-P75.agc'},
        'source_path': {'lm': 'P34-35_P74-75.agc'},
        'base': {'lm': 'lm_p34_p35'},
        'symbol': {'lm': 'lm_p34_p35'},
        # Luminary's P34/P74 and P35/P75, as cm-p34-p35: S33/34.1,
        # S34/35.x, VN1645, INITVEL, the MGA and conic setup routines, R36.
        'entries': ['P34', 'P74', 'P35', 'P75', 'S33/34.1', 'S34/35.1', 'S34/35.2', 'S34/35.3',
                    'S34/35.4', 'S34/35.5', 'LOMAT', 'INTINT', 'VN1645', 'KILCLOCK', 'N45PROC',
                    'CLUPDATE', 'DISPLAYE', 'P3XORP7X', 'INITVEL', 'GET+MGA', 'GET.LVC',
                    'SELECTMU', 'PERIAPO1', 'PERIAPO', 'PRECSET', 'LEMSTORE', 'CSMSTORE',
                    'VECSHIFT', 'SHIFTR1', 'R36', 'LREGCHK'],
        'externals': {'BANKCALL': 'auto', 'ALARM': 'auto', 'AVFLAGA': 'auto', 'AVFLAGP': 'auto',
                      'P20FLGON': 'auto', 'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'PHASCHNG': 'auto',
                      'COMPTGO': 'auto', 'POSTJUMP': 1, 'GOTOPOOH': None, 'ENDOFJOB': None,
                      'ENDEXT': None, 'ENDMARK': None, 'VNPOOH': 0},
        'noreturn': ['POSTJUMP'],
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'}, 'SGNAGREE': None,
                'DPMODE': {'lm': 'rtb_dpmode(s,io,RTB_LM)'}, 'TPMODE': {'lm': 'rtb_tpmode(s,io,RTB_LM)'}},
        'includes': ['rtb_angles.h', 'rtb_modes.h'],
        'native_sources': ['src/rtb_angles.c', 'src/rtb_modes.c'],
        'fixed_index': {'MUTABLE': [2, 10]},
        'states': {'SETMUER': {'vac+046': [0o77775, 0o77765]}},
        # RTX1 and RTX2 (E5,1776-1777): -2 and 0 for the earth, -10 and 2
        # for the moon. INITVELX's DLOAD* MUTABLE -2,1 indexes by RTX1.
        'all_states': {'02776': [0o77775, 0o77765], '02777': [0, 2]},
        'target': 'tests/lm_p34_p35_target.c',
        'freeze_stride': 512,
    },
    'lm-p32-p35': {
        'c64_cuts': True,
        'missions': ('lm',),
        'file': {'lm': 'P32-P35,_P72-P75.agc'},
        'source_path': {'lm': 'P32-P35_P72-P75.agc'},
        'base': {'lm': 'lm_p32_p35'},
        'symbol': {'lm': 'lm_p32_p35'},
        # Luminary's P32/P72 (CSI) and P33/P73 (CDH) with their displays,
        # the flag subroutines, the CSI search, S32/33.1, S32/33.X, CDHMVR,
        # ROTATE, INTINT2C/3P and COMPTGO.
        'entries': ['P32', 'P72', 'P33', 'P73', 'ALMXIT', 'ALMXITA', 'AVFLAGA', 'AVFLAGP',
                    'P20FLGON', 'DISDVLVC', 'CSI/A', 'CDHMVR', 'S32/33.1', 'S32/33.X', 'ROTATE',
                    'INTINT2C', 'INTINT3P', 'COMPTGO'],
        'externals': {'BANKCALL': 'auto', 'ALARM': 'auto', 'VARALARM': 'auto', 'VNPOOH': 'auto',
                      'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'WAITLIST': 'auto', '2PHSCHNG': 'auto',
                      'PHASCHNG': 'auto', 'GOTOPOOH': None, 'ENDOFJOB': None},
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        # ALMXIT's alarm table, indexed by -CSIALRM (alarms 1-7); PMINE
        # -2,1 with X1 = RTX1 (-2 earth, -10 moon).
        'fixed_index': {'ALARM/TB': list(range(1, 8)), 'PMINE': [2, 10]},
        # SUBEXIT and CSIALRM (TITER) are E7 words (E7,1470 and 1613).
        'ebank': 7,
        'all_states': {'03613': list(range(1, 8)), '02776': [0o77775, 0o77765], '02777': [0, 2]},
        'states': {'ALMXITA': {'vac+047': list(range(1, 8))},
                   'ROTATE': {'0166': ['vac+0', 'vac+6']},
                   'ADVANCE+5': {'0166': ['vac+0', 'vac+6']},
                   'ADVANCE+20': {'0166': ['vac+0', 'vac+6', 'vac+14']}},
        'self_loops': True,
        # With resume_keys the loops (P32/P72F's 60MIN passes) run over
        # several routines.
        'max_steps': 10000,
        'target': 'tests/lm_p32_p35_target.c',
        'edge_cases': True,
        'resume_keys': True,
        'resume_flow': True,
        'c64_rows': 'cover',
        'cover_stride': 16,
        'freeze_stride': 256,
    },
    'lm-conic-compiled': {
        'missions': ('lm',),
        'file': {'lm': 'CONIC_SUBROUTINES.agc'},
        'base': {'lm': 'lm_conic_compiled'},
        'symbol': {'lm': 'lm_conic_compiled'},
        'entries': ['KEPLERN', 'TIMETHET', 'LAMBERT', 'TIMERAD', 'APSIDES', 'DELTIME', 'ITERATOR',
                    'LAMENTER', 'PARAM', 'GEOM', 'GETX', 'INITV', 'TARGETV'],
        'externals': {'POLY': 'auto', 'POODOO': 'auto'},
        'noreturn': ['POODOO'],
        # MUTABLE,1: X1 is 0 (Earth) or -8 (Moon).
        'indexed_fixed': {'MUTABLE': (0, 8)},
        # X1 the body (MUTABLE,1: 0 Earth, -8 Moon); X2 and S2 the GETX and
        # INVRSEQN loop count (AXT,2 3, SSP S2 1).
        'all_states': {'vac+046': [0, 0o77767], 'vac+047': [0, 1, 2, 3], 'vac+051': [1]},
        # PERIODCH subtracts the period (0D) from TAU. (2073) until less than
        # one is left. A random TAU. over a random period loops for millions
        # of passes, so resumes before it take TAU. +0 (its own entries keep
        # any TAU.) and resumes in the loop a period of at least 10000 octal
        # against a TAU. of a few. The period is in MPAC at PERIODCH and its
        # GOTO (PUSHLOC 0D), pushed at 0D in between (PUSHLOC 2D).
        'states': {**{'PERIODCH%+d' % n: {'2073': [0], '2074': [0]} for n in range(-0o66, 0)},
                   **{'PERIODCH+%d' % n: {'2073': [0, 1, 0o77776], 'vac+0': [0o10000, 0o20000, 0o30000],
                                         **({'0154': [0o10000, 0o20000, 0o30000], '0166': ['vac+0']}
                                            if n in (0, 0o10) else {'0166': ['vac+2']})}
                      for n in range(0, 0o11)}},
        'rtb': {'TPMODE': {'lm': '(lmi_tpmode(s),RTB_OK)'}, 'CHECKCTR': {'lm': 'lm_conic_checkctr(s)'}},
        'native_ranges': {'CHECKCTR': (7, 'lm_conic_checkctr')},
        'includes': ['rtb_angles.h', 'lm_conic_checkctr.h'],
        'native_sources': ['src/lm_conic_checkctr.c'],
        'target': 'tests/lm_conic_compiled_target.c',
        'flow_states': True,
        'resume_keys': True,
        # (a LAMBERT iteration runs about 100 pairs, up to 20 of them)
        'max_steps': 5000,
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        # (No resume_flow: the CHANG2 resumes in PERIODCH's loop keep the
        # TAU. and period pins above; a snapshot from a random TAU. loops
        # for millions of passes.)
        'edge_cases': True,
        # KEPLERN's routines before PERIODCH take no edge inputs: from an
        # edge state vector PERIODCH can loop forever in the original as
        # well (yaAGC times out, the native run stops at max_steps).
        'edge_skip': [('KEPLERN', 'PERIODCH')],
        # The C64 rows cover every branch arc of the host cases.
        'c64_rows': 'cover',
        'rope_bin': True,
    },
    'lm-stable-orbit': {
        'missions': ('lm',),
        'file': {'lm': 'STABLE_ORBIT_-_P38-P39.agc'},
        'source_path': {'lm': 'STABLE_ORBIT.agc'},
        'base': {'lm': 'lm_stable_orbit'},
        'symbol': {'lm': 'lm_stable_orbit'},
        # P38/P78 (stable orbit rendezvous) and P39/P79 (its midcourse), the
        # major modes FRESH_START_AND_RESTART's tables start.
        'entries': ['P38', 'P78', 'P39', 'P79'],
        'externals': {'BANKCALL': 'auto', 'BLANKET': 'auto', 'GOTOPOOH': None, 'ENDOFJOB': None},
        # GOFLASH returns to the first, second or third word after its CADR
        # (terminate, proceed, recycle); GOFLASHR also to the fourth at once.
        'returns': {'GOFLASH': 3, 'GOFLASHR': 4},
        'resume_orphans': True,
        'resume_keys': True,
        # EBANK= SUBEXIT: the P38/P39 temporaries are E7 words.
        'ebank': 7,
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/lm_stable_orbit_target.c',
        'edge_cases': True,
        # The C64 rows are the fewest cases that take every branch arc of
        # every traced host case (every case: 101 routines are cheap).
        'c64_rows': 'cover',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        # A CHANG2 resume starts from the state the program reaches it in.
        'resume_flow': True,
        'rope_bin': True,
    },
    'lm-p20-p25': {
        'missions': ('lm',),
        'file': {'lm': 'P20-P25.agc'},
        'base': {'lm': 'lm_p20_p25'},
        'symbol': {'lm': 'lm_p20_p25'},
        # PROG20/22/25 from FRESH_START, the rest the labels other files call.
        # Channel instructions wait for the shared output-channel model
        # (spec 'channels', from Lane F and Lane D2): a case reaching one
        # refuses and is listed, not compared. So does RRLIMCHK's entry.
        # (Shared compiler options every refreeze sets now: the C64 INDEX
        # end-around carry, the fixed $7000 image, and a CCS arm on the
        # word after an EXTEND refused.)
        'index_carry16': True,
        'image_w': True,
        'refuse_unextended': True,
        # Opt-in basic code forms: INDEX Q reads of a caller's inline word
        # (RRLIMCHK's ADRES), INDEX L/Q; TC returns past the caller's, and
        # TS FBANK/BBANK ending the routine unless FB stays.
        'inline_reads': True,
        'ts_bank_guard': True,
        # A TS overflow skip, or an INCR Q return, landing on the word after
        # an EXTEND would run it unextended (DGCHECK's TS ITEMP1 skipping
        # RAND CHAN33's EXTEND): refused.
        'refuse_skip_unextended': True,
        'partial': {'lm': 'the channel instructions (RAND, WAND, WOR and RXOR on channels 12, 13 and 14, '
                          'LCHAN and SUPERBNK) wait for the shared I/O channel model; RRLIMCHK\'s double index '
                          '(EXTEND, INDEX Q, INDEX 0) is not compiled yet'},
        'entries': ['PROG20', 'PROG25', 'R21LEM', 'R22LEM', 'CLRADMOD', 'DORREPOS', 'GOTOV56', 'LRALT', 'LRPOS2',
                    'LRVEL', 'LRVELX', 'LRVELY', 'LRVELZ', 'P20LEMC1', 'P25LEM1', 'R12LITES', 'R22RSTRT',
                    'R29', 'RADAREAD', 'RRDESNB', 'RRLIMCHK', 'RRRANGE', 'RRRDOT', 'RRTURNON', 'RRZERO', 'SETTRKF',
                    'V67CALL'],
        'externals': {'SPSIN': 'auto', 'SPCOS': 'auto', 'MPACVBUF': 'auto', 'USPRCADR': 'auto', 'RESUME': 'auto', 'CHANG1': 'auto', 'BANKJUMP': 'auto', 'LONGCALL': 'auto', 'FALTON': 'auto', 'MAKECADR': 'auto', 'KILLTASK': 'auto', 'TWIDDLE': 'auto', 'BANKCALL': 'auto', 'IBNKCALL': 'auto', 'PHASCHNG': 'auto', '2PHSCHNG': 'auto',
                      'UPFLAG': 'auto', 'DOWNFLAG': 'auto', 'FINDVAC': 'auto', 'NOVAC': 'auto',
                      'WAITLIST': 'auto', 'FIXDELAY': 'auto', 'VARDELAY': 'auto', 'BLANKET': 'auto',
                      'ALARM': 'auto', 'PRIOCHNG': 'auto', 'JOBWAKE': 'auto', 'JOBSLEEP': None,
                      'TASKOVER': None, 'ENDOFJOB': None, 'GOTOPOOH': None, 'POSTJUMP': 1},
        'noreturn': ['POSTJUMP'],
        # Returns a case starts with only as their producers leave them:
        # G+N,AUTO's A (0 in AUTO, else positive) and LRS22.1's MPAC (0 good
        # data, 1 or 2 for its two bad-data returns).
        # (LITIT +3, TRKFLON's return: QXCH ITEMP6 takes back the caller's
        # Q that LITIT saved there, a basic return outside the file:
        # LODSAMPT's address stands in for it, the case's Q and stop too.)
        'states': {'LITIT+3': {'ITEMP6': [0o4400], '2': [0o4400]},
                   'R61LEM1+21': {'0000': [0, 1, 0o20000]}, 'R22LEM12+19': {'0154': [0, 1, 2]},
                   # (X1 an index, as the STORE,1 that TC INTPRET at
                   # R21LEM7 +1 is to the interpreter assumes)
                   'R21LEM12+14': {'vac+46': [0, 2, 4, 6, 0o77775]},
                   'R21LEM12+18': {'vac+46': [0, 2, 4, 6, 0o77775]},
                   'R21LEM12+15': {'vac+46': [0, 2, 4, 6, 0o77775]},
                   # (S1 6 from V67WW's SSP S1 on)
                   'V67WW+10': {'vac+50': [6]}, 'V67WW+11': {'vac+50': [6]},
                   'V67WW+12': {'vac+50': [6]}, 'V67WW+13': {'vac+50': [6]},
                   # (R29's pass flag: 1 on the first pass, then 0)
                   'TESTCOS+6': {'TANG': [0, 1]},
                   # (RR1AXIS's axis index, 0 or 1, through RR1AX2)
                   **{'RR1AX2+%d' % n: {'RRINDEX': [0, 1]} for n in range(30)}},
        # V67's W-matrix loop (NXPOSVEL ... ADDPOS ... TIX,1 NXPOSVEL): X1
        # steps from 54 by S1 = 6.
        'joint_states': dict([('ADDPOS+%d' % n, v67_loop()) for n in range(20)]
                             + [('NXPOSVEL+%d' % n, v67_loop()) for n in range(4)]
                             + [('R22LEM96+%d' % n, n49_wait()) for n in (9, 10, 12)]),
        'channel_inputs': True,
        'resume_flow': True,
        'edge_cases': True,
        'counters': True,
        'c64_rows': 'cover',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        'resume_keys': True,
        'resume_orphans': True,
        'rtb': {'LOADTIME': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_LOAD_TIME)'},
                'CDULOGIC': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_CDU_LOGIC)'},
                'TPMODE': {'lm': '(lmi_tpmode(s),RTB_OK)'},
                'NORMUNX1': None, 'READCDUD': None},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        # INDEX MPAC; TC +1 after LPS20.2: within or beyond 400 n.m.
        'tc_tables': {'P20LEM1+13': 2, 'R22LEM12+20': 3, 'R22LEM2+11': 2, 'LRVEL+3': 5},
        'basic_fixed_index': {'HFLSHBIT': [0, -1]},   # (ONLITES, HLIGHT: H or V)
        'target': 'tests/lm_p20_p25_target.c',
        'rope_bin': True,
    },
    # Lane A2: Luminary099 FINDCDUW--GUIDAP_INTERFACE (bank 30), basic and
    # interpretive.
    'findcduw': {
        'missions': ('lm',),
        'file': {'lm': 'FINDCDUW_-_GUIDAP_INTERFACE.agc'},
        'base': {'lm': 'lm_findcduw'},
        'symbol': {'lm': 'lm_findcduw'},
        # Entries from other files: INITCDUW (STCALL/CALL from BURNBABY and
        # P70/P71), FINDCDUW -2 (CALL, BPL from the guidance equations),
        # SPARCSIN -1 (BANKCALL from LUNAR_LANDING_GUIDANCE_EQUATIONS).
        'entries': {'lm': ['INITCDUW', 'FINDCDUW-2', 'SPARCSIN-1']},
        'externals': {'ALARM': 1, 'IBNKCALL': 1},
        'rtb': {'NORMUNIT': {'lm': 'lm_findcduw_normunit(s,io)'},
                'QUICTRIG': {'lm': 'lm_findcduw_quictrig(s,io)'}},
        # PAUTNO's RAND CHAN30 (bit 10, PGNCS in control, 0 = yes) and RAND
        # CHAN31 (bit 14, auto mode, 0 = yes) read the caller's latched
        # inputs (include/agc_input_channels.h), random per case.
        'channel_inputs': True,
        # CCS A after each RAND: its -0 arm is the word after the next
        # EXTEND, refused rather than run unextended.
        'refuse_unextended': True,
        # FINDCDUW runs in the guidance equations' job (SERVICER) and
        # INITCDUW in BURNBABY's or P70/P71's: any DANZIG can find a job
        # waiting.
        'resume_keys': True,
        # CDUX/CDUY/CDUZ, TIME1/TIME2 and the other counters seeded too, edge
        # vectors at the operands, and the words the basic code names often
        # at +-0, small or saturating values (every CCS, BZF and BZMF arm,
        # TS overflow).
        'counters': True,
        'edge_values': True,
        'basic_edges': True,
        'includes': ['lm_findcduw_rtb.h'],
        'native_sources': ['src/lm_findcduw_rtb.c', 'src/rtb_vector.c', 'src/rtb_unit.c',
                           'src/guidance_unit_phased.c', 'src/guidance_length_phased.c', 'src/guidance_norm.c',
                           'src/lm_powered_quick.c', 'src/lm_single_trig.c', 'src/rtb_angles.c'],
        'target': 'tests/findcduw_target.c',
        # The runs through PAUTNO's RTBs only fit one C64 program (below
        # $c000, the image under KERNAL at $e000) as the large integration
        # files build theirs.
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
        # INDEX's end-around carry also on the C64 (SPARCSIN's INDEX A with
        # A = -1 carries out of 16 bits).
        'index_carry16': True,
        # SPARCSIN's INDEX A; CS LIMITS after TS SR overflowed: A is +-1.
        # INDEX NDXCDUW (0 docked, 1 not) on the gain and limit pairs.
        'basic_fixed_index': {'LIMITS': [-1, 1], 'GAINFLTR': [0, 1], 'DAZMAX': [0, 1], 'DAY/2MAX': [0, 1],
                              'DAXMAX': [0, 1]},
        'c64_rows': 'cover',
        'ebank': 6,
        # NDXCDUW (E6 1650) as FINDCDUW sets it: 0 docked, 1 not. OVFIND as
        # the interpreter sets it (TS of an overflowed A: +-1), +0 in three
        # cases of four: FINDCDUW's BOV FINDCDUW to itself counts any other
        # word down for thousands of passes.
        'all_states': {'3250': [0, 1], '121': [0, 0, 0, 0, 0, 0, 1, 0o77776]},
        # Keys inside the file's loops and subroutines, with the counter or
        # saved return their caller left: TEM2 the CDUWXFR index (2, 1, 0)
        # or FLTRSUB's saved Q (its two calls), TEM4 ARCTRGSP's saved Q
        # (its three calls). FINDCDUW runs in vector mode from FINDCDUW-2's
        # VLOAD to PAUTNO's RTB NORMUNIT, which unitizes that vector (MODE -1),
        # and again from STOVL UNWC/2 to the RTB NORMUNIT at PAUTNO +5; the
        # CHANG2 resumes in between (FINDCDUW +3, PAUTNO +5) keep it.
        'states': {'FINDCDUW': {'163': [0o77776]}, 'FINDCDUW+3': {'163': [0o77776]},
                   'FINDCDUW+5': {'163': [0o77776]}, 'PAUTNO+1': {'163': [0o77776]},
                   'PAUTNO+5': {'163': [0o77776]},
                   'CDUWXFR+6': {'142': [0, 1, 2]}, 'CDUWXFR+24': {'142': [0, 1, 2]},
                   'FLTRSUB+11': {'142': [0o3207, 0o3213]}, 'FLTRSUB+15': {'142': [0o3207, 0o3213]},
                   'ARCTRGSP+21': {'144': [0o3556, 0o3565, 0o3574]}, 'ARCTRGSP+22': {'144': [0o3556, 0o3565, 0o3574]},
                   'ARCTRGSP+25': {'144': [0o3556, 0o3565, 0o3574]}, 'ARCTRGSP+28': {'144': [0o3556, 0o3565, 0o3574]}},
    },
}
