LM_INTERPRETER_CHANGE_CORE = src/agc_word.c src/lm_interpreter_change.c
LM_INTERPRETER_CHANGE_TEST = tests/lm_interpreter_change_cases.c tests/test_lm_interpreter_change.c
.PHONY: test-lm-interpreter-change sanitize-lm-interpreter-change differential-lm-interpreter-change c64-lm-interpreter-change
test-lm-interpreter-change:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LM_INTERPRETER_CHANGE_CORE) $(LM_INTERPRETER_CHANGE_TEST) -o build/test-lm-interpreter-change
	./build/test-lm-interpreter-change
sanitize-lm-interpreter-change:
	$(CC) $(CPPFLAGS) -std=c99 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(LM_INTERPRETER_CHANGE_CORE) $(LM_INTERPRETER_CHANGE_TEST) -o build/test-lm-interpreter-change-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-interpreter-change-san
differential-lm-interpreter-change:
	python3 tools/lm_interpreter_change_differential.py
	python3 tools/lm_interpreter_change_differential.py --sanitize
c64-lm-interpreter-change:
	python3 tools/lm_interpreter_change_target_build.py --cc $(MOS_CC) --c1541 $(C1541)
	python3 tools/lm_interpreter_change_vice_test.py --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
