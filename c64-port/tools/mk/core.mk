# SPDX-License-Identifier: MIT
# CI handles for the recipes that `make test` and `make sanitize` run
# directly, so tools/ci_run.py can schedule them like any other target.
.PHONY: test-core sanitize-core
test-core: build/test-host build/test-irq build/test-lifecycle build/test-waitlist
	./build/test-host
	./build/test-irq
	./build/test-lifecycle
	./build/test-waitlist
sanitize-core: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) tests/test_host.c -o build/test-sanitize
	./build/test-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) tests/scenarios.c tests/test_irq.c -o build/test-irq-sanitize
	./build/test-irq-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CORE) $(SCENARIOS) $(LIFECYCLE) tests/lifecycle_check.c tests/test_lifecycle.c -o build/test-lifecycle-sanitize
	./build/test-lifecycle-sanitize
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(TASK_CORE) $(WAIT_TEST) tests/test_waitlist.c -o build/test-waitlist-sanitize
	./build/test-waitlist-sanitize
