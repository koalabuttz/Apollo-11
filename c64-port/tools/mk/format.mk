# Formatting of changed C lines (tools/format_changed.py, .clang-format).
# `make format` formats what you changed since HEAD and installs the
# pre-commit hook that does it on every commit; `make format-check
# FORMAT_BASE=<commit>` reports what a branch left unformatted.
FORMAT_BASE ?= HEAD
.PHONY: format format-check hooks
hooks:
	git config core.hooksPath c64-port/tools/githooks
format: hooks
	python3 tools/format_changed.py
format-check:
	python3 tools/format_changed.py --check --base "$(FORMAT_BASE)"
