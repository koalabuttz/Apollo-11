#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-line original Executive/Waitlist inventory; bounded proofs are explicit."""
import pathlib, re, hashlib, json
R = pathlib.Path(__file__).resolve().parents[1]
listing = (R/'build/fixed-data/Comanche055/assembly.lst').read_text().splitlines()
assert hashlib.sha256((R/'build/fixed-data/Comanche055/MAIN.agc.bin').read_bytes()).hexdigest() == '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
profiles = {
 'EXECUTIVE.agc': [
  (37,69,'admission','cm-admission-complete'), (72,85,'context exchange','cm-context-change'),
  (90,94,'sleep','cm-sleep'), (97,105,'wake','cm-wake'),
  (107,114,'priority','cm-priority-complete'), (117,119,'finish','cm-basic-finish'),
  (121,126,'admission return/constants','cm-admission-complete'),
  (134,205,'admission','cm-admission-complete'),
  (207,274,'shared exchange','cm-context-change'),
  (278,287,'sleep','cm-sleep'), (287,304,'shared scan/idle','cm-idle-dispatch'),
  (307,353,'wake','cm-wake'), (355,365,'priority','cm-priority-complete'),
  (368,447,'shared finish/sleep/priority scan','cm-basic-finish'),
  (455,493,'idle/basic dispatch/constants','cm-idle-dispatch')],
 'WAITLIST.agc': [
  (129,161,'WAITLIST/TWIDDLE/return','cm-waitlist-extended'),
  (165,182,'FIXDELAY/VARDELAY','cm-delay-image'),
  (188,214,'SVCT3/constants','cm-svct3'),
  (221,376,'WAITLIST insertion/full-list alarm entry','cm-waitlist-extended'),
  (378,419,'T3 queue scan','cm-entry-tasks'),
  (423,443,'TASKOVER/RESUME','cm-task-transfer'),
  (488,556,'LONGCALL/cycle/return/constants','cm-longcall')]
}
data_ops = {'OCT','DEC','CADR','BBCON','ADRES','2CADR','-2CADR','GENADR','FCADR'}
layout_ops = {'BLOCK','BANK','SETLOC','EBANK=','COUNT','COUNT*','EQUALS'}
missions = []
for filename, groups in profiles.items():
    source = R.parent/'Comanche055'/filename
    original = source.read_bytes()
    start = next(i for i,x in enumerate(listing) if '$'+filename in x)
    end = next(i for i in range(start+1,len(listing)) if 'End of include-file '+filename in listing[i])
    assembled = {}
    for line in listing[start+1:end]:
        m = re.match(r'\d+,(\d+):\s+((?:[0-7]{2},)?[0-7]{4})\s+([0-7]{5})(?:\s+([0-7]{5}))?\s',line)
        if m:
            assembled[int(m[1])] = {'address_octal':m[2], 'assembled_words_octal':[x for x in m.groups()[2:] if x]}
    rows = []
    for number, text in enumerate(original.decode().splitlines(),1):
        tokens = text.split('#',1)[0].split()
        row = dict(line=number, original_text=text)
        if number in assembled:
            row.update(assembled[number])
            row['classification'] = 'immutable data' if any(x in data_ops for x in tokens) else 'compiled instruction'
            group = next((x for x in groups if x[0]<=number<=x[1]),None)
            row['native_source_group'] = group[2] if group else 'unmapped'
            row['proof_prefix'] = group[3] if group else None
            row['qualification'] = 'bounded primitive source domains; no complete-file valid-caller certificate'
            if 'NO.CORES' in tokens:
                row['classification'] = 'immutable numeric word with source fault instruction alias'
                row['fault_alias'] = 'DEC6 is an executed TC6 word only after an invalid +0 core-priority CCS path; that fault domain remains open'
        elif tokens:
            row['classification'] = 'assembler directive' if any(x in layout_ops for x in tokens) else 'non-emitting source declaration'
        else:
            row['classification'] = 'comment/blank'
        rows.append(row)
    assert all(row.get('native_source_group') != 'unmapped' for row in rows), [(row['line'], row['original_text']) for row in rows if row.get('native_source_group') == 'unmapped']
    missions.append(dict(file=filename, original_sha256=hashlib.sha256(original).hexdigest(),
                         source_lines=len(rows), emitted_source_lines=len(assembled),
                         bounded_proof_files=[{'path':'evidence/'+prefix+'-proof.json','sha256':hashlib.sha256((R/'evidence'/(prefix+'-proof.json')).read_bytes()).hexdigest()} for prefix in sorted(set(g[3] for g in groups)) if (R/'evidence'/(prefix+'-proof.json')).exists()],
                         emitted_words=sum(len(x['assembled_words_octal']) for x in assembled.values()),
                         lines=rows))
report = dict(mission='Comanche055', original_layout_rope_sha256='6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061',
              complete_file_qualified=False, files=missions,
              open_domains=['source CCSHOLE/+0 priority fault paths and their eventual alarm/restart bodies',
                            'raw/high Q/index/return caller closure beyond each explicit compiled ABI',
                            'all actual application/task bodies selected by these services and a final resident registry',
                            'arbitrary interrupt timing outside certified compiled source safepoints',
                            'source state combinations outside each retained corpus/predicate'],
              scope='whole-line/instruction/data/directive inventory only; bounded helper presence does not imply full original file qualification')
(R/'evidence/cm-scheduler-source-audit.json').write_text(json.dumps(report,indent=2)+'\n')
print('Mapped all original Executive/Waitlist emitting lines; complete-file qualification FALSE')
for file in missions:
    print(file['file'],file['emitted_words'],'words',file['source_lines'],'original lines')
