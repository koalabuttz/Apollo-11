# Run Luminary INTER-BANK entries as native routines

Lane A. Milestone `lm-interbank-transfer`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The 13 Luminary099 INTER-BANK_COMMUNICATION entries are native routines in the control-transfer ABI registry, and the CADR after every genuine `TC BANKCALL/IBNKCALL/POSTJUMP/USPRCADR` (448 sites) is a compiled constant. BANKCALL and IBNKCALL round trips run through the trampoline into a native callee and back via SWRETURN/ISWRETRN. 1,792 cases and 15,826 strict and sanitizer checks match `lm_interbank` fed the original rope words, and every fourth case matches the host digests in VICE.

## Scope

Luminary099 INTER-BANK_COMMUNICATION.agc. Auditor finding 4 (2026-10-11): the certified native code set Z/FB to the destination and returned; nothing called the destination. Now every entry is registered at its original fixed-fixed address and transfers end in `agc_transfer_run` (docs/NATIVE_TRANSFER_ABI.md). POSTJUMP/USPRCADR destinations in files not yet translated resolve to `EXEC_UNMAPPED`, which is the destination file's open work.

## Source map

| AGC (fixed-fixed) | Native |
|---|---|
| BANKCALL 04616, SWCALL 04622, SWRETURN 04631, POSTJUMP 04635, BANKJUMP 04640, QJUMP 04643, MAKECADR 04645, SUPDACAL 04651, IBNKCALL 04674, ISWCALLL 04700, ISWRETRN 04707, USPRCADR 04713, SUPERSW 04727 | `lm_interbank_routines` (`src/lm_interbank_routines.c`), each wrapping the certified `lm_interbank` step |
| inline CADR after each caller's `TC` | `lm_interbank_sites[448]` (`src/lm_interbank_sites.inc`, `tools/agc_sites_generate.py lm-interbank`) |
| SUPDACAL passive constant read | the program's `fixed_data` reader |

## Evidence

Hashes: `evidence/lm-interbank-transfer/SHA256.json`. Bulk files are assets of the `evidence-lm-interbank-transfer` release
(`python3 tools/evidence.py fetch lm-interbank-transfer`).

- `evidence/lm-interbank-transfer/lm-interbank-transfer-proof.json`: host routine-vs-step comparison, three-step round trips and the unmapped-destination check
- Independent assembly comparison: `evidence/lm-interbank-transfer/lm-interbank-transfer-sites.json`: the 448 sites, each checked against the source text and the assembled rope
- Host and sanitizer results: `make test-lm-interbank-transfer sanitize-lm-interbank-transfer` (`evidence/lm-interbank-transfer/lm-interbank-transfer-host.log`)
- C64 execution results: `make vice-lm-interbank-transfer` (`evidence/lm-interbank-transfer/lm-interbank-transfer-vice.log`)
