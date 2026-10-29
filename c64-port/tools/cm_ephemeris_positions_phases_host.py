#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,ast,os,subprocess,shlex
R=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--sanitize',action='store_true');a=p.parse_args()
line=next(x for x in(R/'tools/cm_ephemeris_positions_host.py').read_text().splitlines()if x.startswith('core='));core=ast.literal_eval(line[5:]);flags=shlex.split(a.cc)+['-DCM_EPHEMERIS_POSITIONS_DEBUG','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
for stage in range(1):
 for cohort in range(2):
  binary=f'build/cm-ephemeris-positions-phase-test-{stage}-{cohort}'+('-san'if a.sanitize else'');subprocess.run(flags+[f'-DCM_EPHEMERIS_POSITIONS_STAGE={stage}',f'-DCM_EPHEMERIS_POSITIONS_COHORT={cohort}','tests/test_cm_ephemeris_positions_phase.c','tests/cm_ephemeris_positions_phase_target.c','tests/frozen_words.c']+core+['-o',binary],cwd=R,check=True);subprocess.run([str(R/binary)],cwd=R,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
