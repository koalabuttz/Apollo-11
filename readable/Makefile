# SPDX-License-Identifier: MIT
# The readable C (README.md). Run from readable/.
#
#   make check            the kernel cases, then the x86 quick scenarios
#   make kernel-cases     every case of the resident-kernel corpus on the
#                         readable Executive and WAITLIST (seconds)
#   make interbank-cases  the readable inter-bank routines against the
#                         faithful port's, case by case (seconds)
#   make phase-cases      the readable phase table maintenance against the
#                         original program's digests and the faithful port's
#                         routines, case by case (seconds)
#   make restart-cases    the readable restarts against the original program's
#                         digests and the faithful port's routines (seconds)
#   make machine-cases    the readable machine's instructions against the
#                         faithful port's on random states (seconds)
#   make interpreter-cases
#                         the readable interpreter against the faithful port's
#                         interpreter core on random interpretive programs
#                         (a minute; needs the assembled ropes, which
#                         c64-port/tools/setup_cloud.sh makes)
#   make alarm-cases      the readable alarms and aborts against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make sp-cases         the readable SPSIN and SPCOS against the original
#                         program's return states and the faithful port's
#                         routines (seconds)
#   make service-cases    the readable service routines against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make downtelemetry-cases
#                         the readable down-telemetry against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make keyrupt-cases    the readable keyboard and uplink interrupts against
#                         the original program's digests and the faithful
#                         port's routines (seconds)
#   make rupt-cases       the readable interrupt lead-ins, RESUME and the LM's
#                         T6 interrupt against the original program's digests
#                         and the faithful port's routines (seconds)
#   make t4rupt-cases     the readable T4RUPT routines against the original
#                         program's digests and the faithful port's routines
#                         (seconds)
#   make names-check      src/agc/<m>_names.inc against the listings
#   make coverage         how much of the program is readable C (COVERAGE)
#   make x86-flight       the x86_64 flight program with the readable
#                         Executive and WAITLIST linked in
#   make x86-scenarios-quick / x86-scenarios
#                         the x86 exact scenario suite on that program (needs
#                         the port's toolchain: c64-port/tools/setup_cloud.sh)
#
# The readable sources build with READABLE_CFLAGS; the faithful port's
# sources they are tested with build with the port's own flags.
PORT := ../c64-port
BUILD := build
CC ?= cc
READABLE_CFLAGS := -std=c99 -O2 -g -Wall -Wextra -Werror -pedantic
PORT_CFLAGS := -O2 -g -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic
INCLUDES := -I$(PORT)/include -I$(PORT)/src -I$(PORT)/tests
LM := -DLUMINARY099=1
CM := -DCOMANCHE055=1

KERNEL_SOURCES := src/agc/agc.c src/executive/executive.c src/waitlist/waitlist.c \
	src/binding/binding.c src/binding/machine.c
INTERBANK_SOURCES := src/agc/agc.c src/interbank/interbank.c src/binding/machine.c \
	src/binding/interbank_tables.c
PHASE_SOURCES := src/agc/agc.c src/phase_table/phase_table.c src/binding/machine.c \
	src/binding/phase_table_tables.c
RESTART_SOURCES := src/agc/agc.c src/restart/restarts.c src/restart/restart_tables.c \
	src/binding/machine.c src/binding/restart_routines.c
INTERPRETER_SOURCES := src/agc/agc.c $(sort $(wildcard src/interpreter/*.c)) src/interbank/interbank.c \
	src/binding/machine.c src/binding/interpreter_core.c src/binding/interpreter_basic.c
ALARM_SOURCES := src/agc/agc.c src/alarm/alarm.c src/interbank/interbank.c src/binding/machine.c \
	src/binding/interbank_tables.c src/binding/alarm_routines.c
SP_SOURCES := src/agc/agc.c src/single_precision/single_precision.c src/binding/machine.c \
	src/binding/sp_routines.c
SERVICE_SOURCES := src/agc/agc.c src/service/service.c $(sort $(wildcard src/interpreter/*.c)) \
	src/interbank/interbank.c src/binding/machine.c src/binding/service_routines.c
DNTM_SOURCES := src/agc/agc.c src/downtelemetry/downtelemetry.c src/downtelemetry/downlink_lists.c \
	src/binding/machine.c src/binding/downtelemetry_routines.c
FS_SOURCES := src/agc/agc.c src/fresh_start/fresh_start.c src/binding/machine.c \
	src/binding/fresh_start_routines.c
V37_SOURCES := $(FS_SOURCES) src/fresh_start/v37.c src/binding/v37_routines.c
STATEUP_SOURCES := $(INTERPRETER_SOURCES) src/fresh_start/stateup.c src/binding/stateup_routines.c
KEYRUPT_SOURCES := src/agc/agc.c src/keyrupt/keyrupt.c src/binding/machine.c src/binding/keyrupt_routines.c
T4RUPT_SOURCES := src/agc/agc.c src/t4rupt/t4rupt.c src/binding/machine.c src/binding/t4rupt_routines.c
T4IMU_SOURCES := src/agc/agc.c src/t4rupt/imu_monitor.c src/binding/machine.c \
	src/binding/t4rupt_imu_routines.c
T4RR_SOURCES := src/agc/agc.c src/t4rupt/rr_monitor.c src/binding/machine.c \
	src/binding/t4rupt_rr_routines.c
T4OPT_SOURCES := src/agc/agc.c src/t4rupt/optics.c src/binding/machine.c \
	src/binding/t4rupt_optics_routines.c
RCSMON_SOURCES := src/agc/agc.c src/rcs_failure_monitor/rcs_failure_monitor.c src/binding/machine.c \
	src/binding/rcs_failure_monitor_routines.c
AGS_SOURCES := src/agc/agc.c src/ags_initialization/ags_initialization.c src/binding/machine.c \
	$(INTERPRETER_SOURCES) src/binding/ags_initialization_routines.c
AOTMARK_SOURCES := $(INTERPRETER_SOURCES) src/aotmark/aotmark.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/aotmark_routines.c
SXTMARK_SOURCES := src/agc/agc.c src/sxtmark/sxtmark.c src/binding/machine.c \
	src/binding/sxtmark_routines.c
EV_SOURCES := $(INTERPRETER_SOURCES) src/extended_verbs/front.c src/extended_verbs/short_verbs.c \
	src/extended_verbs/align.c src/extended_verbs/radar.c src/extended_verbs/dap.c \
	src/extended_verbs/time.c src/extended_verbs/track.c \
	src/binding/extended_verbs_routines.c
NOUNS_SOURCES := src/agc/agc.c src/pinball_noun_tables/readers.c src/pinball_noun_tables/tables.c \
	src/binding/noun_reader_routines.c
GEOMETRY_SOURCES := $(INTERPRETER_SOURCES) src/geometry/geometry.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/geometry_routines.c
IMU_COMP_SOURCES := src/agc/agc.c src/imu_compensation/imu_compensation.c src/binding/machine.c \
	src/binding/imu_compensation_routines.c
R63_SOURCES := $(INTERPRETER_SOURCES) src/r63/r63.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/r63_routines.c
AMR_SOURCES := $(INTERPRETER_SOURCES) src/attitude_maneuver/attitude_maneuver.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/attitude_maneuver_routines.c
GLA_SOURCES := $(INTERPRETER_SOURCES) src/gimbal_lock/gimbal_lock.c \
	src/binding/gimbal_lock_routines.c
KAL_SOURCES := $(INTERPRETER_SOURCES) src/kalcmanu_steering/kalcmanu_steering.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/kalcmanu_steering_routines.c
R60_SOURCES := $(INTERPRETER_SOURCES) src/r60_62/r60_62.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/r60_62_routines.c
RLR_SOURCES := src/agc/agc.c src/binding/machine.c src/radar_leadin/radar_leadin.c \
	src/binding/radar_leadin_routines.c
SBA_SOURCES := $(INTERPRETER_SOURCES) src/sband_antenna/sband_antenna.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/sband_antenna_routines.c
P20_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p20_p25/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/p20_p25_routines.c
CP20_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p20_p25_cm/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/cm_p20_p25_routines.c
P30_SOURCES := $(INTERPRETER_SOURCES) src/p30/p30.c src/binding/p30_routines.c
CP30_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p30_cm/*.c)) \
	src/binding/cm_p30_routines.c
P32_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p32_p35/*.c)) \
	src/binding/p32_routines.c
CP32_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p32_p33_cm/*.c)) \
	src/binding/cm_p32_routines.c
P31_SOURCES := $(INTERPRETER_SOURCES) src/lambert_aimpoint/lambert_aimpoint.c \
	src/binding/lambert_aimpoint_routines.c
P21_SOURCES := $(INTERPRETER_SOURCES) src/ground_track/ground_track.c \
	src/binding/ground_track_routines.c
P34_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p34_p35/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c \
	src/binding/p34_routines.c
P40_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p40_p47/*.c)) \
	src/binding/p40_routines.c
CP40_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p40_p47_cm/*.c)) \
	src/binding/cm_p40_routines.c
R31_SOURCES := $(INTERPRETER_SOURCES) src/r31/r31.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/r31_routines.c
P76_SOURCES := $(INTERPRETER_SOURCES) src/p76/p76.c src/binding/p76_routines.c
R30_SOURCES := $(INTERPRETER_SOURCES) src/r30/r30.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/r30_routines.c
SO_SOURCES := $(INTERPRETER_SOURCES) src/stable_orbit/stable_orbit.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/stable_orbit_routines.c
BB_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/burn_baby/*.c)) \
	src/binding/burn_baby_routines.c
LLD_SOURCES := $(INTERPRETER_SOURCES) src/lunar_landing/lunar_landing.c \
	src/binding/lunar_landing_routines.c
THR_SOURCES := $(INTERPRETER_SOURCES) src/throttle/throttle.c src/landing_guidance/subroutines.c \
	src/binding/throttle_routines.c
LLGE_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/landing_guidance/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/llge_routines.c
P70_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p70_p71/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/p70_routines.c
P12_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p12/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/p12_routines.c
ASC_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/ascent/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/ascent_routines.c
SVC_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/servicer/*.c)) \
	src/binding/servicer_routines.c
FCW_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/findcduw/*.c)) \
	src/powered_flight/quictrig.c src/single_precision/single_precision.c \
	src/rtb_op_codes/rtb_op_codes.c src/binding/findcduw_routines.c
P51_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p51_p53/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/p51_routines.c
CP51_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p51_p53_cm/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/cm_p51_routines.c
EPH_SOURCES := $(INTERPRETER_SOURCES) src/ephemerides/ephemerides.c \
	src/binding/ephemerides_routines.c
CP61_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p61_p67/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/cm_p61_routines.c
CSVC_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/servicer_cm/*.c)) \
	src/binding/cm_servicer_routines.c
CRENT_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/reentry/*.c)) \
	src/binding/cm_reentry_routines.c
CBAT_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/body_attitude/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/cm_body_attitude_routines.c
CP37_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/p37_p70/*.c)) \
	src/rtb_op_codes/rtb_op_codes.c src/binding/cm_p37_p70_routines.c
CTVCI_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_init_routines.c
CTVCX_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_exec_routines.c
CTVCM_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_massprop_routines.c
CTVCR_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_restarts_routines.c
CTVCD_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_daps_routines.c
CTVCS_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_stroke_routines.c
CTVCL_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/tvc/*.c)) \
	src/binding/cm_tvc_roll_routines.c
CMYS_SOURCES := src/agc/agc.c src/binding/machine.c src/cm_dap/mysubs.c \
	src/binding/cm_mysubs_routines.c
CRCS_SOURCES := src/agc/agc.c src/binding/machine.c src/cm_dap/indexing.c \
	src/cm_dap/rcsatt.c src/cm_dap/needles.c src/cm_dap/initialize.c src/cm_dap/manual.c \
	src/binding/cm_rcs_dap_routines.c
CAM_SOURCES := src/agc/agc.c src/binding/machine.c src/cm_dap/indexing.c \
	src/cm_dap/attitude_hold.c src/cm_dap/switching.c src/cm_dap/jet_time.c \
	src/binding/cm_auto_man_routines.c
CRDX_SOURCES := src/agc/agc.c src/binding/machine.c src/cm_dap/dap_executive.c \
	src/binding/cm_rcs_dap_exec_routines.c
CJS_SOURCES := src/agc/agc.c src/binding/machine.c src/cm_dap/indexing.c src/cm_dap/jet_select.c \
	src/cm_dap/jet_times.c src/cm_dap/jet_words.c src/cm_dap/jet_sort.c src/cm_dap/jet_t6.c \
	src/binding/cm_jet_select_routines.c
LAD_SOURCES := src/agc/agc.c src/binding/machine.c src/landing_displays/landing_displays.c \
	src/binding/landing_displays_routines.c
IPT_SOURCES := $(INTERPRETER_SOURCES) src/imu_performance/imu_performance.c \
	src/binding/imu_performance_routines.c
CIC_SOURCES := $(INTERPRETER_SOURCES) src/rtb_op_codes/rtb_op_codes.c $(sort $(wildcard src/imu_calibration/*.c)) \
	src/binding/cm_imu_cal_routines.c
CP11_SOURCES := $(INTERPRETER_SOURCES) src/rtb_op_codes/rtb_op_codes.c $(sort $(wildcard src/p11/*.c)) \
	src/binding/cm_p11_routines.c
CTPI_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/tpi_search/*.c)) \
	src/binding/cm_tpi_search_routines.c
CED_SOURCES := src/agc/agc.c src/binding/machine.c $(sort $(wildcard src/entry_dap/*.c)) \
	src/binding/cm_entry_dap_routines.c
FFCP_SOURCES := src/agc/agc.c $(sort $(wildcard src/constant_pool/*.c))
LL_SOURCES := $(INTERPRETER_SOURCES) src/lat_long/lat_long.c src/binding/lat_long_routines.c
PL_SOURCES := $(INTERPRETER_SOURCES) src/planetary/planetary.c src/binding/planetary_routines.c
MINC_SOURCES := $(INTERPRETER_SOURCES) $(sort $(wildcard src/measurement_inc/*.c)) \
	src/integration_init/moves.c src/rtb_op_codes/rtb_op_codes.c \
	src/binding/measurement_inc_routines.c
SL_SOURCES := $(sort $(INTERPRETER_SOURCES) $(KERNEL_SOURCES) src/standard_lead/standard_lead.c \
	src/binding/standard_lead_routines.c)
PIN_SOURCES := src/agc/agc.c $(sort $(wildcard src/interpreter/*.c)) src/interbank/interbank.c \
	src/binding/machine.c $(sort $(wildcard src/pinball/*.c)) src/pinball_noun_tables/readers.c src/pinball_noun_tables/tables.c \
	src/binding/pinball_routines.c src/executive/executive.c
RUPT_SOURCES := src/agc/agc.c src/lead_ins/lead_ins.c src/waitlist/waitlist.c src/t6rupt/t6rupt.c \
	src/binding/machine.c src/binding/rupt_routines.c
SOURCES := $(sort $(KERNEL_SOURCES) $(INTERBANK_SOURCES) $(PHASE_SOURCES) $(RESTART_SOURCES) \
	$(INTERPRETER_SOURCES) $(ALARM_SOURCES) $(SP_SOURCES) $(SERVICE_SOURCES) $(DNTM_SOURCES) $(V37_SOURCES) $(STATEUP_SOURCES) \
	$(KEYRUPT_SOURCES) $(RUPT_SOURCES) $(T4RUPT_SOURCES) $(T4IMU_SOURCES) $(T4RR_SOURCES) $(T4OPT_SOURCES) \
	$(RCSMON_SOURCES) $(AGS_SOURCES) $(AOTMARK_SOURCES) $(SXTMARK_SOURCES) $(EV_SOURCES) \
	$(NOUNS_SOURCES) $(GEOMETRY_SOURCES) $(IMU_COMP_SOURCES) $(R63_SOURCES) $(AMR_SOURCES) $(GLA_SOURCES) $(KAL_SOURCES) $(R60_SOURCES) \
	$(SBA_SOURCES) $(RLR_SOURCES) $(P20_SOURCES) $(CP20_SOURCES) $(P30_SOURCES) $(CP30_SOURCES) $(P32_SOURCES) $(CP32_SOURCES) $(P31_SOURCES) $(P21_SOURCES) $(P34_SOURCES) $(P40_SOURCES) $(CP40_SOURCES) $(R31_SOURCES) $(P76_SOURCES) $(R30_SOURCES) $(SO_SOURCES) $(BB_SOURCES) $(LLD_SOURCES) $(THR_SOURCES) $(LLGE_SOURCES) $(P70_SOURCES) $(P12_SOURCES) $(ASC_SOURCES) $(SVC_SOURCES) $(LAD_SOURCES) $(FCW_SOURCES) $(P51_SOURCES) $(CP51_SOURCES) $(EPH_SOURCES) $(CP61_SOURCES) $(CSVC_SOURCES) $(CRENT_SOURCES) $(CBAT_SOURCES) $(CP37_SOURCES) $(CTVCI_SOURCES) $(CTVCX_SOURCES) $(CTVCM_SOURCES) $(CTVCR_SOURCES) $(CTVCD_SOURCES) $(CTVCS_SOURCES) $(CTVCL_SOURCES) $(CMYS_SOURCES) $(CRCS_SOURCES) $(CAM_SOURCES) $(CRDX_SOURCES) $(CJS_SOURCES) $(CIC_SOURCES) $(CP11_SOURCES) $(CTPI_SOURCES) $(CED_SOURCES) $(FFCP_SOURCES) $(LL_SOURCES) $(PL_SOURCES) $(MINC_SOURCES) $(SL_SOURCES) $(IPT_SOURCES) $(PIN_SOURCES))
HEADERS := $(wildcard src/*/*.h src/*/*.inc)

.PHONY: all check kernel-cases interbank-cases phase-cases restart-cases machine-cases interpreter-cases alarm-cases \
	sp-cases service-cases downtelemetry-cases fresh-start-cases v37-cases stateup-cases keyrupt-cases rupt-cases t4rupt-cases rcs-monitor-cases ags-cases aotmark-cases sxtmark-cases extended-verbs-cases pinball-noun-tables-cases geometry-cases imu-compensation-cases r63-cases attitude-maneuver-cases gimbal-lock-cases kalcmanu-steering-cases standard-lead-cases imu-performance-cases pinball-cases r60-r62-cases sband-antenna-cases radar-leadin-cases p20-p25-cases cm-p20-p25-cases p30-cases cm-p30-cases p32-cases cm-p32-cases lambert-aim-cases ground-track-cases p34-cases r31-cases p76-cases r30-cases stable-orbit-cases burn-baby-cases p40-cases cm-p40-cases lunar-landing-cases throttle-cases llge-cases p70-cases p12-cases ascent-cases servicer-cases landing-displays-cases findcduw-cases p51-p53-cases cm-p51-p53-cases ephemerides-cases cm-p61-p67-cases cm-servicer-cases cm-reentry-cases cm-body-attitude-cases cm-p37-p70-cases cm-tvc-init-cases cm-tvc-exec-cases cm-tvc-massprop-cases cm-tvc-restarts-cases cm-tvc-daps-cases cm-tvc-stroke-cases cm-tvc-roll-cases cm-mysubs-cases cm-rcs-dap-cases cm-auto-man-cases cm-rcs-dap-exec-cases cm-jet-select-cases cm-imu-cal-cases cm-p11-cases cm-tpi-search-cases cm-entry-dap-cases constant-pool-cases lat-long-cases planetary-cases measurement-inc-cases names-check \
	coverage x86-flight x86-scenarios-quick x86-scenarios format clean
all: names-check coverage kernel-cases interbank-cases phase-cases restart-cases machine-cases interpreter-cases \
	alarm-cases sp-cases service-cases downtelemetry-cases fresh-start-cases v37-cases stateup-cases \
	keyrupt-cases rupt-cases t4rupt-cases rcs-monitor-cases ags-cases aotmark-cases sxtmark-cases \
	extended-verbs-cases pinball-noun-tables-cases geometry-cases imu-compensation-cases r63-cases \
	attitude-maneuver-cases gimbal-lock-cases kalcmanu-steering-cases standard-lead-cases \
	imu-performance-cases pinball-cases r60-r62-cases sband-antenna-cases radar-leadin-cases \
	p20-p25-cases cm-p20-p25-cases p30-cases cm-p30-cases p32-cases cm-p32-cases lambert-aim-cases ground-track-cases p34-cases \
	r31-cases p76-cases r30-cases stable-orbit-cases burn-baby-cases p40-cases cm-p40-cases \
	lunar-landing-cases throttle-cases llge-cases p70-cases p12-cases ascent-cases servicer-cases landing-displays-cases findcduw-cases p51-p53-cases cm-p51-p53-cases ephemerides-cases cm-p61-p67-cases cm-servicer-cases cm-reentry-cases \
	cm-body-attitude-cases cm-p37-p70-cases cm-tvc-init-cases cm-tvc-exec-cases \
	cm-tvc-massprop-cases cm-tvc-restarts-cases cm-tvc-daps-cases cm-tvc-stroke-cases \
	cm-tvc-roll-cases cm-mysubs-cases cm-rcs-dap-cases cm-auto-man-cases cm-rcs-dap-exec-cases \
	cm-jet-select-cases cm-imu-cal-cases cm-p11-cases cm-tpi-search-cases cm-entry-dap-cases \
	constant-pool-cases lat-long-cases planetary-cases measurement-inc-cases
check: all x86-scenarios-quick

names-check:
	python3 tools/listing_names.py --check
	python3 tools/p20_p25_keys.py --check
	python3 tools/restart_tables.py --check

coverage:
	python3 tools/readable_coverage.py --check

# ---- The resident-kernel corpus ----
# c64-port's test (tests/test_resident_kernel.c) over every case of the six
# certified service milestones: each run must match the original program's
# yaAGC digest and the certified routines' final image word for word. Here
# the readable objects stand in for src/lm_kernel.c and src/cm_kernel.c.
RK_MK := $(PORT)/tools/mk/resident-kernel.mk
RK_LM_CASES := $(addprefix $(PORT)/,$(shell sed -n 's/^RK_LM_CASES = //p' $(RK_MK)))
RK_CM_CASES := $(addprefix $(PORT)/,$(shell sed -n 's/^RK_CM_CASES = //p' $(RK_MK)))
RK_COMMON := $(PORT)/src/agc_image_ops.c $(PORT)/tests/resident_kernel_cases.c $(PORT)/tests/test_resident_kernel.c
RK_LM_FLAGS := -DRK_LM=1 -DP66_NATIVE_DISPATCH_ONLY -DRK_SEAMS=1 -ffunction-sections -fdata-sections -Wl,--gc-sections
RK_CM_FLAGS := -DRK_CM=1 -DRK_SEAMS=1 -ffunction-sections -fdata-sections -Wl,--gc-sections

$(BUILD)/cases-lm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -c $< -o $@
$(BUILD)/cases-cm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(CM) -DRK_SEAMS=1 -c $< -o $@

$(BUILD)/kernel-cases-lm: $(KERNEL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(RK_LM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_LM_FLAGS) $(RK_LM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@
$(BUILD)/kernel-cases-cm: $(KERNEL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(RK_CM_CASES) $(RK_COMMON)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RK_CM_FLAGS) $(RK_CM_CASES) $(RK_COMMON) $(filter %.o,$^) -o $@

kernel-cases: $(BUILD)/kernel-cases-lm $(BUILD)/kernel-cases-cm
	./$(BUILD)/kernel-cases-lm
	./$(BUILD)/kernel-cases-cm

# ---- The inter-bank routines, case by case ----
# tests/interbank_cases.c runs each entry of the readable tables and of the
# faithful port's (built here with faithful_ before each table's name) on
# the same random states.
IB_LM_FAITHFUL := src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank_dap.c \
	src/lm_interbank.c
IB_CM_FAITHFUL := src/cm_interbank_routines.c src/cm_interbank_idle.c \
	src/cm_interbank_postjump.c src/cm_interbank_complete.c
IB_LM_NAMES := lm_interbank_routines lm_interbank_site_table lm_interbank_idle_chunk \
	lm_ib_bankcall lm_ib_swcall lm_ib_swcall_m1 lm_ib_swreturn lm_ib_postjump \
	lm_interbank_dap_hot_routines lm_interbank_postjump_hot_routines \
	lm_interbank_postjump_t4_hot_routines lm_phase_table_routines lm_phase_table_operands \
	lm_restart_routines lm_restart_operands lm_restart_select
IB_CM_NAMES := cm_interbank_routines cm_interbank_site_table cm_interbank_idle_chunk \
	cm_ib_bankcall cm_ib_swcall cm_ib_swcall_m1 cm_ib_swreturn cm_ib_postjump \
	cm_interbank_postjump_hot_routines cm_phase_table_routines cm_phase_table_operands \
	cm_restart_routines cm_restart_operands cm_restart_routines_select
IB_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c

$(BUILD)/faithful-lm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(IB_LM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/faithful-cm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(IB_CM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/interbank-cases-lm: $(INTERBANK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(IB_LM_FAITHFUL:src/%.c=$(BUILD)/faithful-lm/%.o) tests/interbank_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(IB_COMMON) $(filter %.o,$^) tests/interbank_cases.c -o $@
$(BUILD)/interbank-cases-cm: $(INTERBANK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(IB_CM_FAITHFUL:src/%.c=$(BUILD)/faithful-cm/%.o) tests/interbank_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(IB_COMMON) $(filter %.o,$^) tests/interbank_cases.c -o $@

interbank-cases: $(BUILD)/interbank-cases-lm $(BUILD)/interbank-cases-cm
	./$(BUILD)/interbank-cases-lm
	./$(BUILD)/interbank-cases-cm

# ---- Phase table maintenance ----
# The port's own test (tests/test_<m>_phase_table.c) on the readable
# routines: every caller site of the original program against its yaAGC
# digest. Then tests/phase_table_cases.c: the readable routines and the
# faithful port's on the same random states.
PT_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c
PT_FAITHFUL_LM := src/lm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c
PT_FAITHFUL_CM := src/cm_phase_table.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_multiword.c
PT_ORACLE_SOURCES := $(sort $(PHASE_SOURCES) $(INTERBANK_SOURCES))

$(BUILD)/phase-oracle-lm: $(PT_ORACLE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PT_COMMON) $(PORT)/tests/lm_phase_table_cases.c \
		$(PORT)/tests/test_lm_phase_table.c $(filter %.o,$^) -o $@
$(BUILD)/phase-oracle-cm: $(PT_ORACLE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PT_COMMON) $(PORT)/tests/cm_phase_table_cases.c \
		$(PORT)/tests/test_cm_phase_table.c $(filter %.o,$^) -o $@
$(BUILD)/phase-cases-lm: $(PHASE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(PT_FAITHFUL_LM:src/%.c=$(BUILD)/faithful-lm/%.o) tests/phase_table_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PT_COMMON) $(filter %.o,$^) tests/phase_table_cases.c -o $@
$(BUILD)/phase-cases-cm: $(PHASE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(PT_FAITHFUL_CM:src/%.c=$(BUILD)/faithful-cm/%.o) tests/phase_table_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PT_COMMON) $(filter %.o,$^) tests/phase_table_cases.c -o $@

phase-cases: $(BUILD)/phase-oracle-lm $(BUILD)/phase-oracle-cm $(BUILD)/phase-cases-lm \
		$(BUILD)/phase-cases-cm
	./$(BUILD)/phase-oracle-lm
	./$(BUILD)/phase-oracle-cm
	./$(BUILD)/phase-cases-lm
	./$(BUILD)/phase-cases-cm

# ---- Restarts ----
# The port's own tests (tests/test_lm_restart_routines.c, test_cm_restart_
# routines.c) on the readable routines: every restart table entry and
# variable phase against the original program's yaAGC digests. Then
# tests/restart_cases.c: the readable routines and the faithful port's on
# the same random states.
RS_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c
RS_FAITHFUL_LM := src/lm_restart_select.c src/lm_restart_routines.c
RS_FAITHFUL_CM := src/cm_restart_routines.c

$(BUILD)/restart-oracle-lm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RS_COMMON) $(PORT)/tests/lm_restart_select_cases.c \
		$(PORT)/tests/lm_restart_routines_cases.c $(PORT)/tests/test_lm_restart_routines.c \
		$(filter %.o,$^) -o $@
$(BUILD)/restart-oracle-cm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RS_COMMON) $(PORT)/tests/cm_restart_routines_cases.c \
		$(PORT)/tests/test_cm_restart_routines.c $(filter %.o,$^) -o $@
$(BUILD)/restart-cases-lm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(RS_FAITHFUL_LM:src/%.c=$(BUILD)/faithful-lm/%.o) tests/restart_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RS_COMMON) $(filter %.o,$^) tests/restart_cases.c -o $@
$(BUILD)/restart-cases-cm: $(RESTART_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(RS_FAITHFUL_CM:src/%.c=$(BUILD)/faithful-cm/%.o) tests/restart_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RS_COMMON) $(filter %.o,$^) tests/restart_cases.c -o $@

restart-cases: $(BUILD)/restart-oracle-lm $(BUILD)/restart-oracle-cm $(BUILD)/restart-cases-lm \
		$(BUILD)/restart-cases-cm
	./$(BUILD)/restart-oracle-lm
	./$(BUILD)/restart-oracle-cm
	./$(BUILD)/restart-cases-lm
	./$(BUILD)/restart-cases-cm

# ---- The machine's instructions ----
# tests/machine_cases.c: each instruction of the readable machine (src/agc/)
# and the faithful port's (src/agc_image_ops.c) on the same random states.
MC_FAITHFUL := $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/agc_word.c \
	$(PORT)/src/agc_multiword.c

$(BUILD)/machine-cases: $(BUILD)/cases-lm/agc/agc.o tests/machine_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(MC_FAITHFUL) $(filter %.o,$^) tests/machine_cases.c -o $@

machine-cases: $(BUILD)/machine-cases
	./$(BUILD)/machine-cases

# ---- The interpreter ----
# tests/interpreter_fuzz.c: the readable interpreter and the faithful port's
# interpreter core (src/<m>_interp_core.c, built with faithful_ before its
# names) on the same random interpretive programs, pass by pass, seeds 1
# to 40 per mission (under a second each). Its generator is the port's own
# fuzz's (tools/interp_core_fuzz.c).
INTERPRETER_CASES ?= 20000
INTERPRETER_SEEDS ?= $(shell seq 1 40)
ROPES := $(PORT)/build/fixed-data/Luminary099/MAIN.agc.bin $(PORT)/build/fixed-data/Comanche055/MAIN.agc.bin
IC_COMMON := $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c $(PORT)/src/agc_hardware_divide.c
IC_CFLAGS := -std=gnu99 -O2 -g -Wall -Wextra -Werror -Dexit=interp_core_fuzz_exit

$(BUILD)/faithful-%/interp_core.o: $(PORT)/src/%_interp_core.c
	@mkdir -p $(dir $@)
	$(CC) $(filter-out -Wconversion,$(PORT_CFLAGS)) $(INCLUDES) $(foreach n,interp_core interp_core_basic interp_core_mct,-D$*_$(n)=faithful_$*_$(n)) \
		-c $< -o $@

$(BUILD)/interpreter-cases-lm: $(INTERPRETER_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/faithful-lm/interp_core.o tests/interpreter_fuzz.c $(PORT)/tools/interp_core_fuzz.c
	$(CC) $(IC_CFLAGS) $(INCLUDES) $(LM) $(IC_COMMON) $(filter %.o,$^) tests/interpreter_fuzz.c -o $@
$(BUILD)/interpreter-cases-cm: $(INTERPRETER_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/faithful-cm/interp_core.o tests/interpreter_fuzz.c $(PORT)/tools/interp_core_fuzz.c
	$(CC) $(IC_CFLAGS) $(INCLUDES) $(CM) $(IC_COMMON) $(filter %.o,$^) tests/interpreter_fuzz.c -o $@

interpreter-cases: $(BUILD)/interpreter-cases-lm $(BUILD)/interpreter-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	@for m in lm cm; do for s in $(INTERPRETER_SEEDS); do \
		printf 'seed %s: ' $$s; ./$(BUILD)/interpreter-cases-$$m $(ROPES) $(INTERPRETER_CASES) $$s || exit 1; \
	done; done

# ---- Alarms and aborts ----
# The port's own tests (tests/test_lm_alarm.c, test_cm_alarm.c) on the
# readable routines: 4,096 cases per mission, every alarm site of the
# original program and every entry, against the original program's yaAGC
# digests, with the refusals they check. Then tests/alarm_cases.c: the
# readable routines and the faithful port's on the same random states.
AL_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c
AL_FAITHFUL := src/agc_image_ops.c src/agc_hardware_divide.c
AL_LM_NAMES := lm_alarm_routines lm_alarm_operand_table lm_interbank_routines lm_interbank_idle_chunk \
	lm_interbank_dap_hot_routines lm_interbank_postjump_hot_routines lm_interbank_postjump_t4_hot_routines \
	lm_interbank_site_table lm_ib_bankcall lm_ib_swcall lm_ib_swcall_m1 lm_ib_swreturn lm_ib_postjump
AL_CM_NAMES := cm_alarm_routines cm_alarm_operand_table

$(BUILD)/alarm-faithful-lm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AL_LM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/alarm-faithful-cm/%.o: $(PORT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AL_CM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/alarm-oracle-lm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AL_COMMON) $(PORT)/tests/lm_alarm_cases.c \
		$(PORT)/tests/test_lm_alarm.c $(filter %.o,$^) -o $@
$(BUILD)/alarm-oracle-cm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AL_COMMON) $(PORT)/tests/cm_alarm_cases.c \
		$(PORT)/tests/test_cm_alarm.c $(filter %.o,$^) -o $@
$(BUILD)/alarm-cases-lm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(patsubst src/%.c,$(BUILD)/alarm-faithful-lm/%.o,src/lm_alarm.c $(AL_FAITHFUL)) tests/alarm_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AL_COMMON) $(filter %.o,$^) tests/alarm_cases.c -o $@
$(BUILD)/alarm-cases-cm: $(ALARM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(patsubst src/%.c,$(BUILD)/alarm-faithful-cm/%.o,src/cm_alarm.c $(AL_FAITHFUL)) tests/alarm_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AL_COMMON) $(filter %.o,$^) tests/alarm_cases.c -o $@

alarm-cases: $(BUILD)/alarm-oracle-lm $(BUILD)/alarm-oracle-cm $(BUILD)/alarm-cases-lm \
		$(BUILD)/alarm-cases-cm
	./$(BUILD)/alarm-oracle-lm
	./$(BUILD)/alarm-oracle-cm
	./$(BUILD)/alarm-cases-lm
	./$(BUILD)/alarm-cases-cm

# ---- Single precision sine and cosine ----
# The port's own test (tests/test_sp_routines.c) on the readable SPSIN and
# SPCOS: every frozen yaAGC return state, entered at the original address.
# Then tests/sp_cases.c: readable and faithful on the same random states.
SP_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/tests/frozen_words.c
SP_ORACLE_LM := $(PORT)/src/lm_single_trig.c $(PORT)/tests/lm_single_trig_cases.c
SP_ORACLE_CM := $(PORT)/src/cm_single_return.c $(PORT)/src/cm_quictrig.c \
	$(PORT)/tests/cm_single_return_cases.c $(PORT)/tests/cm_trig_cases.c
SP_NAMES := lm_sp_routines cm_sp_routines cm_sp_hot_routines

$(BUILD)/sp-faithful-%.o: $(PORT)/src/sp_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DRK_$(shell echo $* | tr a-z A-Z)=1 \
		$(foreach n,$(SP_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@

$(BUILD)/sp-oracle-lm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DRK_LM=1 $(SP_COMMON) $(SP_ORACLE_LM) \
		$(PORT)/tests/test_sp_routines.c $(filter %.o,$^) -o $@
$(BUILD)/sp-oracle-cm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) -DRK_CM=1 $(SP_COMMON) $(SP_ORACLE_CM) \
		$(PORT)/tests/test_sp_routines.c $(filter %.o,$^) -o $@
$(BUILD)/sp-cases-lm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sp-faithful-lm.o tests/sp_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SP_COMMON) $(PORT)/src/lm_single_trig.c \
		$(filter %.o,$^) tests/sp_cases.c -o $@
$(BUILD)/sp-cases-cm: $(SP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sp-faithful-cm.o tests/sp_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SP_COMMON) $(PORT)/src/cm_single_return.c \
		$(PORT)/src/cm_quictrig.c $(filter %.o,$^) tests/sp_cases.c -o $@

sp-cases: $(BUILD)/sp-oracle-lm $(BUILD)/sp-oracle-cm $(BUILD)/sp-cases-lm $(BUILD)/sp-cases-cm
	./$(BUILD)/sp-oracle-lm
	./$(BUILD)/sp-oracle-cm
	./$(BUILD)/sp-cases-lm
	./$(BUILD)/sp-cases-cm

# ---- Service routines ----
# The port's own test (tests/test_service_transfer.c) on the readable
# routines: every UPFLAG, DOWNFLAG and GENTRAN site of both missions and
# every entry, against the original program's yaAGC digests, with the
# refusals it checks. It runs both missions in one program, so each
# mission's readable objects are linked into one object with only its
# table left global. Then tests/service_cases.c: the readable routines and
# the faithful port's on the same random states.
SV_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/service_routines.c
SV_ORACLE := $(PORT)/tests/service_transfer_cases.c $(PORT)/tests/service_transfer_check.c \
	$(PORT)/tests/test_service_transfer.c
SV_NAMES := lm_service_routines cm_service_routines

$(BUILD)/service-faithful.o: $(PORT)/src/service_transfer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SV_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/service-readable-lm.o: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_service_routines $@.all $@
$(BUILD)/service-readable-cm.o: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_service_routines $@.all $@

$(BUILD)/service-oracle: $(BUILD)/service-faithful.o $(BUILD)/service-readable-lm.o \
		$(BUILD)/service-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SV_COMMON) $(SV_ORACLE) $^ -o $@
$(BUILD)/service-cases-lm: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/service-faithful.o \
		tests/service_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SV_COMMON) $(filter %.o,$^) tests/service_cases.c -o $@
$(BUILD)/service-cases-cm: $(SERVICE_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/service-faithful.o \
		tests/service_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SV_COMMON) $(filter %.o,$^) tests/service_cases.c -o $@

service-cases: $(BUILD)/service-oracle $(BUILD)/service-cases-lm $(BUILD)/service-cases-cm
	./$(BUILD)/service-oracle
	./$(BUILD)/service-cases-lm
	./$(BUILD)/service-cases-cm

# ---- Down-telemetry ----
# The port's own tests (tests/test_lm_downtelemetry.c, test_cm_downtelemetry.c)
# on the readable routines: whole downlists and erasable dumps, DOWNRUPT by
# DOWNRUPT, against the original program's yaAGC digests, with every entry
# reached, every downlist word sent and the refusals they check. Then
# tests/downtelemetry_cases.c: the readable routines and the faithful
# port's, step by step on the same states.
DNTM_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c

$(BUILD)/dntm-faithful-%.o: $(PORT)/src/%_downtelemetry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_downtelemetry_routines=faithful_$*_downtelemetry_routines \
		-c $< -o $@
$(BUILD)/dntm-oracle-lm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(DNTM_COMMON) $(PORT)/tests/lm_downtelemetry_cases.c \
		$(PORT)/tests/test_lm_downtelemetry.c $^ -o $@
$(BUILD)/dntm-oracle-cm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(DNTM_COMMON) $(PORT)/tests/cm_downtelemetry_cases.c \
		$(PORT)/tests/test_cm_downtelemetry.c $^ -o $@
$(BUILD)/dntm-cases-lm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/dntm-faithful-lm.o \
		tests/downtelemetry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(DNTM_COMMON) $(filter %.o,$^) tests/downtelemetry_cases.c -o $@
$(BUILD)/dntm-cases-cm: $(DNTM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/dntm-faithful-cm.o \
		tests/downtelemetry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(DNTM_COMMON) $(filter %.o,$^) tests/downtelemetry_cases.c -o $@

downtelemetry-cases: $(BUILD)/dntm-oracle-lm $(BUILD)/dntm-oracle-cm $(BUILD)/dntm-cases-lm \
		$(BUILD)/dntm-cases-cm
	./$(BUILD)/dntm-oracle-lm
	./$(BUILD)/dntm-oracle-cm
	./$(BUILD)/dntm-cases-lm
	./$(BUILD)/dntm-cases-cm

# ---- Fresh start and restart ----
# The port's own tests (tests/test_lm_fresh_start.c, test_cm_fresh_start.c)
# on the readable routines: every entry from random states, against the
# original program's yaAGC digests, with the refusals they check. Then
# tests/fresh_start_cases.c: the readable routines and the faithful port's
# on the same random states.
FS_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c
FS_LM_INTERBANK := $(addprefix $(PORT)/src/,lm_interbank_routines.c lm_interbank_idle.c lm_interbank.c)
FS_CM_INTERBANK := $(addprefix $(PORT)/src/,cm_interbank_routines.c cm_interbank_idle.c cm_interbank_complete.c)

$(BUILD)/fs-faithful-%.o: $(PORT)/src/%_fresh_start.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_fresh_start_routines=faithful_$*_fresh_start_routines \
		-D$*_fresh_entries=faithful_$*_fresh_entries -c $< -o $@
$(BUILD)/fs-oracle-lm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_LM_INTERBANK) $(PORT)/tests/lm_fresh_start_cases.c \
		$(PORT)/tests/test_lm_fresh_start.c $^ -o $@
$(BUILD)/fs-oracle-cm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_CM_INTERBANK) $(PORT)/tests/cm_fresh_start_cases.c \
		$(PORT)/tests/test_cm_fresh_start.c $^ -o $@
$(BUILD)/fs-cases-lm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/fs-faithful-lm.o \
		tests/fresh_start_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(FS_COMMON) $(FS_LM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/lm_fresh_start_cases.c tests/fresh_start_cases.c -o $@
$(BUILD)/fs-cases-cm: $(FS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/fs-faithful-cm.o \
		tests/fresh_start_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(FS_COMMON) $(FS_CM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/cm_fresh_start_cases.c tests/fresh_start_cases.c -o $@

fresh-start-cases: $(BUILD)/fs-oracle-lm $(BUILD)/fs-oracle-cm $(BUILD)/fs-cases-lm $(BUILD)/fs-cases-cm
	./$(BUILD)/fs-oracle-lm
	./$(BUILD)/fs-oracle-cm
	./$(BUILD)/fs-cases-lm
	./$(BUILD)/fs-cases-cm

# ---- Verb 37 and P00 ----
# The port's own tests (tests/test_lm_v37.c, test_cm_v37.c) on the readable
# routines, with the readable fresh start beside them: every entry from
# random states, against the original program's yaAGC digests. Then
# tests/v37_cases.c: the readable routines and the faithful port's on the
# same random states, and V37's tables against the listing's words.
$(BUILD)/v37-faithful-%.o: $(PORT)/src/%_v37.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_v37_routines=faithful_$*_v37_routines \
		-D$*_v37_entries=faithful_$*_v37_entries -c $< -o $@
$(BUILD)/v37-oracle-lm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_LM_INTERBANK) $(PORT)/tests/lm_v37_cases.c \
		$(PORT)/tests/test_lm_v37.c $^ -o $@
$(BUILD)/v37-oracle-cm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FS_COMMON) $(FS_CM_INTERBANK) $(PORT)/tests/cm_v37_cases.c \
		$(PORT)/tests/test_cm_v37.c $^ -o $@
$(BUILD)/v37-cases-lm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/v37-faithful-lm.o \
		tests/v37_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(FS_COMMON) $(FS_LM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/lm_v37_cases.c tests/v37_cases.c -o $@
$(BUILD)/v37-cases-cm: $(V37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/v37-faithful-cm.o \
		tests/v37_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(FS_COMMON) $(FS_CM_INTERBANK) $(filter %.o,$^) \
		$(PORT)/tests/cm_v37_cases.c tests/v37_cases.c -o $@

v37-cases: $(BUILD)/v37-oracle-lm $(BUILD)/v37-oracle-cm $(BUILD)/v37-cases-lm $(BUILD)/v37-cases-cm
	./$(BUILD)/v37-oracle-lm
	./$(BUILD)/v37-oracle-cm
	./$(BUILD)/v37-cases-lm
	./$(BUILD)/v37-cases-cm

# ---- STATEUP: interpretive code ----
# The port's own tests (tests/test_lm_stateup.c, test_cm_stateup.c) on the
# readable interpreter running the listing's words (src/fresh_start/
# stateup.c): every order pair from random states, against the original
# program's yaAGC digests. Then tests/stateup_cases.c: the words against
# the rope's, and the readable pairs and the faithful port's on the same
# random states.
STATEUP_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/rope_image_host.c
LM_STATEUP_FAITHFUL := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c
CM_STATEUP_FAITHFUL := $(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c
$(BUILD)/stateup-faithful-%.o: $(PORT)/src/%_stateup.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_stateup_interpretive=faithful_$*_stateup_interpretive \
		-D$*_stateup_entries=faithful_$*_stateup_entries -c $< -o $@
# The port's test with its main() renamed: tests/with_ropes.c loads the
# ropes the readable interpreter reads, then runs it.
$(BUILD)/stateup-test-%.o: $(PORT)/tests/test_%_stateup.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/stateup-oracle-lm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/stateup-test-lm.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(STATEUP_COMMON) $(PORT)/tests/lm_stateup_cases.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/stateup-oracle-cm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/stateup-test-cm.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(STATEUP_COMMON) $(PORT)/tests/cm_stateup_cases.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/stateup-cases-lm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/stateup-faithful-lm.o tests/stateup_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(STATEUP_COMMON) $(LM_STATEUP_FAITHFUL) $(filter %.o,$^) \
		$(PORT)/tests/lm_stateup_cases.c tests/stateup_cases.c -o $@
$(BUILD)/stateup-cases-cm: $(STATEUP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/stateup-faithful-cm.o tests/stateup_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(STATEUP_COMMON) $(CM_STATEUP_FAITHFUL) $(filter %.o,$^) \
		$(PORT)/tests/cm_stateup_cases.c tests/stateup_cases.c -o $@

stateup-cases: $(BUILD)/stateup-oracle-lm $(BUILD)/stateup-oracle-cm $(BUILD)/stateup-cases-lm \
		$(BUILD)/stateup-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/stateup-oracle-lm $(ROPES)
	./$(BUILD)/stateup-oracle-cm $(ROPES)
	./$(BUILD)/stateup-cases-lm $(ROPES)
	./$(BUILD)/stateup-cases-cm $(ROPES)

# ---- The keyboard and uplink interrupts ----
# The port's own test (tests/test_keyrupt_uprupt.c) on the readable
# routines: 1,536 states per mission over the six entries, against the
# original program's yaAGC digests, with every uplink path reached and the
# refusals it checks. It runs both missions in one program, so each
# mission's readable objects are linked into one object with only its
# table left global. Then tests/keyrupt_cases.c: the readable routines and
# the faithful port's on the same random states.
KU_COMMON := $(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c
KU_NAMES := lm_keyrupt_routines cm_keyrupt_routines

$(BUILD)/keyrupt-faithful.o: $(PORT)/src/keyrupt_uprupt.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KU_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/keyrupt-readable-lm.o: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_keyrupt_routines $@.all $@
$(BUILD)/keyrupt-readable-cm.o: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_keyrupt_routines $@.all $@

$(BUILD)/keyrupt-oracle: $(BUILD)/keyrupt-readable-lm.o $(BUILD)/keyrupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/keyrupt_uprupt_cases.c \
		$(PORT)/tests/test_keyrupt_uprupt.c $^ -o $@
$(BUILD)/keyrupt-cases-lm: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/keyrupt-faithful.o \
		tests/keyrupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/keyrupt_cases.c -o $@
$(BUILD)/keyrupt-cases-cm: $(KEYRUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/keyrupt-faithful.o \
		tests/keyrupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/keyrupt_cases.c -o $@

keyrupt-cases: $(BUILD)/keyrupt-oracle $(BUILD)/keyrupt-cases-lm $(BUILD)/keyrupt-cases-cm
	./$(BUILD)/keyrupt-oracle
	./$(BUILD)/keyrupt-cases-lm
	./$(BUILD)/keyrupt-cases-cm

# ---- The interrupt lead-ins, RESUME and the LM's T6 interrupt ----
# The port's own tests on the readable routines: tests/test_rupt_lead_ins.c
# (both missions' lead-ins and the CM's RESUME entries, 128 states per
# entry) and tests/test_lm_t6rupt.c (the LM's T6RUPT lead-in, T6 routines
# and RESUME entries, every path), against the original program's yaAGC
# digests, with the refusals they check. Each mission's readable objects
# are linked into one object with only its tables left global. Then
# tests/rupt_cases.c: the readable routines and the faithful port's on the
# same random states.
RUPT_COMMON := $(KU_COMMON)
RUPT_LM_NAMES := lm_lead_in_routines lm_lead_in_hot_routines lm_t6rupt_routines \
	lm_t6rupt_hot_routines lm_t6rupt_jets_hot_routines lm_dntm_rupt_routines
RUPT_CM_NAMES := cm_lead_in_routines cm_dntm_rupt_routines
RUPT_NAMES := $(RUPT_LM_NAMES) $(RUPT_CM_NAMES)

RUPT_FAITHFUL := rupt_lead_ins lm_t6rupt lm_dntm_rupt cm_dntm_rupt
$(BUILD)/rupt-faithful.o: $(RUPT_FAITHFUL:%=$(PORT)/src/%.c)
	@mkdir -p $(dir $@)
	$(foreach f,$(RUPT_FAITHFUL),$(CC) $(PORT_CFLAGS) $(INCLUDES) \
		$(foreach n,$(RUPT_NAMES),-D$(n)=faithful_$(n)) -c $(PORT)/src/$(f).c -o $@.$(f) &&) true
	ld -r $(RUPT_FAITHFUL:%=$@.%) -o $@
$(BUILD)/rupt-readable-lm.o: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RUPT_LM_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/rupt-readable-cm.o: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RUPT_CM_NAMES),--keep-global-symbol=$(n)) $@.all $@

$(BUILD)/rupt-lead-ins-oracle: $(BUILD)/rupt-readable-lm.o $(BUILD)/rupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RUPT_COMMON) $(PORT)/tests/rupt_lead_ins_cases.c \
		$(PORT)/tests/test_rupt_lead_ins.c $^ -o $@
$(BUILD)/lm-t6rupt-oracle: $(BUILD)/rupt-readable-lm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RUPT_COMMON) $(PORT)/tests/lm_t6rupt_cases.c \
		$(PORT)/tests/test_lm_t6rupt.c $^ -o $@
$(BUILD)/rupt-cases-lm: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rupt-faithful.o \
		tests/rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(RUPT_COMMON) $(filter %.o,$^) tests/rupt_cases.c -o $@
$(BUILD)/rupt-cases-cm: $(RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/rupt-faithful.o \
		tests/rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(RUPT_COMMON) $(filter %.o,$^) tests/rupt_cases.c -o $@

rupt-cases: $(BUILD)/rupt-lead-ins-oracle $(BUILD)/lm-t6rupt-oracle $(BUILD)/rupt-cases-lm \
		$(BUILD)/rupt-cases-cm
	./$(BUILD)/rupt-lead-ins-oracle
	./$(BUILD)/lm-t6rupt-oracle
	./$(BUILD)/rupt-cases-lm
	./$(BUILD)/rupt-cases-cm

# ---- T4RUPT: the DSKY's relays and the once-a-second checks ----
# The port's own test (tests/test_t4rupt_core.c) on the readable core:
# 2,048 states at T4RUPT and 64 at each return entry per mission, every
# continuation and display path, against the original program's yaAGC
# digests, with the refusals it checks. Each mission's readable objects
# are linked into one object with only its tables left global. Then
# tests/t4rupt_cases.c: the readable routines and the faithful port's on
# the same random states.
T4RUPT_LM_NAMES := lm_t4rupt_core_routines lm_t4rupt_core_hot_routines
T4RUPT_NAMES := $(T4RUPT_LM_NAMES) cm_t4rupt_core_routines

$(BUILD)/t4rupt-faithful.o: $(PORT)/src/t4rupt_core.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(T4RUPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/t4rupt-readable-lm.o: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(T4RUPT_LM_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/t4rupt-readable-cm.o: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_t4rupt_core_routines $@.all $@

$(BUILD)/t4rupt-oracle: $(BUILD)/t4rupt-readable-lm.o $(BUILD)/t4rupt-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/t4rupt_core_cases.c \
		$(PORT)/tests/test_t4rupt_core.c $^ -o $@
$(BUILD)/t4rupt-cases-lm: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4rupt-faithful.o \
		tests/t4rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_cases.c -o $@
$(BUILD)/t4rupt-cases-cm: $(T4RUPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4rupt-faithful.o \
		tests/t4rupt_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_cases.c -o $@

# Part 2, the IMU monitor: the port's own tests (tests/test_lm_t4rupt_imu.c
# and test_cm_t4rupt_imu.c) on the readable routines, against the original
# program's yaAGC digests, with the refusals they check; then
# tests/t4rupt_imu_cases.c, the readable routines and the faithful port's
# on the same random states.
$(BUILD)/t4imu-faithful-%.o: $(PORT)/src/%_t4rupt_imu.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_t4rupt_imu_routines=faithful_$*_t4rupt_imu_routines -c $< -o $@
$(BUILD)/t4imu-readable-lm.o: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_t4rupt_imu_routines $@.all $@
$(BUILD)/t4imu-readable-cm.o: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_t4rupt_imu_routines $@.all $@
$(BUILD)/t4imu-oracle-%: $(BUILD)/t4imu-readable-%.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/$*_t4rupt_imu_cases.c \
		$(PORT)/tests/test_$*_t4rupt_imu.c $^ -o $@
$(BUILD)/t4imu-cases-lm: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4imu-faithful-lm.o \
		tests/t4rupt_imu_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_imu_cases.c -o $@
$(BUILD)/t4imu-cases-cm: $(T4IMU_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4imu-faithful-cm.o \
		tests/t4rupt_imu_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_imu_cases.c -o $@

# Part 3, the LM's radar checks and GPMATRIX: the port's test
# (tests/test_lm_t4rupt_rr.c) on the readable routines, then
# tests/t4rupt_rr_cases.c side by side.
$(BUILD)/t4rr-faithful.o: $(PORT)/src/lm_t4rupt_rr.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_t4rupt_rr_routines=faithful_lm_t4rupt_rr_routines -c $< -o $@
$(BUILD)/t4rr-readable.o: $(T4RR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_t4rupt_rr_routines $@.all $@
$(BUILD)/t4rr-oracle: $(BUILD)/t4rr-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KU_COMMON) $(PORT)/tests/lm_t4rupt_rr_cases.c \
		$(PORT)/tests/test_lm_t4rupt_rr.c $^ -o $@
$(BUILD)/t4rr-cases: $(T4RR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/t4rr-faithful.o \
		tests/t4rupt_rr_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_rr_cases.c -o $@

# Part 4, the CM's optics: the port's test (tests/test_cm_t4rupt_optics.c,
# its frozen cases run to their stops through the program of
# tests/cm_t4rupt_optics_target.c) on the readable routines, then
# tests/t4rupt_optics_cases.c side by side.
T4OPT_NAMES := cm_t4rupt_optics_basic cm_t4rupt_optics_interpretive
T4OPT_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) $(PORT)/tests/interp_case.c

$(BUILD)/t4opt-faithful.o: $(PORT)/src/cm_t4rupt_optics.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(T4OPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/t4opt-readable.o: $(T4OPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(T4OPT_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/t4opt-oracle: $(BUILD)/t4opt-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(T4OPT_COMMON) $(PORT)/tests/cm_t4rupt_optics_target.c \
		$(PORT)/tests/test_cm_t4rupt_optics.c $^ -o $@
$(BUILD)/t4opt-cases: $(T4OPT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/t4opt-faithful.o \
		tests/t4rupt_optics_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(filter %.o,$^) tests/t4rupt_optics_cases.c -o $@

t4rupt-cases: $(BUILD)/t4rupt-oracle $(BUILD)/t4rupt-cases-lm $(BUILD)/t4rupt-cases-cm \
		$(BUILD)/t4imu-oracle-lm $(BUILD)/t4imu-oracle-cm $(BUILD)/t4imu-cases-lm $(BUILD)/t4imu-cases-cm \
		$(BUILD)/t4rr-oracle $(BUILD)/t4rr-cases $(BUILD)/t4opt-oracle $(BUILD)/t4opt-cases
	./$(BUILD)/t4rupt-oracle
	./$(BUILD)/t4rupt-cases-lm
	./$(BUILD)/t4rupt-cases-cm
	./$(BUILD)/t4imu-oracle-lm
	./$(BUILD)/t4imu-oracle-cm
	./$(BUILD)/t4imu-cases-lm
	./$(BUILD)/t4imu-cases-cm
	./$(BUILD)/t4rr-oracle
	./$(BUILD)/t4rr-cases
	./$(BUILD)/t4opt-oracle
	./$(BUILD)/t4opt-cases

# RCS_FAILURE_MONITOR, the LM's: the port's test (tests/test_lm_rcsmon.c,
# its frozen cases run to their stops through the program of
# tests/lm_rcsmon_target.c) on the readable routines, then
# tests/rcs_failure_monitor_cases.c side by side.
RCSMON_NAMES := lm_rcsmon_basic lm_rcsmon_interpretive
RCSMON_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/tests/interp_case.c

$(BUILD)/rcsmon-faithful.o: $(PORT)/src/lm_rcsmon.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(RCSMON_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/rcsmon-readable.o: $(RCSMON_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(RCSMON_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/rcsmon-oracle: $(BUILD)/rcsmon-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(RCSMON_COMMON) $(PORT)/tests/lm_rcsmon_target.c \
		$(PORT)/tests/test_lm_rcsmon.c $^ -o $@
$(BUILD)/rcsmon-cases: $(RCSMON_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rcsmon-faithful.o \
		tests/rcs_failure_monitor_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KU_COMMON) $(filter %.o,$^) \
		tests/rcs_failure_monitor_cases.c -o $@

rcs-monitor-cases: $(BUILD)/rcsmon-oracle $(BUILD)/rcsmon-cases
	./$(BUILD)/rcsmon-oracle
	./$(BUILD)/rcsmon-cases

# AGS_INITIALIZATION, the LM's R47: tests/ags_initialization_oracle.c runs
# the port's frozen cases (c64-port/tests/lm_ags_init_vectors.h, yaAGC
# digests) and refusals on the readable routines; then
# tests/ags_initialization_cases.c runs readable and faithful side by side,
# as the host runs them and as the flight program does (stopping at the
# interpreter, -DAGC_FLIGHT_INTERPRETER).
AGS_NAMES := lm_ags_init_routines lm_ags_init_interpretive ags_routine_keys
AGS_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/lm_ags_init_cases.c
AGS_FLIGHT_OBJECTS := $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
AGS_FLIGHT_OBJECTS := $(filter-out %/ags_initialization_routines.o,$(AGS_FLIGHT_OBJECTS)) \
	$(BUILD)/ags-flight/ags_initialization_routines.o

$(BUILD)/ags-flight/ags_initialization_routines.o: src/binding/ags_initialization_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/ags-faithful.o: $(PORT)/src/lm_ags_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY \
		$(foreach n,$(AGS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ags-faithful-flight.o: $(PORT)/src/lm_ags_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(AGS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ags-oracle: $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) tests/ags_initialization_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_oracle.c -o $@
$(BUILD)/ags-cases: $(AGS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ags-faithful.o \
		tests/ags_initialization_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_cases.c -o $@
$(BUILD)/ags-cases-flight: $(AGS_FLIGHT_OBJECTS) $(BUILD)/ags-faithful-flight.o \
		tests/ags_initialization_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DAGC_FLIGHT_INTERPRETER $(AGS_COMMON) $(filter %.o,$^) \
		tests/ags_initialization_cases.c -o $@

ags-cases: $(BUILD)/ags-oracle $(BUILD)/ags-cases $(BUILD)/ags-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ags-oracle $(ROPES)
	./$(BUILD)/ags-cases $(ROPES)
	./$(BUILD)/ags-cases-flight $(ROPES)

# AOTMARK, the LM's sighting marks: the port's test (tests/test_lm_aotmark.c,
# its frozen cases run to their stops through the program of
# tests/lm_aotmark_target.c, against the original program's yaAGC digests)
# on the readable routines, with tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/aotmark_cases.c side by side.
AOTMARK_NAMES := lm_aotmark_basic lm_aotmark_interpretive
AOTMARK_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/aotmark-faithful.o: $(PORT)/src/lm_aotmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AOTMARK_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/aotmark-test.o: $(PORT)/tests/test_lm_aotmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/aotmark-oracle: $(AOTMARK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/aotmark-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AOTMARK_COMMON) $(PORT)/tests/lm_aotmark_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/aotmark-cases: $(AOTMARK_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/aotmark-faithful.o \
		tests/aotmark_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AOTMARK_COMMON) $(PORT)/src/rtb_angles.c \
		$(filter %.o,$^) tests/aotmark_cases.c -o $@

aotmark-cases: $(BUILD)/aotmark-oracle $(BUILD)/aotmark-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/aotmark-oracle $(ROPES)
	./$(BUILD)/aotmark-cases $(ROPES)

# SXTMARK, the CM's sextant marks: the port's test (tests/test_cm_sxtmark.c,
# its frozen cases run to their stops through the program of
# tests/cm_sxtmark_target.c, against the original program's yaAGC digests)
# on the readable routines; then tests/sxtmark_cases.c side by side.
SXTMARK_NAMES := cm_sxtmark_basic cm_sxtmark_interpretive

$(BUILD)/sxtmark-faithful.o: $(PORT)/src/cm_sxtmark.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SXTMARK_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sxtmark-readable.o: $(SXTMARK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(SXTMARK_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/sxtmark-oracle: $(BUILD)/sxtmark-readable.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(T4OPT_COMMON) $(PORT)/tests/cm_sxtmark_target.c \
		$(PORT)/tests/test_cm_sxtmark.c $^ -o $@
$(BUILD)/sxtmark-cases: $(SXTMARK_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sxtmark-faithful.o \
		tests/sxtmark_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KU_COMMON) $(PORT)/tests/interp_case.c \
		$(filter %.o,$^) tests/sxtmark_cases.c -o $@

sxtmark-cases: $(BUILD)/sxtmark-oracle $(BUILD)/sxtmark-cases
	./$(BUILD)/sxtmark-oracle
	./$(BUILD)/sxtmark-cases

# EXTENDED_VERBS, both missions, part by part. Each readable part takes
# the place of the faithful port's module for the part
# (c64-port/src/lanec_ev_<part>.c) under its name. The tests link both
# missions' readable objects, each built into one object with the
# module's name renamed readable_<m>_..., and tests/extended_verbs_missions.c
# puts them back under the module's name. First the port's frozen oracle
# rows (the original program's results in Virtual AGC) on the readable
# routines, then tests/extended_verbs_<part>_cases.c side by side with the
# faithful module (built with its dispatch renamed faithful_...; from the
# short verbs on, it also gives the port's suite its list of entries).
# (The readable parts link the readable interpreter, part 8's, which reads
# the rope: rope_image_host.c. Only parts 8 and 9 load the ropes.)
LANEC_COMMON := $(PORT)/src/agc_word.c $(PORT)/src/lanec.c $(PORT)/tests/lanec_case.c \
	$(PORT)/src/rope_image_host.c
EV_PARTS := front short align radar dap time track lmi
EV_NAMES := $(EV_PARTS:%=lanec_ev_%_dispatch)
# Part 8's tables, which its test checks against the rope: the LM's only
# (ATTACHIT_CODE and TCPIN_CODE are both missions' names, so the CM's stay
# local here; part 9's test links the CM's own).
EV_TABLES := RRLOSDSP_CODE TRMTRAK1_CODE ATTACHIT_CODE ATTACHIT_CODE_2 TCPIN_CODE

$(BUILD)/ev-%-faithful.o: $(PORT)/src/lanec_ev_%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlanec_ev_$*_dispatch=faithful_lanec_ev_$*_dispatch -c $< -o $@
$(BUILD)/ev-readable-lm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
$(BUILD)/ev-readable-cm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
$(BUILD)/ev-readable-%.o:
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(EV_NAMES) $(if $(filter lm,$*),$(EV_TABLES)),--keep-global-symbol=$(n)) \
		$@.all $@.kept
	objcopy $(foreach n,$(EV_NAMES),--redefine-sym $(n)=readable_$*_$(n:lanec_%=%)) $@.kept $@
EV_READABLE := $(BUILD)/ev-readable-lm.o $(BUILD)/ev-readable-cm.o tests/extended_verbs_missions.c
$(BUILD)/ev-front-oracle: $(EV_READABLE) tests/extended_verbs_front_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_front_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-front-cases: $(EV_READABLE) $(BUILD)/ev-front-faithful.o tests/extended_verbs_front_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_front_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-short-oracle: $(EV_READABLE) $(BUILD)/ev-short-faithful.o tests/extended_verbs_short_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_short_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-short-cases: $(EV_READABLE) $(BUILD)/ev-short-faithful.o tests/extended_verbs_short_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_short_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-align-oracle: $(EV_READABLE) $(BUILD)/ev-align-faithful.o tests/extended_verbs_align_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_align_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-align-cases: $(EV_READABLE) $(BUILD)/ev-align-faithful.o tests/extended_verbs_align_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_align_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-radar-oracle: $(EV_READABLE) $(BUILD)/ev-radar-faithful.o tests/extended_verbs_radar_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_radar_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-radar-cases: $(EV_READABLE) $(BUILD)/ev-radar-faithful.o tests/extended_verbs_radar_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_radar_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-dap-oracle: $(EV_READABLE) $(BUILD)/ev-dap-faithful.o tests/extended_verbs_dap_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_dap_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-dap-cases: $(EV_READABLE) $(BUILD)/ev-dap-faithful.o tests/extended_verbs_dap_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_dap_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-time-oracle: $(EV_READABLE) $(BUILD)/ev-time-faithful.o tests/extended_verbs_time_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_time_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-time-cases: $(EV_READABLE) $(BUILD)/ev-time-faithful.o tests/extended_verbs_time_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_time_cases.c \
		$(filter %.o %.c,$^) -o $@

$(BUILD)/ev-track-oracle: $(EV_READABLE) $(BUILD)/ev-track-faithful.o tests/extended_verbs_track_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_track_cases.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/ev-track-cases: $(EV_READABLE) $(BUILD)/ev-track-faithful.o tests/extended_verbs_track_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(PORT)/tests/lanec_ev_track_cases.c \
		$(filter %.o %.c,$^) -o $@

# Part 8's words run on the readable interpreter, which reads the
# program's constants from the rope: its tests load the ropes.
EV_INTERP_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c $(PORT)/src/agc_multiword.c
$(BUILD)/ev-lmi-oracle: $(EV_READABLE) $(BUILD)/ev-lmi-faithful.o tests/extended_verbs_lmi_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(EV_INTERP_COMMON) \
		$(PORT)/tests/lanec_ev_lmi_cases.c $(filter %.o %.c,$^) -o $@
$(BUILD)/ev-lmi-cases: $(EV_READABLE) $(BUILD)/ev-lmi-faithful.o tests/extended_verbs_lmi_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LANEC_COMMON) $(EV_INTERP_COMMON) \
		$(PORT)/tests/lanec_ev_lmi_cases.c $(filter %.o %.c,$^) -o $@

# Part 9, the CM's interpretive words, and with them the CM's file whole:
# the readable file's two tables (cm_extended_verbs_file_basic and
# _interpretive, the names of the port's compiled copy of the file) and
# the parts, as one program (tests/extended_verbs_file_program.c). First
# the port's frozen cases for the compiled file (the original program's
# results in Virtual AGC) on it, then tests/extended_verbs_file_cases.c
# side by side with the compiled file.
EV_FILE_NAMES := cm_extended_verbs_file_basic cm_extended_verbs_file_interpretive cm_interpreter_basic \
	$(filter-out lanec_ev_radar_dispatch lanec_ev_lmi_dispatch,$(EV_NAMES)) \
	TRACKTRM_CODE SETVHFLG_CODE RESETVHF_CODE ATTACHIT_CODE TACHEXIT_CODE TCPIN_CODE LMTOCM_CODE
EV_FILE_COMMON := $(LANEC_COMMON) $(EV_INTERP_COMMON) $(PORT)/src/cm_interp_orders.c \
	$(PORT)/src/cm_interp_steps.c $(PORT)/src/agc_transfer.c $(PORT)/tests/interp_case.c \
	-DCM_ROPE='"$(word 2,$(ROPES))"' tests/extended_verbs_file_program.c
$(BUILD)/ev-file-cm.o: $(EV_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(EV_FILE_NAMES),--keep-global-symbol=$(n)) $@.all $@
$(BUILD)/ev-file-faithful.o: $(PORT)/src/cm_extended_verbs_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) \
		-Dcm_extended_verbs_file_basic=faithful_cm_extended_verbs_file_basic \
		-Dcm_extended_verbs_file_interpretive=faithful_cm_extended_verbs_file_interpretive \
		-c $< -o $@
$(BUILD)/ev-file-oracle: $(BUILD)/ev-file-cm.o tests/extended_verbs_file_program.c \
		tests/extended_verbs_file_target.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Itests $(EV_FILE_COMMON) $(BUILD)/ev-file-cm.o \
		tests/extended_verbs_file_target.c $(PORT)/tests/test_cm_extended_verbs_file.c -o $@
$(BUILD)/ev-file-cases: $(BUILD)/ev-file-cm.o $(BUILD)/ev-file-faithful.o \
		tests/extended_verbs_file_program.c tests/extended_verbs_file_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Itests $(EV_FILE_COMMON) $(BUILD)/ev-file-cm.o \
		$(BUILD)/ev-file-faithful.o tests/extended_verbs_file_cases.c -o $@

extended-verbs-cases: $(BUILD)/ev-front-oracle $(BUILD)/ev-front-cases $(BUILD)/ev-short-oracle \
		$(BUILD)/ev-short-cases $(BUILD)/ev-align-oracle $(BUILD)/ev-align-cases \
		$(BUILD)/ev-radar-oracle $(BUILD)/ev-radar-cases $(BUILD)/ev-dap-oracle \
		$(BUILD)/ev-dap-cases $(BUILD)/ev-time-oracle $(BUILD)/ev-time-cases \
		$(BUILD)/ev-track-oracle $(BUILD)/ev-track-cases $(BUILD)/ev-lmi-oracle \
		$(BUILD)/ev-lmi-cases $(BUILD)/ev-file-oracle $(BUILD)/ev-file-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ev-front-oracle
	./$(BUILD)/ev-front-cases
	./$(BUILD)/ev-short-oracle
	./$(BUILD)/ev-short-cases
	./$(BUILD)/ev-align-oracle
	./$(BUILD)/ev-align-cases
	./$(BUILD)/ev-radar-oracle
	./$(BUILD)/ev-radar-cases
	./$(BUILD)/ev-dap-oracle
	./$(BUILD)/ev-dap-cases
	./$(BUILD)/ev-time-oracle
	./$(BUILD)/ev-time-cases
	./$(BUILD)/ev-track-oracle
	./$(BUILD)/ev-track-cases
	./$(BUILD)/ev-lmi-oracle $(ROPES)
	./$(BUILD)/ev-lmi-cases $(ROPES)
	./$(BUILD)/ev-file-oracle
	./$(BUILD)/ev-file-cases

# PINBALL_NOUN_TABLES, both missions: the readers in the faithful port's
# place, as apollo_noun_reader (c64-port/include/noun_reader.h). Each
# mission's objects are built into one object with that name, and the
# tables' word function, renamed readable_<m>_...; tests/
# noun_reader_missions.c puts the first back, by mission. First the port's
# own test (c64-port/tests/test_noun_reader.c: every supported case runs,
# the unsupported ones are refused) on them, then the original program's
# frozen results (tests/pinball_noun_tables_oracle.c), then
# tests/pinball_noun_tables_cases.c: the tables against the ropes, and the
# readers side by side with the faithful one.
NOUNS_NAMES := apollo_noun_reader noun_table_word
$(BUILD)/nouns-readable-lm.o: $(NOUNS_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
$(BUILD)/nouns-readable-cm.o: $(NOUNS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
$(BUILD)/nouns-readable-%.o:
	ld -r $^ -o $@.all
	objcopy $(foreach n,$(NOUNS_NAMES),--keep-global-symbol=$(n)) $@.all $@.kept
	objcopy --redefine-sym apollo_noun_reader=readable_$*_noun_reader \
		--redefine-sym noun_table_word=readable_$*_noun_table_word $@.kept $@
NOUNS_READABLE := $(BUILD)/nouns-readable-lm.o $(BUILD)/nouns-readable-cm.o tests/noun_reader_missions.c
$(BUILD)/nouns-faithful.o: $(PORT)/src/noun_reader.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dapollo_noun_reader=faithful_apollo_noun_reader -c $< -o $@
$(BUILD)/nouns-port-test: $(NOUNS_READABLE)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/tests/noun_reader_cases.c $(PORT)/tests/test_noun_reader.c \
		$(filter %.o %.c,$^) -o $@
$(BUILD)/nouns-oracle: $(NOUNS_READABLE) tests/pinball_noun_tables_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/tests/noun_reader_cases.c $(filter %.o %.c,$^) -o $@
$(BUILD)/nouns-cases: $(NOUNS_READABLE) $(BUILD)/nouns-faithful.o tests/pinball_noun_tables_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(PORT)/src/agc_word.c $(PORT)/src/fixed_data.c \
		$(PORT)/src/rope_image_host.c $(PORT)/tests/noun_reader_cases.c $(filter %.o %.c,$^) -o $@

pinball-noun-tables-cases: $(BUILD)/nouns-port-test $(BUILD)/nouns-oracle $(BUILD)/nouns-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/nouns-port-test
	./$(BUILD)/nouns-oracle
	./$(BUILD)/nouns-cases $(ROPES)

# ---- LEM_GEOMETRY, CSM_GEOMETRY ----
# The port's test (tests/test_geometry.c, its frozen cases run to their
# stops through the program of tests/geometry_target.c, against the
# original program's yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/geometry_cases.c side by
# side, for each mission.
GEOMETRY_NAMES_LM := lm_geometry_basic lm_geometry_interpretive
GEOMETRY_NAMES_CM := cm_geometry_basic cm_geometry_interpretive
GEOMETRY_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rtb_angles.c $(PORT)/src/rtb_agreement.c $(PORT)/src/geometry_rtb.c \
	$(PORT)/src/cm_integration_moves.c $(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/geometry-faithful-lm.o: $(PORT)/src/lm_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GEOMETRY_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/geometry-faithful-cm.o: $(PORT)/src/cm_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GEOMETRY_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/geometry-test.o: $(PORT)/tests/test_geometry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/geometry-oracle-lm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/geometry-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GEOMETRY_COMMON) $(PORT)/src/cm_geometry.c \
		$(PORT)/tests/geometry_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/geometry-oracle-cm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/geometry-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GEOMETRY_COMMON) $(PORT)/src/lm_geometry.c \
		$(PORT)/tests/geometry_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/geometry-cases-lm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/geometry-faithful-lm.o tests/geometry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(GEOMETRY_COMMON) $(filter %.o,$^) \
		tests/geometry_cases.c -o $@
$(BUILD)/geometry-cases-cm: $(GEOMETRY_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/geometry-faithful-cm.o tests/geometry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(GEOMETRY_COMMON) $(filter %.o,$^) \
		tests/geometry_cases.c -o $@

geometry-cases: $(BUILD)/geometry-oracle-lm $(BUILD)/geometry-oracle-cm $(BUILD)/geometry-cases-lm \
		$(BUILD)/geometry-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/geometry-oracle-lm $(ROPES)
	./$(BUILD)/geometry-oracle-cm $(ROPES)
	./$(BUILD)/geometry-cases-lm $(ROPES)
	./$(BUILD)/geometry-cases-cm $(ROPES)

# ---- PINBALL_GAME_BUTTONS_AND_LIGHTS ----
# The readable parts take the place of the port's Pinball trampoline
# (c64-port/src/lanec_pin_trampoline.c) under its names, lanec_pin_run_lm
# and lanec_pin_run_cm, and hand it (renamed faithful_lanec_pin_run_<m>)
# only a step the readable code does not model (src/binding/
# pinball_routines.c). Each mission's readable objects are built into one
# object that keeps only its trampoline name. The gate is
# the port's own Pinball tests, run live against the original program in
# Virtual AGC with the readable parts in place (tests/pinball_transfer.py).
# Pinball's NOVAC (DSPMM) and JOBWAKE (RECAL1) run the readable Executive,
# which switches no job and ends none there: its seams do nothing
# (tests/pinball_seams.c).
$(BUILD)/pin-readable-lm.o: $(PIN_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/pin-seams-lm.o
$(BUILD)/pin-readable-cm.o: $(PIN_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/pin-seams-cm.o
$(BUILD)/pin-seams-lm.o: tests/pinball_seams.c $(HEADERS)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -c $< -o $@
$(BUILD)/pin-seams-cm.o: tests/pinball_seams.c $(HEADERS)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(CM) -c $< -o $@
$(BUILD)/pin-readable-%.o:
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lanec_pin_run_$* $@.all $@

pinball-cases: $(BUILD)/pin-readable-lm.o $(BUILD)/pin-readable-cm.o tests/pinball_transfer.py
	python3 tests/pinball_transfer.py $(BUILD)/pin-readable-lm.o $(BUILD)/pin-readable-cm.o

# ---- The x86_64 flight program ----
# tools/build_x86.py builds the port's x86 program with these objects in
# place of the faithful kernel (build/x86-flight/).
# R47's, P07's, BURN, BABY, BURN's and THE_LUNAR_LANDING's routines stop
# where they reach the
# interpreter in the flight program, as the port's flight builds of their
# files do.
$(BUILD)/x86-lm/binding/ags_initialization_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/binding/imu_performance_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/binding/burn_baby_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/binding/lunar_landing_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/binding/servicer_routines.o: FLIGHT_DEFINES := -DAGC_FLIGHT_INTERPRETER
$(BUILD)/x86-lm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) $(FLIGHT_DEFINES) -c $< -o $@
$(BUILD)/x86-cm/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(CM) -c $< -o $@
X86_OBJECTS := $(SOURCES:src/%.c=$(BUILD)/x86-lm/%.o) $(SOURCES:src/%.c=$(BUILD)/x86-cm/%.o)

# The harness's frozen-clock yaAGC engine (the port builds it).
HARNESS_ENGINE := $(PORT)/build/agc_engine_hardware_dv.o
$(HARNESS_ENGINE):
	$(MAKE) -C $(PORT) SKIP_EVIDENCE_RESTORE=1 build/agc_engine_hardware_dv.o

x86-flight: $(X86_OBJECTS)
	python3 tests/x86_readable.py build

x86-scenarios-quick: $(X86_OBJECTS) $(HARNESS_ENGINE)
	python3 tests/x86_readable.py scenarios --quick

x86-scenarios: $(X86_OBJECTS) $(HARNESS_ENGINE)
	python3 tests/x86_readable.py scenarios

format:
	$(PORT)/deps/llvm-mos/bin/clang-format -i --style=file:$(PORT)/.clang-format $(SOURCES) $(filter %.h,$(HEADERS))

clean:
	rm -rf $(BUILD)

# IMU_COMPENSATION_PACKAGE, both missions: the port's test
# (tests/test_imu_compensation.c, its frozen cases run to their stops,
# composed with the faithful INTER-BANK_COMMUNICATION and Executive,
# against the original program's yaAGC digests) on the readable routines,
# both missions' in one program: each mission's readable objects built
# into one object that keeps only its table's name global. Then
# tests/imu_compensation_cases.c side by side, mission by mission.
ICP_MK := $(PORT)/tools/mk/imu-compensation.mk
ICP_PORT := $(addprefix $(PORT)/,$(filter-out src/imu_compensation.c,$(shell sed -n 's/^ICP_SRC = //p' $(ICP_MK))))
ICP_PORT_FLAGS := -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -fdata-sections -Wl,--gc-sections

$(BUILD)/imu-compensation-readable-lm.o: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=lm_imu_compensation_routines $@.all $@
$(BUILD)/imu-compensation-readable-cm.o: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o)
	ld -r $^ -o $@.all
	objcopy --keep-global-symbol=cm_imu_compensation_routines $@.all $@
$(BUILD)/imu-compensation-oracle: $(BUILD)/imu-compensation-readable-lm.o \
		$(BUILD)/imu-compensation-readable-cm.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(ICP_PORT_FLAGS) $(ICP_PORT) $(PORT)/tests/test_imu_compensation.c \
		$^ -o $@
$(BUILD)/imu-compensation-faithful.o: $(PORT)/src/imu_compensation.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_imu_compensation_routines=faithful_lm_imu_compensation_routines \
		-Dcm_imu_compensation_routines=faithful_cm_imu_compensation_routines -c $< -o $@
$(BUILD)/imu-compensation-cases-lm: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/imu-compensation-faithful.o tests/imu_compensation_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c \
		$(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
		$(filter %.o,$^) tests/imu_compensation_cases.c -o $@
$(BUILD)/imu-compensation-cases-cm: $(IMU_COMP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/imu-compensation-faithful.o tests/imu_compensation_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PORT)/src/agc_image_ops.c $(PORT)/src/agc_hardware_divide.c \
		$(PORT)/src/agc_transfer.c $(PORT)/src/agc_word.c $(PORT)/src/agc_multiword.c \
		$(filter %.o,$^) tests/imu_compensation_cases.c -o $@

imu-compensation-cases: $(BUILD)/imu-compensation-oracle $(BUILD)/imu-compensation-cases-lm \
		$(BUILD)/imu-compensation-cases-cm
	./$(BUILD)/imu-compensation-oracle
	./$(BUILD)/imu-compensation-cases-lm
	./$(BUILD)/imu-compensation-cases-cm

# R63, the LM's V89 final attitude: the port's test (tests/test_lm_r63.c,
# its frozen cases run to their stops through the program of
# tests/lm_r63_target.c, against the original program's yaAGC digests) on
# the readable routines, with tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/r63_cases.c side by side.
R63_NAMES := lm_r63_basic lm_r63_interpretive
R63_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rtb_angles.c $(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/r63-faithful.o: $(PORT)/src/lm_r63.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R63_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r63-test.o: $(PORT)/tests/test_lm_r63.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r63-oracle: $(R63_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r63-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/lm_r63_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r63-cases: $(R63_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r63-faithful.o \
		tests/r63_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) tests/r63_cases.c -o $@

r63-cases: $(BUILD)/r63-oracle $(BUILD)/r63-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/r63-oracle $(ROPES)
	./$(BUILD)/r63-cases $(ROPES)

# ATTITUDE_MANEUVER_ROUTINE (the LM's) and ANGLFIND (the CM's), KALCMANU's
# start and matrix routines: the port's test (tests/test_attitude_maneuver.c,
# its frozen cases run to their stops through the programs of
# tests/attitude_maneuver_target.c, against the original programs' yaAGC
# digests) with each mission's readable routines and the other mission's
# faithful ones, tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/attitude_maneuver_cases.c side by side, for
# each mission.
AMR_NAMES_LM := lm_attitude_maneuver_basic lm_attitude_maneuver_interpretive
AMR_NAMES_CM := cm_attitude_maneuver_basic cm_attitude_maneuver_interpretive
AMR_COMMON := $(R63_COMMON)

$(BUILD)/amr-faithful-lm.o: $(PORT)/src/lm_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AMR_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/amr-faithful-cm.o: $(PORT)/src/cm_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(AMR_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/amr-test.o: $(PORT)/tests/test_attitude_maneuver.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/amr-oracle-lm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/amr-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AMR_COMMON) $(PORT)/src/cm_attitude_maneuver.c \
		$(PORT)/tests/attitude_maneuver_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/amr-oracle-cm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/amr-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(AMR_COMMON) $(PORT)/src/lm_attitude_maneuver.c \
		$(PORT)/tests/attitude_maneuver_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/amr-cases-lm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/amr-faithful-lm.o \
		tests/attitude_maneuver_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(AMR_COMMON) $(filter %.o,$^) \
		tests/attitude_maneuver_cases.c -o $@
$(BUILD)/amr-cases-cm: $(AMR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/amr-faithful-cm.o \
		tests/attitude_maneuver_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(AMR_COMMON) $(filter %.o,$^) \
		tests/attitude_maneuver_cases.c -o $@

attitude-maneuver-cases: $(BUILD)/amr-oracle-lm $(BUILD)/amr-oracle-cm $(BUILD)/amr-cases-lm \
		$(BUILD)/amr-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/amr-oracle-lm $(ROPES)
	./$(BUILD)/amr-oracle-cm $(ROPES)
	./$(BUILD)/amr-cases-lm $(ROPES)
	./$(BUILD)/amr-cases-cm $(ROPES)

# GIMBAL_LOCK_AVOIDANCE, both missions' LOCSKIRT: the port's test
# (tests/test_gimbal_lock.c, its frozen cases run to their stops through
# the programs of tests/gimbal_lock_target.c, against the original
# programs' yaAGC digests) with each mission's readable routines and the
# other mission's faithful ones, tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/gimbal_lock_cases.c side by side,
# for each mission.
GLA_NAMES_LM := lm_gimbal_lock_basic lm_gimbal_lock_interpretive
GLA_NAMES_CM := cm_gimbal_lock_basic cm_gimbal_lock_interpretive
GLA_COMMON := $(R63_COMMON)

$(BUILD)/gla-faithful-lm.o: $(PORT)/src/lm_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GLA_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/gla-faithful-cm.o: $(PORT)/src/cm_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(GLA_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/gla-test.o: $(PORT)/tests/test_gimbal_lock.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/gla-oracle-lm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/gla-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GLA_COMMON) $(PORT)/src/cm_gimbal_lock.c \
		$(PORT)/tests/gimbal_lock_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/gla-oracle-cm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/gla-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(GLA_COMMON) $(PORT)/src/lm_gimbal_lock.c \
		$(PORT)/tests/gimbal_lock_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/gla-cases-lm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/gla-faithful-lm.o \
		tests/gimbal_lock_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(GLA_COMMON) $(filter %.o,$^) \
		tests/gimbal_lock_cases.c -o $@
$(BUILD)/gla-cases-cm: $(GLA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/gla-faithful-cm.o \
		tests/gimbal_lock_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(GLA_COMMON) $(filter %.o,$^) \
		tests/gimbal_lock_cases.c -o $@

gimbal-lock-cases: $(BUILD)/gla-oracle-lm $(BUILD)/gla-oracle-cm $(BUILD)/gla-cases-lm \
		$(BUILD)/gla-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/gla-oracle-lm $(ROPES)
	./$(BUILD)/gla-oracle-cm $(ROPES)
	./$(BUILD)/gla-cases-lm $(ROPES)
	./$(BUILD)/gla-cases-cm $(ROPES)

# KALCMANU_STEERING, both missions' maneuver steering: the port's test
# (tests/test_kalcmanu_steering.c, its frozen cases run to their stops
# through the programs of tests/kalcmanu_steering_target.c, against the
# original programs' yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/kalcmanu_steering_cases.c
# side by side, for each mission.
KAL_NAMES_LM := lm_kalcmanu_steering_basic lm_kalcmanu_steering_interpretive
KAL_NAMES_CM := cm_kalcmanu_steering_basic cm_kalcmanu_steering_interpretive
KAL_COMMON := $(R63_COMMON)

$(BUILD)/kal-faithful-lm.o: $(PORT)/src/lm_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KAL_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/kal-faithful-cm.o: $(PORT)/src/cm_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(KAL_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/kal-test.o: $(PORT)/tests/test_kalcmanu_steering.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/kal-oracle-lm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/kal-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KAL_COMMON) $(PORT)/src/cm_kalcmanu_steering.c \
		$(PORT)/tests/kalcmanu_steering_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/kal-oracle-cm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/kal-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(KAL_COMMON) $(PORT)/src/lm_kalcmanu_steering.c \
		$(PORT)/tests/kalcmanu_steering_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/kal-cases-lm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/kal-faithful-lm.o \
		tests/kalcmanu_steering_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(KAL_COMMON) $(filter %.o,$^) \
		tests/kalcmanu_steering_cases.c -o $@
$(BUILD)/kal-cases-cm: $(KAL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/kal-faithful-cm.o \
		tests/kalcmanu_steering_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(KAL_COMMON) $(filter %.o,$^) \
		tests/kalcmanu_steering_cases.c -o $@

kalcmanu-steering-cases: $(BUILD)/kal-oracle-lm $(BUILD)/kal-oracle-cm $(BUILD)/kal-cases-lm \
		$(BUILD)/kal-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/kal-oracle-lm $(ROPES)
	./$(BUILD)/kal-oracle-cm $(ROPES)
	./$(BUILD)/kal-cases-lm $(ROPES)
	./$(BUILD)/kal-cases-cm $(ROPES)

# R60_62, both missions' maneuver requests (R60), V49 (R62), BALLANGS and
# VECPOINT: the port's test (tests/test_r60_r62.c, its frozen cases run to
# their stops through the programs of tests/r60_r62_target.c, against the
# original programs' yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/r60_62_cases.c side by
# side, for each mission.
R60_NAMES_LM := lm_r60_r62_basic lm_r60_r62_interpretive
R60_NAMES_CM := cm_r60_r62_basic cm_r60_r62_interpretive
R60_COMMON := $(R63_COMMON)

$(BUILD)/r60-faithful-lm.o: $(PORT)/src/lm_r60_r62.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R60_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r60-faithful-cm.o: $(PORT)/src/cm_r60_r62.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R60_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r60-test.o: $(PORT)/tests/test_r60_r62.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r60-oracle-lm: $(R60_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r60-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R60_COMMON) $(PORT)/src/cm_r60_r62.c \
		$(PORT)/tests/r60_r62_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r60-oracle-cm: $(R60_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r60-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R60_COMMON) $(PORT)/src/lm_r60_r62.c \
		$(PORT)/tests/r60_r62_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r60-cases-lm: $(R60_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r60-faithful-lm.o \
		tests/r60_62_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R60_COMMON) $(filter %.o,$^) \
		tests/r60_62_cases.c -o $@
$(BUILD)/r60-cases-cm: $(R60_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r60-faithful-cm.o \
		tests/r60_62_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(R60_COMMON) $(filter %.o,$^) \
		tests/r60_62_cases.c -o $@

r60-r62-cases: $(BUILD)/r60-oracle-lm $(BUILD)/r60-oracle-cm $(BUILD)/r60-cases-lm \
		$(BUILD)/r60-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/r60-oracle-lm $(ROPES)
	./$(BUILD)/r60-oracle-cm $(ROPES)
	./$(BUILD)/r60-cases-lm $(ROPES)
	./$(BUILD)/r60-cases-cm $(ROPES)

# S-BAND_ANTENNA_FOR_LM and _FOR_CM, R05 (verb 64): the port's test
# (tests/test_sband_antenna.c, its frozen cases run to their stops through
# the programs of tests/sband_antenna_target.c, against the original
# programs' yaAGC digests) with each mission's readable routines and the
# other mission's faithful ones, tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/sband_antenna_cases.c side by
# side, for each mission.
SBA_NAMES_LM := lm_sband_antenna_basic lm_sband_antenna_interpretive
SBA_NAMES_CM := cm_sband_antenna_basic cm_sband_antenna_interpretive
SBA_COMMON := $(R63_COMMON)

$(BUILD)/sba-faithful-lm.o: $(PORT)/src/lm_sband_antenna.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SBA_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sba-faithful-cm.o: $(PORT)/src/cm_sband_antenna.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SBA_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sba-test.o: $(PORT)/tests/test_sband_antenna.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/sba-oracle-lm: $(SBA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sba-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SBA_COMMON) $(PORT)/src/cm_sband_antenna.c \
		$(PORT)/tests/sband_antenna_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/sba-oracle-cm: $(SBA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sba-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SBA_COMMON) $(PORT)/src/lm_sband_antenna.c \
		$(PORT)/tests/sband_antenna_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/sba-cases-lm: $(SBA_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sba-faithful-lm.o \
		tests/sband_antenna_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SBA_COMMON) $(filter %.o,$^) \
		tests/sband_antenna_cases.c -o $@
$(BUILD)/sba-cases-cm: $(SBA_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sba-faithful-cm.o \
		tests/sband_antenna_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SBA_COMMON) $(filter %.o,$^) \
		tests/sband_antenna_cases.c -o $@

sband-antenna-cases: $(BUILD)/sba-oracle-lm $(BUILD)/sba-oracle-cm $(BUILD)/sba-cases-lm \
		$(BUILD)/sba-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/sba-oracle-lm $(ROPES)
	./$(BUILD)/sba-oracle-cm $(ROPES)
	./$(BUILD)/sba-cases-lm $(ROPES)
	./$(BUILD)/sba-cases-cm $(ROPES)

# RADAR_LEADIN_ROUTINES, the LM's radar sampling loop: the port's test
# (tests/test_lm_radar_leadin_routines.c, its frozen cases run to their
# stops through the program of tests/lm_radar_leadin_routines_target.c,
# against the original program's yaAGC digests) on the readable routines,
# with tests/with_ropes.c loading the rope VARADAR reads; then
# tests/radar_leadin_cases.c side by side.
RLR_NAMES := lm_radar_leadin_routines_basic lm_radar_leadin_routines_interpretive

$(BUILD)/rlr-faithful.o: $(PORT)/src/lm_radar_leadin_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(RLR_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/rlr-test.o: $(PORT)/tests/test_lm_radar_leadin_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/rlr-oracle: $(RLR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rlr-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) \
		$(PORT)/tests/lm_radar_leadin_routines_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/rlr-cases: $(RLR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/rlr-faithful.o \
		tests/radar_leadin_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) \
		tests/radar_leadin_cases.c -o $@

radar-leadin-cases: $(BUILD)/rlr-oracle $(BUILD)/rlr-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/rlr-oracle $(ROPES)
	./$(BUILD)/rlr-cases $(ROPES)

# The oracles of P20-P25 to P40-P47 link the readable tables and no
# faithful object: the port's test calls only the tables it is given, so a
# key with no readable routine fails its digests there. The renamed
# faithful objects are for the side-by-side cases alone. The later oracles
# link one for its data, under its own names: BURN, BABY, BURN's
# (bb-oracle) its case keys and KILLTASK's sites, THE_LUNAR_LANDING's and
# SERVICER's (lld-, svc-oracle) their case keys (and SERVICER's flag
# sites), FINDCDUW's, P51-P53's and LUNAR_LANDING_GUIDANCE_EQUATIONS'
# (fcw-, p51-, llge-oracle) their points, external routines and CADR
# words.

# P20-P25, the LM's rendezvous programs and radar routines, readable part
# by part: the port's test (tests/test_lm_p20_p25.c, its frozen cases run
# to their stops through the program of tests/lm_p20_p25_target.c, against
# the original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/p20_p25_cases.c side by side with the faithful routines (renamed
# faithful_).
P20_NAMES := lm_p20_p25_basic lm_p20_p25_interpretive
P20_COMMON := $(R63_COMMON) $(PORT)/src/lm_integration_moves.c

$(BUILD)/p20-faithful.o: $(PORT)/src/lm_p20_p25.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P20_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p20-test.o: $(PORT)/tests/test_lm_p20_p25.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p20-oracle: $(P20_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p20-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P20_COMMON) \
		$(PORT)/tests/lm_p20_p25_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p20-cases: $(P20_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p20-faithful.o \
		tests/p20_p25_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P20_COMMON) $(filter %.o,$^) \
		tests/p20_p25_cases.c -o $@

p20-p25-cases: $(BUILD)/p20-oracle $(BUILD)/p20-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p20-oracle $(ROPES)
	./$(BUILD)/p20-cases $(ROPES)

# The CM's P20-P25: the port's test
# (tests/test_cm_p20_p25.c, its frozen cases run to their stops through the
# program of tests/cm_p20_p25_target.c, against the original program's
# yaAGC digests) through the readable tables, with tests/with_ropes.c
# loading the rope the readable interpreter reads; then
# tests/cm_p20_p25_cases.c side by side with the faithful routines (renamed
# faithful_).
CP20_NAMES := cm_p20_p25_basic cm_p20_p25_interpretive
CP20_COMMON := $(R63_COMMON) $(PORT)/src/rtb_modes.c

$(BUILD)/cp20-faithful.o: $(PORT)/src/cm_p20_p25.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP20_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp20-test.o: $(PORT)/tests/test_cm_p20_p25.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp20-oracle: $(CP20_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp20-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP20_COMMON) \
		$(PORT)/tests/cm_p20_p25_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp20-cases: $(CP20_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp20-faithful.o \
		tests/cm_p20_p25_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP20_COMMON) $(filter %.o,$^) \
		tests/cm_p20_p25_cases.c -o $@

cm-p20-p25-cases: $(BUILD)/cp20-oracle $(BUILD)/cp20-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp20-oracle $(ROPES)
	./$(BUILD)/cp20-cases $(ROPES)

# P30_P37.agc, the LM's P30 and S30.1: the port's test
# (tests/test_lm_p30_file.c, its frozen cases run to their stops through
# the program of tests/lm_p30_file_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/p30_cases.c side by side with the faithful routines (renamed
# faithful_).
P30_NAMES := lm_p30_file_basic lm_p30_file_interpretive
P30_COMMON := $(R63_COMMON)

$(BUILD)/p30-faithful.o: $(PORT)/src/lm_p30_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P30_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p30-test.o: $(PORT)/tests/test_lm_p30_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p30-oracle: $(P30_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p30-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P30_COMMON) $(PORT)/tests/lm_p30_file_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p30-cases: $(P30_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p30-faithful.o \
		tests/p30_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P30_COMMON) $(filter %.o,$^) tests/p30_cases.c -o $@

p30-cases: $(BUILD)/p30-oracle $(BUILD)/p30-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p30-oracle $(ROPES)
	./$(BUILD)/p30-cases $(ROPES)

# P30-P37.agc, the CM's P30, P31, S30.1, S31.1, DELRSPL and AVFLAGA: the
# port's test (tests/test_cm_p30_p37.c, its frozen cases run to their stops
# through the program of tests/cm_p30_p37_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/cm_p30_cases.c side by side with the faithful routines (renamed
# faithful_).
CP30_NAMES := cm_p30_p37_basic cm_p30_p37_interpretive
CP30_COMMON := $(R63_COMMON)

$(BUILD)/cp30-faithful.o: $(PORT)/src/cm_p30_p37.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP30_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp30-test.o: $(PORT)/tests/test_cm_p30_p37.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp30-oracle: $(CP30_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp30-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP30_COMMON) $(PORT)/tests/cm_p30_p37_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp30-cases: $(CP30_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp30-faithful.o \
		tests/cm_p30_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP30_COMMON) $(filter %.o,$^) tests/cm_p30_cases.c -o $@

cm-p30-cases: $(BUILD)/cp30-oracle $(BUILD)/cp30-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp30-oracle $(ROPES)
	./$(BUILD)/cp30-cases $(ROPES)

# P32-P35_P72-P75.agc, the LM's CSI and CDH targeting: the port's test (tests/test_lm_p32_p35.c, its frozen cases run to
# their stops through the program of tests/lm_p32_p35_target.c, against
# the original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/p32_cases.c side by side with the faithful routines (renamed
# faithful_).
P32_NAMES := lm_p32_p35_basic lm_p32_p35_interpretive
P32_COMMON := $(R63_COMMON)

$(BUILD)/p32-faithful.o: $(PORT)/src/lm_p32_p35.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P32_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p32-test.o: $(PORT)/tests/test_lm_p32_p35.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p32-oracle: $(P32_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p32-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P32_COMMON) $(PORT)/tests/lm_p32_p35_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p32-cases: $(P32_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p32-faithful.o \
		tests/p32_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P32_COMMON) $(filter %.o,$^) tests/p32_cases.c -o $@

p32-cases: $(BUILD)/p32-oracle $(BUILD)/p32-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p32-oracle $(ROPES)
	./$(BUILD)/p32-cases $(ROPES)

# P32-P33_P72-P73.agc, the CM's CSI and CDH targeting: the port's test
# (tests/test_cm_p32_p33.c, its frozen cases run to their stops through
# the program of tests/cm_p32_p33_target.c, against the original program's
# yaAGC digests) through the readable tables, with tests/with_ropes.c
# loading the rope the readable interpreter reads; then
# tests/cm_p32_cases.c side by side with the faithful routines (renamed
# faithful_).
CP32_NAMES := cm_p32_p33_basic cm_p32_p33_interpretive
CP32_COMMON := $(R63_COMMON)

$(BUILD)/cp32-faithful.o: $(PORT)/src/cm_p32_p33.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP32_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp32-test.o: $(PORT)/tests/test_cm_p32_p33.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp32-oracle: $(CP32_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp32-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP32_COMMON) $(PORT)/tests/cm_p32_p33_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp32-cases: $(CP32_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp32-faithful.o \
		tests/cm_p32_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP32_COMMON) $(filter %.o,$^) tests/cm_p32_cases.c -o $@

cm-p32-cases: $(BUILD)/cp32-oracle $(BUILD)/cp32-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp32-oracle $(ROPES)
	./$(BUILD)/cp32-cases $(ROPES)

# GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc, the LM's P31: the port's test
# (tests/test_lm_lambert_aim.c, its frozen cases run to their stops
# through the program of tests/lm_lambert_aim_target.c, against the
# original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/lambert_aim_cases.c side by side with the faithful routines
# (renamed faithful_).
P31_NAMES := lm_lambert_aim_basic lm_lambert_aim_interpretive
P31_COMMON := $(R63_COMMON)

$(BUILD)/p31-faithful.o: $(PORT)/src/lm_lambert_aim.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P31_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p31-test.o: $(PORT)/tests/test_lm_lambert_aim.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p31-oracle: $(P31_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p31-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P31_COMMON) $(PORT)/tests/lm_lambert_aim_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p31-cases: $(P31_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p31-faithful.o \
		tests/lambert_aim_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P31_COMMON) $(filter %.o,$^) tests/lambert_aim_cases.c -o $@

lambert-aim-cases: $(BUILD)/p31-oracle $(BUILD)/p31-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p31-oracle $(ROPES)
	./$(BUILD)/p31-cases $(ROPES)

# GROUND_TRACKING_DETERMINATION_PROGRAM.agc, both missions' P21: each
# mission's port test (tests/test_lm_ground_track.c and
# test_cm_ground_track.c, their frozen cases run to their stops through the
# programs of tests/lm_ and cm_ground_track_target.c, against the original
# programs' yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads; then
# tests/ground_track_cases.c side by side with the faithful routines
# (renamed faithful_), for each mission, with the older module's tables
# (renamed too) for their keys.
P21_COMMON := $(R63_COMMON)

$(BUILD)/p21-faithful-%.o: $(PORT)/src/%_ground_track.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_ground_track_basic=faithful_$*_ground_track_basic \
		-D$*_ground_track_interpretive=faithful_$*_ground_track_interpretive -c $< -o $@
$(BUILD)/p21-older-%.o: $(PORT)/src/%_p21_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_p21_basic_routines=faithful_$*_p21_basic_routines \
		-D$*_p21_interpretive_routines=faithful_$*_p21_interpretive_routines -c $< -o $@
$(BUILD)/p21-test-%.o: $(PORT)/tests/test_%_ground_track.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p21-oracle-lm: $(P21_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p21-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P21_COMMON) $(PORT)/tests/lm_ground_track_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p21-oracle-cm: $(P21_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p21-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P21_COMMON) $(PORT)/tests/cm_ground_track_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p21-cases-lm: $(P21_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p21-faithful-lm.o \
		$(BUILD)/p21-older-lm.o tests/ground_track_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P21_COMMON) $(filter %.o,$^) tests/ground_track_cases.c -o $@
$(BUILD)/p21-cases-cm: $(P21_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p21-faithful-cm.o \
		$(BUILD)/p21-older-cm.o tests/ground_track_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(P21_COMMON) $(filter %.o,$^) tests/ground_track_cases.c -o $@

ground-track-cases: $(BUILD)/p21-oracle-lm $(BUILD)/p21-oracle-cm $(BUILD)/p21-cases-lm \
		$(BUILD)/p21-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p21-oracle-lm $(ROPES)
	./$(BUILD)/p21-oracle-cm $(ROPES)
	./$(BUILD)/p21-cases-lm $(ROPES)
	./$(BUILD)/p21-cases-cm $(ROPES)

# P34-35_P74-75.agc, both missions' TPI and mid-course programs: each
# mission's port test (tests/test_lm_p34_p35.c and test_cm_p34_p35.c,
# their frozen cases run to their stops through the programs of
# tests/lm_ and cm_p34_p35_target.c, against the original programs' yaAGC
# digests) through the readable tables, with tests/with_ropes.c loading
# the ropes the readable interpreter reads; then tests/p34_cases.c side by
# side with the faithful routines (renamed faithful_), for each mission.
P34_COMMON := $(R63_COMMON) $(PORT)/src/rtb_modes.c

$(BUILD)/p34-faithful-%.o: $(PORT)/src/%_p34_p35.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -D$*_p34_p35_basic=faithful_$*_p34_p35_basic \
		-D$*_p34_p35_interpretive=faithful_$*_p34_p35_interpretive -c $< -o $@
$(BUILD)/p34-test-%.o: $(PORT)/tests/test_%_p34_p35.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p34-oracle-lm: $(P34_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p34-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P34_COMMON) $(PORT)/tests/lm_p34_p35_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p34-oracle-cm: $(P34_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p34-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P34_COMMON) $(PORT)/tests/cm_p34_p35_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p34-cases-lm: $(P34_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p34-faithful-lm.o \
		tests/p34_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P34_COMMON) $(filter %.o,$^) tests/p34_cases.c -o $@
$(BUILD)/p34-cases-cm: $(P34_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p34-faithful-cm.o \
		tests/p34_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(P34_COMMON) $(filter %.o,$^) tests/p34_cases.c -o $@

p34-cases: $(BUILD)/p34-oracle-lm $(BUILD)/p34-oracle-cm $(BUILD)/p34-cases-lm \
		$(BUILD)/p34-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p34-oracle-lm $(ROPES)
	./$(BUILD)/p34-oracle-cm $(ROPES)
	./$(BUILD)/p34-cases-lm $(ROPES)
	./$(BUILD)/p34-cases-cm $(ROPES)

# R31.agc, both missions' rendezvous parameter display (verb 83): each
# mission's port test (tests/test_lm_r31_file.c and test_cm_r31.c, their
# frozen cases run to their stops through the programs of
# tests/lm_r31_file_target.c and cm_r31_target.c, against the original
# programs' yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads; then
# tests/r31_cases.c side by side with the faithful routines (renamed
# faithful_), for each mission.
R31_COMMON := $(R63_COMMON) $(PORT)/src/lm_normunit.c $(PORT)/src/rtb_vector.c \
	$(PORT)/src/rtb_unit.c $(PORT)/src/guidance_unit_phased.c $(PORT)/src/guidance_length_phased.c \
	$(PORT)/src/guidance_norm.c

$(BUILD)/r31-faithful-lm.o: $(PORT)/src/lm_r31_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_r31_file_basic=faithful_lm_r31_file_basic \
		-Dlm_r31_file_interpretive=faithful_lm_r31_file_interpretive -c $< -o $@
$(BUILD)/r31-faithful-lm-older.o: $(PORT)/src/lm_r31_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_r31_basic_routines=faithful_lm_r31_basic_routines \
		-Dlm_r31_interpretive_routines=faithful_lm_r31_interpretive_routines -c $< -o $@
$(BUILD)/r31-faithful-cm.o: $(PORT)/src/cm_r31.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dcm_r31_basic=faithful_cm_r31_basic \
		-Dcm_r31_interpretive=faithful_cm_r31_interpretive -c $< -o $@
$(BUILD)/r31-test-lm.o: $(PORT)/tests/test_lm_r31_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r31-test-cm.o: $(PORT)/tests/test_cm_r31.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r31-oracle-lm: $(R31_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r31-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R31_COMMON) $(PORT)/tests/lm_r31_file_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r31-oracle-cm: $(R31_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r31-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R31_COMMON) $(PORT)/tests/cm_r31_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r31-cases-lm: $(R31_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r31-faithful-lm.o \
		$(BUILD)/r31-faithful-lm-older.o tests/r31_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R31_COMMON) $(filter %.o,$^) tests/r31_cases.c -o $@
$(BUILD)/r31-cases-cm: $(R31_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r31-faithful-cm.o \
		tests/r31_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(R31_COMMON) $(filter %.o,$^) tests/r31_cases.c -o $@

r31-cases: $(BUILD)/r31-oracle-lm $(BUILD)/r31-oracle-cm $(BUILD)/r31-cases-lm \
		$(BUILD)/r31-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/r31-oracle-lm $(ROPES)
	./$(BUILD)/r31-oracle-cm $(ROPES)
	./$(BUILD)/r31-cases-lm $(ROPES)
	./$(BUILD)/r31-cases-cm $(ROPES)

# P76.agc, both missions' target delta V program (verb 37 noun 76): each
# mission's port test (tests/test_lm_p76_file.c and test_cm_p76_file.c,
# their frozen cases run to their stops through the programs of
# tests/lm_p76_file_target.c and cm_p76_file_target.c, against the original
# programs' yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads; then
# tests/p76_cases.c side by side with the faithful routines, the compiled
# file's and the older module's (renamed faithful_), for each mission.
P76_COMMON := $(R63_COMMON) $(PORT)/src/lm_work_matrix.c $(PORT)/src/matrix_native.c \
	$(PORT)/src/guidance_product.c $(PORT)/src/guidance_norm.c $(PORT)/src/inverse_trig.c \
	$(PORT)/src/rtb_agreement.c $(PORT)/src/fixed_data.c

$(BUILD)/p76-faithful-lm.o: $(PORT)/src/lm_p76_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_p76_file_basic=faithful_lm_p76_file_basic \
		-Dlm_p76_file_interpretive=faithful_lm_p76_file_interpretive -c $< -o $@
$(BUILD)/p76-faithful-lm-older.o: $(PORT)/src/lm_p76_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_p76_basic_routines=faithful_lm_p76_basic_routines \
		-Dlm_p76_interpretive_routines=faithful_lm_p76_interpretive_routines -c $< -o $@
$(BUILD)/p76-faithful-cm.o: $(PORT)/src/cm_p76_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dcm_p76_file_basic=faithful_cm_p76_file_basic \
		-Dcm_p76_file_interpretive=faithful_cm_p76_file_interpretive -c $< -o $@
$(BUILD)/p76-faithful-cm-older.o: $(PORT)/src/cm_p76_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dcm_p76_basic_routines=faithful_cm_p76_basic_routines \
		-Dcm_p76_interpretive_routines=faithful_cm_p76_interpretive_routines -c $< -o $@
$(BUILD)/p76-test-lm.o: $(PORT)/tests/test_lm_p76_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p76-test-cm.o: $(PORT)/tests/test_cm_p76_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p76-oracle-lm: $(P76_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p76-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P76_COMMON) $(PORT)/tests/lm_p76_file_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p76-oracle-cm: $(P76_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p76-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P76_COMMON) $(PORT)/tests/cm_p76_file_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p76-cases-lm: $(P76_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p76-faithful-lm.o \
		$(BUILD)/p76-faithful-lm-older.o tests/p76_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P76_COMMON) $(filter %.o,$^) tests/p76_cases.c -o $@
$(BUILD)/p76-cases-cm: $(P76_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/p76-faithful-cm.o \
		$(BUILD)/p76-faithful-cm-older.o tests/p76_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(P76_COMMON) $(filter %.o,$^) tests/p76_cases.c -o $@

p76-cases: $(BUILD)/p76-oracle-lm $(BUILD)/p76-oracle-cm $(BUILD)/p76-cases-lm \
		$(BUILD)/p76-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p76-oracle-lm $(ROPES)
	./$(BUILD)/p76-oracle-cm $(ROPES)
	./$(BUILD)/p76-cases-lm $(ROPES)
	./$(BUILD)/p76-cases-cm $(ROPES)

# R30.agc, both missions' orbital parameters display (verb 82): the port's
# test (tests/test_r30.c, its frozen cases run to their stops through the
# programs of tests/r30_target.c, against the original programs' yaAGC
# digests) with each mission's readable routines and the other mission's
# faithful ones, tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/r30_cases.c side by side with the faithful
# routines (renamed faithful_), for each mission.
R30_NAMES_LM := lm_r30_basic lm_r30_interpretive
R30_NAMES_CM := cm_r30_basic cm_r30_interpretive
R30_COMMON := $(R63_COMMON)

$(BUILD)/r30-faithful-lm.o: $(PORT)/src/lm_r30.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R30_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r30-faithful-cm.o: $(PORT)/src/cm_r30.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(R30_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/r30-test.o: $(PORT)/tests/test_r30.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/r30-oracle-lm: $(R30_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r30-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R30_COMMON) $(PORT)/src/cm_r30.c $(PORT)/tests/r30_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r30-oracle-cm: $(R30_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r30-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R30_COMMON) $(PORT)/src/lm_r30.c $(PORT)/tests/r30_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/r30-cases-lm: $(R30_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/r30-faithful-lm.o \
		tests/r30_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R30_COMMON) $(filter %.o,$^) tests/r30_cases.c -o $@
$(BUILD)/r30-cases-cm: $(R30_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/r30-faithful-cm.o \
		tests/r30_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(R30_COMMON) $(filter %.o,$^) tests/r30_cases.c -o $@

r30-cases: $(BUILD)/r30-oracle-lm $(BUILD)/r30-oracle-cm $(BUILD)/r30-cases-lm $(BUILD)/r30-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/r30-oracle-lm $(ROPES)
	./$(BUILD)/r30-oracle-cm $(ROPES)
	./$(BUILD)/r30-cases-lm $(ROPES)
	./$(BUILD)/r30-cases-cm $(ROPES)

# STABLE_ORBIT_-_P38-P39.agc, both missions' stable orbit programs (P38,
# P39, P78, P79): each mission's port test (tests/test_lm_stable_orbit.c,
# tests/test_cm_stable_orbit.c: its frozen cases run to their stops through
# the program of its target, against the original program's yaAGC
# digests) through the readable tables, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/stable_orbit_cases.c side
# by side with the faithful routines (renamed faithful_), for each mission.
SO_NAMES_LM := lm_stable_orbit_basic lm_stable_orbit_interpretive
SO_NAMES_CM := cm_stable_orbit_basic cm_stable_orbit_interpretive
SO_COMMON := $(R63_COMMON)

$(BUILD)/so-faithful-lm.o: $(PORT)/src/lm_stable_orbit.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SO_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/so-faithful-cm.o: $(PORT)/src/cm_stable_orbit.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SO_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/so-test-lm.o: $(PORT)/tests/test_lm_stable_orbit.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/so-test-cm.o: $(PORT)/tests/test_cm_stable_orbit.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/so-oracle-lm: $(SO_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/so-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SO_COMMON) $(PORT)/tests/lm_stable_orbit_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/so-oracle-cm: $(SO_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/so-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SO_COMMON) $(PORT)/tests/cm_stable_orbit_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/so-cases-lm: $(SO_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/so-faithful-lm.o \
		tests/stable_orbit_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SO_COMMON) $(filter %.o,$^) tests/stable_orbit_cases.c \
		-o $@
$(BUILD)/so-cases-cm: $(SO_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/so-faithful-cm.o \
		tests/stable_orbit_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SO_COMMON) $(filter %.o,$^) tests/stable_orbit_cases.c \
		-o $@

stable-orbit-cases: $(BUILD)/so-oracle-lm $(BUILD)/so-oracle-cm $(BUILD)/so-cases-lm \
		$(BUILD)/so-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/so-oracle-lm $(ROPES)
	./$(BUILD)/so-oracle-cm $(ROPES)
	./$(BUILD)/so-cases-lm $(ROPES)
	./$(BUILD)/so-cases-cm $(ROPES)

# BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, the LM's master
# ignition routine: tests/burn_baby_oracle.c runs the port's frozen cases
# (c64-port/tests/lm_burn_baby_vectors.h, yaAGC digests) and refusals on
# the readable routines; then tests/burn_baby_cases.c runs readable and
# faithful side by side, as the host runs them and as the flight program
# does (stopping at the interpreter, -DAGC_FLIGHT_INTERPRETER). The
# faithful object, its tables renamed, gives the oracle the case keys and
# KILLTASK's sites too.
BB_NAMES := lm_burn_baby_routines lm_burn_baby_interpretive
BB_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/lm_burn_baby_cases.c
BB_FLIGHT_OBJECTS := $(BB_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
BB_FLIGHT_OBJECTS := $(filter-out %/burn_baby_routines.o,$(BB_FLIGHT_OBJECTS)) \
	$(BUILD)/bb-flight/burn_baby_routines.o

$(BUILD)/bb-flight/burn_baby_routines.o: src/binding/burn_baby_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/bb-faithful.o: $(PORT)/src/lm_burn_baby.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY \
		$(foreach n,$(BB_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/bb-faithful-flight.o: $(PORT)/src/lm_burn_baby.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(BB_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/bb-oracle: $(BB_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/bb-faithful.o \
		tests/burn_baby_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(BB_COMMON) $(filter %.o,$^) \
		tests/burn_baby_oracle.c -o $@
$(BUILD)/bb-cases: $(BB_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/bb-faithful.o \
		tests/burn_baby_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(BB_COMMON) $(filter %.o,$^) \
		tests/burn_baby_cases.c -o $@
$(BUILD)/bb-cases-flight: $(BB_FLIGHT_OBJECTS) $(BUILD)/bb-faithful-flight.o \
		tests/burn_baby_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DAGC_FLIGHT_INTERPRETER $(BB_COMMON) $(filter %.o,$^) \
		tests/burn_baby_cases.c -o $@

burn-baby-cases: $(BUILD)/bb-oracle $(BUILD)/bb-cases $(BUILD)/bb-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/bb-oracle $(ROPES)
	./$(BUILD)/bb-cases $(ROPES)
	./$(BUILD)/bb-cases-flight $(ROPES)

# THE_LUNAR_LANDING.agc, the LM's P63 set-up and P68:
# tests/lunar_landing_oracle.c runs the port's frozen cases
# (c64-port/tests/lm_lunar_landing_vectors.h, yaAGC digests) and refusals
# on the readable routines; then tests/lunar_landing_cases.c runs readable
# and faithful side by side, as the host runs them and as the flight
# program does (stopping at the interpreter, -DAGC_FLIGHT_INTERPRETER).
# The faithful object, its tables renamed, gives the oracle the case keys.
LLD_NAMES := lm_lunar_landing_routines lm_lunar_landing_interpretive
LLD_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/lm_lunar_landing_cases.c
LLD_FLIGHT_OBJECTS := $(LLD_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
LLD_FLIGHT_OBJECTS := $(filter-out %/lunar_landing_routines.o,$(LLD_FLIGHT_OBJECTS)) \
	$(BUILD)/lld-flight/lunar_landing_routines.o

$(BUILD)/lld-flight/lunar_landing_routines.o: src/binding/lunar_landing_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/lld-faithful.o: $(PORT)/src/lm_lunar_landing.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY \
		$(foreach n,$(LLD_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/lld-faithful-flight.o: $(PORT)/src/lm_lunar_landing.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(LLD_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/lld-oracle: $(LLD_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/lld-faithful.o \
		tests/lunar_landing_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LLD_COMMON) $(filter %.o,$^) \
		tests/lunar_landing_oracle.c -o $@
$(BUILD)/lld-cases: $(LLD_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/lld-faithful.o \
		tests/lunar_landing_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LLD_COMMON) $(filter %.o,$^) \
		tests/lunar_landing_cases.c -o $@
$(BUILD)/lld-cases-flight: $(LLD_FLIGHT_OBJECTS) $(BUILD)/lld-faithful-flight.o \
		tests/lunar_landing_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DAGC_FLIGHT_INTERPRETER $(LLD_COMMON) $(filter %.o,$^) \
		tests/lunar_landing_cases.c -o $@

lunar-landing-cases: $(BUILD)/lld-oracle $(BUILD)/lld-cases $(BUILD)/lld-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/lld-oracle $(ROPES)
	./$(BUILD)/lld-cases $(ROPES)
	./$(BUILD)/lld-cases-flight $(ROPES)

# THROTTLE_CONTROL_ROUTINES.agc, the LM's throttle, with
# LUNAR_LANDING_GUIDANCE_EQUATIONS' FASTCHNG: tests/throttle_oracle.c runs
# the port's frozen cases (c64-port/tests/lm_throttle_vectors.h, yaAGC
# digests) and refusals on the readable routines; then
# tests/throttle_cases.c runs readable and faithful side by side, one call
# each, the faithful table renamed.
THR_COMMON := $(KU_COMMON) $(PORT)/src/rope_image_host.c $(PORT)/tests/lm_throttle_cases.c

$(BUILD)/thr-faithful.o: $(PORT)/src/lm_throttle.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_throttle_routines=faithful_lm_throttle_routines -c $< -o $@
$(BUILD)/thr-oracle: $(THR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) tests/throttle_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(THR_COMMON) $(filter %.o,$^) \
		tests/throttle_oracle.c -o $@
$(BUILD)/thr-cases: $(THR_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/thr-faithful.o \
		tests/throttle_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(THR_COMMON) $(filter %.o,$^) \
		tests/throttle_cases.c -o $@

throttle-cases: $(BUILD)/thr-oracle $(BUILD)/thr-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/thr-oracle $(ROPES)
	./$(BUILD)/thr-cases $(ROPES)

# LANDING_ANALOG_DISPLAYS.agc, the LM's landing analog displays (R10):
# tests/landing_displays_oracle.c runs the port's frozen cases
# (c64-port/tests/lm_landing_displays_vectors.h, yaAGC digests) and
# refusals on the readable routines; then tests/landing_displays_cases.c
# runs readable and faithful side by side, one call each, the faithful
# tables renamed.
LAD_COMMON := $(KU_COMMON) $(PORT)/src/rope_image_host.c $(PORT)/tests/lm_landing_displays_cases.c

$(BUILD)/lad-faithful.o: $(PORT)/src/lm_landing_displays.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_landing_displays_routines=faithful_lm_landing_displays_routines \
		-Dlm_landing_displays_sites=faithful_lm_landing_displays_sites -c $< -o $@
$(BUILD)/lad-oracle: $(LAD_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) tests/landing_displays_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LAD_COMMON) $(filter %.o,$^) \
		tests/landing_displays_oracle.c -o $@
$(BUILD)/lad-cases: $(LAD_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/lad-faithful.o \
		tests/landing_displays_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LAD_COMMON) $(filter %.o,$^) \
		tests/landing_displays_cases.c -o $@

landing-displays-cases: $(BUILD)/lad-oracle $(BUILD)/lad-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/lad-oracle $(ROPES)
	./$(BUILD)/lad-cases $(ROPES)

# FINDCDUW--GUIDAP_INTERFACE.agc, the interface between the LM's powered
# flight guidance and its autopilot: the port's test (tests/test_findcduw.c,
# its frozen cases run to their stops through the program of
# tests/findcduw_target.c, against the original program's yaAGC digests)
# through the readable tables, with tests/with_ropes.c loading the rope the
# readable interpreter reads; then tests/findcduw_cases.c side by side with
# the faithful routines (renamed faithful_).
FCW_NAMES := lm_findcduw_basic lm_findcduw_interpretive
FCW_COMMON := $(R63_COMMON) $(addprefix $(PORT)/src/,lm_findcduw_rtb.c rtb_vector.c rtb_unit.c \
	guidance_unit_phased.c guidance_length_phased.c guidance_norm.c lm_powered_quick.c \
	lm_single_trig.c)

$(BUILD)/fcw-faithful.o: $(PORT)/src/lm_findcduw.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(FCW_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/fcw-test.o: $(PORT)/tests/test_findcduw.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/fcw-oracle: $(FCW_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/fcw-faithful.o \
		$(BUILD)/fcw-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(FCW_COMMON) $(PORT)/tests/findcduw_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/fcw-cases: $(FCW_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/fcw-faithful.o \
		tests/findcduw_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(FCW_COMMON) $(filter %.o,$^) tests/findcduw_cases.c -o $@

findcduw-cases: $(BUILD)/fcw-oracle $(BUILD)/fcw-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/fcw-oracle $(ROPES)
	./$(BUILD)/fcw-cases $(ROPES)

# P51-P53, the IMU alignment programs: the port's
# test (tests/test_p51_p53.c, its frozen cases of both missions run to their
# stops through the programs of tests/p51_p53_target.c, against the original
# programs' yaAGC digests) through the readable LM tables and the faithful
# CM ones, with tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/p51_cases.c side by side with the faithful
# routines (renamed faithful_).
P51_NAMES := lm_p51_p53_basic lm_p51_p53_interpretive
P51_COMMON := $(R63_COMMON) $(PORT)/src/rtb_agreement.c $(PORT)/src/p51_p53_rtb.c

$(BUILD)/p51-faithful.o: $(PORT)/src/lm_p51_p53.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P51_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p51-test.o: $(PORT)/tests/test_p51_p53.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p51-oracle: $(P51_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p51-faithful.o \
		$(BUILD)/p51-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P51_COMMON) $(PORT)/src/cm_p51_p53.c \
		$(PORT)/tests/p51_p53_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p51-cases: $(P51_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p51-faithful.o \
		tests/p51_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P51_COMMON) $(filter %.o,$^) tests/p51_cases.c -o $@

p51-p53-cases: $(BUILD)/p51-oracle $(BUILD)/p51-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p51-oracle $(ROPES)
	./$(BUILD)/p51-cases $(ROPES)

# P51-P53.agc, the CM's alignment programs: the same port test through
# the readable CM tables and the faithful LM ones, with tests/with_ropes.c
# loading the ropes the readable interpreter reads; then
# tests/cm_p51_cases.c side by side with the faithful routines (renamed
# faithful_).
CP51_NAMES := cm_p51_p53_basic cm_p51_p53_interpretive

$(BUILD)/cp51-faithful.o: $(PORT)/src/cm_p51_p53.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP51_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp51-oracle: $(CP51_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp51-faithful.o \
		$(BUILD)/p51-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P51_COMMON) $(PORT)/src/lm_p51_p53.c \
		$(PORT)/tests/p51_p53_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp51-cases: $(CP51_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp51-faithful.o \
		tests/cm_p51_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(P51_COMMON) $(filter %.o,$^) tests/cm_p51_cases.c -o $@

cm-p51-p53-cases: $(BUILD)/cp51-oracle $(BUILD)/cp51-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp51-oracle $(ROPES)
	./$(BUILD)/cp51-cases $(ROPES)

# LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES.agc. The LM's: the port's test
# of its routine table through the transfer trampoline
# (tests/test_lm_ephemeris_routines.c: every sampled entry, step by step,
# must leave the image of the faithful composition, lm_ephemeris(), which
# the port's frozen original words check) with the readable table, then
# tests/lm_ephemerides_cases.c side by side with the faithful table
# (renamed faithful_). The CM's: the port's test (tests/test_cm_ephemeris.c,
# its frozen cases run to their stops through the program of
# tests/cm_ephemeris_target.c, against the original program's yaAGC
# digests) through the readable table, then tests/ephemerides_cases.c side
# by side with the faithful routines (renamed faithful_). tests/with_ropes.c
# loads the ropes the readable interpreter reads.
EPH_NAMES_CM := cm_ephemeris_basic cm_ephemeris_interpretive
EPH_COMMON := $(R63_COMMON)
EPH_LM_PORT := $(addprefix $(PORT)/src/,lm_ephemeris.c ephemeris_time_scale.c ephemeris_add.c \
	ephemeris_day.c ephemeris_shift.c sband_scalar.c sband_trig.c matrix_native.c rtb_unit.c \
	fixed_data.c guidance_dp_divide.c guidance_trig.c guidance_product.c \
	guidance_length_phased.c guidance_unit_phased.c rtb_vector.c guidance_norm.c \
	guidance_shift.c lm_ephemeris_routines.c) $(PORT)/tests/lm_ephemeris_cases.c

$(BUILD)/eph-faithful-lm.o: $(PORT)/src/lm_ephemeris_routine_table.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_ephemeris_routines=faithful_lm_ephemeris_routines -c $< -o $@
$(BUILD)/eph-test-lm.o: $(PORT)/tests/test_lm_ephemeris_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/eph-oracle-lm: $(EPH_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/eph-test-lm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(EPH_COMMON) $(EPH_LM_PORT) tests/with_ropes.c \
		$(filter %.o,$^) -o $@
$(BUILD)/eph-cases-lm: $(EPH_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/eph-faithful-lm.o \
		tests/lm_ephemerides_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(EPH_COMMON) $(EPH_LM_PORT) $(filter %.o,$^) \
		tests/lm_ephemerides_cases.c -o $@

$(BUILD)/eph-faithful-cm.o: $(PORT)/src/cm_ephemeris.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(EPH_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/eph-test-cm.o: $(PORT)/tests/test_cm_ephemeris.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/eph-oracle-cm: $(EPH_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/eph-test-cm.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(EPH_COMMON) $(PORT)/tests/cm_ephemeris_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/eph-cases-cm: $(EPH_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/eph-faithful-cm.o \
		tests/ephemerides_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(EPH_COMMON) $(filter %.o,$^) \
		tests/ephemerides_cases.c -o $@

ephemerides-cases: $(BUILD)/eph-oracle-lm $(BUILD)/eph-cases-lm $(BUILD)/eph-oracle-cm \
		$(BUILD)/eph-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/eph-oracle-lm $(ROPES)
	./$(BUILD)/eph-cases-lm $(ROPES)
	./$(BUILD)/eph-oracle-cm $(ROPES)
	./$(BUILD)/eph-cases-cm $(ROPES)

# P61-P67.agc, the CM's entry programs: the port's test
# (tests/test_cm_p61_p67.c, its frozen cases run to their stops through
# the program of tests/cm_p61_p67_target.c, against the original program's
# yaAGC digests) through the readable tables, with tests/with_ropes.c
# loading the ropes the readable interpreter reads; then
# tests/cm_p61_cases.c side by side with the faithful routines (renamed
# faithful_).
CP61_NAMES := cm_p61_p67_basic cm_p61_p67_interpretive
CP61_COMMON := $(R63_COMMON) $(PORT)/src/cm_integration_moves.c

$(BUILD)/cp61-faithful.o: $(PORT)/src/cm_p61_p67.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP61_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp61-test.o: $(PORT)/tests/test_cm_p61_p67.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp61-oracle: $(CP61_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp61-faithful.o \
		$(BUILD)/cp61-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_p61_p67_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp61-cases: $(CP61_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp61-faithful.o \
		tests/cm_p61_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) tests/cm_p61_cases.c -o $@

cm-p61-p67-cases: $(BUILD)/cp61-oracle $(BUILD)/cp61-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp61-oracle $(ROPES)
	./$(BUILD)/cp61-cases $(ROPES)

# SERVICER207.agc, the CM's average-G loop: the port's test
# (tests/test_cm_servicer.c, its frozen cases run to their stops through
# the program of tests/cm_servicer_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads;
# then tests/cm_servicer_cases.c side by side with the faithful routines
# (renamed faithful_).
CSVC_NAMES := cm_servicer_basic cm_servicer_interpretive

$(BUILD)/csvc-faithful.o: $(PORT)/src/cm_servicer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CSVC_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/csvc-test.o: $(PORT)/tests/test_cm_servicer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/csvc-oracle: $(CSVC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/csvc-faithful.o \
		$(BUILD)/csvc-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/cm_servicer_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/csvc-cases: $(CSVC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/csvc-faithful.o \
		tests/cm_servicer_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(R63_COMMON) $(filter %.o,$^) tests/cm_servicer_cases.c \
		-o $@

cm-servicer-cases: $(BUILD)/csvc-oracle $(BUILD)/csvc-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/csvc-oracle $(ROPES)
	./$(BUILD)/csvc-cases $(ROPES)

# REENTRY_CONTROL.agc, the CM's entry guidance: the port's test
# (tests/test_cm_reentry.c, its frozen cases run to their stops through
# the program of tests/cm_reentry_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads;
# then tests/cm_reentry_cases.c side by side with the faithful routines
# (renamed faithful_).
CRENT_NAMES := cm_reentry_basic cm_reentry_interpretive

$(BUILD)/crent-faithful.o: $(PORT)/src/cm_reentry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CRENT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/crent-test.o: $(PORT)/tests/test_cm_reentry.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/crent-oracle: $(CRENT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crent-faithful.o \
		$(BUILD)/crent-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_reentry_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/crent-cases: $(CRENT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crent-faithful.o \
		tests/cm_reentry_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) tests/cm_reentry_cases.c \
		-o $@

cm-reentry-cases: $(BUILD)/crent-oracle $(BUILD)/crent-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/crent-oracle $(ROPES)
	./$(BUILD)/crent-cases $(ROPES)

# CM_BODY_ATTITUDE.agc, the CM's attitude to the air stream in entry: the
# port's test (tests/test_cm_body_attitude.c, its frozen cases run to
# their stops through the program of tests/cm_body_attitude_target.c,
# against the original program's yaAGC digests) through the readable
# tables, with tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/cm_body_attitude_cases.c side by side with
# the faithful routines (renamed faithful_).
CBAT_NAMES := cm_body_attitude_basic cm_body_attitude_interpretive

$(BUILD)/cbat-faithful.o: $(PORT)/src/cm_body_attitude.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CBAT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cbat-test.o: $(PORT)/tests/test_cm_body_attitude.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cbat-oracle: $(CBAT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cbat-faithful.o \
		$(BUILD)/cbat-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_body_attitude_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cbat-cases: $(CBAT_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cbat-faithful.o \
		tests/cm_body_attitude_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_body_attitude_cases.c -o $@

cm-body-attitude-cases: $(BUILD)/cbat-oracle $(BUILD)/cbat-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cbat-oracle $(ROPES)
	./$(BUILD)/cbat-cases $(ROPES)

# P37_P70.agc, the CM's return to earth: the port's test
# (tests/test_cm_p37_p70.c, its frozen cases run to their stops through
# the program of tests/cm_p37_p70_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads;
# then tests/cm_p37_p70_cases.c side by side with the faithful routines
# (renamed faithful_).
CP37_NAMES := cm_p37_p70_basic cm_p37_p70_interpretive
CP37_COMMON := $(CP61_COMMON) $(PORT)/src/rtb_modes.c

$(BUILD)/cp37-faithful.o: $(PORT)/src/cm_p37_p70.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP37_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp37-test.o: $(PORT)/tests/test_cm_p37_p70.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp37-oracle: $(CP37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp37-faithful.o \
		$(BUILD)/cp37-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP37_COMMON) $(PORT)/tests/cm_p37_p70_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp37-cases: $(CP37_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp37-faithful.o \
		tests/cm_p37_p70_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP37_COMMON) $(filter %.o,$^) \
		tests/cm_p37_p70_cases.c -o $@

cm-p37-p70-cases: $(BUILD)/cp37-oracle $(BUILD)/cp37-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp37-oracle $(ROPES)
	./$(BUILD)/cp37-cases $(ROPES)

# TVCINITIALIZE.agc, the start of the CM's TVC DAP: the port's test
# (tests/test_cm_tvc_init.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_init_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_init_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCI_NAMES := cm_tvc_init_basic cm_tvc_init_interpretive

$(BUILD)/ctvci-faithful.o: $(PORT)/src/cm_tvc_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCI_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvci-test.o: $(PORT)/tests/test_cm_tvc_init.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvci-oracle: $(CTVCI_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvci-faithful.o \
		$(BUILD)/ctvci-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_init_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvci-cases: $(CTVCI_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvci-faithful.o \
		tests/cm_tvc_init_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_init_cases.c -o $@

cm-tvc-init-cases: $(BUILD)/ctvci-oracle $(BUILD)/ctvci-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvci-oracle $(ROPES)
	./$(BUILD)/ctvci-cases $(ROPES)

# TVCEXECUTIVE.agc, the TVC DAP's half-second task: the port's test
# (tests/test_cm_tvc_exec.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_exec_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_exec_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCX_NAMES := cm_tvc_exec_basic cm_tvc_exec_interpretive

$(BUILD)/ctvcx-faithful.o: $(PORT)/src/cm_tvc_exec.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCX_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcx-test.o: $(PORT)/tests/test_cm_tvc_exec.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcx-oracle: $(CTVCX_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcx-faithful.o \
		$(BUILD)/ctvcx-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_exec_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcx-cases: $(CTVCX_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcx-faithful.o \
		tests/cm_tvc_exec_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_exec_cases.c -o $@

cm-tvc-exec-cases: $(BUILD)/ctvcx-oracle $(BUILD)/ctvcx-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcx-oracle $(ROPES)
	./$(BUILD)/ctvcx-cases $(ROPES)

# TVCMASSPROP.agc, the CSM's mass properties for the TVC DAP: the port's test
# (tests/test_cm_tvc_massprop.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_massprop_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_massprop_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCM_NAMES := cm_tvc_massprop_basic cm_tvc_massprop_interpretive

$(BUILD)/ctvcm-faithful.o: $(PORT)/src/cm_tvc_massprop.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcm-test.o: $(PORT)/tests/test_cm_tvc_massprop.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcm-oracle: $(CTVCM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcm-faithful.o \
		$(BUILD)/ctvcm-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_massprop_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcm-cases: $(CTVCM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcm-faithful.o \
		tests/cm_tvc_massprop_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_massprop_cases.c -o $@

cm-tvc-massprop-cases: $(BUILD)/ctvcm-oracle $(BUILD)/ctvcm-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcm-oracle $(ROPES)
	./$(BUILD)/ctvcm-cases $(ROPES)

# TVCRESTARTS.agc, the TVC restart package: the port's test
# (tests/test_cm_tvc_restarts.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_restarts_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_restarts_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCR_NAMES := cm_tvc_restarts_basic cm_tvc_restarts_interpretive

$(BUILD)/ctvcr-faithful.o: $(PORT)/src/cm_tvc_restarts.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCR_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcr-test.o: $(PORT)/tests/test_cm_tvc_restarts.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcr-oracle: $(CTVCR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcr-faithful.o \
		$(BUILD)/ctvcr-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_restarts_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcr-cases: $(CTVCR_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcr-faithful.o \
		tests/cm_tvc_restarts_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_restarts_cases.c -o $@

cm-tvc-restarts-cases: $(BUILD)/ctvcr-oracle $(BUILD)/ctvcr-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcr-oracle $(ROPES)
	./$(BUILD)/ctvcr-cases $(ROPES)

# TVCDAPS.agc, the pitch and yaw TVC DAPs: the port's test
# (tests/test_cm_tvc_daps.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_daps_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_daps_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCD_NAMES := cm_tvc_daps_basic cm_tvc_daps_interpretive

$(BUILD)/ctvcd-faithful.o: $(PORT)/src/cm_tvc_daps.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCD_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcd-test.o: $(PORT)/tests/test_cm_tvc_daps.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcd-oracle: $(CTVCD_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcd-faithful.o \
		$(BUILD)/ctvcd-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_daps_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcd-cases: $(CTVCD_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcd-faithful.o \
		tests/cm_tvc_daps_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_daps_cases.c -o $@

cm-tvc-daps-cases: $(BUILD)/ctvcd-oracle $(BUILD)/ctvcd-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcd-oracle $(ROPES)
	./$(BUILD)/ctvcd-cases $(ROPES)

# TVCSTROKETEST.agc, the stroke test: the port's test
# (tests/test_cm_tvc_stroke.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_stroke_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_stroke_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCS_NAMES := cm_tvc_stroke_basic cm_tvc_stroke_interpretive

$(BUILD)/ctvcs-faithful.o: $(PORT)/src/cm_tvc_stroke.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcs-test.o: $(PORT)/tests/test_cm_tvc_stroke.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcs-oracle: $(CTVCS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcs-faithful.o \
		$(BUILD)/ctvcs-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_stroke_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcs-cases: $(CTVCS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcs-faithful.o \
		tests/cm_tvc_stroke_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_stroke_cases.c -o $@

cm-tvc-stroke-cases: $(BUILD)/ctvcs-oracle $(BUILD)/ctvcs-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcs-oracle $(ROPES)
	./$(BUILD)/ctvcs-cases $(ROPES)

# TVCROLLDAP.agc, the TVC roll autopilot: the port's test
# (tests/test_cm_tvc_roll.c, its frozen cases run to their stops through
# the program of tests/cm_tvc_roll_target.c, against the original
# program's yaAGC digests) through the readable table, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tvc_roll_cases.c side by side with the faithful routines
# (renamed faithful_).
CTVCL_NAMES := cm_tvc_roll_basic cm_tvc_roll_interpretive

$(BUILD)/ctvcl-faithful.o: $(PORT)/src/cm_tvc_roll.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTVCL_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctvcl-test.o: $(PORT)/tests/test_cm_tvc_roll.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctvcl-oracle: $(CTVCL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcl-faithful.o \
		$(BUILD)/ctvcl-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_tvc_roll_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctvcl-cases: $(CTVCL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctvcl-faithful.o \
		tests/cm_tvc_roll_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_tvc_roll_cases.c -o $@

cm-tvc-roll-cases: $(BUILD)/ctvcl-oracle $(BUILD)/ctvcl-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctvcl-oracle $(ROPES)
	./$(BUILD)/ctvcl-cases $(ROPES)

# MYSUBS.agc, the CM DAP's SMALLMP and DPADD: the port's frozen cases
# (c64-port/tests/test_cm_mysubs.c, states of the original program) on the
# readable routines (tests/cm_mysubs_oracle.c), then tests/cm_mysubs_cases.c
# side by side with the faithful routines (renamed faithful_).
$(BUILD)/cmys-faithful.o: $(PORT)/src/cm_mysubs_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dcm_mysubs_routines=faithful_cm_mysubs_routines -c $< -o $@
$(BUILD)/cmys-oracle: $(CMYS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) tests/cm_mysubs_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SP_COMMON) $(PORT)/tests/cm_mysubs_cases.c \
		$(filter %.o,$^) tests/cm_mysubs_oracle.c -o $@
$(BUILD)/cmys-cases: $(CMYS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cmys-faithful.o \
		tests/cm_mysubs_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SP_COMMON) $(PORT)/src/cm_mysubs.c \
		$(filter %.o,$^) tests/cm_mysubs_cases.c -o $@

cm-mysubs-cases: $(BUILD)/cmys-oracle $(BUILD)/cmys-cases
	./$(BUILD)/cmys-oracle
	./$(BUILD)/cmys-cases

# RCS-CSM_DIGITAL_AUTOPILOT.agc, the CM's RCS autopilot: the port's test
# (tests/test_cm_rcs_dap.c, its frozen cases run to their stops through the
# program of tests/cm_rcs_dap_target.c, against the original program's
# yaAGC digests) through the readable tables, with tests/with_ropes.c
# loading the ropes the readable code reads; then tests/cm_rcs_dap_cases.c
# side by side with the faithful routines (renamed faithful_).
CRCS_NAMES := cm_rcs_dap_basic cm_rcs_dap_interpretive cm_rcs_dap_hot_routines
CRCS_FAST := $(PORT)/src/agc_fast_ops.c $(PORT)/src/agc_fast_window.c

$(BUILD)/crcs-faithful.o: $(PORT)/src/cm_rcs_dap.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CRCS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/crcs-test.o: $(PORT)/tests/test_cm_rcs_dap.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/crcs-oracle: $(CRCS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crcs-faithful.o \
		$(BUILD)/crcs-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(CRCS_FAST) $(PORT)/tests/cm_rcs_dap_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/crcs-cases: $(CRCS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crcs-faithful.o \
		tests/cm_rcs_dap_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(CRCS_FAST) $(filter %.o,$^) \
		tests/cm_rcs_dap_cases.c -o $@

cm-rcs-dap-cases: $(BUILD)/crcs-oracle $(BUILD)/crcs-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/crcs-oracle $(ROPES)
	./$(BUILD)/crcs-cases $(ROPES)

# AUTOMATIC_MANEUVERS.agc, the CM's attitude hold and jet switching logic:
# the port's test (tests/test_cm_auto_man.c, its frozen cases run to their
# stops through the program of tests/cm_auto_man_target.c, against the
# original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_auto_man_cases.c side by side with the faithful routines
# (renamed faithful_).
CAM_NAMES := cm_auto_man_basic cm_auto_man_interpretive cm_auto_man_hot_routines

$(BUILD)/cam-faithful.o: $(PORT)/src/cm_auto_man.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CAM_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cam-test.o: $(PORT)/tests/test_cm_auto_man.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cam-oracle: $(CAM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cam-faithful.o \
		$(BUILD)/cam-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(CRCS_FAST) $(PORT)/tests/cm_auto_man_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cam-cases: $(CAM_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cam-faithful.o \
		tests/cm_auto_man_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(CRCS_FAST) $(filter %.o,$^) \
		tests/cm_auto_man_cases.c -o $@

cm-auto-man-cases: $(BUILD)/cam-oracle $(BUILD)/cam-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cam-oracle $(ROPES)
	./$(BUILD)/cam-cases $(ROPES)

# RCS-CSM_DAP_EXECUTIVE_PROGRAMS.agc, the CM's once-a-second AMGB update:
# the port's test (tests/test_cm_rcs_dap_exec.c, its frozen cases run to
# their stops through the program of tests/cm_rcs_dap_exec_target.c,
# against the original program's yaAGC digests) through the readable
# table, with tests/with_ropes.c loading the ropes the readable code reads;
# then tests/cm_rcs_dap_exec_cases.c side by side with the faithful
# routines (renamed faithful_).
CRDX_NAMES := cm_rcs_dap_exec_basic cm_rcs_dap_exec_interpretive

$(BUILD)/crdx-faithful.o: $(PORT)/src/cm_rcs_dap_exec.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CRDX_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/crdx-test.o: $(PORT)/tests/test_cm_rcs_dap_exec.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/crdx-oracle: $(CRDX_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crdx-faithful.o \
		$(BUILD)/crdx-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_rcs_dap_exec_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/crdx-cases: $(CRDX_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/crdx-faithful.o \
		tests/cm_rcs_dap_exec_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_rcs_dap_exec_cases.c -o $@

cm-rcs-dap-exec-cases: $(BUILD)/crdx-oracle $(BUILD)/crdx-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/crdx-oracle $(ROPES)
	./$(BUILD)/crdx-cases $(ROPES)

# JET_SELECTION_LOGIC.agc, phase 3 of the CM's RCS autopilot, part by part:
# the port's test (tests/test_cm_jet_select.c, its frozen cases run to
# their stops through the program of tests/cm_jet_select_target.c, against
# the original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_jet_select_cases.c side by side with the faithful routines
# (renamed faithful_, which run the keys not yet written).
CJS_NAMES := cm_jet_select_basic cm_jet_select_interpretive cm_jet_select_hot_routines

$(BUILD)/cjs-faithful.o: $(PORT)/src/cm_jet_select.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CJS_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cjs-test.o: $(PORT)/tests/test_cm_jet_select.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cjs-oracle: $(CJS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cjs-faithful.o \
		$(BUILD)/cjs-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP61_COMMON) $(PORT)/tests/cm_jet_select_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cjs-cases: $(CJS_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cjs-faithful.o \
		tests/cm_jet_select_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP61_COMMON) $(filter %.o,$^) \
		tests/cm_jet_select_cases.c -o $@

cm-jet-select-cases: $(BUILD)/cjs-oracle $(BUILD)/cjs-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cjs-oracle $(ROPES)
	./$(BUILD)/cjs-cases $(ROPES)

# IMU_CALIBRATION_AND_ALIGNMENT.agc, the CM's IMU performance tests and
# prelaunch alignment (written part by part): the port's test
# (tests/test_cm_imu_calibration.c, its frozen cases run to their stops
# through the program of tests/cm_imu_calibration_target.c, against the
# original program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_imu_cal_cases.c side by side with the faithful routines (renamed
# faithful_, which also run the keys no part has written yet).
CIC_NAMES := cm_imu_calibration_basic cm_imu_calibration_interpretive
CIC_COMMON := $(R63_COMMON) $(PORT)/src/agc_fast_ops.c $(PORT)/src/agc_fast_window.c

$(BUILD)/cic-faithful.o: $(PORT)/src/cm_imu_calibration.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CIC_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cic-test.o: $(PORT)/tests/test_cm_imu_calibration.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cic-oracle: $(CIC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cic-faithful.o \
		$(BUILD)/cic-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CIC_COMMON) $(PORT)/tests/cm_imu_calibration_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cic-cases: $(CIC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cic-faithful.o \
		tests/cm_imu_cal_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CIC_COMMON) $(filter %.o,$^) \
		tests/cm_imu_cal_cases.c -o $@

cm-imu-cal-cases: $(BUILD)/cic-oracle $(BUILD)/cic-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cic-oracle $(ROPES)
	./$(BUILD)/cic-cases $(ROPES)

# P11.agc, the CM's earth orbit insertion monitor and Saturn takeover
# (written part by part): the port's test (tests/test_cm_p11.c, its frozen
# cases run to their stops through the program of tests/cm_p11_target.c,
# against the original program's yaAGC digests) through the readable
# tables, with tests/with_ropes.c loading the ropes the readable code
# reads; then tests/cm_p11_cases.c side by side with the faithful routines
# (renamed faithful_, which also run the keys no part has written yet).
CP11_NAMES := cm_p11_basic cm_p11_interpretive
CP11_COMMON := $(R63_COMMON)

$(BUILD)/cp11-faithful.o: $(PORT)/src/cm_p11.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP11_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp11-test.o: $(PORT)/tests/test_cm_p11.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp11-oracle: $(CP11_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp11-faithful.o \
		$(BUILD)/cp11-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CP11_COMMON) $(PORT)/tests/cm_p11_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp11-cases: $(CP11_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp11-faithful.o \
		tests/cm_p11_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CP11_COMMON) $(filter %.o,$^) \
		tests/cm_p11_cases.c -o $@

cm-p11-cases: $(BUILD)/cp11-oracle $(BUILD)/cp11-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp11-oracle $(ROPES)
	./$(BUILD)/cp11-cases $(ROPES)

# TPI_SEARCH.agc, the CM's TPI search (written part by part): the port's
# test (tests/test_cm_tpi_search.c, its frozen cases run to their stops
# through the program of tests/cm_tpi_search_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable code reads; then
# tests/cm_tpi_search_cases.c side by side with the faithful routines
# (renamed faithful_, which also run the keys no part has written yet).
CTPI_NAMES := cm_tpi_search_basic cm_tpi_search_interpretive
CTPI_COMMON := $(R63_COMMON)

$(BUILD)/ctpi-faithful.o: $(PORT)/src/cm_tpi_search.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CTPI_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ctpi-test.o: $(PORT)/tests/test_cm_tpi_search.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ctpi-oracle: $(CTPI_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctpi-faithful.o \
		$(BUILD)/ctpi-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CTPI_COMMON) $(PORT)/tests/cm_tpi_search_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ctpi-cases: $(CTPI_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ctpi-faithful.o \
		tests/cm_tpi_search_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CTPI_COMMON) $(filter %.o,$^) \
		tests/cm_tpi_search_cases.c -o $@

cm-tpi-search-cases: $(BUILD)/ctpi-oracle $(BUILD)/ctpi-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ctpi-oracle $(ROPES)
	./$(BUILD)/ctpi-cases $(ROPES)

# CM_ENTRY_DIGITAL_AUTOPILOT.agc, the CM's entry DAP (written part by
# part): the port's test (tests/test_cm_entry_dap.c, its frozen cases run
# to their stops through the program of tests/cm_entry_dap_target.c,
# against the original program's yaAGC digests) through the readable
# tables, with tests/with_ropes.c loading the ropes the readable code
# reads; then tests/cm_entry_dap_cases.c side by side with the faithful
# routines (renamed faithful_, which run the keys not yet written).
CED_NAMES := cm_entry_dap_basic cm_entry_dap_interpretive cm_entry_dap_hot_routines
CED_COMMON := $(R63_COMMON) $(PORT)/src/agc_fast_ops.c $(PORT)/src/agc_fast_window.c

$(BUILD)/ced-faithful.o: $(PORT)/src/cm_entry_dap.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CED_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ced-test.o: $(PORT)/tests/test_cm_entry_dap.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ced-oracle: $(CED_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ced-faithful.o \
		$(BUILD)/ced-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CED_COMMON) $(PORT)/tests/cm_entry_dap_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ced-cases: $(CED_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ced-faithful.o \
		tests/cm_entry_dap_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(CED_COMMON) $(filter %.o,$^) \
		tests/cm_entry_dap_cases.c -o $@

cm-entry-dap-cases: $(BUILD)/ced-oracle $(BUILD)/ced-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ced-oracle $(ROPES)
	./$(BUILD)/ced-cases $(ROPES)

# FIXED_FIXED_CONSTANT_POOL.agc, the constants every bank reaches, and
# INTERPRETIVE_CONSTANT(S).agc, the interpreter's in each half of the rope
# (data only): tests/constant_pool_cases.c, once per mission, checks each
# word laid out against the assembled rope and each label against the
# listing.
$(BUILD)/constant-pool-cases-lm: $(FFCP_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		tests/constant_pool_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(PORT)/src/rope_image_host.c $(filter %.o,$^) \
		tests/constant_pool_cases.c -o $@
$(BUILD)/constant-pool-cases-cm: $(FFCP_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		tests/constant_pool_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(PORT)/src/rope_image_host.c $(filter %.o,$^) \
		tests/constant_pool_cases.c -o $@

constant-pool-cases: $(BUILD)/constant-pool-cases-lm $(BUILD)/constant-pool-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/constant-pool-cases-lm $(PORT)/build/fixed-data/Luminary099/MAIN.agc.bin \
		../Luminary099/FIXED_FIXED_CONSTANT_POOL.agc ../Luminary099/INTERPRETIVE_CONSTANT.agc
	./$(BUILD)/constant-pool-cases-cm $(PORT)/build/fixed-data/Comanche055/MAIN.agc.bin \
		../Comanche055/FIXED_FIXED_CONSTANT_POOL.agc ../Comanche055/INTERPRETIVE_CONSTANTS.agc

# LATITUDE_LONGITUDE_SUBROUTINES.agc, both missions: the port's test
# (tests/test_lat_long.c, its frozen cases run to their stops through the
# program of tests/lat_long_target.c, against the original program's yaAGC
# digests) with each mission's readable routines and the other mission's
# faithful ones, tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/lat_long_cases.c side by side, for each
# mission.
LL_NAMES_LM := lm_lat_long_basic lm_lat_long_interpretive
LL_NAMES_CM := cm_lat_long_basic cm_lat_long_interpretive
LL_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c \
	$(PORT)/src/cm_interp_orders.c $(PORT)/src/cm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/interp_case.c

$(BUILD)/ll-faithful-lm.o: $(PORT)/src/lm_lat_long.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(LL_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ll-faithful-cm.o: $(PORT)/src/cm_lat_long.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(LL_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ll-test.o: $(PORT)/tests/test_lat_long.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ll-oracle-lm: $(LL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ll-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LL_COMMON) $(PORT)/src/cm_lat_long.c \
		$(PORT)/tests/lat_long_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ll-oracle-cm: $(LL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ll-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LL_COMMON) $(PORT)/src/lm_lat_long.c \
		$(PORT)/tests/lat_long_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ll-cases-lm: $(LL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ll-faithful-lm.o \
		tests/lat_long_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LL_COMMON) $(filter %.o,$^) \
		tests/lat_long_cases.c -o $@
$(BUILD)/ll-cases-cm: $(LL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/ll-faithful-cm.o \
		tests/lat_long_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(LL_COMMON) $(filter %.o,$^) \
		tests/lat_long_cases.c -o $@

lat-long-cases: $(BUILD)/ll-oracle-lm $(BUILD)/ll-oracle-cm $(BUILD)/ll-cases-lm \
		$(BUILD)/ll-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ll-oracle-lm $(ROPES)
	./$(BUILD)/ll-oracle-cm $(ROPES)
	./$(BUILD)/ll-cases-lm $(ROPES)
	./$(BUILD)/ll-cases-cm $(ROPES)

# PLANETARY_INERTIAL_ORIENTATION.agc, both missions: the port's test
# (tests/test_planetary.c, its frozen cases run to their stops through the
# program of tests/planetary_target.c, against the original program's
# yaAGC digests) with each mission's readable routines and the other
# mission's faithful ones, tests/with_ropes.c loading the ropes the
# readable interpreter reads; then tests/planetary_cases.c side by side,
# for each mission.
PL_NAMES_LM := lm_planetary_basic lm_planetary_interpretive
PL_NAMES_CM := cm_planetary_basic cm_planetary_interpretive

$(BUILD)/pl-faithful-lm.o: $(PORT)/src/lm_planetary.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(PL_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/pl-faithful-cm.o: $(PORT)/src/cm_planetary.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(PL_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/pl-test.o: $(PORT)/tests/test_planetary.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/pl-oracle-lm: $(PL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/pl-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LL_COMMON) $(PORT)/src/cm_planetary.c \
		$(PORT)/tests/planetary_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/pl-oracle-cm: $(PL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/pl-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LL_COMMON) $(PORT)/src/lm_planetary.c \
		$(PORT)/tests/planetary_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/pl-cases-lm: $(PL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/pl-faithful-lm.o \
		tests/planetary_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LL_COMMON) $(filter %.o,$^) \
		tests/planetary_cases.c -o $@
$(BUILD)/pl-cases-cm: $(PL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/pl-faithful-cm.o \
		tests/planetary_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(LL_COMMON) $(filter %.o,$^) \
		tests/planetary_cases.c -o $@

planetary-cases: $(BUILD)/pl-oracle-lm $(BUILD)/pl-oracle-cm $(BUILD)/pl-cases-lm \
		$(BUILD)/pl-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/pl-oracle-lm $(ROPES)
	./$(BUILD)/pl-oracle-cm $(ROPES)
	./$(BUILD)/pl-cases-lm $(ROPES)
	./$(BUILD)/pl-cases-cm $(ROPES)

# MEASUREMENT_INCORPORATION.agc, both missions: the port's test
# (tests/test_measurement_inc.c, its frozen cases run to their stops
# through the program of tests/measurement_inc_target.c, against the
# original program's yaAGC digests) with each mission's readable routines
# and the other mission's faithful ones, tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/measurement_inc_cases.c
# side by side, for each mission. The faithful routines run the state
# vector moves (c64-port/src/lm_integration_moves.c and
# cm_integration_moves.c) and TPMODE (rtb_modes.c) from their RTBs.
MINC_NAMES_LM := lm_measurement_inc_basic lm_measurement_inc_interpretive
MINC_NAMES_CM := cm_measurement_inc_basic cm_measurement_inc_interpretive
MINC_COMMON := $(LL_COMMON) $(PORT)/src/rtb_angles.c $(PORT)/src/rtb_modes.c \
	$(PORT)/src/lm_integration_moves.c $(PORT)/src/cm_integration_moves.c

$(BUILD)/minc-faithful-lm.o: $(PORT)/src/lm_measurement_inc.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(MINC_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/minc-faithful-cm.o: $(PORT)/src/cm_measurement_inc.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(MINC_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/minc-test.o: $(PORT)/tests/test_measurement_inc.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/minc-oracle-lm: $(MINC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/minc-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(MINC_COMMON) $(PORT)/src/cm_measurement_inc.c \
		$(PORT)/tests/measurement_inc_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/minc-oracle-cm: $(MINC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/minc-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(MINC_COMMON) $(PORT)/src/lm_measurement_inc.c \
		$(PORT)/tests/measurement_inc_target.c tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/minc-cases-lm: $(MINC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) \
		$(BUILD)/minc-faithful-lm.o tests/measurement_inc_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(MINC_COMMON) $(filter %.o,$^) \
		tests/measurement_inc_cases.c -o $@
$(BUILD)/minc-cases-cm: $(MINC_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) \
		$(BUILD)/minc-faithful-cm.o tests/measurement_inc_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(MINC_COMMON) $(filter %.o,$^) \
		tests/measurement_inc_cases.c -o $@

measurement-inc-cases: $(BUILD)/minc-oracle-lm $(BUILD)/minc-oracle-cm $(BUILD)/minc-cases-lm \
		$(BUILD)/minc-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/minc-oracle-lm $(ROPES)
	./$(BUILD)/minc-oracle-cm $(ROPES)
	./$(BUILD)/minc-cases-lm $(ROPES)
	./$(BUILD)/minc-cases-cm $(ROPES)

# LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, the LM's landing guidance: the
# port's test (tests/test_lm_llge.c, its frozen cases run to their stops
# through the program of tests/lm_llge_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/llge_cases.c side by side with the faithful routines (renamed
# faithful_).
LLGE_NAMES := lm_llge_basic lm_llge_interpretive
LLGE_COMMON := $(R63_COMMON) $(addprefix $(PORT)/src/,lm_normunit.c rtb_vector.c rtb_unit.c \
	guidance_unit_phased.c guidance_length_phased.c guidance_norm.c guidance_product.c)

$(BUILD)/llge-faithful.o: $(PORT)/src/lm_llge.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(LLGE_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/llge-test.o: $(PORT)/tests/test_lm_llge.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/llge-oracle: $(LLGE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/llge-faithful.o \
		$(BUILD)/llge-test.o tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LLGE_COMMON) $(PORT)/tests/lm_llge_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/llge-cases: $(LLGE_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/llge-faithful.o \
		tests/llge_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(LLGE_COMMON) $(filter %.o,$^) tests/llge_cases.c -o $@

llge-cases: $(BUILD)/llge-oracle $(BUILD)/llge-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/llge-oracle $(ROPES)
	./$(BUILD)/llge-cases $(ROPES)

# P70-P71.agc, the LM's aborts: the port's test (tests/test_lm_p70_p71_file.c,
# its frozen cases run to their stops through the program of
# tests/lm_p70_p71_file_target.c, against the original program's yaAGC
# digests) through the readable tables, with tests/with_ropes.c loading the
# rope the readable interpreter reads; then tests/p70_cases.c side by side
# with the faithful routines (renamed faithful_), with the older module's
# tables (renamed too) for their keys.
P70_NAMES := lm_p70_p71_file_basic lm_p70_p71_file_interpretive
P70_COMMON := $(R63_COMMON)

$(BUILD)/p70-faithful.o: $(PORT)/src/lm_p70_p71_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P70_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p70-older.o: $(PORT)/src/lm_p70_p71.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_p70_p71_basic=faithful_lm_p70_p71_basic \
		-Dlm_p70_p71_interpretive=faithful_lm_p70_p71_interpretive -c $< -o $@
$(BUILD)/p70-test.o: $(PORT)/tests/test_lm_p70_p71_file.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p70-oracle: $(P70_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p70-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(P70_COMMON) $(PORT)/tests/lm_p70_p71_file_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p70-cases: $(P70_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p70-faithful.o \
		$(BUILD)/p70-older.o tests/p70_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(P70_COMMON) $(filter %.o,$^) tests/p70_cases.c -o $@

p70-cases: $(BUILD)/p70-oracle $(BUILD)/p70-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p70-oracle $(ROPES)
	./$(BUILD)/p70-cases $(ROPES)

# P12.agc, the LM's powered ascent: the port's test (tests/test_lm_p12.c,
# its frozen cases run through the program of tests/lm_p12_target.c,
# against the original program's yaAGC digests) through the readable
# tables, with tests/with_ropes.c loading the rope the readable interpreter
# reads; then tests/p12_cases.c side by side with the faithful routines
# (renamed faithful_).
P12_NAMES := lm_p12_basic lm_p12_interpretive

$(BUILD)/p12-faithful.o: $(PORT)/src/lm_p12.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(P12_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/p12-test.o: $(PORT)/tests/test_lm_p12.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p12-oracle: $(P12_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p12-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/lm_p12_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p12-cases: $(P12_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p12-faithful.o \
		tests/p12_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) tests/p12_cases.c -o $@

p12-cases: $(BUILD)/p12-oracle $(BUILD)/p12-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p12-oracle $(ROPES)
	./$(BUILD)/p12-cases $(ROPES)

# ASCENT_GUIDANCE.agc, the LM's ascent guidance: the port's test
# (tests/test_lm_ascent.c, its frozen cases run through the program of
# tests/lm_ascent_target.c, against the original program's yaAGC digests)
# through the readable tables, with tests/with_ropes.c loading the rope the
# readable interpreter reads; then tests/ascent_cases.c side by side with
# the faithful routines (renamed faithful_).
ASC_NAMES := lm_ascent_basic lm_ascent_interpretive

$(BUILD)/ascent-faithful.o: $(PORT)/src/lm_ascent.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(ASC_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ascent-test.o: $(PORT)/tests/test_lm_ascent.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/ascent-oracle: $(ASC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ascent-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/lm_ascent_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ascent-cases: $(ASC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ascent-faithful.o \
		tests/ascent_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) tests/ascent_cases.c -o $@

ascent-cases: $(BUILD)/ascent-oracle $(BUILD)/ascent-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ascent-oracle $(ROPES)
	./$(BUILD)/ascent-cases $(ROPES)

# SERVICER.agc, the LM's average-G loop: tests/servicer_oracle.c runs the
# port's frozen cases (c64-port/tests/lm_servicer_vectors.h, yaAGC
# digests) and refusals on the readable routines; then
# tests/servicer_cases.c runs readable and faithful side by side, as the
# host runs them and as the flight program does (stopping at the
# interpreter, -DAGC_FLIGHT_INTERPRETER). The faithful object, its tables
# renamed, gives the oracle the case keys, and runs the part of the file
# that is not readable yet.
SVC_NAMES := lm_servicer_routines lm_servicer_interpretive
SVC_COMMON := $(PORT)/src/lm_interp_orders.c $(PORT)/src/lm_interp_steps.c $(KU_COMMON) \
	$(PORT)/src/rope_image_host.c $(PORT)/tests/lm_servicer_cases.c
SVC_FLIGHT_OBJECTS := $(SVC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)
SVC_FLIGHT_OBJECTS := $(filter-out %/servicer_routines.o,$(SVC_FLIGHT_OBJECTS)) \
	$(BUILD)/svc-flight/servicer_routines.o

$(BUILD)/svc-flight/servicer_routines.o: src/binding/servicer_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/svc-faithful.o: $(PORT)/src/lm_servicer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY \
		$(foreach n,$(SVC_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/svc-faithful-flight.o: $(PORT)/src/lm_servicer.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -DP66_NATIVE_DISPATCH_ONLY -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(SVC_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/svc-oracle: $(SVC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/svc-faithful.o \
		tests/servicer_oracle.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SVC_COMMON) $(filter %.o,$^) \
		tests/servicer_oracle.c -o $@
$(BUILD)/svc-cases: $(SVC_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/svc-faithful.o \
		tests/servicer_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SVC_COMMON) $(filter %.o,$^) \
		tests/servicer_cases.c -o $@
$(BUILD)/svc-cases-flight: $(SVC_FLIGHT_OBJECTS) $(BUILD)/svc-faithful-flight.o \
		tests/servicer_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) -DAGC_FLIGHT_INTERPRETER $(SVC_COMMON) $(filter %.o,$^) \
		tests/servicer_cases.c -o $@

servicer-cases: $(BUILD)/svc-oracle $(BUILD)/svc-cases $(BUILD)/svc-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/svc-oracle $(ROPES)
	./$(BUILD)/svc-cases $(ROPES)
	./$(BUILD)/svc-cases-flight $(ROPES)

# P40-P47.agc, the LM's burn programs: the port's test
# (tests/test_lm_p40_p47.c, its frozen cases run to their stops through
# the program of tests/lm_p40_p47_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the ropes the readable interpreter reads;
# then tests/p40_cases.c side by side with the faithful routines (renamed
# faithful_).
$(BUILD)/p40-faithful.o: $(PORT)/src/lm_p40_p47.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dlm_p40_p47_basic=faithful_lm_p40_p47_basic \
		-Dlm_p40_p47_interpretive=faithful_lm_p40_p47_interpretive -c $< -o $@
$(BUILD)/p40-test.o: $(PORT)/tests/test_lm_p40_p47.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/p40-oracle: $(P40_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p40-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/lm_p40_p47_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/p40-cases: $(P40_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/p40-faithful.o \
		tests/p40_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(R63_COMMON) $(filter %.o,$^) tests/p40_cases.c -o $@

p40-cases: $(BUILD)/p40-oracle $(BUILD)/p40-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/p40-oracle $(ROPES)
	./$(BUILD)/p40-cases $(ROPES)

# P40-P47.agc, the CM's burn programs: the port's test
# (tests/test_cm_p40_p47.c, its frozen cases run to their stops through
# the program of tests/cm_p40_p47_target.c, against the original
# program's yaAGC digests) through the readable tables, with
# tests/with_ropes.c loading the rope the readable interpreter reads; then
# tests/cm_p40_cases.c side by side with the faithful routines (renamed
# faithful_).
CP40_NAMES := cm_p40_p47_basic cm_p40_p47_interpretive

$(BUILD)/cp40-faithful.o: $(PORT)/src/cm_p40_p47.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(CP40_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/cp40-test.o: $(PORT)/tests/test_cm_p40_p47.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/cp40-oracle: $(CP40_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp40-test.o \
		tests/with_ropes.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(R63_COMMON) $(PORT)/tests/cm_p40_p47_target.c \
		tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/cp40-cases: $(CP40_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/cp40-faithful.o \
		tests/cm_p40_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(R63_COMMON) $(filter %.o,$^) tests/cm_p40_cases.c -o $@

cm-p40-cases: $(BUILD)/cp40-oracle $(BUILD)/cp40-cases
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/cp40-oracle $(ROPES)
	./$(BUILD)/cp40-cases $(ROPES)

# SYSTEM_TEST_STANDARD_LEAD_INS, both missions: the port's three yaAGC
# oracles with each mission's readable tables and the other mission's
# faithful ones (tests/standard_lead_oracle_cases.c: every entry run to the
# first point outside the file; tests/cm_lead_tests_cases.c: the CM's
# COMPVER, GTSCPSS1 and REDO; tests/interp_entry_cases.c: INTPRET, INTPRET
# +2 and INTRSM), tests/with_ropes.c loading the ropes the readable
# interpreter reads; then tests/standard_lead_cases.c side by side, for each
# mission.
SL_NAMES_LM := standard_lead_routines lm_interpreter_routines standard_lead_interpretive_routines \
	lm_interpreter_rvq
SL_NAMES_CM := cm_standard_lead_routines cm_interpreter_routines \
	cm_standard_lead_interpretive_routines cm_interpreter_rvq
SL_PORT := $(addprefix $(PORT)/src/,agc_transfer.c interp_entry.c standard_lead.c \
	standard_lead_interp.c standard_lead_wake.c lm_basic_wake.c executive.c cm_basic_wake.c \
	cm_native_image.c cm_dispatch.c cm_executive.c lm_interbank_routines.c lm_interbank_idle.c \
	lm_interbank.c cm_interbank_routines.c cm_interbank_idle.c cm_interbank_complete.c agc_word.c \
	agc_multiword.c agc_hardware_divide.c rope_image_host.c) tests/no_seams.c
SLO_TESTS := $(addprefix $(PORT)/tests/,standard_lead_oracle_cases.c standard_lead_oracle_check.c \
	standard_lead_cases.c)
CLT_TESTS := $(addprefix $(PORT)/tests/,cm_lead_tests_cases.c cm_lead_tests_check.c)
IE_TESTS := $(PORT)/tests/interp_entry_cases.c

$(BUILD)/sl-faithful-lm.o: $(PORT)/src/standard_lead_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SL_NAMES_LM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sl-faithful-cm.o: $(PORT)/src/standard_lead_routines.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(foreach n,$(SL_NAMES_CM),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/sl-test-%.o: $(PORT)/tests/test_%.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) -Dmain=port_main -c $< -o $@
$(BUILD)/slo-oracle-lm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sl-faithful-lm.o \
		$(BUILD)/sl-test-standard_lead_oracle.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SL_PORT) $(SLO_TESTS) tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/slo-oracle-cm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sl-faithful-cm.o \
		$(BUILD)/sl-test-standard_lead_oracle.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SL_PORT) $(SLO_TESTS) tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/clt-oracle: $(SL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sl-faithful-cm.o \
		$(BUILD)/sl-test-cm_lead_tests.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SL_PORT) $(CLT_TESTS) tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ie-oracle-lm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sl-faithful-lm.o \
		$(BUILD)/sl-test-interp_entry.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SL_PORT) $(IE_TESTS) tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/ie-oracle-cm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sl-faithful-cm.o \
		$(BUILD)/sl-test-interp_entry.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(SL_PORT) $(IE_TESTS) tests/with_ropes.c $(filter %.o,$^) -o $@
$(BUILD)/sl-cases-lm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/sl-faithful-lm.o \
		tests/standard_lead_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(SL_PORT) $(filter %.o,$^) tests/standard_lead_cases.c \
		-o $@
$(BUILD)/sl-cases-cm: $(SL_SOURCES:src/%.c=$(BUILD)/cases-cm/%.o) $(BUILD)/sl-faithful-cm.o \
		tests/standard_lead_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(CM) $(SL_PORT) $(filter %.o,$^) tests/standard_lead_cases.c \
		-o $@

standard-lead-cases: $(BUILD)/slo-oracle-lm $(BUILD)/slo-oracle-cm $(BUILD)/clt-oracle \
		$(BUILD)/ie-oracle-lm $(BUILD)/ie-oracle-cm $(BUILD)/sl-cases-lm $(BUILD)/sl-cases-cm
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/slo-oracle-lm $(ROPES)
	./$(BUILD)/slo-oracle-cm $(ROPES)
	./$(BUILD)/clt-oracle $(ROPES)
	./$(BUILD)/ie-oracle-lm $(ROPES)
	./$(BUILD)/ie-oracle-cm $(ROPES)
	./$(BUILD)/sl-cases-lm $(ROPES)
	./$(BUILD)/sl-cases-cm $(ROPES)

# IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4, the LM's P07: the
# port's test (tests/test_lm_imu_performance.c: 64,512 cases, every key in
# 384 states, against the original program's yaAGC digests, and its
# refusals) on the readable routines, with tests/with_ropes.c loading the
# ropes the readable interpreter reads; then tests/imu_performance_cases.c,
# readable and faithful side by side, as the host runs them and as the
# flight program does (stopping at the interpreter, -DAGC_FLIGHT_INTERPRETER).
IPT_MK := $(PORT)/tools/mk/lm-imu-performance.mk
IPT_PORT := $(addprefix $(PORT)/,$(filter-out src/lm_imu_performance.c,$(shell sed -n 's/^IPT_SRC = //p' $(IPT_MK)))) \
	$(PORT)/src/rope_image_host.c
IPT_NAMES := lm_imu_performance_routines lm_imu_performance_interpretive ipt_routine_keys
IPT_FLAGS := -DP66_NATIVE_DISPATCH_ONLY
IPT_FLIGHT_OBJECTS := $(filter-out %/imu_performance_routines.o,$(IPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o)) \
	$(BUILD)/ipt-flight/imu_performance_routines.o

$(BUILD)/ipt-flight/imu_performance_routines.o: src/binding/imu_performance_routines.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(READABLE_CFLAGS) $(INCLUDES) $(LM) -DRK_SEAMS=1 -DAGC_FLIGHT_INTERPRETER -c $< -o $@
$(BUILD)/ipt-faithful.o: $(PORT)/src/lm_imu_performance.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(IPT_FLAGS) $(foreach n,$(IPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ipt-faithful-flight.o: $(PORT)/src/lm_imu_performance.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(IPT_FLAGS) -DAGC_FLIGHT_INTERPRETER \
		$(foreach n,$(IPT_NAMES),-D$(n)=faithful_$(n)) -c $< -o $@
$(BUILD)/ipt-test.o: $(PORT)/tests/test_lm_imu_performance.c
	@mkdir -p $(dir $@)
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(IPT_FLAGS) -Dmain=port_main -c $< -o $@
$(BUILD)/ipt-oracle: $(IPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ipt-test.o
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(IPT_FLAGS) $(IPT_PORT) tests/with_ropes.c \
		$(filter %.o,$^) -o $@
$(BUILD)/ipt-cases: $(IPT_SOURCES:src/%.c=$(BUILD)/cases-lm/%.o) $(BUILD)/ipt-faithful.o \
		tests/imu_performance_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(IPT_FLAGS) $(IPT_PORT) $(filter %.o,$^) \
		tests/imu_performance_cases.c -o $@
$(BUILD)/ipt-cases-flight: $(IPT_FLIGHT_OBJECTS) $(BUILD)/ipt-faithful-flight.o \
		tests/imu_performance_cases.c
	$(CC) $(PORT_CFLAGS) $(INCLUDES) $(LM) $(IPT_FLAGS) -DAGC_FLIGHT_INTERPRETER $(IPT_PORT) \
		$(filter %.o,$^) tests/imu_performance_cases.c -o $@

imu-performance-cases: $(BUILD)/ipt-oracle $(BUILD)/ipt-cases $(BUILD)/ipt-cases-flight
	@for r in $(ROPES); do test -f $$r || { echo "no $$r: run c64-port/tools/setup_cloud.sh"; exit 1; }; done
	./$(BUILD)/ipt-oracle $(ROPES)
	./$(BUILD)/ipt-cases $(ROPES)
	./$(BUILD)/ipt-cases-flight $(ROPES)
