# Original LM and CM system-test lead-ins

The native C controls cover both complete byte-original
`SYSTEM_TEST_STANDARD_LEAD_INS.agc` files: Luminary's 32 emitted words and
Comanche's 36, including Comanche's additional system-test entries and MM07
literal. Every original source declaration, placement directive and emitted
word has a source-line/address/native-control map in
`evidence/standard-lead-source-audit.json`. The original assembled layout is
retained: LM rope `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`
and CM rope `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The lead-in files need no source repairs. The fresh original CM helper copies
have individually recorded indentation-only pseudo-label repairs; assembly
must reproduce the same complete original rope. Original source bytes remain
unchanged in the historical trees.

| Original path | Native controls and completed source return |
|---|---|
| E/BKCALL and E/SWITCH | `standard_lead_stage`, `standard_lead_bank_call/return`: save aliased BUF2/EBUF2, resolve inline CADR, actual SWCALL, actual SWRETURN, all three return branches and DTCB to compiled caller |
| E/CALL | Basic preparation, indirect interpreter CALL/QPRET, caller-owned fixed native subroutine, RVQ, original EXIT pair, LXCH EBUF2+1 and INTPRET+2 through erasable caller NEWOPS |
| E/JOBWAK | `standard_lead_wake_lm/cm`: actual native JOBWAKE, indexed LOC flag clearing, and saved RUPTREG3 caller TC; both successful and unsuccessful lookup complete |
| CM COMPVER/GTSCPSS1 | `standard_lead_cm_test`: actual TC/Q frames into external GCOMPVER/GTSCPSS native entries |
| CM REDO | Source MM07, actual CM NEWMODEX/SETUPDSP and seven-core NOVAC admission of DSPMMJOB40,3441/BBCON060102, then original IMUTEST entry; no-core stops at actual BAILOUT7755 before its external recovery body |

The external fixed routine and compiled erasable caller are explicit native
continuations. The packet proves the containing lead-in file's controls and
its required call/wake/admission helpers; it does not translate the external
GCOMPVER, GTSCPSS, IMUTEST, display-job or arbitrary fixed-subroutine bodies.
The continuous bank-call oracle executes genuine original TCQ as its external
subroutine. The continuous interpreter oracle executes the genuine standalone
RVQ pair in each original POWERED file. These source continuations are
implemented as explicit native control frames, never by reading or dispatching
an instruction word. Separate source probes execute return1/return2 and
all remaining basic entry stages.

E/CALL's final `TCF INTPRET+2` deliberately skips EXTEND and RELINT. Its encoded
QXCH word therefore executes as basic LXCH LOC. Only the initial actual
INTPRET entry enables interrupts; the isolated final epilogue retains incoming
IRQ0 or IRQ1. The source and target proofs check both isolated epilogue states.
The wrapper does not fetch the erasable caller's next interpretive word.
The returned C frames expose original controls and do not introduce an
Executive poll or independently prove live application scheduling.

Source memory is one canonical 2048-word image; physical indexes preserve
all original aliases. Input A/L may carry raw16 overflow where the source
accepts it; ordinary memory words remain15-bit. Bank fields are coherent,
ZERO is0, saved TC descriptors and caller returns are ordinary valid addresses.
The caller resolves fixed inline CADR data through its native passive reader;
erasable inline words are read after the source's aliased stores. EBUF2/BUF2
and the caller's VAC/source scratch must remain owned through the returned
frames. A fixed callback preserves the source superbanks required by its
caller, matching the original prohibition on switching between S3/S4 here.
External SUPERBANK is retained by these controls. Both missions' E/BKCALL,
E/CALL and wake banks admit all eight latch forms; CM TESTLEAD is original
physical bank33 and requires bit100 clear (forms0/20/40/60).

E/JOBWAK requires inhibited interrupts, an active positive-priority core0 and
the inherited native JOBWAKE admitted-core domain; candidate priorities are
positive, dormant or negative zero. A no-match result retains the source
LOCCTR=-1 side effect at physical LOC-1. CM REDO inherits seven-core admission
and NEWJOB/-0/ordinary displacement contracts, rejects priority+0 fault
contexts before mutation, and covers every available core plus no-core.
The latter yields native pending alarm01202 metadata at BAILOUT entry; the
BAILOUT recovery body and its hardware effects remain external.
Source instructions normalize physical L as required; saved raw-L debris
and transient EBANK effects are retained rather than inferred from the final
caller bank. Guard tests compare the entire image and external metadata after
invalid ownership, VAC base, Executive pointer and source-bank preflights.

The full source/native/ASAN-UBSAN proof compares 35,328 complete 2048-word
checkpoints, with original SUPER/IRQ assertions at each boundary. Lossless XZ
objects retain every complete original checkpoint; source/native/sanitizer
identities are recorded individually. The C64 fixtures store only changes
from independently constructed source input, then compare every2048 words,
source status and latches individually. This is exact expected-word storage,
not a hash verdict. The second diagnostic image is a transient expected-input
copy used only by verification, not authoritative application storage.

Two stock-ROM PAL C64 diagnostics exercise164 LM and188 CM complete source
states. Actual PRGs/maps, VICE logs and mailboxes are retained. Their measured
load/BSS extents are in `evidence/standard-lead-proof.json`; these diagnostics
fit below the reserved $c000 mailbox. Production resident/overlay integration,
hardware producers and completion of both entire applications remain separate
acceptance work.

Reproduce source/full host/SAN fixtures with
`python3 tools/standard_lead_host_test.py`. Execute target proofs with
`python3 tools/standard_lead_vice_test.py --cc mos-c64-clang --vice x64sc --rom-dir <C64-ROM-directory> --data-dir <VICE-data-directory>`.
`tools/standard_lead_port.mk` supplies standalone host, sanitizer and target
targets without changing the shared build. `tools/package_standard_lead.py`
retains exact artifacts and creates both whole-file certificate recommendations
after the independent reviews and final diagnostics pass.
