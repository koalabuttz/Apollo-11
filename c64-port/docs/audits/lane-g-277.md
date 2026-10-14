# Certificate review: Comanche055/LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` c9f989fd
(after Lane G PR #277), against the certificate drafted by milestone
`cm-ephemeris`. The certificate, `src/cm_ephemeris.c`, the shared interpreter
bodies, the C64 vectors and the `cm-ephemeris` spec are unchanged at 1816baab.

This is the second review. The first (issue #58, tip 3f72179d) was rejected
because S1 was pinned to 6 at every entry, so LSTIME's `SSP S1 6D` write was
never compared.

Verdict: **accepted**.

## What was checked

- **The blocker is fixed.** The spec draws S1 = 6 only from LSTIME+11
  (26,2141, `GOTO X1`) onward. At every entry from 26,2110 through 26,2135,
  256 of 256 cases have S1 ≠ 6, some negative. 12 of the 38 C64 rows enter
  there with S1 ≠ 6, and 7 of them run through to the 06037 stop.
- **A mutant without the SSP fails.** With `cmi_ssp(s,000006)` removed,
  `test-cm-ephemeris` fails 56 of 256 and the yaAGC differential fails
  1814 of 8192 cases.
- **Reproduced at c9f989fd.**
  - `make cm-ephemeris-differential` (real yaAGC, unmodified Comanche055 rope):
    8192 cases equal across 32 routines. Flow and resume_flow are 24 of 24.
    The stop table, vectors and groups are unchanged, and the proof and source
    map re-derive byte for byte.
  - `make test-cm-ephemeris sanitize-cm-ephemeris`: 256 checks each, 0 failures.
  - `make vice-cm-ephemeris`: 2 PRGs, 16 + 22 = 38 checks, 0 failures. This
    equals `vice_targets` and the proof's `c64_rows`, and the PRG hashes equal
    the log.
  - The 38 decoded C64 records equal the cases at the proof's `case_ids`, and
    a host run of them reproduces all 38 expected digests.
  - All evidence hashes match, and `cert_precheck.py` reports 0 FAIL.
- **C64 cover.** An independent `gcc -O0 --coverage` build, keyed per object:
  all 8192 cases take 558 arcs, and the 38 C64 rows take the same 558.
- **Transfer ABI.** It is unchanged from the first review:
  - all 32 entry keys are at their original addresses;
  - every DANZIG pair's NEWJOB path is compared;
  - every Comanche caller, including S-BAND's superbank 0100, is admitted;
  - the outgoing CHANG2 stays the Z=05170 frontier.

  `index_carry16` is set.

## Notes for the lane (not blocking)

- **ROUNDSUB carry.** The redrawn case set no longer reaches the ROUNDSUB carry
  into the top word (`src/lm_interp_orders.c:318-320`, through
  `cm_interp_orders.o`).
  - The old rows reached it in 1 case. Arcs went from 559 to 558, which the docs
    report honestly.
  - `cm-interp-bodies` does not reach it either, so no milestone compares this
    path today.
  - Either fix closes it: an operand edge shape for this spec (MPAC words at
    037777/040000), or a ROUNDSUB carry vector in `cm-interp-bodies`.
- **S1 outside the LSTIME range** never holds 0 or −0. The SSP overwrites S1
  before any reader, so "any value" slightly overstates what was drawn.
- **Still open from the first review:**
  - The PUSHLOC wording says the entries leave PUSHLOC there; it is the caller
    that does not.
  - The interrupt wording should say the scaler is held at 0 and that IE 0/1
    is compared.
  - No superbank-0100 or bank ≥030 caller return is in the compared set.
  - X1/X2 never hold an erasable CADR.
