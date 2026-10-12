# Translate Comanche TVCDAPS (CM pitch and yaw TVC autopilot)

Lane G. Milestone `cm-tvc-daps`.

## Scope

`Comanche055/TVCDAPS.agc`, all of it: bank 20 `2530`-`3500`, 489
assembled words (including the ACTSAT, 1/ACTSAT, PITCHT5, DAPT5, YAWT5,
1/RTLIM, 1-E(-AT) and E(-AT) constants; ERRLIM and 1/ERRLIM are EQUALS of
fixed-fixed BIT13 and BIT3). Nothing is left in the file.

`cm_tvc_daps(image, io)` runs one entry to its exit. `io` holds channel 14
(WOR of bit 11 for pitch, bit 12 for yaw: the TVC drive requests).

| Entry | Reached from |
|---|---|
| `2530` PITCHDAP | T5RUPT via T5LOC (PITCHT5, set by YAWDAP and DAPINIT) |
| `2732` YAWDAP | T5RUPT via T5LOC (YAWT5, set by PITCHDAP) |
| `3107` DAPINIT | T5RUPT via DAPT5 (TVCINITIALIZE) |
| `2541`, `2543`, `2743`, `2745` | return from `TC HACK` (TVCSTROKETEST) |
| `2675` PCOPY+1, `3051` YCOPY+1 | ISWCALL from FINCOPY (TVCEXECUTIVE); end in TC Q |

Admitted state: FB bank 20 (`040000`), EB E6, BB `040006`, any raw A/L,
erasable contents and channel 14; for PCOPY+1/YCOPY+1, Q must be a caller's
return in `4000`-`7777`. `CM_TVC_DAPS_OK` leaves `Z = RESUME 5333` (PDAPEND,
YDAPEND), `Z = NOQRSM 5335` (DAPINIT), `Z = HACK 3533` with Q the TC's
return, or `Z` = the caller's return with `Q = 3`; anything else is
`CM_TVC_DAPS_OUT_OF_SCOPE`, image and io unchanged. The file has no loop.

### How it is translated

As in `cm-tvc-init`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original addresses. Each `TCR` sets Q
and jumps; each `TC Q` goes through one dispatch on the return address, the
way the TC word held in Q does (leaving Q = 3). The dispatch covers the
twelve TCR returns; any other Q is the external caller. MSU and WOR are
written from the yaAGC definitions.

- `TS L` at `3141` after DOUBLE overflows when CMDTMP+1 is large; the skip
  goes to `AD CMDTMP` with A = ±1 (the carry into the high word).
- `TS TIME5` at `3114` overflows with large T5TVCDT; the skip goes to
  `TS T5LOC` with A = ±1.
- The other TS words store an MSU or MP result, a load, a constant or a
  CCS result, so they cannot overflow.
- `CCS A` at `3212` and `3407` sees DAPDATR1 masked with BIT14 or BIT13,
  so only the + and +0 slots occur. The +0/-0 slots of ERRORLIM and ACTLIM
  follow a BZF that has already taken a zero product; they are translated
  (they store the CCS's +0) but no sample reaches them.

## Source map

`evidence/cm-tvc-daps/cm-tvc-daps-source-map.json`
(`tools/cm_tvc_daps_source.py`): every word with its source line, rope
check and native label, the 32 fixed-constant reads matched to the native
literals, the TC Q dispatch matched to the listing's TCR return addresses
and the HACK returns matched to the listing's `TC HACK` words.

Independent assembly comparison: `evidence/cm-tvc-daps/cm-tvc-daps-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`), every word yaYUL emits for the
unchanged file against the original Comanche055 rope, without reading the
native code.

## Evidence

Hashes: `evidence/cm-tvc-daps/SHA256.json`; bulk files in release
`evidence-cm-tvc-daps`.

- Oracle: `deps/cm-flight-fixture` (`tools/cm_flight_prepare_reference.py`,
  original CM rope `6e3150ae...`). The runner stops at HACK, RESUME, NOQRSM
  or, for PCOPY+1/YCOPY+1, the caller's return. Channel 14 starts with the
  bits yaAGC's gyro and CDU drive models act on (`01000`, `070000`) clear.
  yaAGC's optics CDU model empties OPTY/OPTX (`035`/`036`) when bits 11/12
  are set; the program never addresses them, so the runner keeps them as
  the program left them. The runner's scaler is held, so TIME5 does not
  count.
- Host and sanitizer results: `make test-cm-tvc-daps sanitize-cm-tvc-daps`.
  65536 cases (`tests/cm_tvc_daps_cases.c`: every entry; STROKER zero, -0,
  small or raw; CDU readings near and far from their past values; attitude
  errors, rates, sines and cosines, filter coefficients, VARK, offsets and
  filter state zero, -0, small, medium or raw, so the rate limit, ERRORLIM,
  ACTLIM, its DOUBLE overflow, both DAPDATR1 cascade choices and DAPINIT's
  TIME5 overflow all occur; raw A/L/Q and unowned words). Every case reaches
  an exit, and the two sides agree on all of them. Stream identity
  `cm-tvc-daps-source-stream.json`, deltas `cm-tvc-daps-reference.deltas.xz`,
  log `cm-tvc-daps-checks.log`.
- C64 execution results: `make vice-cm-tvc-daps`, 128 frozen rows
  (`tools/cm_tvc_daps_freeze.py`). `cm-tvc-daps-results.json`.

## Open

ABI registration of the entries follows in the Lane G routine-table step,
after Lane A's ISWRETRN fix.
