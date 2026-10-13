---
id: lanec-pin-cert
title: Draft the PINBALL_GAME_BUTTONS_AND_LIGHTS full-file certificates
date: 2026-10-13T10:45Z
lane: C
---
Pending complete-coverage certificates for Luminary099 and Comanche055 PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, for the certificate reviewer. `tools/lanec/wholefile_pin.py` assigns every assembled word (2,378 LM, 2,379 CM) to `lanec-pin-gaps`, to `lanec-pw` (the earlier Pinball modules) or to the file's constants. It shows that every instruction word except two LM and three CM unreachable ones executes in the unchanged original program in Virtual AGC under cases the native code matches. Every entry, external-callee return, BANKCALL return point and POSTJUMP/BANKJUMP target is a transfer-ABI routine (273 LM, 268 CM; `lanec-pin-transfer`). The evidence covers 96 suites with 128,625 oracle cases (strict and ASan/UBSan), zero differing, and 3,878 refusals with the state unchanged. It also includes fresh VICE runs (`vice-lanec-pin-gaps` 50 checks, `vice-lanec-pw` 917, `vice-lanec-pin-transfer` 575) and a C64 branch record in which the frozen rows take every host branch arc apart from the documented exceptions. `tools/cert_precheck.py` reports no failures. See `docs/LANEC-PIN-CERT.md`.
