# P34-35 transfer routines: GET45 +1 through VNPOOH

Lane E. Milestone `lm-p34-transfer`.

## Scope

From `Luminary099/P34-35_P74-75.agc`, bank 35 `03635`-`03722`: 54 words on
54 source lines. This is the basic code after VN1645's computation head (GET45
+1 with TC COMPTGO, the DELAYJOB wait and the V16N45 flash with its three
responses, KILCLOCK, N45PROC and CLUPDATE), CLUPDATE's interpretive CLEAR GOTO,
and the DISPLAYE and VNPOOH flash subroutines with P3XORP7X. The code runs as
routines of the control-transfer ABI (`include/agc_transfer.h`,
`docs/NATIVE_TRANSFER_ABI.md`). The milestone also registers the `lm-vn1645`
steps (VN1645, its GET+MGA pair, GET45) and the `lm-midgim` steps (GET+MGA,
GET.LVC, SELECTMU) as interpretive routines. With these, a run that enters any
of them continues through the trampoline to the first point outside this
file.

`lm_p34_basic_routines` holds 20 basic routines keyed by physical address in
bank 35. They are each entry point the original can reach: the routine heads
and every return point after a TC or BANKCALL that comes back into the file.
`lm_p34_interpretive_routines` holds 7 interpretive routines keyed by LOC under
BANKSET at NEWOPS. `lm_p34_site_table` gives the operands of the two TCR
BANKCALL sites (`35,3700` and `35,3717`, CADR GOFLASH).

Every basic instruction is one `agc_image_ops` register effect on the image,
in source order. Some details:

- TC leaves Q at the next word. RETURN (P3XORP7X) runs the TC that Q holds and
  leaves Q = 3.
- TS VERBNOUN with an overflowing A skips the CA, as the hardware does.
- DISPLAYE's and VNPOOH's proceed returns (TC NORMEX, TC RTRN) run the TC
  stored in that erasable cell. They are refused unchanged unless the stored
  word is a TC address (`02000`-`07777`).
- CLUPDATE's CLEAR GOTO UPDATFLG QSAVED uses the general interpretive bodies,
  CLEAR's SWITCH and GOTO's BZEGOTO, to the NEWOPS of the CADR that VN1645
  saved.

Basic routines admit a canonical image at their entry in FB 35 with EB and BB
coherent, ZERO zero, every word 15 bits, IRQ 0 or 1 and an allowed SUPERBNK.
Interpretive routines admit a canonical NEWOPS image at their LOC with BANKSET
= BB, INTBIT15 as the interpreter sets it for that bank, EDOP, ZERO and NEWJOB
zero, MODE `+0`, `+1` or vector, and the VAC one of the five with PUSHLOC
inside it. Anything else returns `EXEC_OUT_OF_SCOPE` with the image unchanged.

## Source map

`tools/lm_p34_source.py` checks this map
(`evidence/lm-p34-transfer/lm-p34-transfer-source-map.json`):

- Every word is owned by the registered routine entered at its first address,
  with the helpers its TCs fall into.
- CLUPDATE's interpretive words are literals of `lm_p34_clupdate`.
- Both CADR GOFLASH words are native site operands.
- BIT6, 1SEC, HIGH9, V06N55 and V16N45 equal their native literals in the
  fresh rope.

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| P34-35_P74-75.agc:953 | 35,3635 | `TC COMPTGO` | 02432 | `lm_p34_get45_comptgo` |
| P34-35_P74-75.agc:954 | 35,3636 | `CA SUBEXIT` | 31470 | `get45_delay` |
| P34-35_P74-75.agc:955 | 35,3637 | `TS QSAVED` | 55464 | `get45_delay` |
| P34-35_P74-75.agc:956 | 35,3640 | `CAF 1SEC` | 34777 | `get45_delay` |
| P34-35_P74-75.agc:957 | 35,3641 | `TC BANKCALL` | 04616 | `get45_delay` |
| P34-35_P74-75.agc:958 | 35,3642 | `CADR DELAYJOB` | 01735 | `get45_delay` |
| P34-35_P74-75.agc:959 | 35,3643 | `CAF V16N45` | 33730 | `get45_flash` |
| P34-35_P74-75.agc:960 | 35,3644 | `TC BANKCALL` | 04616 | `get45_flash` |
| P34-35_P74-75.agc:961 | 35,3645 | `CADR GOFLASH` | 20476 | `get45_flash` |
| P34-35_P74-75.agc:962 | 35,3646 | `TC KILCLOCK` | 03651 | `get45_terminate` |
| P34-35_P74-75.agc:963 | 35,3647 | `TC N45PROC` | 03654 | `get45_proceed` |
| P34-35_P74-75.agc:964 | 35,3650 | `TC CLUPDATE` | 03664 | `get45_recycle` |
| P34-35_P74-75.agc:965 | 35,3651 | `KILCLOCK CA Z` | 30005 | `r_kilclock` |
| P34-35_P74-75.agc:966 | 35,3652 | `TS DISPDEX` | 55163 | `r_kilclock` |
| P34-35_P74-75.agc:968 | 35,3653 | `TC GOTOPOOH` | 06001 | `r_kilclock` |
| P34-35_P74-75.agc:969 | 35,3654 | `N45PROC CS FLAGWRD2` | 40076 | `r_n45proc` |
| P34-35_P74-75.agc:970 | 35,3655 | `MASK BIT6` | 74746 | `r_n45proc` |
| P34-35_P74-75.agc:971 | 35,3656 | `EXTEND` | 00006 | `r_n45proc` |
| P34-35_P74-75.agc:972 | 35,3657 | `BZF KILCLOCK` | 13651 | `r_n45proc` |
| P34-35_P74-75.agc:973 | 35,3660 | `TC PHASCHNG` | 05353 | `r_n45proc` |
| P34-35_P74-75.agc:974 | 35,3661 | `OCT 04024` | 04024 | `r_n45proc` |
| P34-35_P74-75.agc:975 | 35,3662 | `TC UPFLAG` | 05504 | `n45proc_upflag` |
| P34-35_P74-75.agc:976 | 35,3663 | `ADRES FINALFLG` | 00047 | `n45proc_upflag` |
| P34-35_P74-75.agc:977 | 35,3664 | `CLUPDATE CA Z` | 30005 | `r_clupdate` |
| P34-35_P74-75.agc:978 | 35,3665 | `TS DISPDEX` | 55163 | `r_clupdate` |
| P34-35_P74-75.agc:979 | 35,3666 | `TC PHASCHNG` | 05353 | `r_clupdate` |
| P34-35_P74-75.agc:980 | 35,3667 | `OCT 04024` | 04024 | `r_clupdate` |
| P34-35_P74-75.agc:981 | 35,3670 | `TC INTPRET` | 06037 | `clupdate_intpret` |
| P34-35_P74-75.agc:982 | 35,3671 | `CLEAR GOTO` | 52014 | `lm_p34_clupdate` |
| P34-35_P74-75.agc:983 | 35,3672 | `UPDATFLG` | 00670 | `lm_p34_clupdate` |
| P34-35_P74-75.agc:984 | 35,3673 | `QSAVED` | 03464 | `lm_p34_clupdate` |
| P34-35_P74-75.agc:996 | 35,3674 | `DISPLAYE EXTEND` | 00006 | `r_displaye` |
| P34-35_P74-75.agc:997 | 35,3675 | `QXCH NORMEX` | 23463 | `r_displaye` |
| P34-35_P74-75.agc:998 | 35,3676 | `CAF V06N55` | 33724 | `r_displaye` |
| P34-35_P74-75.agc:999 | 35,3677 | `TCR BANKCALL` | 04616 | `r_displaye` |
| P34-35_P74-75.agc:1000 | 35,3700 | `CADR GOFLASH` | 20476 | `r_displaye` |
| P34-35_P74-75.agc:1001 | 35,3701 | `TCF GOTOPOOH` | 16001 | `displaye_terminate` |
| P34-35_P74-75.agc:1002 | 35,3702 | `TC NORMEX` | 01463 | `displaye_proceed` |
| P34-35_P74-75.agc:1003 | 35,3703 | `TCF -5` | 13676 | `displaye_recycle` |
| P34-35_P74-75.agc:1008 | 35,3704 | `P3XORP7X CAF HIGH9` | 37744 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1009 | 35,3705 | `MASK MODREG` | 71011 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1010 | 35,3706 | `EXTEND` | 00006 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1011 | 35,3707 | `BZF +2` | 13711 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1012 | 35,3710 | `INCR Q` | 24002 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1013 | 35,3711 | `RETURN` | 00002 | `r_p3xorp7x` |
| P34-35_P74-75.agc:1022 | 35,3712 | `VNPOOH EXTEND` | 00006 | `r_vnpooh` |
| P34-35_P74-75.agc:1023 | 35,3713 | `QXCH RTRN` | 23465 | `r_vnpooh` |
| P34-35_P74-75.agc:1024 | 35,3714 | `TS VERBNOUN` | 55615 | `r_vnpooh` |
| P34-35_P74-75.agc:1025 | 35,3715 | `CA VERBNOUN` | 31615 | `r_vnpooh` |
| P34-35_P74-75.agc:1026 | 35,3716 | `TCR BANKCALL` | 04616 | `r_vnpooh` |
| P34-35_P74-75.agc:1027 | 35,3717 | `CADR GOFLASH` | 20476 | `r_vnpooh` |
| P34-35_P74-75.agc:1028 | 35,3720 | `TCF GOTOPOOH` | 16001 | `vnpooh_terminate` |
| P34-35_P74-75.agc:1029 | 35,3721 | `TC RTRN` | 01465 | `vnpooh_proceed` |
| P34-35_P74-75.agc:1030 | 35,3722 | `TCF -5` | 13715 | `vnpooh_recycle` |

## Evidence

Hashes are in `evidence/lm-p34-transfer/SHA256.json`. Bulk files are assets of
the `evidence-lm-p34-transfer` release
(`python3 tools/evidence.py fetch lm-p34-transfer`).

- **Source-to-native map and independent assembly comparison.**
  `python3 tools/lm_p34_source.py` reassembles the unchanged file with the
  pinned yaYUL. The rope equals the certified fixed-data rope, and all 54 words
  map to their native routines.
- **Oracle.** `tools/lm_p34_reference.c` (GPL, host only) runs the unchanged
  Luminary099 in the pinned yaAGC hardware-DV variant from 14336 sampled
  cases. It stops at each case's first point outside the file:
  - the external entries the file transfers to (COMPTGO, DELAYJOB and GOFLASH
    through BANKCALL, PHASCHNG, UPFLAG, GOTOPOOH, CLUPDATE's TC INTPRET);
  - the UNIT and ARCSIN alarm frontiers;
  - a NEWOPS outside the file's interpretive code;
  - the case's own basic returns.

  `tools/lm_p34_differential.py` requires every native trampoline run to
  produce the same 2048 erasable words, interrupt state and SUPERBNK. It pins
  the stream identity (`lm-p34-transfer-source-stream.json`, SHA256
  `9facce6f...7023c2`, 58,863,616 bytes) and archives the stream
  (`lm-p34-transfer-reference.snapshots.bin.xz`).
  - 5120 basic cases (256 per routine) run over poisoned debris with every
    FINALFLG, MODREG (P3X, P7X, 0, any), overflowing-A, IRQ and SUPERBNK
    setting.
  - 1024 cases start at CLUPDATE's CLEAR GOTO with QSAVED set to VN1645's
    returns or another bank's CADR.
  - 4096 cases come from `lm-vn1645` and 4096 from `lm-midgim`.
  - Each stop is reached as the source allows. N45PROC goes both ways
    (GOTOPOOH 128, PHASCHNG 128). The CLUPDATE GOTO reaches COMPTGO 504 times,
    a NEWOPS outside the file 512 times and ARCSIN 1301 8 times. GET.LVC
    returns to its three callers' NEWOPS. 64 stored-return cases are refused
    unchanged. `lm-p34-transfer-proof.json` lists the stops reached per
    routine.
- **Host and sanitizer results.** Run
  `make test-lm-p34-transfer sanitize-lm-p34-transfer`. Both the strict build
  and the ASAN/UBSAN build pass 33903 checks:
  - all 14336 frozen digests;
  - every exit of every basic routine reached;
  - 64 refusals that leave the image unchanged;
  - 148 admission guards (EB, BB, ZERO, 16-bit words, SUPERBNK and IRQ for
    each basic routine; VAC, MODE, INTBIT15, EDOP, NEWJOB, BANKSET and PUSHLOC
    for each interpretive kind).

  See `lm-p34-transfer-checks.log`.
- **C64 execution results.** Run `make vice-lm-p34-transfer`. Three PRGs each
  link only the routines their cases reach and keep the image in the RAM
  under KERNAL ROM (`$01=$35`):
  - every 16th basic case, with the native INTER-BANK routines and the merged
    site table;
  - sampled CLUPDATE, VN1645 and SELECTMU cases;
  - sampled GET.LVC cases.

  Groups 2 and 3 carry their entries' orbit words frozen over the generator's
  integer debris, because the C64 has no floating point. In VICE (PAL, stock
  ROMs) they pass 320, 62 and 12 checks with no failures
  (`lm-p34-transfer-results.json`).

## Limits

- GET+MGA through the trampoline, together with its cases and GET45, does not
  fit the C64 RAM below the mailbox. Its cases, including VN1645's CALL
  GET+MGA path, are proven on the host only here. The GET+MGA phase bodies
  run on the C64 in `lm-midgim` and `lm-vn1645`.
- The generated INTER-BANK site table (`tools/agc_sites_generate.py`, Lane A)
  lists TC BANKCALL sites only. This milestone supplies its own two TCR sites.
  Lane A's INTPRET routine admits only the E/CALL entry, so CLUPDATE's TC
  INTPRET is a stop.
- COMPTGO (P32-P35_P72-P75), the display constants after `35,3722`, P34/P35,
  P74/P75 and R36 are not covered. The file stays `partial`.
- Numerical validity is the original's. No wider claim is made.
