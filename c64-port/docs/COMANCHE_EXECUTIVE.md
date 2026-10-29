# Comanche 055 Executive admission and release

This first independent Command Module Executive milestone translates `NOVAC`,
`FINDVAC`, core admission, and `ENDJOB1` resource release/selection to native C.
The historical Comanche scheduler has **seven core sets**, unlike Luminary 099's
eight. Comanche's `NO.CORES DEC 6` is an inclusive CCS down-counter. It also calls
`BAILOUT`, whereas Luminary uses its distinct `BAILOUT1` entry. These differences
are preserved in a separate `cm_executive` module; no LM scheduler is changed.

## Native contract

`cm_exec_init` initializes the local seven-core, five-VAC state. It does not
perform an AGC fresh start. `cm_exec_admit` receives a positive upper priority
field `01000..037000`, a raw 15-bit 2CADR pair, and the VAC requirement. It finds
only a priority `-0` core free; dormant negative priorities remain occupied.
Five VAC use words are scanned in historical order. `NOVAC` uses `FAKEPRET 0110`;
VAC jobs store the actual work-area base in the low nine priority bits.

All seven cores can be admitted. On VAC exhaustion, the native alarm boundary
marker is `01201`; on core exhaustion, it is `01202`. A VAC already reserved
before the latter bailout remains reserved, matching the source. The stop is
at **BAILOUT entry**, before Comanche's alarm storage, restart, or recovery.
`NEWLOC` exchange debris, `LOCCTR`, packed priority comparison, and core-zero
`FIXLOC`/`OVFIND` effects are retained.

`cm_exec_endjob_select` releases core zero and its VAC, then scans slots 1..6.
The complete packed priority decides selection; the first slot wins a tie.
Dormant jobs are skipped. The return is the selected slot or `-1` for idle;
`NEWJOB` is retained on an idle scan because `DUMMYJOB` has not yet executed.
This boundary does not swap contexts or call a native job.

The caller owns interruption exclusion and the whole state. Valid `NEWJOB`
is `-0` or a displacement divisible by 12, from 0 through 72. An existing
bailout marker and unsupported admission input leave the state untouched.
As in the original supported boundary, an idle core-zero admission requires
`NEWJOB=-0`; inconsistent states entering `CCSHOLE` are not supported.
Existing job priorities and VAC bases must be valid admitted representations.
The state contains 312 `agc_word`s, **624 bytes**: 84 core words, 220 VAC words,
seven scheduler scratch words, and the native alarm marker.

## Historical source qualification

The repository's `Comanche055/EXECUTIVE.agc` remains unchanged. Its SHA256 is
`d76a32a7fe3168d949dfa13af819695d019bfe3a86e1a4782003237ed146b14b`.
The checked-in 2009 transcription contains five malformed pseudo-label
indentations (`CHANG2 +2`, `CHANJOB -2`, `CHANJOB -1`, `CHANJOB +4`, and
`SUPDXCHZ +1`). Three cause fatal yaYUL operand errors. `cm_prepare_reference.py`
copies the exact original to ignored `deps/comanche-fixture/EXECUTIVE.original.agc`
and changes only those five label indentations in the assembled copy. The
reviewable patch is retained as `evidence/cm-executive-indentation.patch`.
No opcode, operand, constant, or historical file is edited.

The supporting full-rope sources and host-only yaYUL/yaAGC tools use Virtual AGC
commit `1b5d6474841a7ab53237ead070f76f35f162f68d`. The preparation script also
assembles that checkout's independently corrected Comanche Executive and
requires the **entire rope** to match the normalized original assembly byte
for byte. Rope SHA256:
`2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`.
This is an original-source-derived assembly oracle with a documented
transcription repair, rather than a claim that the unmodified transcription
assembles successfully. All native routines and tests are MIT; the GPL
emulator and adapter are host-only and never enter the C64 image.

The adapter asserts the actual fixture symbols: `MPAC 0154`, `PRIORITY 0167`,
`VAC1USE 0400`, `NEWJOB 0067`, `FIXLOC 0120`, `OVFIND 0121`, `NOVAC 5134`,
`FINDVAC 5147`, `ENDFIND 5222`, `BAILOUT 7755`, `ENDJOB1 01,3124`,
`CHANJOB 01,2727`, and `DUMMYJOB 01,3223`. Original instruction lines are
`EXECUTIVE.agc:37..205` for admission and `:368..447` for release/selection.

## Verification

The independent input constructors initialize raw words directly and never
call native initialization or scheduling functions. The first 26 scenarios
cover idle admission, all five VAC positions, both resource alarms, the seventh
core, negative occupied cores, stale locations, priority ties, and release.
Another 1,510 seeded raw-word cases vary retained core/VAC data, free/dormant
states, packed priorities, addresses, and scratch state.

All **1,536 cases** agree with the host original-source-derived assembly in
all **312 owned state words** and the operation result. Coverage includes 885
successful admissions, 15 VAC bailouts, 127 core bailouts, 495 selected releases,
and 14 idle releases. Sixteen neighboring erasable words starting at `0300`
remain unchanged, explicitly guarding the region that would be an erroneous
CM eighth core. The engine remains in Executive bank 1 with IRQs excluded at
every stop. Native and reference snapshot SHA256:
`caa5ebe8349c9c3864274def9aeef4cc99cec545aae6aae90c11246097b488d2`.

Host regression and AddressSanitizer/UndefinedBehaviorSanitizer checks pass,
including rejected eighth-core displacements and pending alarms without
mutation. Leak scanning is disabled for the restricted process environment;
the native routines use no heap allocation.

Forty frozen assembly states are compared word by word, covering all first 26
cases and 14 cases distributed across the seeded corpus. Additionally, every
case has a frozen FNV-1a state fingerprint. Fingerprint collisions remain
possible; the full-word host differential comparison is the primary proof.
The PAL C64 diagnostic passes those same 1,536 fingerprints and forty full-state
checks in VICE 3.7.1, with zero failures and mailbox `a500000006`. It uses owned contexts while normal ROM IRQs remain enabled. The VICE
helper preflights image/sections below reserved `$c000` and rejects extra zero
page allocation. The LLVM-MOS image has no AGC instruction decoder.

The PRG contains 43,004 file bytes (43,002 loaded bytes at `$0801..$affa`).
Text is 11,393 bytes at `$080d`; read-only data is 31,597 bytes at `$348e`;
one noinit byte ends at `$affb`. The diagnostic state/fingerprint tables are
verification data, not production scheduler storage. The harness permits up to
six billion PAL cycles for the full corpus; this budget is for regression
verification, not a native scheduler timing or flight deadline claim.

## Reproduce

From `c64-port`, after building pinned yaYUL and yaAGC tools:

```sh
python3 tools/cm_prepare_reference.py
make test-cm-executive
python3 tools/cm_executive_differential.py
ASAN_OPTIONS=detect_leaks=0 make sanitize-cm-executive
make c64-cm-executive MOS_CC=/workspace/toolchains/llvm-mos/bin/mos-c64-clang
python3 tools/cm_executive_vice_test.py \
  --vice /workspace/toolchains/vice/usr/bin/x64sc \
  --rom-dir /workspace/toolchains/roms/C64 \
  --data-dir /workspace/toolchains/vice/usr/share/vice
```

`tools/cm_executive.mk` supplies the build targets. Native C64 timing, WCET,
physical AGC verification, CM dispatch/lifecycle, WAITLIST, restart, navigation,
flight programs, and peripheral services remain subsequent milestones.
