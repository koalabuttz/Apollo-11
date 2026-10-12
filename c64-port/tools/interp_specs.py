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
    # Lane G: Comanche055 SERVICER207 (bank 37), basic and interpretive.
    'cm-servicer': {
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
        'entries': ['PREREAD', 'PREREAD1', 'READACCS', 'REDO5.5', 'CHEKAVEG', 'SERVICER', 'NORMLIZE',
                    'REREADAC', 'SERVEXIT', 'CALCGRAV'],
        'channels': {0o11: 'channel11'},
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
    # Lane G: Comanche055 CM_BODY_ATTITUDE (bank 37), basic and interpretive.
    'cm-body-attitude': {
        'missions': ('cm',),
        'file': {'cm': 'CM_BODY_ATTITUDE.agc'},
        'base': {'cm': 'cm_body_attitude'},
        'symbol': {'cm': 'cm_body_attitude'},
        'basic': 'full',
        # CM/POSE: the entry job P61-P67 starts (2CADR at 26,2431); REDOPOSE
        # (REPOSADR) is the file's own restart point.
        'entries': ['CM/POSE'],
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
}
