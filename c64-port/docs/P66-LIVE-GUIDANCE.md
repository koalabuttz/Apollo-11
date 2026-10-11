# Live P66 through the acceleration command

The extended native driver executes the historical P66 arithmetic from
RODCOMP through the completed STOVL AFC/DELVROD pair at LOC03433. It then
retains the running job and owned VAC at the mapped boundary03434, before
throttle feedback. This is a bounded guidance milestone, not the complete P66
loop or a claim that the whole flight program has been translated.

`p66_native_guidance_mappings` replaces the earlier five-group live mapping.
The earlier numerical entry and published evidence remain reproducible. The
new mapping splits three previously grouped scheduling points: elapsed DSU
before STORE30, the two early velocity pairs, and STORE20 before DAD.

| Native selector | Completed source LOC | Next mapped entry |
|---|---|---|
|1 prepare/basic|03353|03354|
|2 scale/timestamp|03356|03357|
|103 elapsed DSU|03360|03361|
|113 STORE30|03361|03362|
|4 divide/gravity load|03364|03365|
|105 corrected gravity|03366|03367|
|115 current velocity|03370|03371|
|125 saved velocity/store/load|03373|03374|
|6 UNIT|03374|03375|
|136 STORE14|03375|03376|
|146 DOT/SL1|03377|03400|
|156 HDOT store/load|03401|03402|
|166 distance prediction|03404|03405|
|176 radius subtraction|03407|03410|
|186 altitude store/load|03411|03412|
|107 desired acceleration|03414|03415|
|117 gravity length|03416|03417|
|127 divide/shift|03420|03421|
|137 STORE20|03421|03422|
|147 implicit-pushdown DAD|03422|03423|
|8 attitude projection/DOT|03427|03430|
|9 STORE22|03430|03431|
|10 AFC divide/store/reload|03433|03434|

Every interpretive callback returns through the verified Executive exchange
at EDOP0. Pending NEWJOB is honored at the first such point. Intermediate
DANZIG visits with a pending right-hand operator are not checkpoints. The
source oracle explicitly requires EDOP0 to distinguish those visits.

UNIT's square, root and quotient phases are separate disk modules106/116/126.
They preserve job ownership across C ABI returns until UNIT actually completes.
A source DVOVF terminal return skips the remaining UNIT phase and continues
normally; a SQRT alarm commits the verified pre-SQRTABRT source boundary and
reports1302. It does not run an invented alarm/restart continuation.

Projection's trig, rotation and DOT modules108/118/128 similarly retain
ownership during the original basic subcalls. The first legal scheduling
point is DOT at03427. No scheduler exchange is inserted during pending EXIT
or RVQ, and no AGC opcode is fetched or decoded.

## Canonical state and target layout

The only persistent source memory is the2048-word image at$7000..$7fff.
Numerical modules gather a transient field view from explicit physical-word
and C-member-offset tables, commit the owned words, and return normally before
another module is loaded. Shared BUF/VBUF/TEM, MPAC/MODE/OVFIND, trig/rotation
scratch and the owned43-word VAC are bound to one owner per physical word.
All unrelated image words are preserved by the arithmetic adapter.

The Executive view is a transient lease at$c100..$c3ff; metadata is at
$c400..$c7ff, with compile-time size bounds. Proof metadata occupies
$c000..$c003 and the completion mailbox$c080..$c085. The overlay slot remains
$8000..$bfff and the software stack$c800..$cfff. Scheduler views are committed
at owned boundaries and gathered again before reuse. Guard hooks preserve
the native caller's interrupt state and exclude source producers.

The resident uses a compact byte-copy and scalar-offset bridge. Its compiled
selector table parallels explicit native source-address mappings; it is not
an instruction table or an interpreter. Mathematical kernels remain in the
27 disk modules. Two further modules stream frozen inputs and independently
produced source fingerprints, so diagnostic constructors do not enlarge the
resident program. A D81 image accommodates the modules through the existing
IEC loader on a stock C64; the loader and fixed memory framework are unchanged.

The LLVM-MOS SDK ships some integer helpers as static-stack bitcode. The build
copies its CRT archive and recompiles only those existing helper objects with
`-static-stack` disabled in the bitcode target features. Integer algorithms
and the installed SDK are unchanged. The copied archive's source SHA and
member list are recorded in `build/overlay-sdk-runtime/provenance.txt`.
Every emitted arithmetic module must fit16KB and have no BSS/noinit storage.

## Independent verification

Original assembly executes in the pinned hardware-DV engine; translated C
only constructs inputs and captures states in those source oracles.

- 128 integrated source inputs:2944 checkpoints through the23 callbacks.
- 16 pairs of real P66 jobs:400 checkpoints include preemption, saved MPAC,
MODE/PUSHLOC/OVFIND and execution of the higher-priority arithmetic payload.
- 352 pending NEWJOB injections:704 checkpoints cover all22 interpretive
boundaries. Each source run records CHANG2 only after the expected completed
pair and then the actual higher-priority RODCOMP entry.
- 43436 canonical numerical checkpoints compare the split module sequence
with independently verified radial, acceleration and projection states.
- The immutable prefix still passes160 frozen checks, its5775 numerical
checkpoints and112 mixed-job checkpoints, with the original SHA identities.

Live comparisons contain493 explicit words: the166-word integrated input,
arithmetic and scratch view, all96 core and220 VAC words, FIXLOC/OVFIND,
ADDRWD/EDOP/INTBIT15/ZERO and raw EB/FB/BB, plus observed PC/Q from velocity
onward. Earlier prefix PC/Q are normalized because that historical adapter
did not translate those CPU observations. This is a complete stated field
set, not a comparison of every2048 erasable word. Strict and ASAN/UBSAN
regressions retain separate evidence in `evidence/p66-guidance-regression.txt`.

Reproduction:

```sh
python3 tools/p66_guidance_live_differential.py
python3 tools/p66_guidance_pending_differential.py
python3 tools/p66_guidance_regression.py
python3 tools/freeze_p66_guidance.py
python3 tools/p66_guidance_overlay_build.py --cc /path/to/mos-c64-clang
python3 tools/p66_guidance_build.py --cc /path/to/mos-c64-clang --c1541 /path/to/c1541
python3 tools/p66_guidance_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/vice
```

The extended build consumes the existing prefix module manifest; build that
first with `tools/p66_overlay_build.py` on a fresh workspace. New artifacts
use the `p66-guidance-*`/`overlay-p66-extended-*` prefixes and do not replace
published prefix evidence.

The final stock PAL C64 run passes192 complete493-word fingerprints across
two frozen source inputs, both SUPER forms, normal execution and actual P66
preemption, with native interrupts enabled and masked. Mailbox:
`a50000c00002`. The measured software-stack extent is736 of2048 bytes and
there is no extra zero-page allocation. The resident contains25733 text,
759 rodata,25 data and23 BSS bytes, ending at$6fb9. The numerical modules
range from2415 to15301 loaded bytes, all with zero BSS/noinit. Full SHA
identities, manifests, section map and final target dumps are retained in
`evidence/p66-guidance-*`.
