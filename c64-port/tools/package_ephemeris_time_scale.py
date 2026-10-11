#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,json,shutil
p=Path(__file__).resolve().parents[1];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();files=set()
for directory in ['include','src','tests','tools','platform/c64']:
 for f in(p/directory).glob('*ephemeris_time_scale*'):
  if f.is_file():files.add(str(f.relative_to(p)))
files.add('docs/NATIVE-EPHEMERIS-TIME-SCALE.md');names=['ephemeris-time-scale-source-map.json','ephemeris-time-scale-source-audit.log','ephemeris-time-scale-target-source.snapshots.bin.xz','ephemeris-time-scale-target-results.json','ephemeris-time-scale-strict.log','ephemeris-time-scale-sanitize.log','ephemeris-time-scale-c64.log','ephemeris-time-scale.prg','ephemeris-time-scale.map','ephemeris-time-scale-mailbox.bin','ephemeris-time-scale-vice.mon','ephemeris-time-scale-monitor.log','ephemeris-time-scale-vice.log','ephemeris-time-scale-vice-results.json','ephemeris-time-scale-hardware-stack.bin','ephemeris-time-scale-software-stack.bin']
for n in names:shutil.copyfile(p/'build'/n,p/'evidence'/n);files.add('evidence/'+n)
for n in ['ephemeris-time-scale-source-streams.json','ephemeris-time-scale-LM-reference.snapshots.bin.xz','ephemeris-time-scale-CM-reference.snapshots.bin.xz','ephemeris-time-scale-peer-review.json']:files.add('evidence/'+n)
x={'scope':'Original scalar SR14 dependency only; no ephemerides program or whole interpreter credit','source_states':131072,'state_words':2053,'strict_native':'PASS','full_ASAN_UBSAN':'PASS','source_streams':json.loads((p/'evidence/ephemeris-time-scale-source-streams.json').read_text()),'c64':json.loads((p/'evidence/ephemeris-time-scale-vice-results.json').read_text()),'source_map':'evidence/ephemeris-time-scale-source-map.json','whole_containing_files_certified':False,'native_instruction_decoder':False,'dependencies':{n:sha(p/n)for n in ['src/agc_word.c','src/agc_multiword.c','tests/frozen_words.c','include/agc_word.h','include/ephemeris_shift.h','include/agc_multiword.h','include/overlay_runtime.h','tools/qualified_stream_host.py','tools/frozen_words.py','evidence/ephemeris-source-map.json']},'files':{n:sha(p/n)for n in sorted(files)}}
f=p/'evidence/ephemeris-time-scale-proof.json';f.write_text(json.dumps(x,indent=2)+'\n');files.add(str(f.relative_to(p)));(p/'build/ephemeris-time-scale-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print('PACKAGED',len(files),'immutable scalar prerequisite paths')
