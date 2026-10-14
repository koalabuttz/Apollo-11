# Mission scenarios

yaAGC core dumps (`agc_engine_init`'s format: the 512 input channels, the
2048 erasable words, then the CPU state) that interp-flight runs from
instead of a cold start (`tools/interp_flight_capture.c capture ... CORE`).

- `luminary099-p63-ignition.core`: Virtual AGC's
  `scenarios` branch, `Luminary099/Apollo 11 P63 Ignition Algorithm.core`
  (commit f572d789, 2016, from NASSP; line ends made LF). The LGC about ten
  minutes before PDI on Apollo 11 as flown in NASSP: after DOI, with the LM
  state vector and the landing site loaded. Its readme keys V37E63E (P63's
  ignition-time iteration ends at V06N61, TIG about 102:36:00 MET), PRO,
  then ENTR past V50N25 00014 (fine align), V50N18, V50N25 00500 (landing
  radar) and V50N25 00203, to V06N62 and the countdown; Average G and
  SERVICER start at TIG-35 s. `tools/interp_flight_test.py --p63` keys the
  same (V33E for PRO).
