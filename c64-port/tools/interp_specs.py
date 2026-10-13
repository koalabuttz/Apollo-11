# SPDX-License-Identifier: MIT
"""Milestone specs for tools/interp_compile.py: the file, its entries, the
outside basic routines it calls (with their inline word counts) and the
native routines its RTBs reach."""


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


SPECS = {
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
                    'cm': ['SVDWN1', 'SVDWN2', 'LOG', 'EARROT1', 'EARROT2']},
        'externals': {'POLY': 9, 'SHORTMP': 0},
        'rtb': {'CDULOGIC': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_CDU_LOGIC)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_CDU_LOGIC)'},
                '2V1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_TWO_VECTOR_ONE_TO_TWO)',
                             'cm': 'rtb_angles(s,io,RTB_CM,RTB_TWO_VECTOR_ONE_TO_TWO)'},
                '1STO2S': {'lm': 'rtb_angles(s,io,RTB_LM,RTB_ONE_TO_TWO)',
                           'cm': 'rtb_angles(s,io,RTB_CM,RTB_ONE_TO_TWO)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
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
        # Words no routine compiles yet, and why (the source map records them).
        'partial': {'cm': 'SXTNB, SXTLOGIC, CALCSXA and SXTANG wait for native TRANSP1/TRANSP2'},
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
        # One C64 case per routine: KALCMAN3's run reaches most of the file.
        'c64_per_point': 1,
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
                       'vac+051': [0, 2, 4, 6], 'vac+052': [0, 2, 4, 6],
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
        'states': {lab: {'0164': [loc], '0165': [bank | e for e in range(8)], '0115': [0]}
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
        'rope_bin': True,
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
        'target': 'tests/sband_antenna_target.c',
        # V64 runs SBANDANT as a job: any DANZIG can find a job waiting.
        'resume_keys': True,
        # GOMARKFR's returns (terminate, proceed, recycle, immediate) land on
        # words nothing in the file reaches.
        'resume_orphans': True,
        'freeze_stride': 32,
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
        'rtb': {'LOADTIME': {'cm': 'rtb_angles(s,io,RTB_CM,RTB_LOAD_TIME)'}},
        'includes': ['rtb_angles.h'],
        'native_sources': ['src/rtb_angles.c'],
        'target': 'tests/cm_r31_target.c',
        'freeze_stride': 32,
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
        'states': {'BOTHSHIP': {'vac+047': [0, 2]}, 'THISSHIP+3': {'vac+047': [0, 2]}},
        'target': 'tests/r30_target.c',
        # Every 128th case keeps the vectors under the 1 MB fast-checkout limit.
        'freeze_stride': 128,
    },
    'cm-tpi-search': {
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
        'states': {'VNCOMP17+7': {'03766': [0o3176, 0o3226, 0o3262]}},
        'target': 'tests/cm_tpi_search_target.c',
        'freeze_stride': 128,
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
        'freeze_stride': 128,
    },
    'cm-p32-p33': {
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
        # P32/P72F subtracts 60MIN until negative: hundreds of passes.
        'self_loops': True,
        'target': 'tests/cm_p32_p33_target.c',
        'freeze_stride': 256,
    },
    'cm-p34-p35': {
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
        'target': 'tests/lm_p32_p35_target.c',
        'freeze_stride': 256,
    },
}
