#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Rebuild both original source oracles; exact streaming proof/full sanitizer."""
from pathlib import Path
import os,subprocess
R=Path(__file__).resolve().parents[1]
def run(args,**kwargs):subprocess.run(args,cwd=R,check=True,**kwargs)
def main():
 run(['python3','tools/service_routines_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 native=['tools/service_routines_snapshot.c','tests/service_routines_cases.c','src/service_routines.c','src/agc_word.c']
 run(['cc','-O2',*common,*native,'-o','build/service-routines-native'])
 objects=[f'deps/virtualagc/yaAGC/{name}.o' for name in ('agc_engine','agc_engine_init','rfopen')]
 run(['cc','-O2','-std=c99','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/service_routines_reference.c','tests/service_routines_cases.c','tests/scenarios.c','src/agc_word.c',*objects,'-o','build/service-routines-reference'])
 run(['python3','tools/service_routines_compare.py','native'])
 run(['cc','-O1','-g',*common,'-fsanitize=address,undefined','-fno-omit-frame-pointer',*native,'-o','build/service-routines-sanitize'])
 run(['python3','tools/service_routines_verify_frozen.py','service-routines-sanitize'],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 run(['cc','-O2',*common,'tools/service_routines_input_dump.c','tests/service_routines_cases.c','-o','build/service-routines-input-dump'])
 run(['python3','tools/freeze_service_routines.py'])
if __name__=='__main__':main()
