# Original LM/CM NVSUBEND return

`dsky_nv_return` implements the actual mission-specific fixed-fixed NVSUBEND entry and original SUPDXCHZ through the saved fixed caller entry. It exchanges live raw A/L into NVQTEM/NVBNKTEM with the original correction, restores SUPERBNK, EBANK, FBANK and BBANK, and retains exact A/L/Q/PC effects. It uses the canonical image and explicit `p66_bias_io`, with no scheduler poll, instruction decoder, heap or floating point. Callers exclude producers.

The domain accepts saved fixed PC 02000..07777 and ordinary BBCON containing FB/EB/SUPER bits only, coherent incoming banks, REGZERO zero and legal explicit SUPER/IRQ. Unsupported contexts preserve all image and IO fields. It does not execute the selected caller or a formatter, and is separate from the unchanged NVSUB admission proof.

All 8,192 complete original-source states (2,048 words plus SUPER/IRQ) agree with strict native and full ASAN/UBSAN output, raw SHA256 `23af05ed89e8039eef66bd809b8a384556a2dc8b67d0b3c0be5b7aa5f879e944`. Coverage includes both missions, every saved bank/EB/SUPER combination, raw A/L/Q patterns, incoming banks and interrupt states. The frozen checker compares 64 full states plus 22 unchanged invalid guards.

PAL C64 in VICE passes all 64 original states: mailbox `a500004000`, PRG 4,539 bytes, hardware stack 14 bytes including 14 ROM bytes, software stack 20 bytes, no additional zero-page allocation. Tools are uniquely namespaced in `tools/dsky_nv_return.mk`; `all` builds both the frozen checker and full snapshot route, `test` runs frozen states, and the sanitizer target compares the complete source stream. `evidence/dsky-nv-return-proof.json` records exact source and artifact identities. The original GPL engine is host-only.
