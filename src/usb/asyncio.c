#include <gint/drivers/asyncio.h>
#include <string.h>

void asyncio_op_clear(asyncio_op_t *op)
{
    memset((void *)op, 0, sizeof *op);
}

bool asyncio_op_busy(asyncio_op_t const *op)
{
    /* WAITING and READING states are busy */
    if(op->type == ASYNCIO_READ)
        return op->round_size || op->data_r != NULL;
    /* WRITING, FLYING-WRITE and PENDING states are busy */
    if(op->type == ASYNCIO_WRITE)
        return op->data_w != NULL;
    /* FLYING-COMMIT state is busy */
    if(op->type == ASYNCIO_SYNC)
        return true;

    return false;
}

void asyncio_op_start_write(asyncio_op_t *op, void const *data, size_t size,
    bool use_dma, gint_call_t const *callback)
{
    op->type = ASYNCIO_WRITE;
    op->dma = use_dma;
    op->data_w = data;
    op->size = size;
    op->callback = *callback;
}

void asyncio_op_start_write_round(asyncio_op_t *op, size_t size)
{
    op->round_size = size;
}

void asyncio_op_finish_write_round(asyncio_op_t *op)
{
    op->buffer_used += op->round_size;
    op->data_w += op->round_size;
    op->size -= op->round_size;
    op->round_size = 0;
}

void asyncio_op_start_sync(asyncio_op_t *op, gint_call_t const *callback)
{
    if(op->type != ASYNCIO_WRITE)
        return;

    op->type = ASYNCIO_SYNC;
    op->callback = *callback;
}

void asyncio_op_finish_call(asyncio_op_t *op)
{
    gint_call(op->callback);

    /* Clean up the operation, unless it is a write, in which case keep
       relevant states until the transaction finishes after a fsync(2). */
    if(op->type == ASYNCIO_WRITE) {
        op->dma = false;
        op->data_w = NULL;
        op->size = 0;
        op->callback = GINT_CALL_NULL;
        op->round_size = 0;
    }
    else {
        asyncio_op_clear(op);
    }
}
