# Canonical LM JOBWAKE

`lm_wake_image` executes the literal JOBWAKE preparation, first sleeping-match search, WAKETEST, SETLOC/SPECTEST and ENDFIND return. It includes a dormant running slot, which the frozen basic-only `lm_basic_wake` contract excludes. The original helper and typed Executive algorithms remain unchanged.

The source matching test adds the complemented requested CADR to the sleeping LOC and checks both register zero representations. On success it re-complements the actual packed priority, constructs the two-word wake address using saved EB/SUPER, and compares against the actual queued priority. A dormant core zero with NEWJOB=-0 becomes the ready running job through SPECTEST; with a positive request it follows SETLOC and may retain or replace that request according to the original comparison. No awakened job is called and no IRQ or lifecycle poll is inserted. A failed search returns the actual caller with LOCCTR=-1 and all source scratch/register debris retained.

The independent original-source proof contains 16,384 full 2048-word images plus SUPER/IRQ. Half exercise the historical basic-caller contexts, and half wake dormant core zero with idle or genuinely pending jobs and competing sleeping matches. Native strict and full ASAN/UBSAN streams match the original assembly byte for byte. All 8,192 historical cases also match the frozen wrapper exactly. Sixteen mutation guards and two null interfaces preserve the complete image and explicit I/O on rejection.

The stock PAL ROM/D81 target checks 64 frozen original full-state fingerprints with 32 input/checker modules. Its mailbox is `a50000400002`, software stack 65 bytes, hardware stack 33 bytes and additional zero page zero. Each fixture frame returns before disk replacement; incoming native I/$01 is restored. The source image remains at $7000.

The qualified domain requires nonzero source core priorities, ordinary caller/bank registers, legal NEWJOB offsets and saved sleeping EB/SUPER fields. A sleeping core zero with NEWJOB=+0 reaches literal CCSHOLE and remains a separate exceptional continuation to qualify. This group does not certify that recovery body or the complete containing Executive file.

Use `lm-wake-image-port-test` and `lm-wake-image-port-sanitize` in `tools/lm_wake_image_port_integration.mk` for private fresh-checkout host builds, streamed retained originals and all checked target descriptors.
