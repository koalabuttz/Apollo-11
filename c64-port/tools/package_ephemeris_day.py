#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib,json,shutil
p=Path(__file__).resolve().parents[1];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();files=set()
for directory in ['include','src','tests','tools','platform/c64']:
 for f in(p/directory).glob('*ephemeris_day*'):
  if f.is_file():files.add(str(f.relative_to(p)))
files.add('docs/NATIVE-EPHEMERIS-DAY.md');names=['ephemeris-day-source-map.json','ephemeris-day-source-audit.log','ephemeris-day-target-source.snapshots.bin.xz','ephemeris-day-target-results.json','ephemeris-day-strict.log','ephemeris-day-sanitize.log','ephemeris-day-c64.log','ephemeris-day.prg','ephemeris-day.map','ephemeris-day-mailbox.bin','ephemeris-day-vice.mon','ephemeris-day-monitor.log','ephemeris-day-vice.log','ephemeris-day-vice-results.json','ephemeris-day-hardware-stack.bin','ephemeris-day-software-stack.bin']
for n in names:shutil.copyfile(p/'build'/n,p/'evidence'/n);files.add('evidence/'+n)
for n in ['ephemeris-day-source-streams.json','ephemeris-day-LM-reference.snapshots.bin.xz','ephemeris-day-CM-reference.snapshots.bin.xz','ephemeris-day-peer-review.json']:files.add('evidence/'+n)
x={'scope':'Original literal CSTODAY DDV basic body only; no ephemerides program or whole interpreter credit','source_states':131072,'state_words':2053,'strict_native':'PASS','full_ASAN_UBSAN':'PASS','source_streams':json.loads((p/'evidence/ephemeris-day-source-streams.json').read_text()),'c64':json.loads((p/'evidence/ephemeris-day-vice-results.json').read_text()),'source_map':'evidence/ephemeris-day-source-map.json','whole_containing_files_certified':False,'native_instruction_decoder':False,'dependencies':{n:sha(p/n)for n in ['src/agc_word.c','src/agc_multiword.c','src/guidance_dp_divide.c','tests/frozen_words.c','include/agc_word.h','include/agc_multiword.h','include/overlay_runtime.h','tools/qualified_stream_host.py','tools/frozen_words.py','evidence/ephemeris-source-map.json']},'files':{n:sha(p/n)for n in sorted(files)}}
f=p/'evidence/ephemeris-day-proof.json';f.write_text(json.dumps(x,indent=2)+'\n');files.add(str(f.relative_to(p)));(p/'build/ephemeris-day-commit-files.txt').write_text('\n'.join(sorted(files))+'\n');print('PACKAGED',len(files),'immutable scalar prerequisite paths')
