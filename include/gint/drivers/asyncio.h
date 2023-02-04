//---
// gint:usb:asyncio - Asynchronous I/O common definitions
//---

#ifndef GINT_USB_ASYNCIO
#define GINT_USB_ASYNCIO

#include <gint/defs/types.h>
#include <gint/defs/call.h>

/* Data tracking the progress of a multi-part multi-round async I/O operation.

   * Multi-part refers to writes being constructed over several calls to
     write(2) followed by a "commit" with fsync(2) (for async file descriptors;
     synchronous file descriptors are committed at every write).
   * Multi-round refers to the operation interacting multiple times with
     hardware in order to communicate the complete data.

   The process of performing such an I/O operation, as tracked by this
   structure and use throughout gint, is as follows. For a write:

                                   WRITING ---------------------.
                                     ^ |                        | HW buffer
                       Start writing | | Not full               | full: start
                                     | |                        | transmission
                     write(2)        | v                        v
      --> IDLE ------------------> PENDING <------------- FLYING-WRITE
           ^                         ^ |   DONE interrupt
           | DONE           write(2) | |
           | interrupt               | |
           |                         | | Data exhausted
           |     fsync(2): start     | v
      FLYING-SYNC <------------ IN-PROGRESS
                   transmission

   Initially the operation is in the IDLE state. When a write(2) is issued, it
   interacts with hardware then transitions to the IN-PROGRESS state, where it
   remains for any subsequent write(2). A fsync(2) will properly commit data to
   the hardware, finish the operation and return to the IDLE state.

   The FLYING-WRITE and FLYING-SYNC states refer to waiting periods, after
   issuing hardware commands, during which hardware communicates. Usually an
   interrupt signals when hardware is ready to resume work.

   Note that in a series of write(2), hardware is only instructed to send data
   once the hardware buffer is full. Therefore, a write(2) might transition
   directly from IDLE or IN-PROGRESS, to PENDING, to IN-PROGRESS, without
   actually communicating with the outside world.

   An asynchronous write(2) might return to the caller as soon as writing is
   finished even if the operation is left in the FLYING-WRITE state, and it may
   even return while the operation is in the WRITING state if the DMA is used.

   The invariants and meaning for each state are as follow:

   State          Characterization             Description
   ============================================================================
   IDLE           type == ASYNCIO_NONE         No I/O operation
   PENDING        data_w && !flying_w \        Ready to write pending data
                    && round_size == 0
   WRITING        round_size > 0               CPU/DMA write to HW in progress
   FLYING-WRITE   flying_w && type == WRITE    HW transmission in progress
   IN-PROGRESS    !data_w && type == WRITE     Waiting for write(2) or fsync(2)
   FLYING-SYNC    flying_w && type == SYNC     HW commit in progress
   ============================================================================

   For a read:
                     IN interrupt
     --> IDLE-EMPTY --------------> IDLE-READY
             |  \                       | ^
     read(2) |   \ Transaction  read(2) | | Buffer full with
             |    \ exhausted           | | transaction not exhausted
             |     '----<----------.    | |
             |                      \   | |
             v       IN interrupt    \  v | .---.  Read from
          WAITING ------------------> READING    v hardware
                                            '---'

   On this diagram, the right side indicates the presence of data to read from
   hardware while the bottom side indicates a read(2) request by the user.
   Notice the diagonal arrow back to IDLE-EMPTY, which means that read(2) will
   always return at the end of a transaction even if the user-provided buffer
   is not full (to avoid waiting).

   The invariants and meaning for each state are as follow:

   State          Characterization             Description
   ============================================================================
   IDLE-EMPTY     type == ASYNCIO_NONE         No I/O operation
   IDLE-READY     !data_r && buffer_size > 0   Hardware waiting for us to read
   WAITING        data_r && !buffer_size       Waiting for further HW data
   READING        round_size > 0               DMA/CPU read from HW in progress
   ============================================================================

   States can be checked and transitioned with the API functions below. */

enum { ASYNCIO_NONE, ASYNCIO_READ, ASYNCIO_WRITE, ASYNCIO_SYNC };

typedef volatile struct
{
    /** User-facing information **/

    /* Type of I/O operation (read/write/fsync) */
    uint8_t type;
    /* Whether the DMA should be used for hardware access */
    bool dma;

    union {
        /* Address of data to transfer, incremented gradually [write] */
        void const *data_w;
        /* Address of buffer to store data to, incremented gradually [read] */
        void *data_r;
    };
    /* Size of data left to transfer / buffer space available */
    int size;
    /* Callback at the end of the current write, final commit, or read */
    gint_call_t callback;

    /** Hardware state information **/

    /* Size of data currently in the hardware buffer */
    uint16_t buffer_used;
    /* Size of data being read/written in the current round (which may itself
       be asynchronous if it's using the DMA) */
    uint16_t round_size;
    /* Hardware resource being used for access (meaning depends on hardware).
       Usually, this is assigned for the duration of hardware transaction.
       This value is user-managed and not modified by asyncio_op functions. */
    uint8_t controller;
    /* Whether a hardware operation is in progress ("flying" write states) */
    // TODO: Do we actually set and maintain this member?!
    bool flying_w;

    /** Internal information **/

    /* Number of bytes in short buffer (0..3) */
    uint8_t shbuf_size;
    /* Short buffer */
    uint32_t shbuf;

} asyncio_op_t;

//---
// Initialization and query functions
//---

/* asyncio_op_clear(): Initialize/clear the storage for an I/O operation */
void asyncio_op_clear(asyncio_op_t *op);

/* asyncio_op_busy(): Check whether the transfer is busy for syscalls

   This function checks whether the transfer is in a state where the CPU is
   busy wrt. starting a new syscall, ie. read(2), write(2) or fsync(2). Returns
   true if the CPU is busy and the call has to wait, false if the call can
   proceed immediately. */
bool asyncio_op_busy(asyncio_op_t const *op);

//---
// State transition functions
//---

/* asyncio_op_start_write(): Start a write call */
void asyncio_op_start_write(asyncio_op_t *op, void const *data, size_t size,
    bool use_dma, gint_call_t const *callback);

/* asyncio_op_start_sync(): Transition a write I/O operation to a fsync call */
void asyncio_op_start_sync(asyncio_op_t *op, gint_call_t const *callback);

/* asyncio_op_finish_call(): Update state after a read/write/fsync call

   This function should be called when the read(2)/write(2)/fsync(2) call last
   started on the operation has concluded, including all of the hardware
   effects. This isn't the moment when the syscall returns, rather it is the
   moment when it completes its work. */
void asyncio_op_finish_call(asyncio_op_t *op);

//---
// Write call functions
//---

/* asyncio_op_start_write_round(): Start a single-block write to hardware */
void asyncio_op_start_write_round(asyncio_op_t *op, size_t size);

/* asyncio_op_finish_write_round(): Finish a write round and advance data */
void asyncio_op_finish_write_round(asyncio_op_t *op);

#endif /* GINT_USB_ASYNCIO */
