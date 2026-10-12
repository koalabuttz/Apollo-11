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
        # PAUTNO's tests: channel 30 bit 10 (PGNCS in control, 0 = yes) and
        # channel 31 bit 14 (auto mode, 0 = yes).
        'channels': {0o30: 'chan30', 0o31: 'chan31'},
        'includes': ['lm_findcduw_rtb.h', 'lm_channel_io.h'],
        'native_sources': ['src/lm_findcduw_rtb.c', 'src/rtb_vector.c', 'src/lm_powered_quick.c',
                           'src/lm_single_trig.c', 'src/rtb_angles.c'],
        'target': 'tests/findcduw_target.c',
        # The GUIDAP loop's group only fits the C64 without inlining, and
        # with one case per routine.
        'c64_flags': ['-fno-inline'], 'c64_per_point': 1,
        'ebank': 6,
        'ovfind': [1, 0o77776],
        # NDXCDUW (E6 1650) as FINDCDUW sets it: 0 docked, 1 not.
        'all_states': {'3250': [0, 1]},
        # Keys inside the file's loops and subroutines, with the counter or
        # saved return their caller left: TEM2 the CDUWXFR index (2, 1, 0)
        # or FLTRSUB's saved Q (its two calls), TEM4 ARCTRGSP's saved Q
        # (its three calls).
        # FINDCDUW runs in vector mode from FINDCDUW-2's VLOAD to PAUTNO's
        # RTB NORMUNIT, which unitizes that vector (MODE -1).
        'states': {'FINDCDUW': {'163': [0o77776]}, 'FINDCDUW+5': {'163': [0o77776]}, 'PAUTNO+1': {'163': [0o77776]},
                   'CDUWXFR+6': {'142': [0, 1, 2]}, 'CDUWXFR+24': {'142': [0, 1, 2]},
                   'FLTRSUB+11': {'142': [0o3207, 0o3213]}, 'FLTRSUB+15': {'142': [0o3207, 0o3213]},
                   'ARCTRGSP+21': {'144': [0o3556, 0o3565, 0o3574]}, 'ARCTRGSP+22': {'144': [0o3556, 0o3565, 0o3574]},
                   'ARCTRGSP+25': {'144': [0o3556, 0o3565, 0o3574]}, 'ARCTRGSP+28': {'144': [0o3556, 0o3565, 0o3574]}},
    },
}
