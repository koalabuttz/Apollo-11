# P66 quotient push and gravity load

`p66_push_gravity` translates the DP `PDVL GDT/2` following DDV 4SEC(28),
stopping at DANZIG before VSU. Original source is
LUNAR_LANDING_GUIDANCE_EQUATIONS:1007..1010 and INTERPRETER:589..602,531..543.

The caller owns a preparation context, a 43-word VAC payload and six stable
15-bit gravity words, excludes their producers, and supplies DP MODE=+0.
FIXLOC must name one of the five historical VAC bases; PUSHLOC must leave
two words available. Native scope rejection preserves every owned word.
The input vector must be distinct from the mutable state and VAC.

The operation stores MPAC 0/+1 at PUSHLOC and advances it by two, then loads
the vector into MPAC 0/+1, +3/+4 and +5/+6. MPAC+2 is retained. MODE and A
become -1; L receives the old MPAC+6, sign-extended. The first exchange's old
VAC words are discarded by CCS MODE. DDV scratch, PIPTIME and unrelated
preparation state are retained. Saved elapsed VAC words 30/31 remain intact
unless the explicitly selected push overlaps them, in which case the original
stores replace them. The modeled context now contains 99 words / 198 bytes,
including the six unchanged input words.

The original PDVL entry is fixed-fixed 6557, ENDVLOAD is 6511, VMODE is 6521,
and DANZIG is 6061. GDT/2's physical address is 1236, translated by the
interpreter to ADDRWD=1636 with EBANK=01000. At the stop BBANK is 64002,
FBANK=64000, LOC=3364 and MODE=-1; DANZIG has not yet restored BANKSET.
The native API represents the data transition and retains the previously
documented interpreter control-state boundary.

## Independent verification

The oracle executes unchanged original PDVL and RODCOMP assembly. Input
constructors and serialization are shared; native arithmetic is excluded.
Full-prefix comparisons use the pinned hardware DV model documented in
[DIVIDE.md](DIVIDE.md); direct PDVL does not execute DV. The pinned engine
checkout, twelve original-source overlays and assembled rope are unchanged.

All 3,865 cases / 5,218 checkpoints compare all 99 words:

- 16 signed-zero and raw limit inputs;
- every preparation TS-skip input and three four-step retained histories;
- all five VACs at all 42 legal DP push offsets;
- all 180 positions admitting the complete eight-word vector/quotient prefix;
- 256 operand corners, 2,048 direct and 1,024 integrated random inputs.

The snapshot SHA256 is
`53b5f8e8bb3655d198aebea6b9e231ff78ea245472096fb7f64ba1f032ec669e`.
All surrounding guards and the VAC use/end words are checked independently.
All unsupported 15-bit modes/bases and all unsupported 16-bit push pointers
reject without mutation. Host suites and ASan/UBSan pass.

The PAL C64 diagnostic checks 156 complete states, including every integrated
skip input and three retained histories. Mailbox `a500009c0003c66324` records
zero failures. Frozen expectations use a lossless dictionary/literal/run stream:
30,888 word bytes occupy 20,649 bytes including dictionary and row offsets.
Every decoded expected word is compared directly; no fingerprints are used.
The portable decoder is independently tested over all 65,536 word values,
real/random rows, long runs and malformed/truncated streams. Shared extern
arrays prevent duplicate fixture storage across test and timing translation units.

The PRG is 40,690 bytes. Text occupies 19,665 bytes, read-only data 21,011,
state 198 and noinit one. Image/state end at $a7b7 below reserved $c000.
No extra compiler ZP, heap or floating point is used. Measured stack extents
are 50 bytes below $01f2 and 371 software-stack bytes.
Four PDVL samples take 615..736 PAL cycles (up to 0.747 ms); the complete
prefix sample takes 28,164 cycles (28.586 ms). These finite intervals include
markers and any VIC/IRQ effects. They do not establish a worst-case bound,
producer exclusion budget or flight deadline.

## Reproduction and retained evidence

```sh
make test-gravity
ASAN_OPTIONS=detect_leaks=0 make sanitize-gravity
python3 tools/gravity_differential.py deps/fixture
python3 tools/freeze_gravity.py
python3 tests/test_frozen_words.py
make c64-gravity MOS_CC=/path/to/mos-c64-clang
python3 tools/gravity_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/vice
```

`evidence/gravity-*` retains logs, PRG/map, mailbox/stack dumps, timings and
deterministic gzip snapshots. Decompress snapshots before comparing their
content SHA256. All 218 earlier evidence hashes are preserved; the manifest
contains 239 entries. Historical source files remain unchanged.

Next guidance work is VSU VBIAS and the implicit quotient-fed VXSC, followed
by velocity accumulation and STOVL 24D/R. The full flight application remains
under development; this milestone completes one source operation.
