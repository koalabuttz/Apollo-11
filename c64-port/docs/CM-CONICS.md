# Translate Comanche CONIC_SUBROUTINES

Lane G (CM flight). Milestone `cm-conics`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/CONIC_SUBROUTINES.agc` (`src/cm_conics.c`) is native C in full: The conic subroutines: KEPLER (KEPLERN, its iteration and PERIODCH), LAMBERT, TIMETHET, TIMERAD, APSIDES, GEOM, PARAM, GETX/DELTIME and the iterator, all interpretive with their basic helpers. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

449 routines, keyed by physical address in `cm_conics_basic` and `cm_conics_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 04,3637 PARAM | interpretive | entry PARAM (P37,P70.agc  PARAM) |
| 04,3642  | interpretive | CHANG2 resume |
| 04,3644  | interpretive | CHANG2 resume |
| 04,3650  | interpretive | CALL return |
| 04,3651  | interpretive | CHANG2 resume |
| 04,3654  | interpretive | CHANG2 resume |
| 04,3656  | interpretive | CHANG2 resume |
| 04,3660  | interpretive | CHANG2 resume |
| 04,3662  | interpretive | CHANG2 resume |
| 04,3665  | interpretive | CHANG2 resume |
| 04,3667  | interpretive | CHANG2 resume |
| 04,3671  | interpretive | CHANG2 resume |
| 04,3672  | interpretive | CHANG2 resume |
| 04,3675  | interpretive | CHANG2 resume |
| 04,3700  | interpretive | CHANG2 resume |
| 04,3702 GEOM | interpretive | CALL target |
| 04,3703  | interpretive | CHANG2 resume |
| 04,3705  | interpretive | CHANG2 resume |
| 04,3706  | interpretive | CHANG2 resume |
| 04,3707  | interpretive | CHANG2 resume |
| 04,3710  | interpretive | CHANG2 resume |
| 04,3712  | interpretive | CHANG2 resume |
| 04,3714  | interpretive | CHANG2 resume |
| 04,3716  | interpretive | CHANG2 resume |
| 04,3720  | interpretive | CHANG2 resume |
| 04,3724  | interpretive | CHANG2 resume |
| 04,3726 UNITNORM | interpretive | CHANG2 resume |
| 04,3730  | interpretive | CHANG2 resume |
| 04,3732 COLINEAR | interpretive | BOV target |
| 04,3734 HAVENORM | interpretive | switch branch target |
| 04,3736  | interpretive | CHANG2 resume |
| 12,2042 KEPLERN | interpretive | entry KEPLERN (ORBITAL_INTEGRATION.agc  KEPLERN) |
| 12,2045  | interpretive | BOV target |
| 12,2047  | interpretive | CHANG2 resume |
| 12,2051  | interpretive | CHANG2 resume |
| 12,2054  | interpretive | CHANG2 resume |
| 12,2056  | interpretive | CHANG2 resume |
| 12,2060  | interpretive | CHANG2 resume |
| 12,2062  | interpretive | CHANG2 resume |
| 12,2064  | interpretive | CHANG2 resume |
| 12,2066  | interpretive | CHANG2 resume |
| 12,2070  | interpretive | CHANG2 resume |
| 12,2072  | interpretive | CHANG2 resume |
| 12,2074  | interpretive | CHANG2 resume |
| 12,2075  | interpretive | CHANG2 resume |
| 12,2077  | interpretive | CHANG2 resume |
| 12,2101  | interpretive | CHANG2 resume |
| 12,2102  | interpretive | CHANG2 resume |
| 12,2105  | interpretive | CHANG2 resume |
| 12,2110  | interpretive | CHANG2 resume |
| 12,2112 1REV | interpretive | BPL target |
| 12,2114  | interpretive | CHANG2 resume |
| 12,2116 STOREMAX | interpretive | BOV target |
| 12,2117  | interpretive | CHANG2 resume |
| 12,2122  | interpretive | CHANG2 resume |
| 12,2124  | interpretive | CHANG2 resume |
| 12,2126  | interpretive | CHANG2 resume |
| 12,2131 PERIODCH | interpretive | CHANG2 resume |
| 12,2133  | interpretive | CHANG2 resume |
| 12,2136  | interpretive | CHANG2 resume |
| 12,2140  | interpretive | CHANG2 resume |
| 12,2141  | interpretive | CHANG2 resume |
| 12,2143 MODDONE | interpretive | BOV target |
| 12,2146  | interpretive | CHANG2 resume |
| 12,2147  | interpretive | CHANG2 resume |
| 12,2152  | interpretive | CHANG2 resume |
| 12,2154  | interpretive | CHANG2 resume |
| 12,2157 STORBNDS | interpretive | CHANG2 resume |
| 12,2162  | interpretive | CHANG2 resume |
| 12,2164  | interpretive | CHANG2 resume |
| 12,2166  | interpretive | CHANG2 resume |
| 12,2170 STOREMIN | interpretive | BPL target |
| 12,2172  | interpretive | CHANG2 resume |
| 12,2173 DXCOMP | interpretive | STCALL target |
| 12,2176  | interpretive | CHANG2 resume |
| 12,2177  | interpretive | CHANG2 resume |
| 12,2201 XDIFF | interpretive | CHANG2 resume |
| 12,2203  | interpretive | CHANG2 resume |
| 12,2204 KEPLOOP | interpretive | CHANG2 resume |
| 12,2206  | interpretive | CHANG2 resume |
| 12,2210  | interpretive | CHANG2 resume |
| 12,2213  | interpretive | CHANG2 resume |
| 12,2215  | interpretive | STCALL return |
| 12,2220  | interpretive | CHANG2 resume |
| 12,2221  | interpretive | CHANG2 resume |
| 12,2223  | interpretive | CHANG2 resume |
| 12,2226  | interpretive | CHANG2 resume |
| 12,2231  | interpretive | CHANG2 resume |
| 12,2234  | interpretive | CHANG2 resume |
| 12,2237  | interpretive | CHANG2 resume |
| 12,2241  | interpretive | CHANG2 resume |
| 12,2242  | interpretive | CHANG2 resume |
| 12,2245  | interpretive | CHANG2 resume |
| 12,2246  | interpretive | CHANG2 resume |
| 12,2250  | interpretive | CHANG2 resume |
| 12,2253  | interpretive | CHANG2 resume |
| 12,2256 NDXCHNGE | interpretive | BOV target |
| 12,2261  | interpretive | CHANG2 resume |
| 12,2264 POSDELX | interpretive | BPL target |
| 12,2266  | interpretive | CHANG2 resume |
| 12,2267  | interpretive | CHANG2 resume |
| 12,2271  | interpretive | CHANG2 resume |
| 12,2274  | interpretive | CHANG2 resume |
| 12,2276 NEWDELX | interpretive | GOTO target |
| 12,2277  | interpretive | CHANG2 resume |
| 12,2302  | interpretive | CHANG2 resume |
| 12,2304  | interpretive | CHANG2 resume |
| 12,2305 BRNCHCTR | interpretive | CHANG2 resume |
| 12,2306  | interpretive | pending DANZIG after RTB CHECKCTR |
| 12,2310  | interpretive | after RTB CHECKCTR |
| 12,2312 PDXCHNGE | interpretive | BOV target |
| 12,2315  | interpretive | CHANG2 resume |
| 12,2320 BADX | interpretive | BZE target |
| 12,2322  | interpretive | CHANG2 resume |
| 12,2324  | interpretive | CHANG2 resume |
| 12,2326 TIMEOVFL | interpretive | BOV target |
| 12,2331  | interpretive | CHANG2 resume |
| 12,2332 CMNTOVFL | interpretive | CHANG2 resume |
| 12,2334  | interpretive | CHANG2 resume |
| 12,2335  | interpretive | CHANG2 resume |
| 12,2340  | interpretive | CHANG2 resume |
| 12,2342  | interpretive | CHANG2 resume |
| 12,2344 NEGTOVFL | interpretive | BMN target |
| 12,2346 KEPCONVG | interpretive | BPL target |
| 12,2350  | interpretive | CHANG2 resume |
| 12,2353  | interpretive | CHANG2 resume |
| 12,2355  | interpretive | CHANG2 resume |
| 12,2357  | interpretive | CHANG2 resume |
| 12,2362  | interpretive | CHANG2 resume |
| 12,2365  | interpretive | CHANG2 resume |
| 12,2367  | interpretive | CHANG2 resume |
| 12,2371  | interpretive | CHANG2 resume |
| 12,2372  | interpretive | CHANG2 resume |
| 12,2373  | interpretive | CHANG2 resume |
| 12,2374  | interpretive | CHANG2 resume |
| 12,2376  | interpretive | CHANG2 resume |
| 12,2400  | interpretive | CHANG2 resume |
| 12,2403  | interpretive | CHANG2 resume |
| 12,2405  | interpretive | CHANG2 resume |
| 12,2410  | interpretive | CHANG2 resume |
| 12,2413  | interpretive | CHANG2 resume |
| 12,2415  | interpretive | CHANG2 resume |
| 12,2420  | interpretive | CHANG2 resume |
| 12,2423  | interpretive | CHANG2 resume |
| 12,2424  | interpretive | CHANG2 resume |
| 12,2427  | interpretive | CHANG2 resume |
| 12,2431  | interpretive | CHANG2 resume |
| 12,2433 DELTIME | interpretive | CALL target |
| 12,2434  | basic | after EXIT |
| 12,2462  | basic | return from POLY |
| 12,2463  | interpretive | after TC INTPRET |
| 12,2465  | interpretive | CHANG2 resume |
| 12,2466  | basic | after EXIT |
| 12,2514  | basic | return from POLY |
| 12,2515  | interpretive | after TC INTPRET |
| 12,2520  | interpretive | CHANG2 resume |
| 12,2521  | interpretive | CHANG2 resume |
| 12,2523  | interpretive | CHANG2 resume |
| 12,2524  | interpretive | pending DANZIG after RTB TPMODE |
| 12,2525  | interpretive | after RTB TPMODE |
| 12,2530  | interpretive | CHANG2 resume |
| 12,2532  | interpretive | CHANG2 resume |
| 12,2533  | interpretive | pending DANZIG after RTB TPMODE |
| 12,2535  | interpretive | after RTB TPMODE |
| 12,2537  | interpretive | CHANG2 resume |
| 12,2542  | interpretive | CHANG2 resume |
| 12,2544  | interpretive | CHANG2 resume |
| 12,2546  | interpretive | CHANG2 resume |
| 12,2547  | interpretive | CHANG2 resume |
| 12,2550 ITERATOR | interpretive | CALL target |
| 12,2554  | interpretive | CHANG2 resume |
| 12,2557  | interpretive | CHANG2 resume |
| 12,2562  | interpretive | CHANG2 resume |
| 12,2565  | interpretive | CHANG2 resume |
| 12,2567  | interpretive | CHANG2 resume |
| 12,2572  | interpretive | CHANG2 resume |
| 12,2574 SGNCHECK | interpretive | switch branch target |
| 12,2576  | interpretive | CHANG2 resume |
| 12,2602  | interpretive | CHANG2 resume |
| 12,2603 MINCHECK | interpretive | switch branch target |
| 12,2605  | interpretive | CHANG2 resume |
| 12,2610  | interpretive | CHANG2 resume |
| 12,2612 MODNGDEL | interpretive | BOV target |
| 12,2615  | interpretive | CHANG2 resume |
| 12,2620 FIRSTIME | interpretive | switch branch target |
| 12,2623  | interpretive | CHANG2 resume |
| 12,2626  | interpretive | CHANG2 resume |
| 12,2627  | interpretive | CHANG2 resume |
| 12,2632 POSDEL | interpretive | BPL target |
| 12,2636  | interpretive | CHANG2 resume |
| 12,2637 MAXCHECK | interpretive | switch branch target |
| 12,2641  | interpretive | CHANG2 resume |
| 12,2644 DELOK | interpretive | GOTO target |
| 12,2646 NEWDEL | interpretive | CHANG2 resume |
| 12,2647  | interpretive | CHANG2 resume |
| 12,2650 MODPSDEL | interpretive | BOV target |
| 12,2653  | interpretive | CHANG2 resume |
| 12,2656 CHECKCTR | basic | RTB target |
| 12,2665 NEWSTATE | interpretive | CALL target |
| 12,2667  | interpretive | CHANG2 resume |
| 12,2672  | interpretive | CHANG2 resume |
| 12,2674  | interpretive | CHANG2 resume |
| 12,2676  | interpretive | CHANG2 resume |
| 12,2701  | interpretive | CHANG2 resume |
| 12,2704  | interpretive | CHANG2 resume |
| 12,2706  | interpretive | CHANG2 resume |
| 12,2710  | interpretive | CHANG2 resume |
| 12,2711  | interpretive | CHANG2 resume |
| 12,2712  | interpretive | CHANG2 resume |
| 12,2713 LAMENTER | interpretive | CHANG2 resume |
| 12,2715  | interpretive | CHANG2 resume |
| 12,2717  | interpretive | CHANG2 resume |
| 12,2722  | interpretive | CHANG2 resume |
| 12,2724  | interpretive | CHANG2 resume |
| 12,2727  | interpretive | CHANG2 resume |
| 12,2732  | interpretive | CHANG2 resume |
| 12,2734  | interpretive | CHANG2 resume |
| 12,2737  | interpretive | CHANG2 resume |
| 12,2741  | interpretive | CHANG2 resume |
| 12,2743  | interpretive | CHANG2 resume |
| 12,2744 TIMETHET | interpretive | entry TIMETHET (P34-P35,_P74-P75.agc  TIMETHET) |
| 12,2747  | interpretive | CHANG2 resume |
| 12,2751  | interpretive | BOV target |
| 12,2754  | interpretive | CHANG2 resume |
| 12,2756  | interpretive | CALL return |
| 12,2761 COMMNOUT | interpretive | switch branch target |
| 12,2765  | interpretive | CHANG2 resume |
| 12,2770  | interpretive | CALL return |
| 12,2774  | interpretive | CALL return |
| 12,2776 COGAOVFL | interpretive | BOV target |
| 12,3001 GETX | interpretive | CALL target |
| 12,3005  | interpretive | CHANG2 resume |
| 12,3007  | interpretive | CHANG2 resume |
| 12,3011  | interpretive | CHANG2 resume |
| 12,3013  | interpretive | CHANG2 resume |
| 12,3016  | interpretive | CHANG2 resume |
| 12,3017  | interpretive | CHANG2 resume |
| 12,3021  | interpretive | CHANG2 resume |
| 12,3023  | interpretive | CHANG2 resume |
| 12,3025 WLOOP | interpretive | CHANG2 resume |
| 12,3026  | interpretive | CHANG2 resume |
| 12,3031  | interpretive | CHANG2 resume |
| 12,3032  | interpretive | CHANG2 resume |
| 12,3034  | interpretive | CHANG2 resume |
| 12,3035  | interpretive | CHANG2 resume |
| 12,3040  | interpretive | CHANG2 resume |
| 12,3043 POLYCOEF | interpretive | CHANG2 resume |
| 12,3045  | interpretive | CHANG2 resume |
| 12,3046  | interpretive | CHANG2 resume |
| 12,3051  | interpretive | CHANG2 resume |
| 12,3053  | basic | after EXIT |
| 12,3073  | basic | return from POLY |
| 12,3074  | interpretive | after TC INTPRET |
| 12,3075  | interpretive | CHANG2 resume |
| 12,3100 XCOMMON | interpretive | CHANG2 resume |
| 12,3102  | interpretive | CHANG2 resume |
| 12,3105  | interpretive | CHANG2 resume |
| 12,3107  | interpretive | CHANG2 resume |
| 12,3110  | interpretive | CHANG2 resume |
| 12,3111  | interpretive | CHANG2 resume |
| 12,3112  | interpretive | CHANG2 resume |
| 12,3113  | interpretive | CHANG2 resume |
| 12,3115  | interpretive | CHANG2 resume |
| 12,3120  | interpretive | CHANG2 resume |
| 12,3121  | interpretive | CHANG2 resume |
| 12,3123  | interpretive | CHANG2 resume |
| 12,3125  | interpretive | CHANG2 resume |
| 12,3130  | interpretive | CHANG2 resume |
| 12,3131  | interpretive | CHANG2 resume |
| 12,3132 RESETX2 | interpretive | BOV target |
| 12,3134 360CHECK | interpretive | BOV target |
| 12,3137  | interpretive | CHANG2 resume |
| 12,3141 INVRSEQN | interpretive | BPL target |
| 12,3143  | interpretive | CHANG2 resume |
| 12,3146  | interpretive | CHANG2 resume |
| 12,3150  | interpretive | CHANG2 resume |
| 12,3152  | interpretive | CHANG2 resume |
| 12,3153  | interpretive | CHANG2 resume |
| 12,3156  | interpretive | CHANG2 resume |
| 12,3160  | interpretive | CHANG2 resume |
| 12,3161  | interpretive | CHANG2 resume |
| 12,3163 1/WLOOP | interpretive | CHANG2 resume |
| 12,3164  | interpretive | CHANG2 resume |
| 12,3165  | interpretive | pending DANZIG after RTB TPMODE |
| 12,3167  | interpretive | after RTB TPMODE |
| 12,3171  | interpretive | CHANG2 resume |
| 12,3172  | interpretive | CHANG2 resume |
| 12,3174  | interpretive | CHANG2 resume |
| 12,3175  | interpretive | CHANG2 resume |
| 12,3200  | interpretive | CHANG2 resume |
| 12,3201  | interpretive | CHANG2 resume |
| 12,3204 TRUE360X | interpretive | switch branch target |
| 12,3207  | interpretive | CHANG2 resume |
| 12,3211  | interpretive | CHANG2 resume |
| 12,3214  | interpretive | CHANG2 resume |
| 12,3215  | interpretive | CHANG2 resume |
| 12,3217 INFINITY | interpretive | BMN target |
| 12,3222 OVFLCLR | interpretive | BOV target |
| 12,3224 LAMBERT | interpretive | entry LAMBERT (P34-P35,_P74-P75.agc  LAMBERT) |
| 12,3227  | interpretive | CHANG2 resume |
| 12,3231  | interpretive | BOV target |
| 12,3235  | interpretive | CHANG2 resume |
| 12,3237  | interpretive | CHANG2 resume |
| 12,3241  | interpretive | CHANG2 resume |
| 12,3242  | interpretive | CHANG2 resume |
| 12,3245  | interpretive | CHANG2 resume |
| 12,3250  | interpretive | CALL return |
| 12,3252  | interpretive | CHANG2 resume |
| 12,3255  | interpretive | CHANG2 resume |
| 12,3256  | interpretive | CHANG2 resume |
| 12,3260  | interpretive | CHANG2 resume |
| 12,3262  | interpretive | CHANG2 resume |
| 12,3264  | interpretive | CHANG2 resume |
| 12,3265  | interpretive | CHANG2 resume |
| 12,3267  | interpretive | CHANG2 resume |
| 12,3272  | interpretive | CHANG2 resume |
| 12,3273  | interpretive | CHANG2 resume |
| 12,3275  | interpretive | CHANG2 resume |
| 12,3300  | interpretive | CHANG2 resume |
| 12,3302  | interpretive | CHANG2 resume |
| 12,3304  | interpretive | CHANG2 resume |
| 12,3307  | interpretive | CHANG2 resume |
| 12,3312 UPLIM | interpretive | BOV target |
| 12,3314  | interpretive | CHANG2 resume |
| 12,3315 MAXCOGA | interpretive | BMN target |
| 12,3317  | interpretive | CHANG2 resume |
| 12,3321  | interpretive | CHANG2 resume |
| 12,3324  | interpretive | CHANG2 resume |
| 12,3327  | interpretive | CHANG2 resume |
| 12,3331  | interpretive | CHANG2 resume |
| 12,3333 MINCOGA | interpretive | CHANG2 resume |
| 12,3334  | interpretive | CHANG2 resume |
| 12,3341  | interpretive | CHANG2 resume |
| 12,3343 LAMBLOOP | interpretive | CHANG2 resume |
| 12,3345  | interpretive | CHANG2 resume |
| 12,3347  | interpretive | CHANG2 resume |
| 12,3352  | interpretive | CHANG2 resume |
| 12,3354  | interpretive | CHANG2 resume |
| 12,3357  | interpretive | CHANG2 resume |
| 12,3361  | interpretive | CHANG2 resume |
| 12,3363  | interpretive | CHANG2 resume |
| 12,3366  | interpretive | CHANG2 resume |
| 12,3371  | interpretive | CHANG2 resume |
| 12,3373  | interpretive | CHANG2 resume |
| 12,3376  | interpretive | CALL return |
| 12,3400  | interpretive | CHANG2 resume |
| 12,3402  | interpretive | CHANG2 resume |
| 12,3406  | interpretive | CALL return |
| 12,3411  | interpretive | CHANG2 resume |
| 12,3412  | interpretive | CHANG2 resume |
| 12,3414  | interpretive | CHANG2 resume |
| 12,3417  | interpretive | after RTB CHECKCTR |
| 12,3422  | interpretive | CALL return |
| 12,3425  | interpretive | CHANG2 resume |
| 12,3427  | interpretive | CHANG2 resume |
| 12,3431 NEGP | interpretive | BMN target |
| 12,3434 HIENERGY | interpretive | BOV target |
| 12,3437  | interpretive | CHANG2 resume |
| 12,3440 COMMONLM | interpretive | STCALL target |
| 12,3442  | interpretive | CHANG2 resume |
| 12,3443  | interpretive | CHANG2 resume |
| 12,3446  | interpretive | CHANG2 resume |
| 12,3450 BIGTIME | interpretive | BOV target |
| 12,3452  | interpretive | CHANG2 resume |
| 12,3453 LOENERGY | interpretive | CHANG2 resume |
| 12,3456  | interpretive | CHANG2 resume |
| 12,3460 SUFFCHEK | interpretive | BHIZ target |
| 12,3462  | interpretive | CHANG2 resume |
| 12,3465  | interpretive | CHANG2 resume |
| 12,3467  | interpretive | CHANG2 resume |
| 12,3473 360LAMB | interpretive | BZE target |
| 12,3477 NOGUESS | interpretive | switch branch target |
| 12,3503  | interpretive | CHANG2 resume |
| 12,3505  | interpretive | CHANG2 resume |
| 12,3506  | interpretive | CHANG2 resume |
| 12,3510  | interpretive | CHANG2 resume |
| 12,3512 LOLIM | interpretive | BMN target |
| 12,3515 INITV | interpretive | BPL target |
| 12,3520  | interpretive | CHANG2 resume |
| 12,3522  | interpretive | CHANG2 resume |
| 12,3523  | interpretive | CHANG2 resume |
| 12,3525  | interpretive | CHANG2 resume |
| 12,3527  | interpretive | CHANG2 resume |
| 12,3531  | interpretive | CHANG2 resume |
| 12,3534  | interpretive | CHANG2 resume |
| 12,3535  | interpretive | CHANG2 resume |
| 12,3537  | interpretive | CHANG2 resume |
| 12,3541  | interpretive | CHANG2 resume |
| 12,3543  | interpretive | CHANG2 resume |
| 12,3544  | interpretive | CHANG2 resume |
| 12,3547  | interpretive | CHANG2 resume |
| 12,3551 TARGETV | interpretive | BZE target |
| 12,3554  | interpretive | CALL return |
| 12,3556 TIMERAD | interpretive | entry TIMERAD (P37,P70.agc  TIMERAD) |
| 12,3561  | interpretive | CHANG2 resume |
| 12,3563  | interpretive | BOV target |
| 12,3566  | interpretive | CHANG2 resume |
| 12,3570  | interpretive | CALL return |
| 12,3573  | interpretive | CHANG2 resume |
| 12,3576  | interpretive | CHANG2 resume |
| 12,3600  | interpretive | CHANG2 resume |
| 12,3602  | interpretive | CHANG2 resume |
| 12,3605  | interpretive | CHANG2 resume |
| 12,3607  | interpretive | CHANG2 resume |
| 12,3610  | interpretive | CHANG2 resume |
| 12,3612  | interpretive | CHANG2 resume |
| 12,3615  | interpretive | CHANG2 resume |
| 12,3620  | interpretive | CHANG2 resume |
| 12,3622  | interpretive | CHANG2 resume |
| 12,3625  | interpretive | CHANG2 resume |
| 12,3626  | interpretive | CHANG2 resume |
| 12,3630  | interpretive | CHANG2 resume |
| 12,3633  | interpretive | CHANG2 resume |
| 12,3635  | interpretive | CHANG2 resume |
| 12,3637 TERMNVEC | interpretive | CHANG2 resume |
| 12,3641  | interpretive | CHANG2 resume |
| 12,3643  | interpretive | CHANG2 resume |
| 12,3645  | interpretive | CHANG2 resume |
| 12,3646  | interpretive | CHANG2 resume |
| 12,3651  | interpretive | CHANG2 resume |
| 12,3653  | interpretive | BOV target |
| 12,3655  | interpretive | CHANG2 resume |
| 12,3656  | interpretive | CHANG2 resume |
| 12,3660  | interpretive | CHANG2 resume |
| 12,3662  | interpretive | CHANG2 resume |
| 12,3664  | interpretive | CALL return |
| 12,3667 CIRCULAR | interpretive | BOV target |
| 12,3673 BADR2 | interpretive | BOV target |
| 12,3676  | interpretive | CHANG2 resume |
| 12,3700  | interpretive | CHANG2 resume |
| 12,3703 APSIDES | interpretive | entry APSIDES (P34-P35,_P74-P75.agc  APSIDES) |
| 12,3706  | interpretive | CHANG2 resume |
| 12,3710  | interpretive | BOV target |
| 12,3713  | interpretive | CHANG2 resume |
| 12,3715  | interpretive | CALL return |
| 12,3717 GETECC | interpretive | BOV target |
| 12,3721  | interpretive | CHANG2 resume |
| 12,3723  | interpretive | CHANG2 resume |
| 12,3724  | interpretive | CHANG2 resume |
| 12,3727  | interpretive | CHANG2 resume |
| 12,3731  | interpretive | CHANG2 resume |
| 12,3732  | interpretive | CHANG2 resume |
| 12,3735  | interpretive | CHANG2 resume |
| 12,3740  | interpretive | CHANG2 resume |
| 12,3741  | interpretive | CHANG2 resume |
| 12,3744  | interpretive | CHANG2 resume |
| 12,3746 INFINAPO | interpretive | BOV target |
| 12,3751 ABTCONIC | interpretive | switch branch target |
| 12,3752  | basic | after EXIT |

The RTB operand words whose pair has a right-hand order (12,2306 after RTB CHECKCTR, BHIZ; 12,2524 and 12,2533 in DELTIME and 12,3165, PDDL after RTB TPMODE) are routines keyed at DANZIG (`pending_danzig`): the RTB's TC DANZIG comes back with that order still in EDOP and LOC at the word, and the routine runs DANZIG's BANKSET load and the order, as the interpreter does. The program puts the mission's DANZIG marker first among its interpretive tables (`tests/cm_conics_target.c`). D1's host sweep on 84d550f5 met 12,2524 and 12,2533 in the CM entry flight.

Destinations outside the file: CHANG2, DANZIG, POLY, POODOO, RTB TPMODE. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Return addresses from outside the file (`QPRET`, an outside caller's Q) are never `DANZIG` nor a key the file registers, as in flight, where `QPRET` is the word after a `CALL` (#553). X1 -2 (Earth) or -10 (Moon), as the callers' AXT,1 set it for the MUTABLE,1 table reads. Mid-flow resume points hold MODE and PUSHLOC as the preceding orders leave them; in KEPLERN to PERIODCH, KEPLERN's own entry (12,2042) included, TAU.'s high word is +-0 (|TAU.| < 163.84 s; flight's time since rectification can be longer, a narrowing for the case length) and the period positive, so PERIODCH ends as in flight. Up to KEPLERN's DDV R1 (12,2077), R1 is an orbit's radius: its high word is one of 0310 (Earth parking orbit, 2^29 m), 0340 (lunar orbit, 2^27 m), 02000, 017571 (the Moon's sphere of influence) or 026706 (the Moon's distance), and C2 (KEPC2, RV.V/MU - 1) is a flight value from -1/2 to 3 where MPAC holds it (12,2074, 12,2075; at DDV R1, 12,2077, MPAC is the matching (1/64 - C2/64)/2) (#553, #664: a random tiny R1 or C2 near or past -1 makes ALPHA an orbit far below R1/2, and PERIODCH runs thousands of passes). On KEPLERN's hyperbolic arm (12,2105, 12,2110) ALPHA is negative, as BPL 1REV sends it there, and at BPL 1REV (12,2102) MPAC and ALPHA are the same value, as STORE ALPHA leaves them; from the SQRT to the period's DMP (12,2110-12,2122) XMAX in MPAC is positive (a +0 XMAX with a positive ALPHA gives a period of 0 and PERIODCH never ends; #664). PUSHLOC is vac+0 to vac+12 from NEWSTATE to TIMETHET, as COMMNOUT and DELTIME's loop leave it for NEWSTATE's pushes, and vac+2 at GETX's DDV (12,3016) in DP mode. EDOP is +0 from CHECKCTR to NEWSTATE: CHECKCTR's TC DANZIG is the RTB target of BPL RTB, and the BHIZ that follows RTB CHECKCTR is the key 12,2306 below. VTARGTAG is 0 in half the cases at LAMBERT's SLOAD BZE VTARGTAG (12,3544), as S40.9 steering's NUMIT 0 leaves it, so LAMBERT goes on to TARGETV (#659).

## Evidence

Hashes: `evidence/cm-conics/SHA256.json`; all files are in the tree.

- **Source map.** `cm-conics-source-map.json` lists all 1079 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-conics-differential` runs 256 states per routine (114,944 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-conics-proof.json` (`cm-conics-differential.log` is the run). One case in 32 is frozen in `tests/cm_conics_vectors.h`.
- **Host and sanitizer results.** `make test-cm-conics sanitize-cm-conics` runs the 3592 frozen cases (`cm-conics-host.log`).
- **C64 execution results.** `make cm-conics-vice` runs 29 diagnostic programs with 623 cases (CI: `vice-cm-conics-1` to `-3`, groups 1-10, 11-20 and 21-29, with 323, 171 and 129 cases) (`cm-conics-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 114,944 differential cases take: 4644 of 4644 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 12 of 12 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 2164 of the 2164 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
