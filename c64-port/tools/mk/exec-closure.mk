# SPDX-License-Identifier: MIT
# Milestone exec-closure (lane A): whole-file source maps of EXECUTIVE.agc
# and WAITLIST.agc in both missions, from the exec milestones' own oracle
# runs (tools/exec_closure_source_map.py).
EXC_MILESTONES = lm-exec-services lm-exec-jobs lm-exec-idle cm-exec-services cm-exec-jobs cm-exec-idle
.PHONY: exec-closure-trace exec-closure-source-map test-exec-closure sanitize-exec-closure exec-closure-vice
# Host-only: reruns each milestone's differential with tools/oracle_trace
# first on PATH, so its yaAGC reference records every fixed word it executes
# (build/exec-closure-executed-<milestone>.txt); rupt-lead-ins writes its own.
exec-closure-trace: | build
	for m in $(EXC_MILESTONES); do rm -f build/exec-closure-executed-$$m.txt; \
	 PATH=$(CURDIR)/tools/oracle_trace:$$PATH AGC_EXECUTED_WORDS=$(CURDIR)/build/exec-closure-executed-$$m.txt \
	 $(MAKE) --no-print-directory $$m-differential || exit 1; done
	$(MAKE) --no-print-directory rupt-lead-ins-differential
exec-closure-source-map: exec-closure-trace
	python3 tools/exec_closure_source_map.py
# The native routines' own checks, together. The VICE aggregate is not named
# vice-*: CI runs every vice-* target, and these seven already run on their
# own, so a vice-exec-closure ran them all a second time in one shard.
test-exec-closure: $(addprefix test-,$(EXC_MILESTONES) rupt-lead-ins)
sanitize-exec-closure: $(addprefix sanitize-,$(EXC_MILESTONES) rupt-lead-ins)
exec-closure-vice: $(addprefix vice-,$(EXC_MILESTONES) rupt-lead-ins)
