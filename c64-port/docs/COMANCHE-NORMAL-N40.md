# Comanche normal V06N40 and mixed-noun display caller

`cm_normal_display_caller` translates the actual normal GOPLAY/NVDSP,
NVMONOPT/NVSUB admission, UPDATNN, ENTPAS0/MIXNOUN/VERB6, DSPDCPUT table
selection and normal GOPLAY completion. The new caller reads the original
immutable Comanche noun tables through `apollo_noun_table_read`; it accepts
V06N40 (octal01450) and V06N85 (octal01525). Source noun40 is not replaced
with noun85. The separate linked `cm_normal_display_step` composes the
actual CKNVWRD1/REGODSP/MAKEPLAY priority/wake prefix, these callers and
qualified conversion services through actual ENDOFJOB and idle ADVAN03231.

Noun40's tables are NNADTEM064000, NNTYPTEM024500 and RUTMXTEM016351.
Its physical pairs are TTOGO03660/03661, VGDISP03772/03773 and
DVTOTAL03425/03426. The first row uses actual bank42 M/SOUT followed by
signed DSPDC2NR, middle blank, minutes conversion and the second signed
DSPDC2NR. The remaining rows use velocity3 DP3 factor{01031,021032},
fresh READLO/TPAGREE/DMP and the rounded decimal formatter. Negative
NNADTEM skips SETNCADR and retains EB1 at noun-format return. MIXNOUN's
M/S placeholder reads the high limb, while DP3 placeholders read the low
limb; all scalers subsequently read the authoritative physical pairs afresh.
The shared caller preserves these source differences and CYL component bits.
Noun85 still uses its actual VGBODY pairs03664,03666,03670 and three DP3 rows.

The six caller phases begin at PCs03312,04171,03602,03604,02537,03344.
For noun40, linked routes0..29 include M/S phases at routes21..26, source
NVSUB return27, normal GOPLAY completion28 and actual END29. Noun85 uses
routes0..25 with the three DP3/decimal rows. Native route numbers name
compiled source entries; they do not decode instruction words. The caller
owns exclusion of source register/scratch/input producers. A linked caller
must return before its code overlay is replaced. An unsupported downstream
service or unexpected scheduler-selected continuation is a native fault
boundary, not a transactional rollback promise or an invented source poll.

The admission domain is a normal unblocked verb6 display, no external
monitor, no pending NEWJOB, basic core zero at source priority023000 and six
unused negative-zero core priorities. This packet does not close every
Pinball noun, verb, flashing/busy display or scheduler-resident continuation.
Only relevant Pinball defaults are initialized from source STARTSUB:
eleven dirty blank DSPTAB words073777, NOUT11, no display lock or monitor,
and zero CADRSTOR. Other words are supplied by the caller. The sole
persistent image owns all source erasable words; SUPER, IRQ and channel11
are explicit `cm_native_io` metadata.

The unchanged pinned original rope executes 512 continuous N40 paths and
128 N85 paths, comparing every source boundary without normalization:
18,688 complete images of2,051 fields. Strict native and full ASAN/UBSAN
streams equal the original stream SHA256
`8131b2d0e3d156996220b7f3f380d76e71295b66650771f8d67a680ed8c58a20`.
The time corpus includes both signs and mixed DP signs/zeros, half-second
rounding and minute/clamp corners; source conversion services retain their
independent wider qualifications. SUPER0/20/40/60 and IRQ0/1 are covered.
Sixty-four original before/after caller transitions and108 unchanged invalid
context guards pass strict and sanitizer checks. Actual PAL C64 execution
passes all64 transitions: mailbox `a500004000`, PRG33166 bytes, hardware
stack13 plus14 ROM bytes, software stack104 bytes and zero additional
zero-page bytes. The target imports original inputs at caller boundaries;
the uninterrupted complete chain is proved on the host. The conversion
callees retain their own source/C64 packets; no resident whole-chain or
relay-to-screen claim is made here.

The reference rope is the existing immutable powered-source fixture
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The runner asserts normalized Executive/noun table identity and the fixture's
pre-existing four DSPOCTWO spelling repairs, one CAF EBANKSAV to equivalent
CA mnemonic repair and omission of unused PRIOSAVE EQUALS R1SAVE. These
fixture repairs and exact original/fixture identities are retained in the
proof. Native modules contain integer source arithmetic, no AGC decoder,
emulator, floating point or heap. The GPL engine is an independent host
oracle only.

Reproduce with `tools/cm_normal_display_differential.py`,
`tools/freeze_cm_normal_display.py`, host/sanitize/C64 targets from
`tools/cm_normal_display.mk` and `tools/cm_normal_display_vice_test.py`.
The module makefile also includes complete linked route compilation in
`all`, frozen caller checks in `test` and full original-stream comparison
under `sanitize`. The original compressed stream is retained without raw
duplicate files.
