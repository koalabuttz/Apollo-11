#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Assemble whole-file reviewed scope from independently frozen group proofs."""
import pathlib,json,hashlib
r=pathlib.Path(__file__).resolve().parents[1]
source=json.loads((r/'evidence/lm-powered-file-source-map.json').read_text())
paths=json.loads((r/'build/lm-powered-proof-list.json').read_text())
proofs=[json.loads((r/p).read_text())for p in paths]
assert len(proofs)==7
assert sum(p['complete_states']for p in proofs)==1982464
assert sum(len(p['target_images'])for p in proofs)==38
assert source['emitted_words']==267
x={'original_file':'Luminary099/POWERED_FLIGHT_SUBROUTINES.agc','original_file_sha256':source['original_file_sha256'],'original_emitted_words':267,'source_word_map':'evidence/lm-powered-file-source-map.json','source_audit':'evidence/lm-powered-source-audit.json','rope_sha256':source['rope_sha256'],'native_source_group_proofs':paths,'whole_original_strict_sanitizer_states':1982464,'actual_C64_PRGs':38,'actual_C64_complete_frozen_states':1216,'whole_file_scope':'All emitted containing-source words mapped to reviewed native arithmetic, explicit source controls/returns and immutable constants. Source entry contracts are canonical ordinary words and documented fixed returns; mathematical routines retain raw signed zero/overflow semantics. This is whole-file numerical/control qualification, not a completed resident application.','unreachable_source':'Original AX line288 positive-zero TC RTNSAVER is unreachable under documented +3/-3 DIM direction contract; both directions reach negative zero and execute line290.','external_requirements':['Source Executive scheduling only at genuine completed-operation EDOP0 checks; internal numerical/basic stages retain canonical ownership.','Producer exclusion and shared scratch/VAC ownership across source operation phases.','Resident overlay allocation and application sensor/actuator integration remain separately qualified.','No arbitrary erasable indirect RVQ/caller classification or invalid ECADR source entry is claimed.'],'proof_sha256':{p:hashlib.sha256((r/p).read_bytes()).hexdigest()for p in paths}}
(r/'evidence/lm-powered-file-proof.json').write_text(json.dumps(x,indent=2)+'\n')
doc='''# Luminary POWERED_FLIGHT_SUBROUTINES

All 267 emitted words in the byte-original containing file are mapped to seven independently checked native groups and six fixed constants. The fixture overlays original POWERED, INTERPRETER, RTB_OP_CODES and SINGLE_PRECISION files without edits and retains the original LM declaration/data domain. Original file SHA256 is `379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1`; assembled rope SHA256 is `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.

The seven proofs cover 1,982,464 complete 2048-word-plus-IRQ original/native/full-ASAN-UBSAN checkpoints. Every unowned physical word is compared. Thirty-eight actual C64 diagnostics pass 1,216 complete frozen states reconstructed solely from original assembly. Deterministic XZ objects preserve each complete decoded corpus; identical original/native/sanitizer streams share one object per group.

| Source group | Native API | Complete states | C64 PRGs |
|---|---|---:|---:|
| CDUTRIGS / CD*TR*GS | `lm_powered_cdu_start/cosine/sine/next` | 655,360 | 10 |
| AX*SR*T plus actual TC saved return | `lm_powered_axis/return` | 131,072 | 2 |
| QUICTRIG plus actual TC A | `lm_powered_quick/return` | 65,536 | 2 |
| Eight permanent interfaces | `lm_powered_interface_*` | 868,352 | 8 |
| Four all-bank fixed RVQ origins | `lm_powered_fixed_return` | 131,072 | 4 |
| FLESHPOT complete matrix and source VXV | `lm_fleshpot` | 65,536 | 4 |
| CALCSMSC, FLESHPOT-1, BANKCALL/SWRETURN/reentry | `lm_fleshpot_interface_*` | 65,536 | 8 |

`lm_powered_constant_read` returns the five source SINSLOC/INDEXI words and original XNBECADR02664. Both QUICTRIG target diagnostics compare all six constants. The byte-original emitted-word map records exact source line, bank, physical fixed address, raw word and native group. The positive-zero AX return at original line288 is unreachable under its documented +3/-3 direction contract: source DIM reaches negative zero from either valid direction, then line290 returns.

The standalone FLESHPOT bank-return API accepts the composed FLESHPOT_FINISH output boundary with already normalized A/L. Arbitrary raw-register overflow inputs there are outside its source-return contract.

All native interfaces use the canonical 2048-word source image. Basic helpers preserve ordinary MODE; interpreted interfaces retain exact pending EDOP, LOC, bank and saved-return metadata. CALCSMSC enters bank10/INTBIT15=0, builds actual XNB E5,1664 from source trig inputs, restores source caller controls through SWRETURN and reenters at NEWOPS before RVQ. RVQ directly skips erasable classification and therefore accepts all ordinary fixed FCADR banks0..31. FLESHPOT VXV qualification covers its complete six-product source composition; it does not assert a generic cross-product library.

The source destination/caller contracts are explicit in the group packets. Invalid native boundaries reject without mutation. Source-only fixtures are diagnostic expected data; no expected flight values, opcode decoding or floating-point surrogate are used by the native arithmetic. Numerical phase returns never create Executive checks. External code must retain producer exclusion and shared scratch/VAC ownership, and only yield at genuine completed-source EDOP0 boundaries.

This review qualifies the whole containing source file's arithmetic and explicit return contracts. It does not certify a complete resident flight application: resident16KB overlay allocation, Executive/application composition and hardware producers/consumers remain separate acceptance work. Source provenance, source maps, complete stream identities, loaded extents, measured target stacks and mailboxes are in `evidence/lm-powered-file-proof.json` and its seven referenced group proofs.

Reproduce the fixture with `python tools/lm_powered_prepare_reference.py`, then use each group's standalone `tools/*.mk` all/test/sanitize/check-host/c64/vice-test targets. Their default SELECTOR is0. Run `python tools/lm_powered_file_review.py`, `python tools/package_lm_powered.py`, and `python tools/lm_powered_review.py` to reconstruct this packet after all diagnostics pass.
'''
(r/'docs/LUMINARY-POWERED-FLIGHT.md').write_text(doc)
files=(r/'build/lm-powered-packet-files.txt').read_text().splitlines()+['evidence/lm-powered-file-proof.json','docs/LUMINARY-POWERED-FLIGHT.md','tools/lm_powered_review.py']
files=sorted(set(files));manifest='evidence/lm-powered-evidence.sha256';(r/manifest).write_text(''.join(hashlib.sha256((r/p).read_bytes()).hexdigest()+'  '+p+'\n'for p in files));files.append(manifest)
(r/'build/lm-powered-commit-files.txt').write_text('\n'.join(sorted(files))+'\n')
print(len(files),'ready files; all267 containing words reviewed.')
