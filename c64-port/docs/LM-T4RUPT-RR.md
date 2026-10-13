# Run the LM rendezvous radar monitor and GPMATRIX of T4RUPT as native routines

Lane A. Milestone `lm-t4rupt-rr`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The last part of Luminary099 T4RUPT_PROGRAM.agc (bank 06,3006-3155) holds the two remaining T4JUMP targets. RRAUTCHK watches the rendezvous radar: it follows the RR auto-mode bit (channel 33) and the RR CDU fail bit (channel 30) into RADMODES, starts RRTURNON, raises alarm 515 and updates the tracker fail lamp (SETTRKF), and checks the antenna angles against their limits (RRLIMCHK), starting a reposition (MONREPOS, DORREPOS) when they are out. GPMATRIX (DAPT4S, also NORRGMON) builds the gimbal-to-pilot matrix terms M11, M21, M22, M31, M32 and COSMG for the DAP from CDUZ and CDUX with SPSIN and SPCOS, and ends T4RUPT at RESUME. It is native as `lm_t4rupt_rr_routines` (`src/lm_t4rupt_rr.c`), 12 entries. With `t4rupt-core` and `lm-t4rupt-imu`, every word of the file is native.

| AGC | Native |
|---|---|
| RRAUTCHK 06,3006 | `rr` entry: RR auto-mode change, RADMODES update, WAITLIST RRTURNON, or channel 12 bit 2 off |
| RRCDUCHK, TRKFLCDU | RR CDU fail change; ALARM 515 when P20 or P22 runs (RNDVZFLG); TC SETTRKF |
| RRGIMON, MONREPOS | NORRMON, the servicer and MUNFLAG tests, TC RRLIMCHK (ADRES CDUT); the reposition: REPOSBIT, channel 12, WAITLIST DORREPOS |
| GPMATRIX 06,3132 | TC SPSIN and TC SPCOS for CDUZ and CDUX; M11, COSMG, M22, M31, M32, M21; TC RESUME |

The other ten entries are where the code resumes after WAITLIST, ALARM, SETTRKF, RRLIMCHK (both returns), SPSIN and SPCOS. M11-M32 are in the EB window (EBANK= M11, which T4RPTBB sets), and the native code addresses them through EB like the original. RXOR CHAN30 and CHAN33 read `program->hardware`; WAND CHAN12 writes it.

### Input domain

- The routines run inside T4RUPT with FB 06, as T4JUMP leaves it.
- The SPSIN and SPCOS returns (06,3134, 3137, 3142, 3151) need A without overflow, as those routines leave it: TS would otherwise skip the next instruction.

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image and io unchanged.

## Evidence

Hashes: `evidence/lm-t4rupt-rr/SHA256.json`.

- **Source map.** `tools/lm_t4rupt_rr_source_map.py` (`evidence/lm-t4rupt-rr/lm-t4rupt-rr-source-map.json`) lists the 104 words of 06,3006-3155. The original executed every instruction in the oracle run below. The script also checks the 14 fixed constants and the 7 external entry addresses built into the native code against the assembled program.
- **Independent assembly comparison.** `make lm-t4rupt-rr-differential` runs the unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_t4rupt_rr_reference.c`). It uses 2,048 cases at RRAUTCHK and 128 at each other entry, 3,456 in all.
  - The erasable image is random. In half the cases RADMODES and the flag words have few bits set, and channels 30 and 33 agree with RADMODES in half, so each test goes both ways.
  - EB is E6 in three cases of four and any bank in the rest.

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z, and FB in banked memory. Every case equals the original in all 2,048 erasable words, SUPERBNK and channels 11, 12, 14, 30 and 33. Per-case digests are frozen into `tests/lm_t4rupt_rr_vectors.h`; see `evidence/lm-t4rupt-rr/lm-t4rupt-rr-proof.json`.
- **Host and sanitizer results.** `make test-lm-t4rupt-rr sanitize-lm-t4rupt-rr` (`evidence/lm-t4rupt-rr/lm-t4rupt-rr-host.log`) checks:
  - the digests;
  - that every continuation is reached: SPSIN, SPCOS, SETTRKF, WAITLIST, RRLIMCHK, RESUME and ALARM;
  - four refusals (an overflow in A at each SP trig return) with the image unchanged.
- **C64 execution results.** `make vice-lm-t4rupt-rr` (`evidence/lm-t4rupt-rr/lm-t4rupt-rr-vice.log`) runs every sixteenth case (216). `tools/c64_case_cover.py` confirms that these rows run every branch outcome (control-flow edge) of the native routine that the full host case set runs.
