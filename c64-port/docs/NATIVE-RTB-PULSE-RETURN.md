# Original RTB gyro bank-return continuation

`rtb_pulse_return` executes the actual trailing PULSEIMU instruction after its external IMUPULSE bank return. The word at LM bank10,03711 is TCF DANZIG (16061); CM bank22,03657 is the corresponding word16030. A, Q, pending LOC/EDOP/BANKSET, IRQ, SUPER and all unowned memory remain unchanged. Source L normalization is retained. The function stops at the true DANZIG entry without running its instruction or introducing a scheduling seam.

The declared nonnull/ordinary canonical bank/raw A/L/producer domain matches the existing RTB modules. Successful external returns naturally reach this bank/PC boundary; this additive entry also qualifies standalone ordinary return states. It does not execute the external gyro callee, scheduled tasks, physical IMU device or later interpreter continuation.

Two original-only contexts contain 65,536 deterministic cases each, varying raw register patterns, every ordinary Q word, all8 EBs/SUPER classes, IRQ and nonzero unowned-memory patterns. All 131,072 complete 2048-word source/strict/full-ASAN-UBSAN states match. Both source words are independently checked against the unchanged original ropes and existing RTB source audit. One deterministic XZ stream per mission retains all identical logical roles and decoded identities.

Two actual PAL C64 diagnostics pass 32 frozen complete original states each (64 total). Their PRGs are3512bytes, compiler ZP0; both map allocations remain below$C000 and stack guards pass (14 hardware/51 software bytes measured). The diagnostics do not require a helper warning exception.

This packet closes the sole previously unmapped local LM RTB instruction. The additive LM whole-file coverage ledger retains all130 original emitted words and dependencies; CM's QUALITY1/QUALITY2 algorithm remains separately uncovered. Existing frozen packets and the necessary bank07 IMUPULSE metadata erratum remain unchanged. Whole containing-file source closure does not imply arbitrary interpreter dispatch, physical pulse delivery or complete application execution.
