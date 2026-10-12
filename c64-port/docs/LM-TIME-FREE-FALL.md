# Translate Luminary time of free fall (TFFCONIC/TFFCONMU, TFFRP/RA, CALCTPER, CALCTFF)

Lane B. Milestone `lm-time-free-fall`.

## Scope

`Luminary099/TIME_OF_FREE_FALL.agc`, all of it: bank 27 `03360`-`03772`, 266
assembled words on 253 source lines. That is TFFCONIC `03360`, TFFCONMU
`03361`, TFFRP/RA `03427`, CALCTPER `03465`, CALCTFF `03470`, the T(X) series
table and the bank-27 constants. Nothing is left in the file.

`lm_time_free_fall(state, io)` runs from the original interpretive entry
frontier (NEWOPS after the caller's CALL, LOC = the entry, FBANK 27) to the
caller's NEWOPS frontier selected by QPRET. It returns 1, or 2 at the original
SQRT abort frontier (alarm 1302), or 0 with the image unchanged when the entry
is outside the contract. Admitted entries:

- canonical 2048-word image, coherent EB/FB/BB with BANKSET = BB, all words
  15-bit, Z and NEWJOB zero, one of the five VACs;
- INTBIT15 `040000` (CALL from bank 22), and EDOP `040000` for every entry
  except TFFCONIC, which is admitted with EDOP `+0` (below);
- QPRET one of the actual SR30.1 returns (bank 22): `045567` after TFFCONMU,
  `045571` after TFFRP/RA, `045625` after CALCTPER, `045633` after CALCTFF.
  TFFCONIC shares `045567`.

TFFCONIC has no caller in Luminary099. A CALL from bank 22 leaves EDOP equal
to BBANK bit 15 (`040000`), and TFFCONIC begins with a store code, so its first
DANZIG would go to CHANG2 instead of the next order. The cases sample TFFCONIC
from the EDOP `+0` state, the only one in which its listing runs as written.

`R300K` (`27,3766`) is assembled but referenced by nothing in Luminary099; it is
exported as `lm_time_free_fall_r300k` for completeness.

### How it is translated

Every interpretive pair is compiled. The routine uses the general Luminary
interpretive bodies (`src/lm_interp_orders.c`, Lane B: loads, stores, DAD/DSU/
BDSU/DMP, DDV/BDDV, SQRT, DSQ, VSQ, DOT, VXV, VXSC, UNIT, ABVAL, SIGN, COMP,
TSLC, GSHIFT, short shifts, STADR, PUSH, BOV/BOVB, RTB and POLY) and the
dispatch paths of `src/lm_interp_steps.c` (Lane A). This milestone adds one
general body, `lmi_bplbmn` (BPL/BMN), checked against yaAGC with the other 37
bodies by `tools/lmi_body_differential.py`. Each call passes the order and
operand words of its own source line as literals; nothing is fetched from code
memory. TFF1/4 and TFFZEROS (bank 23) come from the certified fixed-data table;
the bank-27 constants and the 13-word T(X) table are native literals. The RTB
SIGNMPAC path uses the qualified `rtb_agreement` body.

The routine is split into 25 steps between original NEWOPS frontiers
(`enum lm_time_free_fall_phase`, `lm_time_free_fall_step`, and the `ltf_*`
entry points). The image carries every continuation, so each step can become
its own overlay module later.

Original details the translation reproduces, found by stepping the oracle:
an address order followed by an order pair (negative word) takes PUSHUP, as
`RTB DMP` at `03611` does; BMN with CYR `-0` goes straight to DANZIG; BOV only
decrements OVFIND (CCS/TS), so an incoming OVFIND changes MAXRA/RAPO and the
cases cover OVFIND `0` and `+-1`; ALFA is tested with BZE on the full
triple-precision MPAC, so the parabolic branches need an exact zero.

### Interpreter ABI

`lmi_bplbmn` is new shared interpreter code in `lm_interp_orders`. Lane A owns
the interpreter ABI and should review it; it adds no runtime entry points.

## Source map

Every assembled word is a literal argument of the native function that owns
its address (`tools/lm_time_free_fall_source.py`,
`evidence/lm-time-free-fall/lm-time-free-fall-source-map.json`).

| Line | Address | Source | Words | Native |
|---|---|---|---|---|
| 168 | 27,3360 | `TFFCONIC STORE TFF/RTMU` | 00037 | `ltf_entry` |
| 170 | 27,3361 | `TFFCONMU VLOAD UNIT` | 53575 | `ltf_conmu` |
| 171 | 27,3362 | `RONE` | 02207 | `ltf_conmu` |
| 172 | 27,3363 | `PDDL` | 77725 | `ltf_conmu` |
| 173 | 27,3364 | `36D` | 00045 | `ltf_conmu` |
| 174 | 27,3365 | `STORE RMAG1` | 00015 | `ltf_conmu` |
| 176 | 27,3366 | `NORM` | 77701 | `ltf_conmu` |
| 177 | 27,3367 | `X1` | 00047 | `ltf_conmu` |
| 178 | 27,3370 | `STOVL NRMAG` | 24041 | `ltf_conmu` |
| 179 | 27,3371 | `VONE` | 02215 | `ltf_conmu` |
| 180 | 27,3372 | `VXSC` | 77761 | `ltf_conmu` |
| 181 | 27,3373 | `TFF/RTMU` | 00037 | `ltf_conmu` |
| 182 | 27,3374 | `STORE VONE'` | 02170 | `ltf_conmu` |
| 184 | 27,3375 | `VXSC VXV` | 47361 | `ltf_conmu` |
| 185 | 27,3376 | `NRMAG` | 00041 | `ltf_conmu` |
| 187 | 27,3377 | `VSL1 VSQ` | 47572 | `ltf_conmu` |
| 188 | 27,3400 | `STODL TFFNP` | 14035 | `ltf_conmu` |
| 190 | 27,3401 | `TFF1/4` | 06512 | `ltf_conmu` |
| 191 | 27,3402 | `DDV PDVL` | 63271 | `ltf_conmu` |
| 192 | 27,3403 | `NRMAG` | 00041 | `ltf_conmu` |
| 193 | 27,3404 | `VONE'` | 02170 | `ltf_conmu` |
| 194 | 27,3405 | `VSQ DCOMP` | 57436 | `ltf_conmu` |
| 195 | 27,3406 | `STORE TFFVSQ` | 00025 | `ltf_conmu` |
| 197 | 27,3407 | `SR* DAD` | 43257 | `ltf_conmu` |
| 199 | 27,3410 | `0 -6,1` | 20573 | `ltf_conmu` |
| 200 | 27,3411 | `STADR` | 77626 | `ltf_conmu` |
| 202 | 27,3412 | `STORE TFFALFA` | 77744 | `ltf_conmu` |
| 203 | 27,3413 | `SL* PUSH` | 41457 | `ltf_conmu` |
| 204 | 27,3414 | `0 -6,1` | 20173 | `ltf_conmu` |
| 205 | 27,3415 | `ABS SQRT` | 75446 | `ltf_conmu` |
| 206 | 27,3416 | `NORM` | 77701 | `ltf_conmu` |
| 207 | 27,3417 | `X2` | 00050 | `ltf_conmu` |
| 208 | 27,3420 | `STORE TFFRTALF` | 00031 | `ltf_conmu` |
| 209 | 27,3421 | `DSQ SIGN` | 75316 | `ltf_conmu_end` |
| 211 | 27,3422 | `BZE BDDV` | 55254 | `ltf_conmu_end` |
| 212 | 27,3423 | `+2` | 57425 | `ltf_conmu_end` |
| 213 | 27,3424 | `TFF1/4` | 06512 | `ltf_conmu_end` |
| 214 | 27,3425 | `+2 STORE TFF1/ALF` | 00027 | `ltf_conmu_end` |
| 215 | 27,3426 | `DUMPCNIC RVQ` | 77616 | `ltf_conmu_end` |
| 270 | 27,3427 | `TFFRP/RA DLOAD DMP` | 41345 | `ltf_rpra` |
| 271 | 27,3430 | `TFFALFA` | 00033 | `ltf_rpra` |
| 272 | 27,3431 | `TFFNP` | 00035 | `ltf_rpra` |
| 273 | 27,3432 | `SR* DCOMP` | 57457 | `ltf_rpra` |
| 274 | 27,3433 | `0 -8D,1` | 20571 | `ltf_rpra` |
| 275 | 27,3434 | `DAD ABS` | 51415 | `ltf_rpra` |
| 277 | 27,3435 | `DP2(-4)` | 17765 | `ltf_rpra` |
| 278 | 27,3436 | `SQRT DAD` | 43366 | `ltf_rpra` |
| 279 | 27,3437 | `TFF1/4` | 06512 | `ltf_rpra` |
| 280 | 27,3440 | `PUSH BDDV` | 55206 | `ltf_rpra` |
| 281 | 27,3441 | `TFFNP` | 00035 | `ltf_rpra` |
| 282 | 27,3442 | `SR* SR*` | 53657 | `ltf_rpra` |
| 283 | 27,3443 | `0,1` | 20601 | `ltf_rpra` |
| 284 | 27,3444 | `0 -7,1` | 20572 | `ltf_rpra` |
| 285 | 27,3445 | `STODL RPER` | 14017 | `ltf_rpra` |
| 287 | 27,3446 | `DMP BOVB` | 41005 | `ltf_rpra` |
| 288 | 27,3447 | `TFF1/ALF` | 00027 | `ltf_rpra` |
| 289 | 27,3450 | `TCDANZIG` | 57753 | `ltf_rpra` |
| 290 | 27,3451 | `BZE SL*` | 53654 | `ltf_rpra` |
| 291 | 27,3452 | `MAXRA` | 57461 | `ltf_rpra` |
| 292 | 27,3453 | `0 -5,2` | 57603 | `ltf_rpra` |
| 293 | 27,3454 | `SL* BOV` | 40057 | `ltf_rpra` |
| 294 | 27,3455 | `0,2` | 57576 | `ltf_rpra` |
| 295 | 27,3456 | `MAXRA` | 57461 | `ltf_rpra` |
| 296 | 27,3457 | `BPL` | 77644 | `ltf_rpra` |
| 297 | 27,3460 | `+3` | 57463 | `ltf_rpra` |
| 298 | 27,3461 | `MAXRA DLOAD` | 77745 | `ltf_maxra` |
| 299 | 27,3462 | `NEARONE` | 17771 | `ltf_maxra` |
| 300 | 27,3463 | `+3 STORE RAPO` | 00021 | `ltf_rapo` |
| 301 | 27,3464 | `DUMPRPRA RVQ` | 77616 | `ltf_rapo` |
| 401 | 27,3465 | `CALCTPER SETGO` | 77614 | `ltf_calctper` |
| 402 | 27,3466 | `TFFSW` | 03436 | `ltf_calctper` |
| 403 | 27,3467 | `+3` | 57472 | `ltf_calctper` |
| 404 | 27,3470 | `CALCTFF CLEAR` | 77614 | `ltf_calctff` |
| 405 | 27,3471 | `TFFSW` | 03676 | `ltf_calctff` |
| 406 | 27,3472 | `+3 STORE RTERM` | 00023 | `ltf_terms` |
| 407 | 27,3473 | `SL*` | 77657 | `ltf_terms` |
| 408 | 27,3474 | `0,1` | 20201 | `ltf_terms` |
| 409 | 27,3475 | `STORE NRTERM` | 00021 | `ltf_terms` |
| 410 | 27,3476 | `DMP BDSU` | 44205 | `ltf_terms` |
| 411 | 27,3477 | `TFFALFA` | 00033 | `ltf_terms` |
| 412 | 27,3500 | `TFF1/4` | 06512 | `ltf_terms` |
| 413 | 27,3501 | `PUSH DMP` | 41206 | `ltf_terms` |
| 414 | 27,3502 | `NRTERM` | 00021 | `ltf_terms` |
| 415 | 27,3503 | `PDDL SR*` | 53725 | `ltf_terms` |
| 417 | 27,3504 | `TFFNP` | 00035 | `ltf_terms` |
| 418 | 27,3505 | `0 -6,1` | 20573 | `ltf_terms` |
| 419 | 27,3506 | `DCOMP DAD` | 43276 | `ltf_terms` |
| 422 | 27,3507 | `SR*` | 77657 | `ltf_terms` |
| 423 | 27,3510 | `0,1` | 20601 | `ltf_terms` |
| 424 | 27,3511 | `BOFF DLOAD` | 71214 | `ltf_terms` |
| 425 | 27,3512 | `TFFSW` | 03756 | `ltf_terms` |
| 426 | 27,3513 | `+2` | 57515 | `ltf_terms` |
| 427 | 27,3514 | `TFFZEROS` | 06522 | `ltf_terms` |
| 428 | 27,3515 | `+2 BMN SQRT` | 75440 | `ltf_qterm` |
| 430 | 27,3516 | `MAXTFF1` | 57650 | `ltf_qterm` |
| 433 | 27,3517 | `DCOMP BOVB` | 41076 | `ltf_qterm` |
| 434 | 27,3520 | `TCDANZIG` | 57753 | `ltf_qterm` |
| 435 | 27,3521 | `STOVL TFFTEM` | 24045 | `ltf_qterm` |
| 436 | 27,3522 | `VONE'` | 02170 | `ltf_qterm` |
| 437 | 27,3523 | `DOT SL3` | 52441 | `ltf_qterm` |
| 438 | 27,3524 | `RONE` | 02207 | `ltf_qterm` |
| 439 | 27,3525 | `STORE TFFQ1` | 00017 | `ltf_qterm` |
| 441 | 27,3526 | `BMN BDSU` | 44240 | `ltf_xterm` |
| 442 | 27,3527 | `INBOUND` | 57550 | `ltf_xterm` |
| 443 | 27,3530 | `TFFTEM` | 00045 | `ltf_xterm` |
| 447 | 27,3531 | `STODL TFFX` | 14043 | `ltf_xterm` |
| 448 | 27,3532 | `TFFALFA` | 00033 | `ltf_xterm` |
| 449 | 27,3533 | `DMP BDSU` | 44205 | `ltf_xterm` |
| 451 | 27,3534 | `NRMAG` | 00041 | `ltf_xterm` |
| 453 | 27,3535 | `SAVEDEN PUSH ABS` | 51406 | `ltf_saveden` |
| 455 | 27,3536 | `DAD BOV` | 40015 | `ltf_saveden` |
| 456 | 27,3537 | `LIM(-22)` | 17757 | `ltf_saveden` |
| 457 | 27,3540 | `TFFXTEST` | 57561 | `ltf_saveden` |
| 458 | 27,3541 | `DLOAD PDDL` | 65345 | `ltf_fallback` |
| 459 | 27,3542 | `TFFZEROS` | 06522 | `ltf_fallback` |
| 461 | 27,3543 | `DLOAD DCOMP` | 57545 | `ltf_fallback` |
| 462 | 27,3544 | `TFFALFA` | 00033 | `ltf_fallback` |
| 463 | 27,3545 | `BMN DLOAD` | 71240 | `ltf_fallback` |
| 464 | 27,3546 | `TFFEL1` | 57655 | `ltf_fallback` |
| 467 | 27,3547 | `DUMPTFF1 RVQ` | 77616 | `ltf_fallback` |
| 471 | 27,3550 | `INBOUND DLOAD` | 77745 | `ltf_inbound` |
| 472 | 27,3551 | `DLOAD DSU` | 45345 | `ltf_inbound` |
| 473 | 27,3552 | `RTERM` | 00023 | `ltf_inbound` |
| 474 | 27,3553 | `RMAG1` | 00015 | `ltf_inbound` |
| 475 | 27,3554 | `STODL TFFX` | 14043 | `ltf_inbound` |
| 476 | 27,3555 | `TFFTEM` | 00045 | `ltf_inbound` |
| 477 | 27,3556 | `DAD GOTO` | 52015 | `ltf_inbound` |
| 478 | 27,3557 | `TFFQ1` | 00017 | `ltf_inbound` |
| 479 | 27,3560 | `SAVEDEN` | 57535 | `ltf_inbound` |
| 481 | 27,3561 | `TFFXTEST DAD PDDL` | 65215 | `ltf_xtest` |
| 483 | 27,3562 | `DP(-22)` | 17761 | `ltf_xtest` |
| 484 | 27,3563 | `TFFX` | 00043 | `ltf_xtest` |
| 485 | 27,3564 | `DMP SR*` | 53605 | `ltf_xtest` |
| 486 | 27,3565 | `TFFRTALF` | 00031 | `ltf_xtest` |
| 487 | 27,3566 | `0 -3,2` | 57201 | `ltf_xtest` |
| 488 | 27,3567 | `DDV` | 77671 | `ltf_xtest` |
| 492 | 27,3570 | `DLOAD BOV` | 40145 | `ltf_xtest` |
| 493 | 27,3571 | `TFFX` | 00043 | `ltf_xtest` |
| 494 | 27,3572 | `TFFELL` | 57653 | `ltf_xtest` |
| 498 | 27,3573 | `DDV STADR` | 45471 | `ltf_series` |
| 501 | 27,3574 | `STORE TFFTEM` | 77732 | `ltf_series` |
| 504 | 27,3575 | `PUSH DSQ` | 63406 | `ltf_series` |
| 505 | 27,3576 | `PUSH DMP` | 41206 | `ltf_series` |
| 506 | 27,3577 | `TFFNP` | 00035 | `ltf_series` |
| 507 | 27,3600 | `SL SIGN` | 75261 | `ltf_series` |
| 508 | 27,3601 | `5` | 20206 | `ltf_series` |
| 509 | 27,3602 | `TFFTEM` | 00045 | `ltf_series` |
| 510 | 27,3603 | `STODL TFFTEM` | 14045 | `ltf_series` |
| 513 | 27,3604 | `PUSH DMP` | 41206 | `ltf_series` |
| 514 | 27,3605 | `TFFALFA` | 00033 | `ltf_series` |
| 515 | 27,3606 | `SL*` | 77657 | `ltf_series` |
| 516 | 27,3607 | `0,1` | 20201 | `ltf_series` |
| 517 | 27,3610 | `STORE TFFX` | 00043 | `ltf_series` |
| 518 | 27,3611 | `RTB DMP` | 41234 | `ltf_poly` |
| 519 | 27,3612 | `T(X)` | 57735 | `ltf_poly` |
| 521 | 27,3613 | `SR2 BDSU` | 44302 | `ltf_poly` |
| 522 | 27,3614 | `RTERM` | 00023 | `ltf_poly` |
| 523 | 27,3615 | `DAD DMP` | 41215 | `ltf_poly` |
| 524 | 27,3616 | `RMAG1` | 00015 | `ltf_poly` |
| 526 | 27,3617 | `SR3 BPL` | 51042 | `ltf_poly` |
| 527 | 27,3620 | `ENDTFF` | 57641 | `ltf_poly` |
| 528 | 27,3621 | `PUSH SIGN` | 75206 | `ltf_poly` |
| 529 | 27,3622 | `TFFQ1` | 00017 | `ltf_poly` |
| 530 | 27,3623 | `BPL DLOAD` | 71244 | `ltf_poly` |
| 531 | 27,3624 | `NEGTFF` | 57645 | `ltf_poly` |
| 532 | 27,3625 | `TFF1/ALF` | 00027 | `ltf_poly` |
| 533 | 27,3626 | `DCOMP BPL` | 51076 | `ltf_poly` |
| 534 | 27,3627 | `NEGTFF` | 57645 | `ltf_poly` |
| 538 | 27,3630 | `DCOMP` | 77676 | `ltf_poly` |
| 539 | 27,3631 | `DMP DDV` | 56205 | `ltf_scale` |
| 540 | 27,3632 | `PI/16` | 17755 | `ltf_scale` |
| 541 | 27,3633 | `TFFRTALF` | 00031 | `ltf_scale` |
| 542 | 27,3634 | `SL* SL*` | 53657 | `ltf_scale` |
| 543 | 27,3635 | `0 -4,2` | 57602 | `ltf_scale` |
| 544 | 27,3636 | `0 -4,2` | 57602 | `ltf_scale` |
| 545 | 27,3637 | `SL* DAD` | 43257 | `ltf_scale` |
| 546 | 27,3640 | `0,2` | 57576 | `ltf_scale` |
| 548 | 27,3641 | `ENDTFF DMP BOV` | 40005 | `ltf_endtff` |
| 549 | 27,3642 | `TFF/RTMU` | 00037 | `ltf_endtff` |
| 550 | 27,3643 | `MAXTFF` | 57651 | `ltf_endtff` |
| 552 | 27,3644 | `DUMPTFF2 RVQ` | 77616 | `ltf_endtff` |
| 555 | 27,3645 | `NEGTFF DLOAD` | 77745 | `ltf_negtff` |
| 557 | 27,3646 | `GOTO` | 77650 | `ltf_negtff` |
| 558 | 27,3647 | `ENDTFF` | 57641 | `ltf_negtff` |
| 560 | 27,3650 | `MAXTFF1 DLOAD` | 77745 | `ltf_maxtff1` |
| 561 | 27,3651 | `MAXTFF DLOAD RVQ` | 43545 | `ltf_maxtff` |
| 562 | 27,3652 | `NEARONE` | 17771 | `ltf_maxtff` |
| 568 | 27,3653 | `TFFELL SL2` | 77712 | `ltf_tffell` |
| 569 | 27,3654 | `BDDV PUSH` | 41465 | `ltf_tffell` |
| 572 | 27,3655 | `TFFEL1 DLOAD DSU` | 45345 | `ltf_tffel1` |
| 573 | 27,3656 | `TFFTEM` | 00045 | `ltf_tffel1` |
| 574 | 27,3657 | `TFFQ1` | 00017 | `ltf_tffel1` |
| 575 | 27,3660 | `STODL TFFDELQ` | 14013 | `ltf_tffel1` |
| 577 | 27,3661 | `STADR` | 77626 | `ltf_tffel1` |
| 578 | 27,3662 | `STORE TFFTEM` | 77732 | `ltf_tffel1` |
| 579 | 27,3663 | `DMP SL*` | 53605 | `ltf_tffel1` |
| 580 | 27,3664 | `TFF1/ALF` | 00027 | `ltf_tffel1` |
| 581 | 27,3665 | `0,2` | 57576 | `ltf_tffel1` |
| 582 | 27,3666 | `PUSH DMP` | 41206 | `ltf_tffel1` |
| 583 | 27,3667 | `TFFTEM` | 00045 | `ltf_tffel1` |
| 584 | 27,3670 | `SL* BOVB` | 41057 | `ltf_tffel1` |
| 585 | 27,3671 | `0,2` | 57576 | `ltf_tffel1` |
| 586 | 27,3672 | `SIGNMPAC` | 21712 | `ltf_tffel1` |
| 587 | 27,3673 | `STORE TFFX` | 00043 | `ltf_tffel1` |
| 588 | 27,3674 | `RTB DMP` | 41234 | `ltf_ellipse` |
| 589 | 27,3675 | `T(X)` | 57735 | `ltf_ellipse` |
| 590 | 27,3676 | `TFFX` | 00043 | `ltf_ellipse` |
| 591 | 27,3677 | `SR3 DSU` | 45242 | `ltf_ellipse` |
| 592 | 27,3700 | `DP2(-3)` | 17763 | `ltf_ellipse` |
| 593 | 27,3701 | `DMP PUSH` | 41405 | `ltf_ellipse` |
| 597 | 27,3702 | `DLOAD DMP` | 41345 | `ltf_ellipse` |
| 598 | 27,3703 | `TFFTEM` | 00045 | `ltf_ellipse` |
| 599 | 27,3704 | `RMAG1` | 00015 | `ltf_ellipse` |
| 600 | 27,3705 | `SL2 DAD` | 43312 | `ltf_ellipse` |
| 601 | 27,3706 | `TFFQ1` | 00017 | `ltf_ellipse` |
| 602 | 27,3707 | `STODL TFFTEM` | 14045 | `ltf_ellipse` |
| 603 | 27,3710 | `TFFNP` | 00035 | `ltf_ellipse` |
| 604 | 27,3711 | `DMP SL*` | 53605 | `ltf_ellipse` |
| 606 | 27,3712 | `TFF1/ALF` | 00027 | `ltf_ellipse` |
| 607 | 27,3713 | `1,2` | 57575 | `ltf_ellipse` |
| 608 | 27,3714 | `SIGN SL*` | 53765 | `ltf_ellipse` |
| 609 | 27,3715 | `TFFTEM` | 00045 | `ltf_ellipse` |
| 610 | 27,3716 | `0,2` | 57576 | `ltf_ellipse` |
| 611 | 27,3717 | `STODL TFFTEM` | 14045 | `ltf_ellipse` |
| 613 | 27,3720 | `TFF1/ALF` | 00027 | `ltf_ellipse` |
| 614 | 27,3721 | `SQRT DMP` | 41366 | `ltf_ellipse` |
| 615 | 27,3722 | `PI/16` | 17755 | `ltf_ellipse` |
| 616 | 27,3723 | `DAD` | 77615 | `ltf_ellipse` |
| 619 | 27,3724 | `SL* DSU` | 45257 | `ltf_ellipse` |
| 620 | 27,3725 | `0 -1,2` | 57577 | `ltf_ellipse` |
| 621 | 27,3726 | `TFFDELQ` | 00013 | `ltf_ellipse` |
| 622 | 27,3727 | `DMP SL*` | 53605 | `ltf_ellipse` |
| 623 | 27,3730 | `TFF1/ALF` | 00027 | `ltf_ellipse` |
| 624 | 27,3731 | `0 -3,2` | 57601 | `ltf_ellipse` |
| 625 | 27,3732 | `SL* GOTO` | 52057 | `ltf_ellipse` |
| 626 | 27,3733 | `0 -4,2` | 57602 | `ltf_ellipse` |
| 627 | 27,3734 | `ENDTFF` | 57641 | `ltf_ellipse` |
| 667 | 27,3735 | `T(X) TC POLY` | 07222 | `rtb_tx` |
| 668 | 27,3736 | `DEC 4` | 00004 | `rtb_tx` |
| 669 | 27,3737 | `2DEC 3.333333333 E-1` | 12525 12525 | `rtb_tx` |
| 671 | 27,3741 | `2DEC* -1.999819135 E-1*` | 71463 57703 | `rtb_tx` |
| 673 | 27,3743 | `2DEC* 1.418148467 E-1*` | 04423 17645 | `rtb_tx` |
| 675 | 27,3745 | `2DEC* -1.01310997 E-1*` | 74604 43667 | `rtb_tx` |
| 677 | 27,3747 | `2DEC* 5.609004986 E-2*` | 01626 37256 | `rtb_tx` |
| 679 | 27,3751 | `2DEC* -1.536156925 E-2*` | 77404 52071 | `rtb_tx` |
| 681 | 27,3753 | `ENDT(X) TC DANZIG` | 06061 | `rtb_tx` |
| 701 | 27,3754 | `PI/16 2DEC 3.141592653 B-4` | 06220 37553 | `constants` |
| 703 | 27,3756 | `LIM(-22) 2OCT 3777737700` | 37777 37700 | `constants` |
| 705 | 27,3760 | `DP(-22) 2OCT 0000000100` | 00000 00100 | `constants` |
| 707 | 27,3762 | `DP2(-3) 2DEC 1 B-3` | 04000 00000 | `constants` |
| 709 | 27,3764 | `DP2(-4) 2DEC 1 B-4` | 02000 00000 | `constants` |
| 715 | 27,3766 | `R300K 2DEC 6464778 B-29` | 00305 11205 | `constants` |
| 717 | 27,3770 | `NEARONE 2DEC .999999999` | 37777 37777 | `constants` |

## Evidence

Hashes: `evidence/lm-time-free-fall/SHA256.json`. Bulk files are assets of the `evidence-lm-time-free-fall` release
(`python3 tools/evidence.py fetch lm-time-free-fall`).

- Source-to-native map and independent assembly comparison:
  `python3 tools/lm_time_free_fall_source.py` reassembles the unchanged file
  with the pinned yaYUL (rope equal to the certified fixed-data rope) and maps
  all 266 words to native literals in 27 functions.
- Oracle: `tools/lm_time_free_fall_reference.c` (GPL, host only) runs the
  unchanged Luminary099 in the pinned yaAGC hardware-DV variant over 65536
  sampled call chains (`tests/lm_time_free_fall_cases.c`): the SR30.1 chains
  TFFCONMU, +TFFRP/RA, +CALCTPER, +CALCTFF and TFFCONIC chains, Earth and Moon
  scaling, elliptic, near-parabolic, exactly parabolic, hyperbolic and
  axis-aligned apse orbits, raw edge words, both IRQ settings, SUPER, EBANK,
  all five VACs and OVFIND. `tools/lm_time_free_fall_differential.py` checks
  the native producer against every chain and pins the stream identity
  (`lm-time-free-fall-source-stream.json`, SHA256 `4ac7d7e5...54bfe2`,
  269,090,816 bytes; 64825 chains return, 711 reach the SQRT abort). Every
  native branch is reached except two unreachable `default` arms.
- Host and sanitizer results: `make test-lm-time-free-fall
  sanitize-lm-time-free-fall`. Strict and ASAN/UBSAN builds each pass 16
  frozen chains, 22 admission guards, 88 frozen phase steps and the full
  65536-chain stream (`lm-time-free-fall-checks.log`).
- Phase frontiers: `tools/freeze_lm_time_free_fall_phases.py` requires each of
  the 247 native frontiers of the 16 frozen chains to equal, in order, an
  original NEWOPS image of the oracle, then freezes 88 steps in four groups.
- C64 execution results: `make vice-lm-time-free-fall`. The whole routine is
  too big for one PRG beside two images, so four PRGs each link one phase
  group and keep both images in the RAM under KERNAL ROM (`$01=$35`). Their
  phase steps start from a debris-only base state, so the 6502 build carries
  no orbit sampling. In VICE (PAL, stock ROMs): group 1 (entry to RAPO) 24
  steps, group 2 (CALCTPER to TFFXTEST) 34 steps, group 3 (series to MAXTFF)
  22 steps, group 4 (ellipse) 8 steps, no failures
  (`lm-time-free-fall-results.json`).

## Limits

- The C64 check runs each phase separately; the whole chain runs on the host.
  Running it as one program needs overlay ABI 2 modules (the 25 phases are the
  module entry points).
- TFFCONIC is qualified from the EDOP `+0` entry only, since no Luminary099
  caller exists.
- Numerical validity of the conic formulas is the original's; no widened claim.
