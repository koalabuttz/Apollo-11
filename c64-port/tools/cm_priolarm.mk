# SPDX-License-Identifier: MIT
CM_PRIOLARM_CORE = src/cm_priolarm.c src/cm_native_image.c src/agc_word.c tests/cm_priolarm_cases.c tests/cm_alarm_record_cases.c tests/cm_admission_complete_cases.c
build/cm-priolarm-native: $(CM_PRIOLARM_CORE) platform/host/cm_priolarm_snapshot.c include/cm_priolarm.h | build
	$(CC) $(CPPFLAGS) -Itools $(CFLAGS) $(CM_PRIOLARM_CORE) platform/host/cm_priolarm_snapshot.c -o $@
cm-priolarm-differential: build/cm-priolarm-native prepare-cm-original-rope
	python3 tools/cm_priolarm_differential.py
.PHONY: test-cm-priolarm sanitize-cm-priolarm
test: test-cm-priolarm
test-cm-priolarm: cm-priolarm-differential
sanitize: sanitize-cm-priolarm
sanitize-cm-priolarm: | build
	$(CC) $(CPPFLAGS) -Itools -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined $(CM_PRIOLARM_CORE) platform/host/cm_priolarm_snapshot.c -o build/cm-priolarm-sanitize
	python3 tools/cm_priolarm_sanitize.py
