# Run the CM IMU monitor of T4RUPT as native routines

Lane A. Milestone `cm-t4rupt-imu`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

On its IMU phase, T4JUMP sends T4RUPT to IMUMON. The IMU monitor compares the IMU status inputs (channel 30) with IMODES30 and services each change: IMU temperature (the TEMP lamp), IMU turn-on and its 90 s settling, IMU fail and CDU fail (SETISSW, the ISS warning light), IMU cage, and IMU operate. It then does the same for channel 33 against IMODES33 (PIPA fail and the downlink and uplink too-fast alarms 1105 and 1106) and checks for gimbal lock on CDUZ. This milestone covers Comanche055 T4RUPT_PROGRAM.agc from IMUMON to the 90SECS constant (bank 06,2174-2764), including CAGESUB, LAMPTEST, the IFAILJMP and C33JMP dispatch tables and the constants, and ZOPFIN3 (06,3065-3067: CCS A; INCR Q; TC Q), which LAMPTEST ends in and the optics code shares. It is native as `cm_t4rupt_imu_routines` (`src/cm_t4rupt_imu.c`), 22 entries. The code is the Luminary099 monitor (`docs/LM-T4RUPT-IMU.md`) with these differences: IMODES30 and IMODES33 are at 01320 and 01321; IMUSEFLG is a bit of STATE; ENDTNON, OPONLY and IMUOP test it there; ISSUP does not touch the drift flag; ITURNON2 does not set RADMODES; IMUCAGE turns the engine off with WAND DSALMOUT (CS BIT13) instead of writing channel 11 and clearing ENGONFLG; CAGESUB masks with BIT15+6.

| AGC | Native |
|---|---|
| IMUMON 06,2172, NXTIFBIT, NXTIFAIL, TLIM, TEMPOK | `imu` entry IMUMON: the channel 30 change word, the bit scan through IFAILJMP |
| ITURNON, ITURNON2, ENDTNON, RETNON, IMUBAD, UNZ2, ISSUP, NOATTOFF | turn-on: CAGESUB, WAITLIST ENDTNON (90 s) or VARDELAY, ZEROICDU, the IBNKCALL NOATTOFF and IMU-fail paths |
| SETISSW (IMUFAIL, ICDUFAIL), ISSWON, ISSWOFF | `setissw`: the ISS warning lamp via channel 11 bit 1, or VARALARM 1105/1106 through ITEMP6 |
| IMUCAGE, ISSZERO, CAGESUB1, CAGESUB2 | cage: channel 14 drive bits and channel 12 cleared, ENGONFLG reset, engine off, IBNKCALL RNDREFDR, the CDU and gyro command counters set to -0 |
| IMUOP, IMUOP2 | operate: IMUSEFLG, ALARM 214, back to ITURNON2 |
| C33TEST, NXTIBT, NXTFL33, PIPFAIL, DNTMFAST, UPTMFAST | the channel 33 change word, the bit scan through C33JMP |
| GLOCKMON, GLOCKCHK, SETGLOCK, NOGIMRUN, GLINVERT, GLAMPTST | gimbal lock: the NO ATT and gimbal lock lamps, SETCOARS, CA+ECE through WAITLIST |
| LAMPTEST, ZOPFIN3 | `lamptest`: the return is Q or Q + 1 |

The other 21 entries are the places the monitor resumes after a service it calls: ALARM, VARALARM, WAITLIST and VARDELAY tasks, ZEROICDU, and IBNKCALL returns. Each routine runs to the next of those services or to RESUME (GLOCKOK). Channels 12, 14, 30 and 33 are the `agc_hardware` members the LM monitor added. READ, WRITE, RAND, ROR, WOR and RXOR use them like yaAGC. WAND CHAN33 keeps the AND in A and sets bits 11-15 of the channel again, as yaAGC does for the latched inputs.

### Input domain

- The routines run inside T4RUPT with FB 06, as T4JUMP leaves it.
- The entries inside a bit scan (the ALARM returns and the IBNKCALL RNDREFDR returns of IMUCAGE and IMUOP; the C33JMP ALARM returns) need RUPTREG1 and RUPTREG2 as the scan leaves them. RUPTREG1 is the last bit handled, and every bit left in RUPTREG2 selects a TCF of IFAILJMP (0-3, 5) or C33JMP (0-2).
- SETISSW's VARALARM return (06,2713) needs ITEMP6 to hold a return in fixed memory. When that return is a scan entry, the scan state above also applies.
- The IMUCAGE RNDREFDR return (06,2573) needs channel 14's drive and gyro bits (077000) clear, as IMUCAGE's WAND CHAN14 leaves them.

Anything else is refused (`EXEC_OUT_OF_SCOPE`) with the image and io unchanged.

## Evidence

Hashes: `evidence/cm-t4rupt-imu/SHA256.json`.

- **Source map.** `tools/cm_t4rupt_imu_source_map.py` (`evidence/cm-t4rupt-imu/cm-t4rupt-imu-source-map.json`) lists the 380 words of 06,2174-2764 and 06,3065-3067. The original executed every instruction in the oracle run below. The script also checks the 37 fixed constants and the 9 external entry addresses built into the native code against the assembled program.
- **Independent assembly comparison.** `make cm-t4rupt-imu-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/cm_t4rupt_imu_reference.c`). It uses 2,048 cases at IMUMON and 128 at each other entry, 4,736 in all.
  - The erasable image is random. In a quarter of the cases IMODES30 and IMODES33 are near their flight values, and channels 30 and 33 are near IMODES30 and IMODES33 in half, so a scan has few changes.
  - Every sixteenth case gives CDUZ +0 or -0, and one IMUMON case in sixteen gives the TEMP change with IMODES30 left +0, to reach those branches of CCS.
  - Scan entries get a valid scan state. Channels 11, 12 and 14 are random.
  - The CDU and gyro command counters (047, 050-052) and the optics counters (035, 036) are +0 on entry. yaAGC's peripheral model empties them when the matching channel 14 drive bit is set, which is the outside world, not the program.

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z, and FB in banked memory. Every case equals the original in all 2,048 erasable words, SUPERBNK and channels 11, 12, 14, 30 and 33. Per-case digests are frozen into `tests/cm_t4rupt_imu_vectors.h`; see `evidence/cm-t4rupt-imu/cm-t4rupt-imu-proof.json`.
- **Host and sanitizer results.** `make test-cm-t4rupt-imu sanitize-cm-t4rupt-imu` (`evidence/cm-t4rupt-imu/cm-t4rupt-imu-host.log`) checks:
  - the digests;
  - that every continuation is reached: RESUME, ALARM, VARALARM, WAITLIST, VARDELAY, TASKOVER, IBNKCALL, POSTJUMP, ZEROICDU and SETISSW's caller;
  - five refusals with the image unchanged, and that the entry addresses do not resolve in another bank.
- **C64 execution results.** `make vice-cm-t4rupt-imu` (`evidence/cm-t4rupt-imu/cm-t4rupt-imu-vice.log`) runs 300 cases: every sixteenth, plus the four that `tools/c64_case_cover.py` adds so that the C64 rows run every branch outcome (control-flow edge) of the native routine that the full host case set runs. The proof records the edge counts.

OPTMON, the CM optics monitor, remains; T4RUPT_PROGRAM.agc stays partial.
