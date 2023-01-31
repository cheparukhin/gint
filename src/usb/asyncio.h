//---
// gint:usb:asyncio - Asynchronous I/O common definitions
//---

#ifndef GINT_USB_ASYNCIO
#define GINT_USB_ASYNCIO

#include <gint/defs/types.h>
#include <gint/defs/call.h>

/* Data tracking the progress of a multi-part multi-round async I/O operation.

   * Multi-part refers to writes being constructed over several calls to
     write(2) followed by a "commit" with sync(2) (for async file descriptors;
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
           |       sync(2): start    | v
     FLYING-COMMIT <------------ IN-PROGRESS
                    transmission

   Initially the operation is in the IDLE state. When a write(2) is issued, it
   interacts with hardware then transitions to the IN-PROGRESS state, where it
   remains for any subsequent write(2). A sync(2) will properly commit data to
   the hardware, finish the operation and return to the IDLE state.

   The FLYING-WRITE and FLYING-COMMIT states refer to waiting periods, after
   issuing hardware commands, during which hardware communicates. Usually an
   interrupt signals when hardware is ready to resume work.

   Note that in a series of write(2), hardware is only instructed to send data
   once the hardware buffer is full. Therefore, a write(2) might transition
   directly from IDLE or IN-PROGRESS, to PENDING, to IN-PROGRESS, without
   actually communicating with the outside world.

   The invariants and meaning for each state are as follow:

   State          Characterization             Description
   ============================================================================
   IDLE           type == ASYNCIO_NONE         No I/O operation
   PENDING        data_w && !flying_w \        Ready to write pending data
                    && round_size == 0
   WRITING        round_size > 0               CPU/DMA write to HW in progress
   FLYING-WRITE   flying_w && !committed_w     HW transmission in progress
   IN-PROGRESS    data_w != NULL && !flying_w  Waiting for write(2) or sync(2)
   FLYING-COMMIT  flying_w && committed_w      HW commit in progress
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

enum { ASYNCIO_NONE, ASYNCIO_READ, ASYNCIO_WRITE };

typedef struct
{
    /** User-facing information **/

    /* Direction of I/O operation */
    uint8_t type;
    /* Whether the DMA should be used for hardware access */
    bool dma;
    /* Whether the data has been committed by sync(2) [write] */
    bool committed_w;
    /* Operation's unit size (meaning depends on hardware) */
    uint8_t unit_size;

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
       Usually, this is assigned during hardware transactions, ie.:
       - During a write, a controller is assigned when leaving the IDLE state
         and returned when re-entering the IDLE state.
       - During a read, a controller is assigne when leaving the IDLE-EMPTY
         state and returned when re-entering the IDLE-EMPTY state. */
    uint8_t controller;
    /* Whether a hardware operation is in progress ("flying" write states) */
    bool flying_w;

} asyncio_op_t;

/* */

#endif /* GINT_USB_ASYNCIO */
