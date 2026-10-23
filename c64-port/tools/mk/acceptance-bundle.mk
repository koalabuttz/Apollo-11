# SPDX-License-Identifier: MIT
# acceptance-bundle-<m>: every whole-program acceptance doc of one program
# (docs/acceptance/) from one commit: runs each category's
# tools/*_acceptance.py at HEAD and checks each doc was rewritten there, its
# evidence resolves and the docs agree on the commit and the images
# (tools/acceptance_bundle.py). A program's flip PR runs it; the VICE runs
# make it long. test-acceptance-bundle: the tool on made-up generators.
.PHONY: acceptance-bundle-lm acceptance-bundle-cm test-acceptance-bundle
acceptance-bundle-lm acceptance-bundle-cm: acceptance-bundle-%:
	python3 tools/acceptance_bundle.py $*
test: test-acceptance-bundle
test-acceptance-bundle:
	python3 tests/test_acceptance_bundle.py
