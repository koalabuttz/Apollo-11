# Comanche 055 native context dispatch and lifecycle

This milestone adds CM context exchange, lifecycle services, and a foreground
native continuation runtime to the seven-core admission/release module.
`cm_dispatch.c` translates `CHANG1/CHANJOB`, `CHANG2`, basic `ENDJOB`, bounded
`NUDIRECT/NUCHANG2`, `JOBSLEEP`, `JOBWAKE`, and `PRIOCHNG`. `cm_native_dispatch.c`
executes explicitly mapped C callbacks and performs context changes after their
C frames return. Saved contexts contain AGC words and bank-qualified historical
addresses; native function pointers stay in typed mapping tables.

This is a verified CM scheduling component. It does not implement the complete
Comanche flight application, hardware IRQ queue delivery, idle self-check,
activity-lamp peripherals, or an AGC opcode decoder. The caller owns exclusive
contexts and IRQ coordination. Ordinary C stack locals do not persist across
job changes; persistent values belong in MPAC/VAC state or external owned data.

## CM source and bank contracts

Comanche has seven core sets and five VACs. All selection loops and legal
`NEWJOB` displacements stop at 72 decimal, rather than Luminary's 84. Native
admission/release is unchanged by this milestone. The historical job exchanges
preserve all seven MPAC words, MODE, packed priority, PUSHLOC, VAC data, and
scratch/exchange debris. Overflow is encoded by complementing outgoing PUSHLOC;
incoming negative PUSHLOC is decoded into its complemented value and `OVFIND=1`,
including incoming `-0`.

Basic return addresses are actual positive 2CADRs. `CHANG2` saves the complement
of the completed interpretive LOC; `ENDPRCHG/INTRSM` resumes at
`complement(saved LOC)+1`. At basic transfer, core-zero LOC is `+0` and BANKSET
retains the old current-bank exchange debris. At interpretive transfer, LOC is
the actual resumed address and BANKSET is physical BBANK masked by `076007`.
`INTBIT15` and `EDOP` are separate caller-owned interpreter control words.
The raw `cm_exec_change_interpretive` boundary stops before `NEWOPS` fetch.

Typed native mappings distinguish BASIC and INTERPRETIVE continuations at the
same historical address. Admission enters BASIC code. Explicit ENTER/EXIT
callback actions model `INTPRET` and `EXIT`; interpreter YIELD supplies the next
operation-pair address and requires `EDOP=+0`. Interpreter jobs explicitly EXIT
into mapped basic bodies before sleeping, completing, or reprioritizing. The
runtime rejects missing incoming mappings before exchange writes and latches
faults. It checks pending alarms before returning idle. Callback entry/return
checks prevent nested continuation frames during the demonstrated sequences.

Sleep stores a one-word fixed CADR, retains the core/VAC, and marks the priority
negative. Wake finds the first matching dormant context across all seven cores;
a miss is nonfatal `EXEC_NOT_FOUND` with `LOCCTR=-1`. Reprioritization retains
low nine priority bits and preserves the source's diminished-priority scan,
including ties and the one-raw-unit comparison edge.

## Assembly provenance

The fixture and five Executive pseudo-label indentation repairs are described
in [COMANCHE_EXECUTIVE.md](COMANCHE_EXECUTIVE.md). Historical repository files
remain unchanged. The complete fixture rope equals the independently assembled
pinned corrected Comanche Executive rope:
`2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`.
Supporting source and host-only yaAGC/yaYUL use Virtual AGC revision
`1b5d6474841a7ab53237ead070f76f35f162f68d`.

`cm_dispatch_differential.py` additionally compares original CM Interpreter
labels, opcodes, and operand tokens with the pinned supporting source at
`INTPRET/INTRSM`, `DANZIG/NOIBNKSW`, and the four-instruction `EXIT` boundary.
They agree after excluding comments and whitespace. These checks qualify the
supporting Interpreter source used by the oracle; the whole historical
Interpreter transcription is not claimed to assemble without correction.
The retained audit records both complete file hashes.

The CM fixture entry points are checked before comparison:

| Boundary | Address |
| --- | --- |
| CHANG1 / CHANG2 | `5164` / `5170` |
| CHANJOB / ENDPRCHG | `01,2727` / `01,3006` |
| ENDJOB1 / EJSCAN | `01,3124` / `01,3141` |
| NUDIRECT / NUCHANG2 | `01,3242` / `01,3032` |
| JOBSLEEP / JOBWAKE / PRIOCHNG | `5175` / `5201` / `5210` |
| INTPRET / INTRSM / DANZIG | `6006` / `6017` / `6030` |
| NEWOPS / EXIT | `6037` / `6712` |

Original Executive ranges include `:83..88`, `:209..305`, `:307..447`, and
`:474..480`; original Interpreter entry/safe-point code is `:47..90`, and EXIT
is `:762..765`. The adapters stop at actual destination PC **and physical
BBANK/SUPERBNK**, avoiding false hits when a continuation address overlaps an
Executive instruction in another bank. Sixteen adjacent erasable guard words
at `0300..0317` remain unchanged across every checkpoint, protecting the region
that would incorrectly become an eighth CM core. Native code is MIT; GPL
Virtual AGC and its adapters remain host-only.

## Verification

All **876 checkpoints** match the assembly fixture in all **312 owned state
words**, status, actual continuation address, and applicable continuation kind
and interpreter-control words:

| Corpus | Checkpoints | Actual native callback sequences |
| --- | ---: | ---: |
| Basic dispatch | 109 | 8 |
| Sleep/wake/reprioritize | 115 | 5 |
| Direct CHANG2 | 512 | — |
| Mixed BASIC/INTERPRETIVE runtime | 140 | 7 |

The constructors supply independent raw-word inputs. No native scheduler or
arithmetic implementation links into the original-assembly adapters. The
native sequence drivers invoke C callbacks rather than applying a scripted
list of scheduler mutations. Source-derived rows include transient exchanges,
retained MPAC/MODE/VAC payload, both zeros, overflow encoding, dormancy, idle
wake-up, priority changes, and BASIC/INTERPRETIVE transitions.

Host and AddressSanitizer/UndefinedBehaviorSanitizer suites pass. Additional
checks verify that pending alarms, eighth-core displacements, and an unmapped
incoming interpreted-yield target leave owned scheduler state unchanged.
Leak scanning is disabled for the restricted process environment; native code
uses no heap allocation.

Four independent stock PAL C64 diagnostics pass all 876 state fingerprints
and **16 complete word-by-word frozen samples**, with zero failures:

| C64 diagnostic | Full samples | PRG bytes | Last noinit address | Mailbox |
| --- | ---: | ---: | --- | --- |
| Basic | 7 | 30,001 | `$7d30` | `a500006d00` |
| Lifecycle | 4 | 36,132 | `$9523` | `a500007300` |
| Direct CHANG2 | 3 | 16,866 | `$49e1` | `a500000002` |
| Mixed runtime | 2 | 31,611 | `$837a` | `a500008c00` |

The combined verification corpus exceeds one C64 image's RAM, so its tests are
split into these images. This does not demonstrate a complete integrated CM
application. Each image loads at `$0801`, runs from its actual BASIC SYS entry,
and stays below reserved `$c000`. The helper rejects unpreserved extra zero
page allocations. Images use LLVM-MOS SDK 23.3.0 and run in VICE 3.7.1 with
normal ROM IRQs enabled and no live scheduler producer.

Measured software-stack sentinel extents are 1,408, 1,402, 1,420, and 1,450
bytes respectively; hardware-stack extents below `$01f2` are 29, 28, 23, and
24 bytes. The latter exclude ROM's already allocated entry frames. These are
finite diagnostic observations, not WCET, physical AGC verification, flight
rate, deadline, or exhaustive stack bounds. State fingerprints can collide;
the complete-word host comparison is the primary evidence.

## Reproduce

After fixture preparation, include `tools/cm_dispatch.mk` in the Makefile:

```sh
make cm-dispatch-differential
make test-cm-dispatch
ASAN_OPTIONS=detect_leaks=0 make sanitize-cm-dispatch
make c64-cm-dispatch MOS_CC=/workspace/toolchains/llvm-mos/bin/mos-c64-clang
for stage in basic lifecycle interpreter mixed; do
  python3 tools/cm_dispatch_vice_test.py --stage "$stage" \
    --vice /workspace/toolchains/vice/usr/bin/x64sc \
    --rom-dir /workspace/toolchains/roms/C64 \
    --data-dir /workspace/toolchains/vice/usr/share/vice
done
```

`cm_freeze_dispatch.py` regenerates frozen vectors from reference snapshots.
`evidence/cm-dispatch-provenance.json` records snapshot hashes, memory sections,
mailboxes, and stack observations. Further CM milestones include WAITLIST,
IRQ delivery, alarm/restart services, numerical routines, and flight programs.
