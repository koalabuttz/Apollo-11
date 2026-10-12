# SPDX-License-Identifier: MIT
"""Milestone specs for tools/interp_compile.py: the file, its entries, the
outside basic routines it calls (with their inline word counts) and the
native routines its RTBs reach."""
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
        # Words no routine compiles yet, and why (the source map records them).
        'partial': {'cm': 'SXTNB, SXTLOGIC, CALCSXA and SXTANG (with NB2NB1, NB1NB2 and their '
                          'constants) wait for a Comanche MXV body and native TRANSP1/TRANSP2'},
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
        'freeze_stride': 32,
        # ISITAUTO leaves +0 or +1 in A; CCS A's negative arms are the
        # interpretive words that follow.
        'states': {'NEWDELHI+2': {'0': [0, 1]}},
        # The file's EBANK= TTEMP (E6, as its 2CADR BBCONs carry): its
        # switched names (KSPNDX, BRATE...) mean nothing under another EB.
        'ebank': 6,
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
}
