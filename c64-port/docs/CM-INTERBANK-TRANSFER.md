# Run Comanche INTER-BANK entries as native routines

Lane A. Milestone `cm-interbank-transfer`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The 13 Comanche055 INTER-BANK_COMMUNICATION entries are native routines in the control-transfer ABI registry, and the CADR after every genuine `TC BANKCALL/IBNKCALL/POSTJUMP/USPRCADR` (354 sites) is a compiled constant. BANKCALL and IBNKCALL round trips go through the trampoline into a native callee and back. 1,416 cases and 12,826 strict and sanitizer checks match `cm_interbank_complete` fed the original rope words, and every fourth case matches the host digests in VICE.

## Scope

Comanche055 INTER-BANK_COMMUNICATION.agc, the CM counterpart of `lm-interbank-transfer`. Destinations in files not yet translated resolve to `EXEC_UNMAPPED`.

## Source map

| AGC (fixed-fixed) | Native |
|---|---|
| 04662, SWCALL -1 04665 (`INCR Q`, then SWCALL; `docs/INTERBANK-Q.md`), 04666, 04675, 04701, 04704, QJUMP 04707, 04711, 04715, 04740, ISWCALL 04744, 04753, 04757, 04773 | `cm_interbank_routines` (`src/cm_interbank_routines.c`), each wrapping the certified `cm_interbank_complete` entry |
| inline CADR after each caller's `TC` | `cm_interbank_sites[354]` (`src/cm_interbank_sites.inc`, `tools/agc_sites_generate.py cm-interbank`) |

## Evidence

Hashes: `evidence/cm-interbank-transfer/SHA256.json`. Bulk files are assets of the `evidence-cm-interbank-transfer` release
(`python3 tools/evidence.py fetch cm-interbank-transfer`).

- `evidence/cm-interbank-transfer/cm-interbank-transfer-proof.json`: host routine-vs-entry comparison and round trips
- Independent assembly comparison: `evidence/cm-interbank-transfer/cm-interbank-transfer-sites.json`: the 354 sites, each checked against the source text and the assembled rope
- Host and sanitizer results: `make test-cm-interbank-transfer sanitize-cm-interbank-transfer` (`evidence/cm-interbank-transfer/cm-interbank-transfer-host.log`)
- C64 execution results: `make vice-cm-interbank-transfer` (`evidence/cm-interbank-transfer/cm-interbank-transfer-vice.log`)
