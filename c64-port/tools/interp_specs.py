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
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'counters': True,
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
    # Lane G: Comanche055 CM_BODY_ATTITUDE (bank 37), basic and interpretive.
    'cm-body-attitude': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'counters': True,
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
        'counters': True,
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
    'cm-p40-p47': {
        'rope_fallback': True,
        # Any DANZIG can find a job waiting: every word after one is a CHANG2
        # resume key.
        'resume_keys': True,
        'counters': True,
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
        'counters': True,
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
        'counters': True,
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
        'counters': True,
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
        'counters': True,
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
        'counters': True,
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
        # CARD reads FCARD +1 .. +6.
        'cells': {0o3257: [0o2541, 0o2543, 0o2743, 0o2745, 0o5324],
                  0o3261: [0o77773, 0o77774, 0o77775, 0o77776]},
        'fixed_tables': {'FCARD': (0, 6)},
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
        'counters': True,
        'missions': ('cm',),
        'file': {'cm': 'TVCINITIALIZE.agc'},
        'base': {'cm': 'cm_tvc_init'},
        'symbol': {'cm': 'cm_tvc_init'},
        'basic': 'full',
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
        'counters': True,
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
        'counters': True,
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
        'rope_fallback': True,
        'counters': True,
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
        # 10 (the Kalman initialisation count).
        'cells': {0o3106: [0, 1, 2], 0o3107: [0, 2, 4],
                  0o3217: [0o77774, 0o77775, 0o77776] + list(range(11)),
                  # RMANNDX/PMANNDX/YMANNDX (E6,1656-1660) as STICKCHK
                  # leaves them (MASK THREE).
                  0o3256: [0, 1, 2, 3], 0o3257: [0, 1, 2, 3], 0o3260: [0, 1, 2, 3],
                  # RATEINDX (1130), the manual rate index into MANTABLE:
                  # 0, 2, 4 or 6, as FRESH START and P40-P47 set it.
                  0o1130: [0, 2, 4, 6]},
        'fixed_tables': {'GAIN1': (-3, 10), 'GAIN2': (-3, 10), 'DACLIMIT': (-1, 1),
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
        'counters': True,
        'missions': ('cm',),
        'file': {'cm': 'AUTOMATIC_MANEUVERS.agc'},
        'base': {'cm': 'cm_auto_man'},
        'symbol': {'cm': 'cm_auto_man'},
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
        'counters': True,
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
    },
    # Lane G: Comanche055 CM_ENTRY_DIGITAL_AUTOPILOT, the whole file (basic and interpretive)
    # as routines at its original addresses.
    'cm-entry-dap': {
        'rope_fallback': True,
        'counters': True,
        'missions': ('cm',),
        'file': {'cm': 'CM_ENTRY_DIGITAL_AUTOPILOT.agc'},
        'base': {'cm': 'cm_entry_dap'},
        # CM/DAPIC's TS CM/GYMDT overflows on some TIME1/PIPTIME states and
        # skips into WAITLIST's 2CADR READGYMB: the AGC runs that word as TC
        # READGYMB (its BBCON then runs as the return), and so does the port.
        'execute_data': ('20,3714', '20,3715'),
        'symbol': {'cm': 'cm_entry_dap'},
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
        'freeze_stride': 64,
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
}
