# Comanche restart phase cleanup controls

`cm_phase_cleanup` executes the actual MR.KLEAN (`05,2501`), P00KLEAN
(`05,2505`) and V37KLEAN (`05,2513`) bodies through their genuine ordinary
TCQ return. MR.KLEAN clears all six phase groups; P00KLEAN preserves group 2;
V37KLEAN clears groups 3, 5 and 6. The source order is retained, and each
inactive pair becomes complement `-0`, phase `+0`, as the actual DCA NEG0
constant pair requires.

The sole canonical 2,048-word image receives all physical phase and raw
register effects. The final A/L contain the prior group-6 complement/phase
words exchanged by the last DXCH. MR.KLEAN inhibits interrupts; direct
P00KLEAN/V37KLEAN entries retain incoming source IRQ state. Physical banks
and SUPERBNK retain the original caller context. Q is an ordinary compiled
return descriptor `0100..07777`; ordinary canonical memory contains 15-bit
source words. Producers remain excluded by the caller.

All 12,288 complete original/native image states match every physical word,
raw A/L/Q and SUPERBNK/IRQ, with strict and full ASAN/UBSAN passes. The source
rope SHA256 is
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`;
the lossless full-state stream SHA256 is
`90642a1ec96733723fa3e586a86522e0a5e099513c9b073b5c8e0389ee8a472b`.
The corpus covers all three entries and 4,096 randomized ordinary images,
physical Ebank/SUPER/IRQ and raw register combinations.

Stock PAL C64 loads actual native control 218 and proof-only input/checker
254/255 through the checked D64 loader. All 48 representative complete source
states pass with mailbox `a500003000`, two direct complete image/I/O samples
and the final whole image. The native payload is 733 bytes and measured
software stack extent is 717 bytes. Both incoming C64 interrupt states are
represented and restored.

Run `make -f Makefile -f tools/cm_phase_cleanup.mk test-cm-phase-cleanup`
and `sanitize-cm-phase-cleanup`. This qualifies these complete source cleanup
controls and caller return; final abort recovery and the whole restart file
remain separate qualifications.
