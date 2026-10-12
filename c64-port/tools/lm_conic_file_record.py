#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-file evidence record for Luminary099/CONIC_SUBROUTINES.agc, which
four milestones translate together: lm-conic-kepler (KEPLERN, DELTIME,
KEPLOOP), lm-conic-timethet (TIMETHET, PARAM, GEOM, GETX), lm-conic-timerad
(TIMERAD, APSIDES) and lm-conic-lambert (LAMBERT, ITERATOR, LAMENTER, the
CONICS1 constants and the whole-file word check).

For each milestone it lists the independent assembly comparison (the pinned
yaAGC differential stream identity and its snapshot archive), the host and
sanitizer log and the C64 results, each with the SHA256 that milestone's
manifest records, and checks the bytes on disk against the manifest. It
writes evidence/lm-conic-lambert/lm-conic-file-record.json (the
certificate's comparison and C64 citation) and
evidence/lm-conic-lambert/lm-conic-file-checks.log (the four checks logs,
each under a header)."""
import hashlib,json,pathlib
R=pathlib.Path(__file__).resolve().parents[1];E=R/'evidence'
MILESTONES=[('lm-conic-kepler','KEPLERN, DELTIME, KEPLOOP',338),('lm-conic-timethet','TIMETHET, PARAM, GEOM, GETX and their tails',265),
 ('lm-conic-timerad','TIMERAD, APSIDES',123),('lm-conic-lambert','LAMBERT, ITERATOR, LAMENTER (and HAVENORM RVQ, CONICS1 constants)',316+1+29)]
def cite(m,name):
 man=json.loads((E/m/'SHA256.json').read_text())['files'][name];path=E/m/name
 if not path.is_file():raise SystemExit(f'FAIL {path} missing (tools/evidence.py fetch {m})')
 h=hashlib.sha256(path.read_bytes()).hexdigest()
 if h!=man['sha256']:raise SystemExit(f'FAIL {path} does not match its manifest')
 return dict(path=f'evidence/{m}/{name}',sha256=h)
record=dict(source_path='Luminary099/CONIC_SUBROUTINES.agc',assembled_words=1072,
 note='Each milestone runs the unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536 sampled entries and requires every 2053-word native row to equal the oracle row; the source map only accounts for words and is not the comparison.',milestones=[])
log=[]
for m,scope,words in MILESTONES:
 stream=json.loads((E/m/f'{m}-source-stream.json').read_text());results=json.loads((E/m/f'{m}-results.json').read_text())
 record['milestones'].append(dict(milestone=m,scope=scope,words=words,
  independent_assembly_comparison=dict(identity=cite(m,f'{m}-source-stream.json'),snapshots=cite(m,f'{m}-reference.snapshots.bin.xz'),
   decoded_sha256=stream['decoded_sha256'],decoded_bytes=stream['decoded_bytes'],states=stream['states'],status_counts=stream['status_counts']),
  host_and_sanitizer_results=cite(m,f'{m}-checks.log'),c64_execution_results=dict(cite(m,f'{m}-results.json'),vice=results['c64']['results']),
  source_map=cite(m,f'{m}-source-map.json')))
 log.append(f'== {m} ({scope})\n'+(E/m/f'{m}-checks.log').read_text())
if sum(x['words']for x in record['milestones'])!=record['assembled_words']:raise SystemExit('FAIL word counts do not add up')
(E/'lm-conic-lambert/lm-conic-file-record.json').write_text(json.dumps(record,indent=1)+'\n')
(E/'lm-conic-lambert/lm-conic-file-checks.log').write_text(''.join(log))
print('PASS CONIC_SUBROUTINES record: 4 milestones, 1072 words, every cited file matches its manifest')
