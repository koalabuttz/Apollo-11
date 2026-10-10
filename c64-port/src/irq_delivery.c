/* SPDX-License-Identifier: MIT */
#include "irq_delivery.h"
void exec_irq_init(exec_irq_queue *q) {
    q->write=0; q->read=0; q->overflow=0; q->rejected=0;
}
enum exec_status exec_irq_post(exec_irq_queue *q, const exec_request *r) {
    uint8_t ticket,slot;
    if (r->priority == 0 || (r->priority & UINT16_C(0140777)) != 0 ||
        r->address.location == 0 || r->address.location > 07777 ||
        r->address.bank > AGC_MASK || r->needs_vac > 1)
        return EXEC_OUT_OF_SCOPE;
    ticket=q->write;
    if (q->overflow != 0 || (uint8_t)(ticket-q->read) >= EXEC_IRQ_CAPACITY) {
        q->overflow=1;
        if (q->rejected != UINT8_MAX) ++q->rejected;
        return EXEC_QUEUE_FULL;
    }
    slot=(uint8_t)(ticket & (EXEC_IRQ_CAPACITY-1));
    q->item[slot].priority=r->priority;
    q->item[slot].address.location=r->address.location;
    q->item[slot].address.bank=r->address.bank;
    q->item[slot].needs_vac=r->needs_vac;
    q->write=(uint8_t)(ticket+1u); /* Publish only after all payload bytes. */
    return EXEC_OK;
}
enum exec_status exec_native_bind_irq(exec_native *r, exec_irq_queue *q,
                                     const exec_exclusion *x) {
    if (r->running != 0 || r->fault != EXEC_OK || q == 0 || x == 0 ||
        x->enter == 0 || x->leave == 0) return EXEC_OUT_OF_SCOPE;
    r->requests=q; r->exclusion=x;
    return EXEC_OK;
}
uint8_t exec_native_lock(exec_native *r) {
    return r->exclusion == 0 ? 0 : r->exclusion->enter(r->exclusion->user);
}
void exec_native_unlock(exec_native *r, uint8_t token) {
    if (r->exclusion != 0) r->exclusion->leave(r->exclusion->user,token);
}
static int mapped(const exec_native *r, exec_address a) {
    uint8_t i;
    for (i=0;i<r->count;++i)
        if (r->mapping[i].address.location == a.location &&
            r->mapping[i].address.bank == a.bank) return 1;
    return 0;
}
enum exec_status exec_irq_service_locked(exec_native *r) {
    exec_irq_queue *q=r->requests;
    uint8_t delivered=0;
    if (q == 0) return EXEC_OK;
    if (q->overflow != 0) return EXEC_QUEUE_FULL;
    if (r->state->alarm != 0) return EXEC_BAILOUT_PENDING;
    while (q->read != q->write && delivered<EXEC_IRQ_CAPACITY) {
        uint8_t ticket=q->read,slot=(uint8_t)(ticket & (EXEC_IRQ_CAPACITY-1));
        exec_request request;
        enum exec_status status;
        request.priority=q->item[slot].priority;
        request.address.location=q->item[slot].address.location;
        request.address.bank=q->item[slot].address.bank;
        request.needs_vac=q->item[slot].needs_vac;
        if (!mapped(r,request.address)) return EXEC_UNMAPPED;
        status=exec_admit(r->state,request.priority,request.address.location,
                          request.address.bank,request.needs_vac);
        if (status != EXEC_OK) return status; /* Failed request remains pending. */
        q->read=(uint8_t)(ticket+1u); ++delivered;
    }
    return EXEC_OK;
}
