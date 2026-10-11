#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild committed complete P66 proof with currentcore and separate names."""
import argparse,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default='mos-c64-clang');p.add_argument('--c1541',default='c1541');p.add_argument('--vice',default='x64sc');p.add_argument('--rom-dir',required=True);p.add_argument('--data-dir',required=True);a=p.parse_args()
# Original builders remain immutable. Their source/test fixture names are
# unchanged; only generated artifact filenames and monitor output paths differ.
for old,stem in [('p66_complete_build','native-exchange-complete-build'),('p66_complete_vice_test','native-exchange-complete-vice')]:
    text=(ROOT/'tools'/f'{old}.py').read_text().replace('p66-complete','native-exchange-complete');script=ROOT/'build'/f'{stem}.py';script.write_text(text)
    # Builders import support modules from tools; copied files live in build.
    if old.endswith('build'):
        text='import sys\nfrom pathlib import Path\nsys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))\n'+text;script.write_text(text)
        command=['python3',str(script),'--cc',a.cc,'--c1541',a.c1541]
    else:command=['python3',str(script),'--vice',a.vice,'--rom-dir',a.rom_dir,'--data-dir',a.data_dir]
    subprocess.run(command,cwd=ROOT,check=True)
