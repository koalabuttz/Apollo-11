# Comanche original fixed-word callers

This milestone translates the actual DSPFMEM (`41,3353`) and COMADRS
(`43,3535`) lead-ins through SUPDACAL (`04715`) and their source returns
`3361`/`3542`. DSPFMEM reads a selected word for memory display; COMADRS reads
a word for self-check. Later octal-display and checksum bodies remain separate
work. The fixed words are passive compatibility data. No instruction decoder,
execution engine, or executable dispatch derived from these words is linked.

`cm_fixed_caller_enter` operates on the sole 2,048-word physical image and
explicit SUPERBNK/IRQ latches. Its return is the genuine source SUPDACAL entry.
`cm_supdacal_address` supplies a physical data index; a passive data payload
returns that word after the control C frame has returned. The following
`cm_supdacal_data` call performs the compiled SUPDACAL operations and returns
to the source-selected caller address. Producer exclusion spans all these
returned frames and slot loads. Data payloads leave the physical image intact.

The input domain is every 1,024-word physical bank `0..39`, for both actual
callers: original populated banks `0..35` and hardware-zero banks `36..39`.
The original-layout rope is the retained 73,728-byte
`fixed-data-cm-original.rope.bin`, SHA256
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
Parity is removed when constructing the immutable 15-bit data words; the
physical `0..3` file-bank exchange is preserved. This differs from the older
corrected supporting-source layout, and its source audit is retained.

The full host comparison runs 81,920 original application entries and
163,840 complete 2,048-word checkpoints, including raw A/L/Q register bits,
SUPERBNK and IRQ. The retained lossless binary streams match with SHA256
`3d1f53fedcc1c1cd107c16b9034609552bfe4f7a17d9451d45d21b0f7a14fd0c`.
Strict compilation and the complete ASAN/UBSAN stream pass. The host-only
reference executes the original assembled instructions using pinned
VirtualAGC `1b5d6474841a7ab53237ead070f76f35f162f68d`.

The C64 diagnostic loads the six actual passive payloads and the compiled
caller-control payload through the checked disk loader. Its representative
cohort contains 32 addresses covering both callers, low/high bank boundaries,
offset boundaries, the superbank range, and an unpopulated bank. Replaying
both incoming C64 interrupt states produces 64 continuous caller sequences
and 128 source checkpoints, with four complete-state samples and the final
2,048-word image compared directly. The separate common passive-storage
packet compares all 81,920 returned C64 data words individually, for all
40,960 addresses and both incoming interrupt states; no fingerprint supplies
that verdict. The caller diagnostic also checks all passive words, physical
image preservation, ROM mapping and interrupt-state restoration.

Data payload IDs `200..205` contain 74,802 loaded bytes, including 1,074 bytes
of native entry/header overhead above the 73,728 original bytes. Compiled
control ID `206` occupies 1,164 loaded bytes. All fit individually in the
`$8000..$bfff` slot; the canonical image remains `$7000..$7fff`. The retained
production D64 allocates these seven files beside the existing CLOCKJOB
mission media. Its existing resident registry is unchanged; allocation is
not a claim that new diagnostic consumers are registered or that their later
algorithms are complete. Proof-only IDs `240/241` stay on the diagnostic disk.

An earlier 80-input caller diagnostic exhausted its 600-second execution
budget without reaching a mailbox. Its exact resident, disk, inputs, modules
and logs remain in `evidence/cm-fixed-compat-timeout-80`. That trial supplies
no pass or failure verdict. The final representative cohort reduces disk
traffic while retaining the exhaustive host and independent exact-word C64
proofs.

Run `make -f Makefile -f tools/cm_fixed_compat.mk test-cm-fixed-compat` and
`sanitize-cm-fixed-compat`. `tools/cm_fixed_compat_build.py` builds actual
disk payloads; `tools/cm_fixed_compat_vice_test.py` compares the stock PAL C64
results. The module proof and exact artifact ledger preserve the target
mailbox, stack measurement, production allocation and source provenance.
