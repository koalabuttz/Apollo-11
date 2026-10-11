# Comanche numeric keyboard source qualification

`cm_charin_number` executes actual Comanche055 CHARIN40,2000 through NUM octal or decimal accumulation, digit output, final R1/R2/R3 fraction/sign conversion and input blocking to real ENDOFJOB5217. It shares the verified native arithmetic/store body after independent execution of the original CM source qualifies every physical word. CM literal source stores remain distinct: DSPIN caller2117 in DSEXIT, MORNUM return2216, blocked NUM return2103, blocked8/9 return2066/2067, and the fraction factor's actual DECON address2223 in ADDRWD. Unrelated preexisting ADDRWD data is preserved whenever the fraction call did not execute.

The qualified domain follows the original numeric input paths: keys1..9/020, bank40/SUPER100 and any retained EB0..7, live DSPCOUNT0..19 except15, DECBRNCH lowbits0..2 with live8/9 requiring decimal, nonzero digit relay entries, decimal V/N prior sum0..99 and R1/R2/R3 sum0..9999. Negative counts block further digits. Octal CYL editing, signed zero, CLPASS branches, TP debris, fifth-character overflow, physical channel11 lamp/event and raw-register normalization are preserved. Unsupported inputs leave image and I/O/event unchanged. No AGC opcode or ordinary source address is used as a native callback.

The independent original CM oracle compares8192 complete2052-field checkpoints: all2048 physical words, SUPER/IRQ and channel11/event. Native and original snapshots have SHA256:
`cec4c49aa161ac1d644592329a0f34ce552dd58240548ae5a3dd78196b9775cc`.
The complete ASan/UBSan stream equals the original byte for byte. The oracle links no native service bodies. Original Pinball, T4 and Interpreter bytes, indentation-only assembler normalization and byte-identical pinned CM rope are recorded by `build/cm-charin-number-source-audit.json`.

Frozen64 complete original states pass strict host, sanitizer and native C64. VICE PAL mailbox `a500004002`; PRG18256 bytes (loaded18254); hardware/software stack19/51 bytes. Target section/PRG limits and extra zero-page storage are checked. `tools/cm_charin_number.mk` supplies portable host-only all/test/sanitize and explicit C64/differential targets. Evidence uses `build/cm-charin-number-*` and `build/cm_charin_number-*`, with complete streams and source assembly listing retained compressed.

Actual CM keyboard interrupt/admission, queued CHARIN selection and Executive completion are separate required composition. V/N key bodies, signs, clear, enter and illegal-key alarms require their own source translation/qualification.
