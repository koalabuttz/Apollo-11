# LM flight job-slot load profile (P63, P66)

Where the LM landing windows spend their job-slot module loads on the C64,
and what the DMP/DMPSUB native calls and the trim gimbal co-location
(docs/FLIGHT-STORE.md, docs/INTERP-COMPILE.md `native_calls`) changed.
Measured 2026-10-18 by the Pinball split lane.

## Method

Step records from the C64 (VICE), 300 cs from each snapshot:

```
FSV_STEPS=1 SKIP_EVIDENCE_RESTORE=1 python3 tools/flight_scenario_vice.py \
    --scenario lm-p63-nassp --snap-cs 90000 --cs 300
FSV_STEPS=1 SKIP_EVIDENCE_RESTORE=1 python3 tools/flight_scenario_vice.py \
    --scenario lm-landing-ascent-nassp --snap-cs 123000 --cs 300
```

A step record's job-slot load bit (`task_loads & 1`) counts a module load;
records map to modules through `build/flight-store/lm-resolver.json`. The
counts include loads in interrupts. Times are the same windows with
`FSV_STEPS=0` (P63 500 cs, P66 300 cs), as the multiple of real time
that `flight_scenario_vice.py` reports.

Both trees carry the LM store fit (#625), so the windows run with no stops:
"before" is the port branch at d34594f0 plus #625, "after" adds this change.

## Result

| Window | loads before | loads after | change | time before | time after |
|---|---|---|---|---|---|
| P63, 90000 cs | 640 | 566 | -11.6% | 21.37x | 20.64x |
| P66, 123000 cs | 937 | 855 | -8.8% | 26.60x | 25.48x |

GTS and ACDT still load the trim gimbal module; PAXFILT +4 and CLOSEOUT
no longer load the P- and Q,R-axis modules after them.

## Module transitions before the change

| P66 (123000) | n | P63 (90000) | n |
|---|---|---|---|
| INTERPRETER DMPSUB <-> POWERED_FLIGHT_SUBROUTINES AX*SR*T loop | 36+33 | INTERPRETER VECAGREE <-> ALSIGNAG (parts 1, 3) | 18+18 |
| INTERPRETER parts SN1 <-> POLY, DMP, DSQSUB | 90 | INTERPRETER UNIT <-> MPACVBUF, ENDDOT, VROTATEX | 24 |
| excursion PAXIS -> Q,R-axis CHEKSTIK -> resident | 30+30 | excursion PAXIS -> trim gimbal GTS -> Q,R-axis CLOSEOUT | 14+15 |
| resident DANZIG -> powered subs TR*GL**P -> interbank USPRCADR | 26+24 | excursion -> trim gimbal ACDT -> P-axis PAXFILT -> excursion | 13+13+13 |
| Pinball display chain | about 25 a display | INTERPRETER DMPSUB <-> AX*SR*T loop | 12+11 |
| FLESHPOT <-> DMP | 5+5 | FLESHPOT <-> DMP/DMPSUB | 8+8 |

## Packs that don't fit

The load sequence replayed with routines moved, and the packs linked:

| Candidate | P66 loads | P63 loads | Fits the 16.3 KB slot? |
|---|---|---|---|
| Pinball display chain in one chunk (17 entries) | -6.4% | -3.1% | no, 20.4 KB over |
| DMPSUB beside the AX*SR*T module | -7.6% | -3.5% | no, DMP/DMPSUB compiled is 5.1 KB, that module has 1.6 KB free (native calls instead, above) |
| CHEKSTIK in the excursion pack | -3.2% | 0 | no, the excursion is full (16,315 B) |
| USPRCADR beside TR*GL**P | -2.9% | 0 | no |

The repeated loads are calls between basic subroutines in different files.
A pack needs both sides in one module, and the compiled code on each side
is too big to share a slot, so the cuts that fit are running a small callee
in place (DMP/DMPSUB) or carrying a small hot table beside its caller (the
trim gimbal).
