# Flight chains stop at the interpreter

Pinball module split lane, for the steward's flight-store overflow list
(`/mnt/project-files/pin-split/store-overflow.md`).

Under the native-interpreter decision, the C64 flight program runs
interpretive code through the interpreter module (`interp.prg`), which
reads the order words from the REU rope. Compiled interpretive routines
stay as the test oracle and as a choice for other targets.

Some files run their basic and interpretive code in one step machine: a
basic chain that reaches `TC INTPRET` goes on through the compiled
interpretive orders in the same C call. Their flight modules then link
every order the file uses. With basic tables only, each still overflowed
the 16 KB job slot, by 12 to 39 KB.

With `-DAGC_FLIGHT_INTERPRETER`, the file's `steps()` ends a chain where it
reaches NEWOPS. Z is then NEWOPS and DANZIG has been done, exactly as the
original leaves it there, so the interpreter takes over. An entry at an
interpretive key is refused, because the interpreter owns it. Without
the flag (host tests, certificates, C64 rows) the code is unchanged.

| File | Lane | Basic-only flight link (LM, bytes) |
|---|---|---|
| `lm_ags_init` | F | 4,139 (was 11,931 over the slot) |
| `lm_lunar_landing` | F | 3,478 (was 24,207 over) |
| `lm_imu_performance` | D1 | 11,328 (was 38,827 over) |
| `lm_burn_baby` | F | 15,310 (was 16,519 over) |
| `lm_servicer` | F | two parts, 14,581 and 14,302 ([LM-SERVICER-FLIGHT-PARTS.md](LM-SERVICER-FLIGHT-PARTS.md)); one module was 5,642 over |

The sizes come from `tools/flight_store_build.py`'s `link_module` with the
file's basic tables and `-DAGC_FLIGHT_INTERPRETER` added to the common
flags. The flight store builds them that way (`FLIGHT_INTERP_STOP` in
`tools/flight_store_build.py`, [FLIGHT-STORE.md](FLIGHT-STORE.md)).

## Evidence

`make flight-interp-check` (`tools/flight_interp_check.py`) builds each
file's host snapshot with the flag. It also builds the file's own yaAGC
reference driver, with the file's leave test widened to stop at NEWOPS.
Every case the flight build runs must equal the original program stopped
there, over all 2048 erasable words and the interrupt state.

| File | Cases | Equal | Refused unchanged (interpretive keys) |
|---|---|---|---|
| `lm_ags_init` | 29,952 | 11,520 | 18,432 |
| `lm_burn_baby` | 22,272 | 19,884 | 2,388 |
| `lm_lunar_landing` | 46,080 | 10,752 | 35,328 |
| `lm_imu_performance` | 64,512 | 15,843 | 48,669 |

For `lm_ags_init`, 1,536 cases stop earlier with the flag than without it,
at NEWOPS, and all of them match the original program there.
