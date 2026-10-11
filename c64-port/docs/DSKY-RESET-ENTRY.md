# Both-program RSET and ERROR key

`dsky_charin_reset` and `cm_charin_reset` execute actual LM CHARIN40,2077 and CM CHARIN40,2000 for RSET022 through ERROR and genuine ENDOFJOB5155/5217. The original DSPLOCK is restored; channel11 caution-reset is asserted and uplink/operator-error bits cleared; channel13 test-alarm is cleared; the DSKY restart-light latch clears. Gimbal-lock/no-attitude bits remain in DSPTAB+11 while other status lamps clear. Failure words clear and the original eleven-entry TSTAB loop marks every digit relay word for reset while preserving both signed zeros and NOUT.

The original source bodies differ: LM resets IMODES30/33 and RADMODES, while CM resets its distinct IMODES30/33 addresses and sets the OPTMODES optical-failure bit. Literal source addresses and return words remain distinct. Positive DSPTAB words set bit12; negative words clear its complemented representation. Clean +0 becomes04000 and -0 becomes073777.

A real source corner is retained: CADRSTOR=-0 makes CCS skip CS ELRCODE1, landing directly on AD MPAC. RSET then genuinely lights key-release and stores that original channel11 value in MPAC before ERROR. Other occupied CADRSTOR values cancel the key and leave MPAC -0; empty +0 leaves +0. The source comment saying all RSET paths bypass key-release is not used to override the assembled instructions.

Bank40/any retained EB0..7/SUPER100 and IRQ0/1 are qualified, with arbitrary ordinary display/fault words and raw L normalization. Channel13 is readable latch state: write-only trap-enable bits034000 must be clear, as on real channel readback. Existing trap latches therefore remain unchanged. The caller supplies the explicit restart-light latch and producer exclusion. Unsupported entry/I/O values leave image and metadata unchanged. Channel output event bits report actual writes to11 and13; they are source events, not an installed C64 IRQ/device driver.

Independent original assembly compares8192 complete2054-field states:2048 physical words plus SUPER/IRQ/channel11/channel13/output-event bits/restart-light latch. Every field matches; complete snapshot SHA256:
`ff175991b21f0609f305bef88f60f0dc8174365ef3e7fc5bf2adc9ba0ca29509`.
The complete ASan/UBSan stream equals the original byte for byte. The oracle links no native body and audits original Pinball/T4/Interpreter bytes, CM indentation-only assembly normalization and byte-identical pinned ropes.

Frozen64 complete images pass strict host, ASan/UBSan and actual native C64. VICE PAL mailbox `a500004002`; PRG10090 bytes (loaded10088); hardware/software stack10/42 bytes. All sections stay belowC000 and no extra compiler zero-page storage is allowed. `tools/dsky_charin_reset.mk` provides host-only all/test/sanitize and explicit differential/C64 targets. Evidence uses build/dsky-charin-reset-* and build/dsky_charin_reset-*.

This package reaches genuine ENDOFJOB. The integrated keyboard application must execute real Executive completion and route the reported hardware outputs; returning from this source body alone does not complete or render a native flight job.
