#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,json,shutil
p=Path(__file__).resolve().parents[1];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();files=set()
for directory in ['include','src','tests','tools','platform/c64']:
 for f in(p/directory).glob('*ephemeris_shift*'):
  if f.is_file():files.add(str(f.relative_to(p)))
files.add('docs/NATIVE-EPHEMERIS-SHIFT.md');names=['ephemeris-shift-source-map.json','ephemeris-shift-source-audit.log','ephemeris-shift-target-source.snapshots.bin.xz','ephemeris-shift-target-results.json','ephemeris-shift-strict.log','ephemeris-shift-sanitize.log','ephemeris-shift-c64.log','ephemeris-shift.prg','ephemeris-shift.map','ephemeris-shift-mailbox.bin','ephemeris-shift-vice.mon','ephemeris-shift-monitor.log','ephemeris-shift-vice.log','ephemeris-shift-vice-results.json','ephemeris-shift-hardware-stack.bin','ephemeris-shift-software-stack.bin']
for n in names:shutil.copyfile(p/'build'/n,p/'evidence'/n);files.add('evidence/'+n)
for n in ['ephemeris-shift-source-streams.json','ephemeris-shift-LM-reference.snapshots.bin.xz','ephemeris-shift-CM-reference.snapshots.bin.xz','ephemeris-shift-peer-review.json']:files.add('evidence/'+n)
x={'scope':'Original scalar SL5/SL8/SL16 dependency only; no ephemerides program or whole interpreter credit','source_states':131072,'state_words':2053,'strict_native':'PASS','full_ASAN_UBSAN':'PASS','source_streams':json.loads((p/'evidence/ephemeris-shift-source-streams.json').read_text()),'c64':json.loads((p/'evidence/ephemeris-shift-vice-results.json').read_text()),'source_map':'evidence/ephemeris-shift-source-map.json','whole_containing_files_certified':False,'native_instruction_decoder':False,'dependencies':{n:sha(p/n)for n in ['src/agc_word.c','src/agc_multiword.c','tests/frozen_words.c','include/agc_word.h','include/agc_multiword.h','include/overlay_runtime.h','tools/qualified_stream_host.py','tools/frozen_words.py','evidence/ephemeris-source-map.json']},'files':{n:sha(p/n)for n in sorted(files)}}
f=p/'evidence/ephemeris-shift-proof.json';f.write_text(json.dumps(x,indent=2)+'\n');files.add(str(f.relative_to(p)));(p/'build/ephemeris-shift-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print('PACKAGED',len(files),'immutable scalar prerequisite paths')
