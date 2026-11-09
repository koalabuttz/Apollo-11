#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The x86_64 flight program with the readable routines.

The faithful port's x86 program (c64-port/docs/X86-FLIGHT.md) links every
routine table of c64-port/src; this builds it with the readable objects
(readable/build/x86-<m>/) in place of the faithful ones that define the
same tables (REPLACED below), into readable/build/x86-flight/. The
scenario suite (c64-port/tools/x86_flight_scenarios.py) then flies that
program against the harness, which still runs the faithful port: every
row must end with the harness's erasable memory, word for word.

  x86_readable.py build [--host-cc CC]
  x86_readable.py scenarios [--quick] [--jobs N] [NAME ...]

Run by `make x86-flight`, `make x86-scenarios-quick` and `make
x86-scenarios` from readable/. Needs the port's toolchain
(c64-port/tools/setup_cloud.sh) for the harness's yaAGC.
"""
import os, subprocess, sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import x86_flight_build as xb  # noqa: E402

# The faithful objects whose routine tables the readable objects define.
REPLACED = (
    # The Executive and WAITLIST (src/executive, src/waitlist).
    'lm_kernel.o', 'cm_kernel.o',
    # INTER-BANK_COMMUNICATION (src/interbank). lm_interbank.o and
    # cm_interbank_complete.o stay: other faithful routines call them.
    'lm_interbank_routines.o', 'lm_interbank_idle.o', 'lm_interbank_dap.o',
    'cm_interbank_routines.o', 'cm_interbank_idle.o', 'cm_interbank_postjump.o',
    # PHASE_TABLE_MAINTENANCE (src/phase_table).
    'lm_phase_table.o', 'cm_phase_table.o',
    # RESTARTS_ROUTINE and RESTART_TABLES (src/restart). cm_restart_select.o
    # stays: the faithful restart path (cm_restart.o) calls it.
    'lm_restart_routines.o', 'lm_restart_select.o', 'cm_restart_routines.o',
    # INTERPRETER.agc (src/interpreter): the interpreter core and the
    # compiled basic routines. The faithful order bodies (lm_interp_orders.o,
    # lm_interp_steps.o and the CM's) stay: the compiled interpretive code
    # of other files calls them, as does the verification interpreter
    # (lm_interp_runtime.o), the holder of last resort.
    'lm_interp_core.o', 'cm_interp_core.o', 'lm_interpreter.o', 'cm_interpreter.o',
    # ALARM_AND_ABORT.agc (src/alarm). The CM's cm_alarm_record.o and the
    # other split files stay: they are the C64's, and nothing on x86 calls
    # them.
    'lm_alarm.o', 'cm_alarm.o',
    # SINGLE_PRECISION_SUBROUTINES.agc (src/single_precision): the routine
    # tables. lm_single_trig.o, cm_single_return.o and cm_quictrig.o stay:
    # other faithful routines call them.
    'sp_routines.o',
    # SERVICE_ROUTINES.agc (src/service): the routine tables.
    # service_routines.o stays: other faithful routines call it.
    'service_transfer.o',
    # DOWN-TELEMETRY_PROGRAM.agc (src/downtelemetry).
    'lm_downtelemetry.o', 'cm_downtelemetry.o',
    # FRESH_START_AND_RESTART.agc, bank 05 and GOTOPOOH (src/fresh_start).
    'lm_fresh_start.o', 'cm_fresh_start.o',
    # FRESH_START_AND_RESTART.agc, verb 37 and P00 (src/fresh_start/v37.c).
    'lm_v37.o', 'cm_v37.o',
    # FRESH_START_AND_RESTART.agc, STATEUP's interpretive code
    # (src/fresh_start/stateup.c, run by the readable interpreter).
    'lm_stateup.o', 'cm_stateup.o',
    # KEYRUPT_UPRUPT.agc (src/keyrupt): both missions' tables.
    'keyrupt_uprupt.o',
    # INTERRUPT_LEAD_INS.agc (src/lead_ins), WAITLIST.agc's RESUME exits
    # (src/waitlist) and the LM's T6-RUPT_PROGRAMS.agc (src/t6rupt). The
    # flight program takes DOWNRUPT's vector and RESUME from the
    # down-telemetry tables (lm_dntm_rupt.o, cm_dntm_rupt.o): the readable
    # binding defines those too.
    'rupt_lead_ins.o', 'lm_t6rupt.o', 'lm_dntm_rupt.o', 'cm_dntm_rupt.o',
    # T4RUPT_PROGRAM.agc: the display and dispatch core, the IMU monitor,
    # the LM's radar checks and the CM's optics (src/t4rupt).
    't4rupt_core.o', 'lm_t4rupt_imu.o', 'cm_t4rupt_imu.o', 'lm_t4rupt_rr.o', 'cm_t4rupt_optics.o',
    # RCS_FAILURE_MONITOR.agc, the LM's (src/rcs_failure_monitor).
    'lm_rcsmon.o',
    # AGS_INITIALIZATION.agc, the LM's R47 (src/ags_initialization).
    'lm_ags_init.o',
    # AOTMARK.agc, the LM's sighting marks (src/aotmark).
    'lm_aotmark.o',
    # SXTMARK.agc, the CM's sighting marks (src/sxtmark).
    'cm_sxtmark.o',
    # EXTENDED_VERBS.agc, part by part (src/extended_verbs/): each the
    # module the port's tables (lanec_ev_routines.o) call. Part 1, the
    # front end; part 2, the short verbs; part 3, alignment and radar
    # designate; part 4, the LM's radar test verbs; part 5, the DAP data
    # load; part 6, the clock update and ATTCK2; part 7, the tracking and
    # state vector verbs; part 8, the LM's interpretive words of those
    # verbs. The CM's file is all readable (part 9): its interpretive words
    # and the four fan words the parts' tables do not list take the place
    # of the port's compiled copy of the file, under its tables' names.
    'lanec_ev_front.o',
    'lanec_ev_short.o',
    'lanec_ev_align.o',
    'lanec_ev_radar.o',
    'lanec_ev_dap.o',
    'lanec_ev_time.o',
    'lanec_ev_track.o',
    'lanec_ev_lmi.o',
    'cm_extended_verbs_file.o',
    # PINBALL_NOUN_TABLES.agc: the readers PINBALL's faithful routines call
    # (apollo_noun_reader, src/pinball_noun_tables). fixed_data.o stays: the
    # faithful CM display code reads the faithful tables.
    'noun_reader.o',
    # LEM_GEOMETRY.agc and CSM_GEOMETRY.agc (src/geometry).
    'lm_geometry.o',
    'cm_geometry.o',
    # IMU_COMPENSATION_PACKAGE.agc, both missions' tables (src/imu_compensation).
    'imu_compensation.o',
    # R63.agc, the LM's V89 final attitude (src/r63).
    'lm_r63.o',
    # ATTITUDE_MANEUVER_ROUTINE.agc and the CM's ANGLFIND.agc, KALCMANU's start
    # (src/attitude_maneuver).
    'lm_attitude_maneuver.o',
    'cm_attitude_maneuver.o',
    # GIMBAL_LOCK_AVOIDANCE.agc, both missions' LOCSKIRT (src/gimbal_lock).
    'lm_gimbal_lock.o',
    'cm_gimbal_lock.o',
    # KALCMANU_STEERING.agc, both missions' maneuver steering (src/kalcmanu_steering).
    'lm_kalcmanu_steering.o',
    'cm_kalcmanu_steering.o',
    # SYSTEM_TEST_STANDARD_LEAD_INS.agc, both missions' lead-ins for erasable
    # programs, with the INTERPRETER entries they reach (src/standard_lead).
    'standard_lead_routines.o',
    # IMU_PERFORMANCE_TEST_2.agc and IMU_PERFORMANCE_TESTS_4.agc, the LM's
    # P07 (src/imu_performance).
    'lm_imu_performance.o',
    # R60_62.agc, both missions' maneuver request (R60), V49 (R62), the LM's
    # BALLANGS and VECPOINT (src/r60_62).
    'lm_r60_r62.o',
    'cm_r60_r62.o',
    # S-BAND_ANTENNA_FOR_LM.agc and S-BAND_ANTENNA_FOR_CM.agc, R05 (verb
    # 64; src/sband_antenna).
    'lm_sband_antenna.o',
    'cm_sband_antenna.o',
    # RADAR_LEADIN_ROUTINES.agc, the LM's radar sampling loop
    # (src/radar_leadin).
    'lm_radar_leadin_routines.o',
    # P30_P37.agc, the LM's P30 and S30.1 (src/p30): the port's compiled
    # file and its older module of the same keys, whose tables the readable
    # binding defines empty.
    'lm_p30_file.o',
    'lm_p30_routines.o',
    # P30-P37.agc, the CM's P30, P31, S30.1, S31.1 and DELRSPL (src/p30_cm).
    'cm_p30_p37.o',
    # GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc, the LM's P31 and REVN1645
    # (src/lambert_aimpoint): the port's compiled file and its older module
    # of the same keys, whose tables the readable binding defines empty.
    'lm_lambert_aim.o',
    'lm_p31_routines.o',
    # GROUND_TRACKING_DETERMINATION_PROGRAM.agc, both missions' P21
    # (src/ground_track): the port's compiled files and their older modules
    # of the same keys, whose tables the readable binding defines empty.
    'lm_ground_track.o',
    'lm_p21_routines.o',
    'cm_ground_track.o',
    'cm_p21_routines.o',
    # R31.agc, both missions' rendezvous parameter display (src/r31): the
    # port's compiled files and the LM's older module of the same keys,
    # whose tables the readable binding defines empty.
    'lm_r31_file.o',
    'lm_r31_routines.o',
    'cm_r31.o',
    # P76.agc, both missions' target delta V program (src/p76): the port's
    # compiled files and their older modules of the same keys, whose basic
    # tables the readable binding serves too (the flight program tries them
    # first) and whose order pair tables it defines empty.
    'lm_p76_file.o',
    'lm_p76_routines.o',
    'cm_p76_file.o',
    'cm_p76_routines.o',
    # R30.agc, both missions' orbital parameters display (src/r30): the
    # port's compiled files.
    'lm_r30.o',
    'cm_r30.o',
    # STABLE_ORBIT_-_P38-P39.agc, both missions' stable orbit programs
    # (src/stable_orbit): the port's compiled files.
    'lm_stable_orbit.o',
    'cm_stable_orbit.o',
    # BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, the LM's master
    # ignition routine (src/burn_baby): the port's compiled file.
    'lm_burn_baby.o',
    # THE_LUNAR_LANDING.agc, the LM's P63 set-up and P68
    # (src/lunar_landing): the port's compiled file.
    'lm_lunar_landing.o',
    # THROTTLE_CONTROL_ROUTINES.agc, the LM's throttle (src/throttle), and
    # FASTCHNG (src/landing_guidance): the port's compiled file.
    'lm_throttle.o',
    # P70-P71.agc, the LM's aborts (src/p70_p71): the port's compiled file
    # and its older module of the same routines, whose tables the readable
    # binding defines empty.
    'lm_p70_p71_file.o',
    'lm_p70_p71.o',
    # P12.agc, the LM's powered ascent (src/p12): the port's compiled file.
    'lm_p12.o',
    # ASCENT_GUIDANCE.agc, the LM's ascent guidance (src/ascent): the port's
    # compiled file.
    'lm_ascent.o',
    # LANDING_ANALOG_DISPLAYS.agc, the LM's landing analog displays
    # (src/landing_displays): the port's compiled file.
    'lm_landing_displays.o',
    # LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc (src/ephemerides): the
    # LM's routine table (its steps, lm_ephemeris.o and
    # lm_ephemeris_routines.o, are no longer called) and the CM's compiled
    # file.
    'lm_ephemeris_routine_table.o',
    'cm_ephemeris.o',
    # MYSUBS.agc, the CM DAP's SMALLMP and DPADD (src/cm_dap): the routine
    # table. cm_mysubs.o, which nothing else calls, is no longer called
    # (--gc-sections drops it).
    'cm_mysubs_routines.o',
)
# The faithful objects the readable objects take part of the place of:
# each is linked with the names the readable objects define renamed
# faithful_<name>, for the readable code to hand on to. PINBALL_GAME_BUTTONS_
# AND_LIGHTS, part by part (src/pinball): its trampoline, the mission's
# lanec_pin_run_<m>, which every routine of the port's Pinball tables calls,
# runs the readable entries and hands the rest to the faithful one.
# P20-P25 (src/p20_p25, src/p20_p25_cm): each mission's routine tables,
# all readable; the faithful ones, renamed, are no longer called
# (--gc-sections drops them).
# P32-P35_P72-P75 (src/p32_p35): the LM's routine tables, all readable;
# the faithful ones, renamed, are no longer called (--gc-sections drops
# them). P32-P33_P72-P73 (src/p32_p33_cm): the CM's routine tables, all
# readable; the faithful ones, renamed, are no longer called
# (--gc-sections drops them). P34-35_P74-75 (src/p34_p35): both missions'
# routine tables, all readable; the faithful ones, renamed, are no longer
# called (--gc-sections drops them). P40-P47 (src/p40_p47): the LM's
# routine tables, all readable; the faithful ones, renamed, are no longer
# called (--gc-sections drops them). P40-P47 of the CM (src/p40_p47_cm):
# the CM's routine tables, all readable; the faithful ones, renamed, are
# no longer called (--gc-sections drops them).
# LUNAR_LANDING_GUIDANCE_EQUATIONS (src/landing_guidance): the LM's
# routine tables, all readable; the faithful ones, renamed, are no longer
# called (--gc-sections drops them). SERVICER (src/servicer): the LM's
# routine tables, all readable; the faithful ones, renamed, are no longer
# called (--gc-sections drops them). FINDCDUW--GUIDAP_INTERFACE
# (src/findcduw): the LM's routine tables, all readable; the faithful ones,
# renamed, are no longer called (--gc-sections drops them). P51-P53
# (src/p51_p53): the LM's routine tables, all readable; the faithful ones,
# renamed, are no longer called (--gc-sections drops them). P51-P53 of the CM
# (src/p51_p53_cm): the CM's routine tables, all readable; the faithful
# ones, renamed, are no longer called (--gc-sections drops them). P61-P67
# (src/p61_p67): the CM's routine tables, all readable; the faithful ones,
# renamed, are no longer called (--gc-sections drops them). SERVICER207
# (src/servicer_cm): the CM's routine tables, all readable; the faithful
# ones, renamed, are no longer called (--gc-sections drops them).
# REENTRY_CONTROL (src/reentry): the CM's routine tables, all readable; the
# faithful ones, renamed, are no longer called (--gc-sections drops them).
# CM_BODY_ATTITUDE (src/body_attitude): the CM's routine tables, all
# readable; the faithful ones, renamed, are no longer called
# (--gc-sections drops them). P37_P70 (src/p37_p70): the CM's routine
# tables, all readable; the faithful ones, renamed, are no longer called
# (--gc-sections drops them). TVCINITIALIZE, TVCEXECUTIVE, TVCMASSPROP and
# TVCRESTARTS (src/tvc): the CM's routine tables, all readable; the
# faithful ones, renamed, are no longer called (--gc-sections drops them).
# RCS-CSM_DIGITAL_AUTOPILOT (src/cm_dap): the CM's routine tables (and the
# table of the keys one pass reaches), all readable; the faithful ones,
# renamed, are no longer called (--gc-sections drops them).
# AUTOMATIC_MANEUVERS (src/cm_dap): the CM's routine tables (and the table
# of the keys one pass reaches), all readable; the faithful ones, renamed,
# are no longer called (--gc-sections drops them).
# RCS-CSM_DAP_EXECUTIVE_PROGRAMS (src/cm_dap): the CM's routine tables, all
# readable; the faithful ones, renamed, are no longer called.
# JET_SELECTION_LOGIC (src/cm_dap): the CM's routine tables (and the table
# of the keys one pass reaches), all readable; the faithful ones, renamed,
# are no longer called.
# IMU_CALIBRATION_AND_ALIGNMENT (src/imu_calibration): the CM's routine
# tables, all readable; the faithful ones, renamed, are no longer called.
# P11 (src/p11): the CM's routine tables, all readable; the faithful ones,
# renamed, are no longer called.
# TPI_SEARCH (src/tpi_search): the CM's routine tables, all readable; the
# faithful ones, renamed, are no longer called.
# CM_ENTRY_DIGITAL_AUTOPILOT (src/entry_dap): the CM's routine tables (and
# the table of the 0.1 s cycle's keys), all readable; the faithful ones,
# renamed, are no longer called.
# LATITUDE_LONGITUDE_SUBROUTINES (src/lat_long): both missions' routine
# tables, all readable; the faithful ones, renamed, are no longer called.
# PLANETARY_INERTIAL_ORIENTATION (src/planetary): the same.
# MEASUREMENT_INCORPORATION (src/measurement_inc): the same.
# CONIC_SUBROUTINES (src/conics): both missions' routine tables, all
# readable; the faithful ones, renamed, are no longer called.
# INTEGRATION_INITIALIZATION (src/integration_init): both missions'
# routine tables, all readable; the faithful ones, renamed, are no longer
# called.
# ORBITAL_INTEGRATION (src/orbital_integration): both missions' routine
# tables, all readable; the faithful ones, renamed, are no longer called.
RENAMED = {'lanec_pin_trampoline.o': lambda m: [f'lanec_pin_run_{m}'],
           'lm_p20_p25.o': lambda m: ['lm_p20_p25_basic', 'lm_p20_p25_interpretive'] if m == 'lm' else [],
           'lm_p32_p35.o': lambda m: ['lm_p32_p35_basic', 'lm_p32_p35_interpretive'] if m == 'lm' else [],
           'cm_p20_p25.o': lambda m: ['cm_p20_p25_basic', 'cm_p20_p25_interpretive'] if m == 'cm' else [],
           'cm_p32_p33.o': lambda m: ['cm_p32_p33_basic', 'cm_p32_p33_interpretive'] if m == 'cm' else [],
           'lm_p34_p35.o': lambda m: ['lm_p34_p35_basic', 'lm_p34_p35_interpretive'] if m == 'lm' else [],
           'cm_p34_p35.o': lambda m: ['cm_p34_p35_basic', 'cm_p34_p35_interpretive'] if m == 'cm' else [],
           'lm_p40_p47.o': lambda m: ['lm_p40_p47_basic', 'lm_p40_p47_interpretive'] if m == 'lm' else [],
           'cm_p40_p47.o': lambda m: ['cm_p40_p47_basic', 'cm_p40_p47_interpretive'] if m == 'cm' else [],
           'lm_llge.o': lambda m: ['lm_llge_basic', 'lm_llge_interpretive'] if m == 'lm' else [],
           'lm_servicer.o': lambda m: ['lm_servicer_routines', 'lm_servicer_interpretive'] if m == 'lm' else [],
           'lm_findcduw.o': lambda m: ['lm_findcduw_basic', 'lm_findcduw_interpretive'] if m == 'lm' else [],
           'lm_p51_p53.o': lambda m: ['lm_p51_p53_basic', 'lm_p51_p53_interpretive'] if m == 'lm' else [],
           'cm_p51_p53.o': lambda m: ['cm_p51_p53_basic', 'cm_p51_p53_interpretive'] if m == 'cm' else [],
           'cm_p61_p67.o': lambda m: ['cm_p61_p67_basic', 'cm_p61_p67_interpretive'] if m == 'cm' else [],
           'cm_servicer.o': lambda m: ['cm_servicer_basic', 'cm_servicer_interpretive'] if m == 'cm' else [],
           'cm_reentry.o': lambda m: ['cm_reentry_basic', 'cm_reentry_interpretive'] if m == 'cm' else [],
           'cm_body_attitude.o': lambda m: ['cm_body_attitude_basic', 'cm_body_attitude_interpretive'] if m == 'cm' else [],
           'cm_p37_p70.o': lambda m: ['cm_p37_p70_basic', 'cm_p37_p70_interpretive'] if m == 'cm' else [],
           'cm_tvc_init.o': lambda m: ['cm_tvc_init_basic', 'cm_tvc_init_interpretive'] if m == 'cm' else [],
           'cm_tvc_exec.o': lambda m: ['cm_tvc_exec_basic', 'cm_tvc_exec_interpretive'] if m == 'cm' else [],
           'cm_tvc_massprop.o': lambda m: ['cm_tvc_massprop_basic', 'cm_tvc_massprop_interpretive'] if m == 'cm' else [],
           'cm_tvc_restarts.o': lambda m: ['cm_tvc_restarts_basic', 'cm_tvc_restarts_interpretive'] if m == 'cm' else [],
           'cm_tvc_daps.o': lambda m: ['cm_tvc_daps_basic', 'cm_tvc_daps_interpretive'] if m == 'cm' else [],
           'cm_tvc_stroke.o': lambda m: ['cm_tvc_stroke_basic', 'cm_tvc_stroke_interpretive'] if m == 'cm' else [],
           'cm_tvc_roll.o': lambda m: ['cm_tvc_roll_basic', 'cm_tvc_roll_interpretive'] if m == 'cm' else [],
           'cm_rcs_dap.o': lambda m: ['cm_rcs_dap_basic', 'cm_rcs_dap_interpretive',
                                      'cm_rcs_dap_hot_routines'] if m == 'cm' else [],
           'cm_auto_man.o': lambda m: ['cm_auto_man_basic', 'cm_auto_man_interpretive',
                                       'cm_auto_man_hot_routines'] if m == 'cm' else [],
           'cm_rcs_dap_exec.o': lambda m: ['cm_rcs_dap_exec_basic', 'cm_rcs_dap_exec_interpretive']
                                if m == 'cm' else [],
           'cm_jet_select.o': lambda m: ['cm_jet_select_basic', 'cm_jet_select_interpretive',
                                         'cm_jet_select_hot_routines'] if m == 'cm' else [],
           'cm_imu_calibration.o': lambda m: ['cm_imu_calibration_basic',
                                              'cm_imu_calibration_interpretive'] if m == 'cm' else [],
           'cm_p11.o': lambda m: ['cm_p11_basic', 'cm_p11_interpretive'] if m == 'cm' else [],
           'cm_tpi_search.o': lambda m: ['cm_tpi_search_basic', 'cm_tpi_search_interpretive']
                              if m == 'cm' else [],
           'cm_entry_dap.o': lambda m: ['cm_entry_dap_basic', 'cm_entry_dap_interpretive',
                                        'cm_entry_dap_hot_routines'] if m == 'cm' else [],
           'lm_lat_long.o': lambda m: ['lm_lat_long_basic', 'lm_lat_long_interpretive']
                            if m == 'lm' else [],
           'cm_lat_long.o': lambda m: ['cm_lat_long_basic', 'cm_lat_long_interpretive']
                            if m == 'cm' else [],
           'lm_planetary.o': lambda m: ['lm_planetary_basic', 'lm_planetary_interpretive']
                             if m == 'lm' else [],
           'cm_planetary.o': lambda m: ['cm_planetary_basic', 'cm_planetary_interpretive']
                             if m == 'cm' else [],
           'lm_measurement_inc.o': lambda m: ['lm_measurement_inc_basic',
                                              'lm_measurement_inc_interpretive']
                                   if m == 'lm' else [],
           'cm_measurement_inc.o': lambda m: ['cm_measurement_inc_basic',
                                              'cm_measurement_inc_interpretive']
                                   if m == 'cm' else [],
           'lm_conic_compiled.o': lambda m: ['lm_conic_compiled_basic',
                                             'lm_conic_compiled_interpretive']
                                  if m == 'lm' else [],
           'cm_conics.o': lambda m: ['cm_conics_basic', 'cm_conics_interpretive']
                          if m == 'cm' else [],
           'lm_integration_init.o': lambda m: ['lm_integration_init_basic',
                                               'lm_integration_init_interpretive']
                                    if m == 'lm' else [],
           'cm_integration_init.o': lambda m: ['cm_integration_init_basic',
                                               'cm_integration_init_interpretive']
                                    if m == 'cm' else [],
           'lm_orbital_integration.o': lambda m: ['lm_orbital_integration_basic',
                                                  'lm_orbital_integration_interpretive']
                                       if m == 'lm' else [],
           'cm_orbital_integration.o': lambda m: ['cm_orbital_integration_basic',
                                                  'cm_orbital_integration_interpretive']
                                       if m == 'cm' else []}
# A symbol each readable subsystem defines, which must be in the program.
READABLE_SYMBOLS = ('novac', 'bankcall', 'phschng2', 'getpart2', 'newops', 'whimper', 'spsin', 'gentran',
                    'dodowntm', 'slap1', 'v37xeq', 'interpret_listing', 'keyrupt1', 'go_lead_in',
                    't4jump', 'imumon', 'goextvb', 'showsum', 'imufinek', 'lodnntab', 'one_pipa',
                    'e_bkcall', 'charin', 'sbandant', 'p3xorp7x', 'r31call', 'endp76', 'v82call',
                    'vndsply_proceed')
# And each mission's own: R63 is the LM's alone; GIMBAL_LOCK_AVOIDANCE's
# table has each mission's name, and the CM's KALCMANU_STEERING and ANGLFIND
# have routines the LM's have not (STOPRATE, SECAD's basic code), as has
# its SYSTEM_TEST_STANDARD_LEAD_INS (COMPVER). The IMU performance test
# (GEOIMUTT) is the LM's alone. R60 has each mission's name (R60LEM,
# R60CSM). The radar sampling loop (RADSAMP) is the LM's alone, and each
# mission's P20 has labels the other's has not (the LM's PROG20A, the CM's
# PIKUP20). Each mission's P30 has labels the other's has not (the LM's
# P30N33, the CM's P30PHSI). Each mission's P32 has a P32STRT, the LM's in
# basic code (p32strt alone), the CM's in interpretive code too
# (P32STRT_CODE). The LM's P31 is GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc's
# (p31_after_p20flgon), the CM's its P30-P37.agc's. Both missions have
# P21 (prog21). Each mission's R31 has labels the other's has not (the
# LM's REV83, the CM's R34LOS). The LM's P76 starts with interpretive
# code (P76_CODE), the CM's does not. The CM's R30 updates the splash
# error in P00 and P11 (candel), the LM's does not. BURN, BABY, BURN is the
# LM's alone (killtsk2), as is the NSTEER of its P40-P47 and THE_LUNAR_LANDING
# (ddumcalc) and THROTTLE_CONTROL_ROUTINES (massmult) and P66's RODCOMP
# (rodcomp, LUNAR_LANDING_GUIDANCE_EQUATIONS); the CM's P40-P47
# starts at P40CSM (p40csm). LANDING_ANALOG_DISPLAYS is the LM's alone
# (landisp), as is FINDCDUW--GUIDAP_INTERFACE (fltrsub). Each mission's
# P52 is readable (prog52). Each mission's
# LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES is its own (EPHEMERIDES_CODE).
# The CM's SERVICER207 has the entry's half-second PIPA reading (quikread).
# The CM's REENTRY_CONTROL starts entry (startent_after_exit), and its
# CM_BODY_ATTITUDE updates the attitude angles (cm_atup). The CM's P37
# is its own (rtevn_after_exit), as is its TVC DAP (tvcdapon, its
# half-second task, tvcexec, its mass properties, massprop, its
# restarts, redotvc, its pitch and yaw DAPs, pitchdap, its stroke test,
# strktsti, and its roll DAP, rolldap), and its RCS DAP's MYSUBS (smallmp).
# The CM's IMU calibration and alignment starts at IMUTEST (imutest), and
# its P11 has the Saturn takeover (satstick).
MISSION_SYMBOLS = {'lm': ('v89recl', 'kalcman3', 'NOGIMLOC_CODE', 'newdelhi', 'geoimutt', 'r60lem',
                          'radsamp', 'prog20a', 'p30n33', 'p32strt',
                          'p31_after_p20flgon', 'prog21', 'rev83', 'P76_CODE', 'killtsk2',
                          'nsteer', 'ddumcalc', 'massmult', 'rodcomp', 'abrtjask', 'p12lm',
                          'atmag', 'rereadac', 'landisp', 'fltrsub', 'prog52', 'EPHEMERIDES_CODE'),
                   'cm': ('WCALC_CODE', 'newdelhi', 'stoprate', 'secad_after_exit', 'compver', 'r60csm',
                          'pikup20', 'p30phsi', 'P32STRT_CODE', 'prog21', 'r34los_after_exit', 'candel',
                          'rtrn', 'p40csm', 'EPHEMERIDES_CODE', 'prog52', 'p61', 'quikread',
                          'startent_after_exit', 'cm_atup', 'rtevn_after_exit', 'tvcdapon',
                          'tvcexec', 'massprop', 'redotvc', 'pitchdap', 'strktsti', 'rolldap',
                          'smallmp', 'imutest', 'satstick')}
xb.OUT = READABLE / 'build/x86-flight'


def readable_objects(m):
    objects = sorted((READABLE / f'build/x86-{m}').rglob('*.o'))
    if not objects:
        sys.exit(f'x86_readable: no readable objects in build/x86-{m} (run make from readable/)')
    return [str(o) for o in objects]


def renamed(path, m):
    """The faithful object at path, or its copy with RENAMED's names
    renamed for mission m."""
    if path.name not in RENAMED:
        return str(path)
    out = xb.OUT / f'renamed-{m}' / path.name
    out.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(['objcopy', *[f'--redefine-sym={n}=faithful_{n}' for n in RENAMED[path.name](m)],
                    str(path), str(out)], check=True)
    return str(out)


def build(cc, m, mode, certified_only=False):
    """x86_flight_build.build, linking the readable kernel for mission m."""
    define, suffix = xb.MODES[mode]
    suffix += '-certified' if certified_only else ''
    flags = xb.flags(m, define, certified_only)
    xb.OUT.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('flight_dispatch', 'flight_main'):
        o = xb.OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', f'platform/x86/{source}.c', '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    host = [renamed(p, m) for p in sorted((xb.STORE / 'host').glob('*.o')) if p.name not in REPLACED]
    exe = xb.OUT / f'x86-flight-{m}{suffix}'
    subprocess.run([cc, *objects, *host, *readable_objects(m), *xb.LINK, '-Wl,--gc-sections', '-lm',
                    '-o', str(exe)], cwd=PORT, check=True)
    # The readable routines must be the ones linked: theirs are present, and
    # the faithful kernel's instruction helpers (k_go and the rest) absent.
    symbols = subprocess.run(['nm', str(exe)], capture_output=True, text=True, check=True).stdout.split('\n')
    names = {line.split()[-1] for line in symbols if line.strip()}
    if not names.issuperset(READABLE_SYMBOLS + MISSION_SYMBOLS[m]) or 'k_go' in names:
        sys.exit(f'x86_readable: {exe.name} does not link the readable routines alone')
    return exe


xb.build = build


def main():
    args = sys.argv[1:]
    if not args or args[0] not in ('build', 'scenarios'):
        sys.exit(__doc__)
    os.chdir(PORT)
    if args[0] == 'build':
        cc = args[args.index('--host-cc') + 1] if '--host-cc' in args else os.environ.get('CC', 'cc')
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', cc],
                       cwd=PORT, check=True)
        for m in ('lm', 'cm'):
            for mode in ('compiled', 'core'):
                print(f'built {build(cc, m, mode)}')
        return 0
    import x86_flight_scenarios as xs
    xs.OUT = xb.OUT / 'scenarios'
    rest = args[1:]
    if '--jobs' not in rest and '-j' not in rest:
        rest = ['--jobs', str(os.cpu_count() or 1)] + rest
    sys.argv = ['x86_flight_scenarios.py'] + rest
    return xs.main()


if __name__ == '__main__':
    sys.exit(main())
