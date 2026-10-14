# SPDX-License-Identifier: MIT
# CI runs each c64-* and vice-* target as one job (port-ci.yml), so a long
# milestone run is cut into parts of consecutive items, c64-<m>-<n> and
# vice-<m>-<n>, each well under ten minutes on a runner building and
# running (tools/ci_target_seconds.json). <m>-c64 and <m>-vice build and run
# every part; the CI regex does not match them. Named _ci-split.mk so it is
# read before the other tools/mk files that call it.
#   $(eval $(call CI_SPLIT_RULE,<m>,<items>,<parts>,<recipe>))
# Both targets of a part depend on its items (the PRGs it runs); <recipe> is
# called as $(call <recipe>,<m>-<n>,<items of the part>) for vice-<m>-<n>.
ci_split_size = $(shell expr \( $(words $(1)) + $(2) - 1 \) / $(2))
ci_split_part = $(wordlist $(shell expr \( $(3) - 1 \) \* $(call ci_split_size,$(1),$(2)) + 1),$(shell expr $(3) \* \
  $(call ci_split_size,$(1),$(2))),$(1))
define CI_SPLIT_PART
.PHONY: c64-$(1)-$(4) vice-$(1)-$(4)
c64-$(1)-$(4): $$(call ci_split_part,$(2),$(3),$(4))
vice-$(1)-$(4): $$(call ci_split_part,$(2),$(3),$(4))
	$$(call $(5),$(1)-$(4),$$(call ci_split_part,$(2),$(3),$(4)))
endef
define CI_SPLIT_RULE
$(foreach n,$(shell seq $(3)),$(eval $(call CI_SPLIT_PART,$(1),$(2),$(3),$(n),$(4))))
.PHONY: $(1)-c64 $(1)-vice
$(1)-c64: $(2)
$(1)-vice: $(foreach n,$(shell seq $(3)),vice-$(1)-$(n))
endef
# A long host target (test-<m> or sanitize-<m>) is cut the same way into
# <target>-<n>, each joining the test or sanitize list in its place; <target>
# stays as the aggregate that runs every part. Every part depends on
# <prereqs>; <recipe> is called as $(call <recipe>,<n>,<parts>).
#   $(eval $(call CI_HOST_SPLIT_RULE,<target>,<parts>,<prereqs>,<recipe>))
define CI_HOST_SPLIT_PART
.PHONY: $(1)-$(3)
$(firstword $(subst -, ,$(1))): $(1)-$(3)
$(1)-$(3): $(4)
	$$(call $(5),$(3),$(2))
endef
define CI_HOST_SPLIT_RULE
$(foreach n,$(shell seq $(2)),$(eval $(call CI_HOST_SPLIT_PART,$(1),$(2),$(n),$(3),$(4))))
.PHONY: $(1)
$(1): $(foreach n,$(shell seq $(2)),$(1)-$(n))
endef
