# Original fixed words as passive native data

Both mission datasets contain all 36 populated physical fixed banks from the
independently assembled original declaration layout. LM uses original rope
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`;
CM uses `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The original source-set hashes and every source-file hash are checked against
the retained fixed-data source audit. Banks 0..3 use packed-file order 2,3,0,1;
banks 4..35 retain their number. Big-endian parity words are divided by two
to obtain the original 15-bit data words. No code is decoded or executed from
this dataset, and it supplies no executable-file coverage certificates.

This storage exists for original fixed-memory display and self-check data
access. It is separate from the much smaller immutable constants tables used
by compiled numerical routines. CM original DSPFMEM/COMADRS/SUPDACAL source
reader/control qualification is owned by the CM interbank packet. The LM
passive reader is ready for explicit native consumers; this packet does not
claim a completed LM display/self-check control path.

`lm_fixed_compat_read` and `cm_fixed_compat_read` accept a physical word index
0..40959 and copy one word to the caller. Populated indices 0..36863 return
exact original words; the four unpopulated banks return zero. Null output,
out-of-range address, and a populated address absent from a selected link
group fail without changing the output. There is no additional authoritative
application RAM image. A data payload ignores and preserves the canonical
2048-word image. No pointer into an overlay is exposed or retained.

Each seven-bank group contains 7168 words; the sixth group contains the final
1024 populated words. Six independently linked native C data payloads load
at $8000..$bfff, with the 16-byte manifest header at $8000 and the ordinary
returning C entry at $8010. The selector is a physical word address, not an
instruction. The resident loader checks exact size, immutable header identity
and CRC before invoking. It returns only after all previous native frames
have unwound. Unsupported selectors return $ffff. The caller owns producer
exclusion during slot loading and restores the incoming C64 interrupt state.

Measured loaded sizes for either mission are 14483, 14528, 14528, 14528,
14528 and 2207 bytes: 74802 total, comprising 73728 data bytes and 1074 bytes
of native reader/header/runtime overhead. Each group fits the existing 16384
byte slot. The standalone exact-word proof resident is 3540 PRG bytes and
fits below $6000. The diagnostic uses $6000..$67ff only for a transient
1024-word observation buffer. Application canonical RAM remains $7000..$7fff;
software stack remains $c800..$cfff.

The data filenames are F200..F205 on separate mission disks. Their proof
module IDs 200..205 are not globally reserved executable IDs. The committed
complete-descent LM disk already has executable G200..G205 IDs; a unified
runtime table must assign distinct IDs or a separate resolver before merging
these datasets. CM control206 and proof240/241 belong to the separate CM
reader packet. No shared production registry has been changed here.

`evidence/passive-storage-lm-production.d64` copies the exact committed
bounded startup/P66 base `evidence/p66-native.d64` and adds only F200..F205.
Every existing file, including its resident and registry, was read back and
compared byte for byte; all six new files were read back and compared with
their linked PRGs. Free blocks decrease from 375 to 76. This proves allocation
on real D64 media for that bounded resident, not a complete LM application
resolver. The CM reader packet allocates the identical CM dataset beside its
existing CLOCKJOB resident, retaining its old registry scope.

Independent strict C99 host and ASAN/UBSAN checks compare every original word
and test all 65536 argument addresses for the complete reader and each of its
six selected link groups, including null and untouched failure outputs. The
all-source provenance check verifies both complete original source sets.

On stock PAL C64 ROMs in VICE, each mission loads its six actual data files
and returns all 40960 physical words under both incoming interrupt states.
The monitor captures every 1024-word block; the verifier compares all 81920
returned words individually against the original assembled rope. This verdict
does not rely on a checksum/hash. Both missions preserve every canonical RAM
word and the ROM mapping; measured software-stack extent is 43 bytes. The
two raw observation streams are retained losslessly compressed with their
decoded identities, alongside actual disk, PRG/map and monitor artifacts.

Run `make -f tools/passive_storage.mk passive-storage-host` for host checks.
The target requires existing CM source-reader payload builds first, then
`passive-storage-target` with the MOS/VICE variables set to installed tools.
`passive-storage-allocation` creates the unchanged-base LM allocation disk.
This packet adds data storage readiness, not new executable algorithm status
or whole-application completion.
