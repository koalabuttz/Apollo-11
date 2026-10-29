# Scheduler overlay routing for interpreted source exchanges

The resident `exec_overlay_module` adapter now routes
`EXEC_EXCHANGED_BASIC` and `EXEC_EXCHANGED_INTERPRETER` to finish family 3,
module 153. The shared scheduler owns these additive source-adoption
semantics: an interpretive-origin callback has already performed CHANG2,
and its positive mapped continuation is adopted without another lifecycle
exchange or IRQ poll. Existing basic-origin adoption guards remain distinct.

The adapter changes only two explicit native-action cases. Strict and
ASAN/UBSAN loader seam tests retain enabled/masked callers, exact guard
restoration, missing-lock rejection, prepare/finish load failures, single
callback execution and pending IRQ preservation. New checks assert both
exchange actions route to 153 and an invalid stage remains unsupported.
The three historical complete original-source display corpora still match
under strict C and full ASAN/UBSAN: 11,776 normal, 5,120 sleep/busy and
12,288 actual-END states. Their raw identities remain unchanged. The
compressed regression tool retains no duplicate raw streams.

The current scheduler images load 7,464, 10,184, 12,241 and 4,291 bytes
for modules 150 through 153; each fits the 16KB slot with no additional
zero page. New images and source hashes are retained under the separate
`native-scheduler-exchange` prefix. Earlier scheduler artifacts remain
unchanged. This packet proves adapter routing, historical regression and
linkage. The Executive-owned canonical CHANG2 real-step and actual disk
exchange proof separately qualifies execution of the new actions.
