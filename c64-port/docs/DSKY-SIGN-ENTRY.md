# Native LM and CM plus/minus input

`dsky_charin_sign` and `cm_charin_sign` execute actual LM CHARIN40,2077 and CM CHARIN40,2000 for +032 and -033. They include DSPLOCK/key-release lamp behavior, SIGNTEST accepted/rejected branches, GETINREL, +ON/-ON with both actual 11DSPIN relay writes, per-component decimal-mode bits and FIXCLPAS through actual ENDOFJOB5155/5217. Every program-specific caller/return address is retained. The shared arithmetic/store body is qualified separately against both original ropes.

The domain is bank40/SUPER100, any retained EB0..7, either IRQ state, ordinary DSPCOUNT/DECBRNCH/CLPASS and NOUT words, including signed zero and NOUT overflow correction. Accepted signs occur only at R1D1/R2D1/R3D1 with no prior sign in the lowtwo decimal bits. Both affected relay words must be nonzero for the source DSPIN domain; rejected signs need no relay access. Unsupported input leaves the complete image, I/O and event unchanged. Producers are excluded by the caller; no source opcode is decoded.

Two4096-case independent original corpora compare8192 complete2052-field checkpoints: all2048 physical words, SUPER/IRQ and channel11/event. Snapshot SHA256:
`b1d93bed1a5bd2a53dad0e1a319d665310fc1d249ea9a2b67b7e846b78eaf823`.
The complete ASan/UBSan stream equals the original byte for byte. The oracle links no native bodies. Both original Pinball/T4/Interpreter files, CM indentation-only normalization and both byte-identical pinned ropes are recorded in build/dsky-charin-sign-source-audit.json.

Frozen64 complete original states pass strict host, sanitizer and native C64. They cover both signs, all CLPASS sign/zero classes, accepted component selection and consecutive-sign rejection for both programs. VICE PAL mailbox `a500004002`; PRG8919 bytes (loaded8917); hardware/software stack16/35 bytes. Target allocation bounds and extra zero-page storage are checked. `tools/dsky_charin_sign.mk` has portable host-only all/test/sanitize plus explicit differential/C64 targets. Full streams and original assembly listings are retained compressed.

Queued-job completion and hardware delivery compose the separately source-qualified keyboard services. Clear, enter, reset, key release and illegal-key alarm bodies remain required translations.
