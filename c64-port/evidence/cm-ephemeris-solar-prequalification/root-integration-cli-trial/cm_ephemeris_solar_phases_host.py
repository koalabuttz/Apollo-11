#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import argparse,ast,os,subprocess
R=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--sanitize',action='store_true');a=p.parse_args()
line=next(x for x in(R/'tools/cm_ephemeris_solar_host.py').read_text().splitlines()if x.startswith('core='));core=ast.literal_eval(line[5:]);flags=['cc','-DCM_EPHEMERIS_SOLAR_DEBUG','-std=c99','-O1'if a.sanitize else'-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-ffunction-sections','-Wl,--gc-sections']
if a.sanitize:flags+=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']
for stage in range(5):
 for cohort in range(4 if stage==3 else 2):
  binary=f'build/cm-ephemeris-solar-phase-test-{stage}-{cohort}'+('-san'if a.sanitize else'');subprocess.run(flags+[f'-DCM_EPHEMERIS_SOLAR_STAGE={stage}',f'-DCM_EPHEMERIS_SOLAR_COHORT={cohort}','tests/test_cm_ephemeris_solar_phase.c','tests/cm_ephemeris_solar_phase_target.c','tests/frozen_words.c']+core+['-o',binary],cwd=R,check=True);subprocess.run([str(R/binary)],cwd=R,check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
