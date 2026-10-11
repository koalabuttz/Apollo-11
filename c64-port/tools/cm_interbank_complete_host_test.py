#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fresh source compilation, exact streaming proof, sanitizer replay, freeze."""
import os,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1]
def run(args,**kwargs):subprocess.run(args,cwd=R,check=True,**kwargs)
def main():
 run(['python3','tools/standard_lead_source.py'])
 run(['python3','tools/cm_interbank_complete_source.py'])
 common=['-std=c99','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
 native=['tools/cm_interbank_complete_snapshot.c','tests/cm_interbank_complete_cases.c','src/cm_interbank_complete.c','src/cm_fixed_compat.c','src/agc_word.c']
 run(['cc','-O2',*common,*native,'-o','build/cm-interbank-complete-native-v2'])
 run(['cc','-O2',*common,*native,'-o','build/cm-interbank-complete-native-v3'])
 objects=[f'deps/virtualagc/yaAGC/{name}.o' for name in ('agc_engine','agc_engine_init','rfopen')]
 run(['cc','-O2','-std=c99','-Iinclude','-Itests','-Ideps/virtualagc/yaAGC','tools/cm_interbank_complete_reference.c','tests/cm_interbank_complete_cases.c','tests/scenarios.c','src/agc_word.c',*objects,'-o','build/cm-interbank-complete-reference-v3'])
 env=dict(os.environ,CM_INTERBANK_COMPLETE_REFERENCE='cm-interbank-complete-reference-v3')
 run(['python3','tools/cm_interbank_complete_compare.py','native-v2','0','917504'],env=env)
 run(['python3','tools/cm_interbank_complete_compare.py','native-v3','917504','921088'],env=env)
 run(['python3','tools/cm_interbank_complete_verify_frozen.py','cm-interbank-complete-native-v3'])
 run(['cc','-O1','-g',*common,'-fsanitize=address,undefined','-fno-omit-frame-pointer',*native,'-o','build/cm-interbank-complete-sanitize'])
 run(['python3','tools/cm_interbank_complete_verify_frozen.py','cm-interbank-complete-sanitize'],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 run(['cc','-O2',*common,'tools/cm_interbank_complete_input_dump.c','tests/cm_interbank_complete_cases.c','src/agc_word.c','-o','build/cm-interbank-complete-input-dump'])
 run(['python3','tools/freeze_cm_interbank_complete.py'])
if __name__=='__main__':main()
