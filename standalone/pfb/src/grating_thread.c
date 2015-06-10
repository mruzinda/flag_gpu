/* grating_thread.c
 *
 * Routine to perform the polyphase filterbank operation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>

#include <xgpu.h>

#include "hashpipe.h"
#include "grating_databuf.h"

// Thread status buffer
static hashpipe_status_t * st_p;

// Run method for the thread
static void *run(hashpipe_thread_args_t * args) {

    // Local aliases to shorten access to args fields
    // Our output buffer happens to be a paper_input_databuf
    grating_gpu_input_databuf_t *db_in = (grating_gpu_input_databuf_t *)args->ibuf;
    grating_output_databuf_t * db_out = (grating_output_databuf_t *)args->obuf;

    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    st_p = &st;	// allow global (this source file) access to the status buffer

    // Set thread to "start" state
    hashpipe_status_lock_safe(&st);
    hputs(st.buf, "GRATREADY", "start");
    hashpipe_status_unlock_safe(&st);

    int rv;
    int curblock_in = 0;
    int curblock_out = 0;
    int mcnt;

    fprintf(stdout, "Gra: Starting Thread!\n");
    
    while (run_threads()) {
        while ((rv=grating_gpu_input_databuf_wait_filled(db_in, curblock_in)) != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "waiting for filled block");
                hashpipe_status_unlock_safe(&st);
            }
            else {
                hashpipe_error(__FUNCTION__, "error waiting for filled databuf block");
                pthread_exit(NULL);
                break;
            }
        }
        while ((rv=grating_output_databuf_wait_free(db_out, curblock_out)) != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "waiting for free block");
                hashpipe_status_unlock_safe(&st);
            }
            else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf block");
                pthread_exit(NULL);
                break;
            }
        }
        mcnt = db_in->block[curblock_in].header.mcnt_start;

        
        
        db_out->block[curblock_out].header.mcnt = mcnt;

        grating_gpu_input_databuf_set_filled(db_out, curblock_out);
        curblock_out = (curblock_out + 1) % db_out->header.n_block;

        grating_input_databuf_set_free(db_in, curblock_in);
        curblock_in = (curblock_in + 1) % db_in->header.n_block;

        /* Will exit if thread has been cancelled */
        pthread_testcancel();
    }

    return NULL;
}


static hashpipe_thread_desc_t trans_thread = {
    name: "grating_transpose_thread",
    skey: "NETSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {grating_input_databuf_create},
    obuf_desc: {grating_gpu_input_databuf_create}
};


static __attribute__((constructor)) void ctor() {
  register_hashpipe_thread(&trans_thread);
}

// vi: set ts=8 sw=4 noet :
