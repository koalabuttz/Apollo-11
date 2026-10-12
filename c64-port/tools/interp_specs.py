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
        # Every 32nd case keeps the vectors under the 1 MB fast-checkout limit.
        'freeze_stride': 32,
        # AXISGEN's loops entered directly: index registers and steps as
        # the loops hold them (X1 and S1 at VAC+046/050, X2 and S2 at
        # VAC+047/051, the saved X1 in 30D at VAC+036). 'LABEL+n' is the
        # word at that address (AXT,1 STARAD +6 and SSP S1 STARAD -6);
        # 'vac+n' the VAC address (PUSHLOC 0166 at 0D, AXISGEN's SETPD).
        'states': {
            'AXISGEN1': {'vac+046': ['AXISGEN+1', 0o14], 'vac+050': ['AXISGEN+3'], '0166': ['vac+0']},
            'AXISGEN2': {'vac+046': [0o22, 0o14, 0o6], 'vac+050': [0o6], 'vac+047': [0o6, 0o4, 0o2],
                         'vac+051': [0o2], 'vac+036': [0o77771], '0166': ['vac+0']},
            'AXISGEN3': {'vac+046': [0o22, 0o14, 0o6], 'vac+050': [0o6], 'vac+047': [0o6, 0o4, 0o2],
                         'vac+051': [0o2], 'vac+036': [0o77771], '0166': ['vac+0']},
        },
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
        # Words no routine compiles yet, and why (the source map records them).
        'partial': {'cm': 'SXTNB, SXTLOGIC, CALCSXA and SXTANG wait for native TRANSP1/TRANSP2'},
        'freeze_stride': 32,
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
        # BUF2 holds the last display's return (a fixed address), as
        # CHKLINUS's restart point (BUF2 -3) assumes; 'NNNN' is erasable.
        'all_states': {'0133': [0o2000, 0o2513, 0o3266, 0o3777]},
        'partial': {'lm': 'G+N,AUTO and ISITAUTO read channels 30 and 31, which wait for an I/O channel model',
                    'cm': 'TOBALLC reads channel 31 (RXOR CHAN31), which waits for an I/O channel model'},
        'freeze_stride': 64,
    },
    'attitude-maneuver': {
        'missions': ('lm', 'cm'),
        'file': {'lm': 'ATTITUDE_MANEUVER_ROUTINE.agc', 'cm': 'ANGLFIND.agc'},
        'base': {'lm': 'lm_attitude_maneuver', 'cm': 'cm_attitude_maneuver'},
        'symbol': {'lm': 'lm_attitude_maneuver', 'cm': 'cm_attitude_maneuver'},
        'entries': ['KALCMAN3', 'MXM3', 'TRANSPOS', 'READCDUK', 'CDUTODCM', 'DELCOMP', 'DCMTOCDU',
                    'VECOFANG', 'NOGO'],
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
        # One C64 case per routine: KALCMAN3's run reaches most of the file.
        'c64_per_point': 1,
        # EBANK= BCDU (E6). X1 and X2 (VAC+046/047) hold the complemented
        # matrix addresses the callers' AXC orders load (MIS, KEL, MFS...).
        'ebank': 6,
        # LOOPSIN is CDUTODCM's TIX loop: X1 3, 2 or 1 with S1 1, and the
        # pushdown list at 0D (its SETPD 0), as the loop's pushes assume.
        # SUHALFA and CALCPHI (DCMTOCDU's THETA quadrant fix) run with PSI
        # and THETA pushed (pushdown at 4D).
        'states': {'LOOPSIN': {'vac+046': [3, 2, 1], 'vac+050': [1], '0166': ['vac+0']},
                   'SUHALFA': {'0166': ['vac+4']}, 'CALCPHI': {'0166': ['vac+4']}},
        'all_states': {'vac+046': {'lm': [0o77777 ^ a for a in (0o3246, 0o2230, 0o3303)],
                                   'cm': [0o77777 ^ a for a in (0o3266, 0o3320, 0o3425)]},
                       'vac+047': {'lm': [0o77777 ^ a for a in (0o3246, 0o2230, 0o3303)],
                                   'cm': [0o77777 ^ a for a in (0o3266, 0o3320, 0o3425)]}},
        # Half the cases start from one MFI or COF set (anglfind_joint).
        'joint_states': {'*': anglfind_joint()},
        # -fno-inline keeps KALCMAN3's whole path in one diagnostic.
        'c64_flags': ['-fno-inline'],
        'freeze_stride': 64,
    },
    'gimbal-lock': {
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
        'all_states': {'2030': [0, 2], '1117': [0, 0o77771, 0o77763, 0o77755, 0o77747, 0o77741, 0o77733, 0o77725, 0o77717], '1500': [0, 0o77763, 0o77747], 'vac+046': [0, 2], 'vac+047': [0, 2],
                       'vac+051': [0, 2, 4, 6], 'vac+052': [0, 2, 4, 6],
                       '0166': ['vac+0', 'vac+2', 'vac+4', 'vac+6', 'vac+10', 'vac+12']},
        # BOVB TCDANZIG: TCDANZIG is ENDT(X) in TIME_OF_FREE_FALL, TC DANZIG.
        'rtb': {'TCDANZIG': {'lm': 'lm_integration_tcdanzig(s)'}, 'ENDT(X)': {'lm': 'lm_integration_tcdanzig(s)'}},
        'includes': ['rtb_angles.h', 'lm_integration_moves.h'],
        'native_sources': ['src/lm_integration_moves.c'],
        'target': 'tests/lm_orbital_integration_target.c',
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
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
        'freeze_stride': 128,
        'c64_flags': ['-fno-inline', '-fstatic-stack', '-mlto-zp=110'],
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
        # X2 (VAC+39D) picks the sphere for SLOAD BHIZ X2: 0 Earth, 2 Moon,
        # and -0.
        'all_states': {'vac+047': [0, 2, 0o77777]},
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
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
        'entries': ['IMUTEST', 'FINETIME', 'EARTHR', 'PROUT', 'P11OUT', 'GCOMPVER', 'RESTEST1'],
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
        # One case per routine: the cases carry many edits, and the header
        # must stay under 1 MB.
        # Host vectors and C64 cases chosen to reach every branch outcome;
        # TIME1/TIME2 and the CDUs vary; a quarter of the cases put edge
        # values (+-0, one LSB, 1/2, axis and plane vectors) in MPAC and
        # the operands.
        'cover': True,
        'counters': True,
        'edge_values': True,
        'freeze_stride': 640,
    },
}
