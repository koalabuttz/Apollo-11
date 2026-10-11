# Staged VAC admission and interpretive context exchange

`exec_spvac` translates SPVAC through the established ENDFIND or BAILOUT1
admission boundary. The caller has staged an unpacked upper priority in NEWPRIO,
provides the incoming A/L 2CADR as an `exec_address`, and provides Q separately.
SPVAC's XCH Q / AD NEG2 / XCH Q preserves the incoming accumulator and adjusts Q
by minus two before entering SPVACIN. The API retains both raw signed-zero
results of that addition and the adjustment when a resource bailout occurs.
Q must be separate from the Executive object. A pending bailout or unsupported
input leaves the state and Q unchanged. Successful/resource-failing admission
uses the existing FINDVAC implementation, including a reserved VAC retained on
1202, low-nine-bit work-area packing, and exact NEWLOC exchange debris.

`exec_change_interpretive` translates CHANG2, CHANJOB and its basic or INTRSM
epilogue. The caller supplies the current SUPERBNK field and separate
`exec_interpreter` and `exec_transfer` objects. Core-zero LOC is the completed
interpretive word's positive address; BANKSET is its BBCON. EDOP must be +0:
this is the source safe point at which DANZIG can enter CHANG2, after a packed
operation pair has been consumed. NEWJOB selects an active saved slot 1..7.

CHANG2 complements the outgoing LOC. CHANJOB combines its saved BANKSET with
the current SUPERBNK and exchanges MPAC/MODE, PUSHLOC/PRIORITY and the saved
address pair. The existing overflow encoding and FIXLOC calculation remain
identical. A basic incoming address follows the established DTCB exchange
boundary; interpreter control scratch is retained. A negative incoming LOC
follows ENDPRCHG's COM / AD ONE / TS LOC and INTRSM. Its resumed LOC advances
by one, including incoming -0 becoming address one. BANKSET becomes the
hardware BBANK value (mask 076007), INTBIT15 becomes its bit 040000, and EDOP
becomes +0 after its register edit. The separately returned address retains the
complete selected BBCON, including SUPERBNK, and marks the transfer as
interpretive. Basic incoming addresses are 1..07777; saved interpreter addresses
must advance into that range. Unsupported input leaves all three owned objects
unchanged. These services require caller-owned exclusion from any producer
that mutates the same contexts.

## Original source and verification

The native code is MIT C and has no instruction decoder, heap allocation,
native bank switching, or suspended C frame. The original public-domain
sources remain unchanged:

- EXECUTIVE.agc: SPVACIN 57..61, SPVAC 65..68, CHANG2 79..85, and
  CHANJOB/ENDPRCHG 212..277.
- INTERPRETER.agc: INTRSM 56..57 and INTPRET's bank/control setup 47..52.

`tools/executive_service_reference.c` runs the original assembly with the pinned
Virtual AGC engine. It links only the engine, input constructors, and row
formatting. It never links the native Executive or arithmetic implementation.
The differential script checks original source identity and six entry/control
symbol addresses before running the comparison. It captures adjusted Q at
SPVACIN before the resource-failure path changes Q for BAILOUT1, and checks actual
BBANK/SUPERBNK independently after each successful dispatch.

There are 256 SPVAC checkpoints and 512 CHANG2 checkpoints. Every row compares
all 96 core words, 220 VAC words, eight Executive globals/marker words, Q,
INTBIT15, EDOP, transfer metadata, and status. Cases cover the staged-priority
range, signed-zero Q adjustment, all VAC/core resource failure paths supplied
by the admission constructors, all seven selected context positions, basic and
interpretive resumes, each supported bank/superbank combination, raw overflow
and push-pointer zero signs, incoming -0, and the last valid resumed address.
The snapshot SHA256 is
`da0a757ef8b59588af0eb0c177cb2da6591f2b9b4eccbd834e6d519167a7cbf3`.

Host regressions freeze all 768 original-derived complete-state fingerprints.
The unit suite additionally rejects every nonzero 15-bit EDOP value without
mutation, rejects every malformed 15-bit NEWJOB displacement, and checks
malformed staged priority and an out-of-range interpreter
resume. Both ordinary and ASan/UBSan runs pass. Existing Executive/dispatch,
IRQ, lifecycle and restart suites pass after these additions. Fingerprints
have possible collisions and supplement the full-word host comparison.

The stock PAL C64 diagnostic also passes all 768 fingerprints, with mailbox
`a500000003`. Its PRG is 17,600 file bytes: 14,154 text bytes and 3,432
read-only bytes, plus one no-init byte, ending at $4cbf. It allocates no extra
zero-page storage beyond the compiler's 32 imaginary registers. ROM IRQs remain
enabled; no IRQ producer owns or mutates the diagnostic contexts. Complete-state
hashing dominates diagnostic execution time and is not a guidance timing claim.
The emulator run has a finite cycle limit and a separate host timeout. Complete snapshots are retained as deterministic gzip files. Other
evidence is stored under `evidence/executive-services-*`.

## Remaining runtime integration

The existing `exec_native` trampoline remains a basic-job dispatcher. These
bounded services do not make it execute or decode interpretive operation pairs.
A source-faithful interpreter continuation runtime still needs explicit native
continuation mappings, dispatch at EDOP=+0 boundaries, bank-qualified
interpretive resume metadata, and basic-to-interpreter completion/exchange
handling. Resume mappings must represent the source's advanced LOC, retain
complete saved MPAC/MODE/VAC state, and finish each C call before changing jobs.
The translated guidance kernels can then supply native continuation bodies.
The new transfer boundary stops before NEWOPS; execution beyond it is not
claimed. Hardware peripheral inputs, full idle self-check, flight timing, and
mission execution remain separate work.
