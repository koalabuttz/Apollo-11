#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Additive frozen-packet correction, preserving original proof identities."""
import hashlib,json,os,pathlib,subprocess
R=pathlib.Path(__file__).resolve().parents[1];B=R/'build';E=R/'evidence'
old=E/'cm-scan-zero-strict-cast-original.c';new=R/'src/cm_scan_zero.c'
assert new.read_text()==old.read_text().replace('buf2=priority-1u;','buf2=(agc_word)(priority-1u);')
assert (E/'cm-scan-zero-strict-cast-original.sha256').read_bytes()==(E/'cm-scan-zero-evidence.sha256').read_bytes()
if (B/'cm-scan-zero-commit-files.txt').exists():
 assert (E/'cm-scan-zero-strict-cast-original-commit-files.txt').read_bytes()==(B/'cm-scan-zero-commit-files.txt').read_bytes()
flags=['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests']
p=subprocess.run(flags+['-fsyntax-only',str(old.relative_to(R))],cwd=R,capture_output=True,text=True)
assert p.returncode!=0 and 'conversion' in p.stderr
diagnostic=E/'cm-scan-zero-strict-cast-original-diagnostic.log';diagnostic_data=(p.stdout+p.stderr).encode();assert not diagnostic.exists() or diagnostic.read_bytes()==diagnostic_data;diagnostic.write_bytes(diagnostic_data)
core=['src/cm_scan_zero.c','src/cm_basic_finish.c','src/cm_basic_sleep.c','src/cm_priority_complete.c','src/cm_dispatch.c','src/cm_executive.c','src/cm_native_image.c','src/agc_word.c','tests/cm_scan_zero_cases.c','platform/host/cm_scan_zero_snapshot.c']
subprocess.run(flags+core+['-o','build/cm-scan-zero-strict-native'],cwd=R,check=True)
subprocess.run(['cc','-std=c99','-O1','-g','-Wall','-Wextra','-Werror','-Wconversion','-pedantic','-Iinclude','-Itests','-fsanitize=address,undefined']+core+['-o','build/cm-scan-zero-strict-sanitize'],cwd=R,check=True)
proof=json.loads((E/'cm-scan-zero-proof.json').read_text());rows=[]
for role in ['native','sanitize']:
 h=hashlib.sha256();size=0
 p=subprocess.Popen([str(B/f'cm-scan-zero-strict-{role}')],cwd=R,stdout=subprocess.PIPE,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
 while True:
  data=p.stdout.read(1048576)
  if not data:break
  h.update(data);size+=len(data)
 assert p.wait()==0 and size==proof['binary_bytes'] and h.hexdigest()==proof['snapshot_sha256']
 rows.append({'role':role,'complete_states':12288,'bytes':size,'source_stream_sha256':h.hexdigest()})
subprocess.run(['python3','tools/cm_scan_zero_build.py','--cc','/workspace/toolchains/llvm-mos/bin/mos-c64-clang','--c1541','/workspace/toolchains/vice/usr/bin/c1541'],cwd=R,check=True)
target=[]
for p in sorted(B.glob('cm-scan-zero-*.prg')):
 original=E/p.name;assert original.exists() and p.read_bytes()==original.read_bytes()
 target.append({'rebuilt_payload':str(p.relative_to(R)),'previous_actual_c64_payload':str(original.relative_to(R)),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'byte_identical':True})
assert len(target)==3
report={'correction':'explicit agc_word cast of positive-priority diminished result; implicit assignment already did same conversion. No numerical/control algorithm change. Original strict-Wconversion compilation rejected missing explicit cast.','original_frozen_source_sha256':hashlib.sha256(old.read_bytes()).hexdigest(),'corrected_source_sha256':hashlib.sha256(new.read_bytes()).hexdigest(),'original_ledger_sha256':hashlib.sha256((E/'cm-scan-zero-evidence.sha256').read_bytes()).hexdigest(),'original_allowlist_sha256':hashlib.sha256((E/'cm-scan-zero-strict-cast-original-commit-files.txt').read_bytes()).hexdigest(),'strict_and_full_san':rows,'fresh_c64_payload_identity_checks':target,'actual_c64_verdict_unchanged':'a500004800','scope':'additive source/compiler bookkeeping correction. Historical source/ledger/allowlist retained byte-identically, every original state and actual target artifact identity unchanged. Run historical exporter only with its retained original source; this additive script validates corrected source without rewriting historical ledger.'}
out=E/'cm-scan-zero-strict-cast-proof.json';data=(json.dumps(report,indent=2)+'\n').encode();assert not out.exists() or out.read_bytes()==data;out.write_bytes(data)
files=['src/cm_scan_zero.c','tools/cm_scan_zero_strict_cast.py']+[p.relative_to(R).as_posix() for p in E.glob('cm-scan-zero-strict-cast-*') if p.is_file()]
ledger=E/'cm-scan-zero-strict-cast-evidence.sha256';files=sorted(set(files)-{ledger.relative_to(R).as_posix()});ledger.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in files));files.append(ledger.relative_to(R).as_posix())
(B/'cm-scan-zero-strict-cast-commit-files.txt').write_text('\n'.join(files)+'\n')
print('PASS: strict and fullSAN12288 source states, all3 freshly compiled C64 payloads byte-identical; historical packet bytes preserved')
