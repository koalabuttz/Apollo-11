# Run the CM optics monitor and drive of T4RUPT as native routines

Lane A (kernel). Milestone `cm-t4rupt-optics`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`). Interface: `docs/NATIVE_TRANSFER_ABI.md`.

On its optics phases, T4JUMP sends T4RUPT to OPTMON and to OPTTEST, whose IBNKCALL runs OPTDRIVE in bank 10. This milestone covers the rest of Comanche055 T4RUPT_PROGRAM.agc after the IMU monitor:
- **OPTMON (06,2765-3262).** The optics monitor watches the OCDU fail bit in channel 30 and the optics mode switch in channel 33 against OPTMODES. It runs the zero-optics sequence: SETZOEND, the ENDZOPT task and its 200 ms VARDELAY, and ZEROPCDU. It also handles the switch changes (CSC, manual, zero optics) through DESOPMOD, SWSAMPLE, WTOPTION and ZOPTCNT. OCDUFTST and the OCDU FAIL lamp are in DSPTAB+11D.
- **OPTDRIVE (10,2000-2202).** The optics CDU drive: shaft zone update, shaft stop avoidance, and the trunnion and shaft commands through CDUTCMD. Channel 12 and channel 14 bits 11-12 send them.

It is native as `cm_t4rupt_optics_basic` (`src/cm_t4rupt_optics.c`, generated). There are 8 routines:
- the entries OPTMON, OPTDRIVE and ENDZOPT (SETZOEND's WAITLIST task);
- the continuations after ALARM (06,3043, 06,3053, 10,2036), after WAITLIST (06,3166) and after VARDELAY (06,3200).

Each runs to RESUME, TASKOVER or the next of those calls. T4RUPT through 90SECS (06,2000-2764) is `t4rupt_core` and `cm_t4rupt_imu`. The spec marks it `outside`, and ZOPFIN3, which LAMPTEST shares, is compiled here as well. With those two milestones, every word of the file is native.

## Spec

`cm-t4rupt-optics` in `tools/interp_specs.py`:
- CDUT and CDUS (counters, 0 unless set) hold values around 0, ±45°, ±90° and ±180°, so that OPTDRIVE's zone and shaft-stop tests go both ways.
- SETOFF's `BZF TCQ`, the no-change return of OCDUFTST, lands on the interpreter's `TC Q` word at 06711 (TCQ = Q+2 +1). It runs in place as the file's own TC Q (`tcq_words`), so Q becomes 3 and the run continues at the caller in the file. It is not an exit to an unregistered fixed address.

- `basic_split`: one body per bank (OPTMON's in 06, OPTDRIVE's in 10). The whole module is about 20 KB, over either slot, but each bank's chunk fits the job slot.

Compiler additions with this file (every other spec's generated files are unchanged, `interp_compile.py <spec> --check`; `cm-p34-p35` already differs on the base):
- `tools/basic_compile.py`: an INDEXed MSU (`INDEX OPTIND; MSU CDUT`) runs on the indexed word when that word is an ordinary erasable one. Anything else is EXEC_INVALID_ACTION.
- `tools/basic_compile.py`, `tools/interp_compile.py`: `tcq_words` (fixed-fixed addresses of outside `TC Q` words) run in place. The oracle treats them as the file's own words.

## Evidence

Hashes are in `evidence/cm-t4rupt-optics/SHA256.json`. The logs are assets of the `evidence-cm-t4rupt-optics` release.

- **Source map.** `cm-t4rupt-optics-source-map.json` lists all 833 words of the file. The 372 words from OPTMON on and RELTAB are compiled routines (319) or constants (53). The 461 words of T4RUPT through 90SECS are marked as native in `t4rupt_core` and `cm_t4rupt_imu`.
- **Independent assembly comparison.** `make cm-t4rupt-optics-differential` runs 512 states per routine (4,096 cases). All of them equal the pinned flight-device yaAGC in Z, all 2,048 erasable words, the interrupt-enable state, and channels 12, 14, 30 and 33 with the 31A/31B/32 trap flags. Every 16th case is frozen in `tests/cm_t4rupt_optics_vectors.h`.
- **Host and sanitizer results.** `make test-cm-t4rupt-optics sanitize-cm-t4rupt-optics` runs the 274 frozen cases (`cm-t4rupt-optics-host.log`).
- **C64 execution results.** `make vice-cm-t4rupt-optics` runs one diagnostic with 46 cases (`cm-t4rupt-optics-vice.log`). The cases reach all 350 labels and all 611 edges that the host cases reach, and take all 385 gcov branch arcs the host cases take over every linked source (`cm-t4rupt-optics-proof.json`: `c64_coverage`, `c64_gcov_branches`).
