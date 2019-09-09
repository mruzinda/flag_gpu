/* flag_net_transpose_thread.c
 *
 * Routine to read packets from network and load them into the buffer.
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
#include <fifo.h>

#include "hashpipe.h"
#include "flag_databuf.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define ELAPSED_NS(start,stop) \
  (((int64_t)stop.tv_sec-start.tv_sec)*1000*1000*1000+(stop.tv_nsec-start.tv_nsec))


// Create thread status buffer
static hashpipe_status_t *st_p;

// Define a packet header type
// First 44 bits are the mcnt (system time index)
// Next 4 bits are the switching signal bits (BLANK | LO BLANK | CAL | SIG/REF)
// Next 8 bits are the Fengine ID from which the packet came
// Next 8 bits are the Xengine ID to which the packet is destined
typedef struct {
  uint64_t mcnt;
//  uint8_t  cal;   // Noise Cal Status Mask
  int      fid;	  // Fengine ID
  int      xid;	  // Xengine ID
} packet_header_t;


// Define a block info type
// The output buffer takes blocks (collections of contiguous packets)
// The info structure keeps track of the following:
//     (1) The XID for this Xengine
//     (2) The reference starting mcnt
//         (the starting mcnt for the first block)
//     (3) The ID of the block that is currently being filled
// This structure will be static (i.e. it will reside in static memory, and not the stack)
typedef struct {
  int      initialized;                  // Boolean to indicate that block has been initialized
  int32_t  self_xid;                     // Xengine ID for this block
  uint64_t mcnt_start;                   // The starting mcnt for this block
  uint64_t mcnt_log_late;                // Counter for logging late mcnts
  int      out_of_seq_cnt;               // Counter for number of times an out of seq pkt arrives
  int      cur_block_idx;                // Current block idx to fill
  int      last_filled;                  // Last filed block idx (consider changing to blk_last_fill
  int      packet_count[N_INPUT_BLOCKS]; // Packet counter for each block
  int      m;                            // mcnt index relative to packet payload destination
  int      f;                            // source fid
  uint64_t sim_pkt_counter;              // counter for simulating sent packets
} block_info_t;

// Method to compute the block index for a given packet mcnt
// Note that the mod operation allows the buffer to be circular
static inline int get_block_idx (uint64_t mcnt) {
  return (mcnt / Nm) % N_INPUT_BLOCKS;
}

// Method to print the header of a received packet
void print_pkt_header (packet_header_t * pkt_header) {

  static long long prior_mcnt;

  printf("packet header : mcnt %012lx (diff from prior %lld) fid %d xid %d\n",
      pkt_header->mcnt, pkt_header->mcnt-prior_mcnt,
      pkt_header->fid, pkt_header->xid);

  prior_mcnt = pkt_header->mcnt;
}

void print_block_packet_count (block_info_t *binfo) {

  int i;
  for (i=0; i < N_INPUT_BLOCKS; ++i) {
    if (i == binfo->cur_block_idx) {
      printf("block %d: %d (active)\n", i, binfo->packet_count[i]);
    } else {
      printf("block %d: %d\n", i, binfo->packet_count[i]);
    }
  }
}

void print_block_info (block_info_t * binfo) {
    printf("binfo : mcnt_start=%lld mcnt_log_late=%lld pkt_block_idx=%d last_filled=%d m=%d f=%d\n",
           (long long)binfo->mcnt_start, (long long)binfo->mcnt_log_late,
           binfo->cur_block_idx, binfo->last_filled, binfo->m, binfo->f);
}

void print_block_mcnts (flag_input_databuf_t *db) {

  int i;
  for (i=0; i < N_INPUT_BLOCKS; ++i) {
    printf("block id: %d mcnt: %lld\n", i, (long long int)db->block[i].header.mcnt_start);
  }
}

// Monitor the rx order of mcnts from the network -- anticipate that there will
// be multiplep mcnts because there are multiple FIDs
#define LOG_MAX_MCNT (1024*1024)
static uint64_t rx_mcnt[LOG_MAX_MCNT];
static int rx_mcnt_idx = 0;

void print_rx_mcnts () {

  FILE *f = fopen("rx_mcnt.log", "w");
  if (f == NULL) {
    fprintf(stderr, "NET: could not open rx_mcnt.log for writting\n");
  }

  int i;
  for (i=0; i < LOG_MAX_MCNT; ++i) {
    fprintf(f, "%lld\n", (long long)rx_mcnt[i]);
  }
  fclose(f);
}

//#define SIM_PACKETS
// Method to extract a packet's header information and store it
static inline void get_header (block_info_t *binfo,
                               struct hashpipe_udp_packet *p,
                               packet_header_t * pkt_header)
{
#ifdef SIM_PACKETS
  // Nf (N_FENGINES) is set to 8 due to macro nonsense... not 5 and so 5 is hard coded here
  pkt_header->mcnt = binfo->sim_pkt_counter / 5;
  pkt_header->xid = 0;
  pkt_header->fid = binfo->sim_pkt_counter % 5;
  binfo->sim_pkt_counter++;
#else
  uint64_t raw_header;
  raw_header = be64toh(*(unsigned long long *)p->data);
  
  //pkt_header->mcnt        = raw_header >> 20;
  //pkt_header->cal         = (raw_header >> 16) & 0x000000000000000F;
  //pkt_header->xid         = raw_header         & 0x00000000000000FF;
  //pkt_header->fid         = (raw_header >> 8)  & 0x00000000000000FF;

  // MR: Changed to match  ONR packet header (version 2) ////////////////
  pkt_header->mcnt        = raw_header >> 8;
  pkt_header->xid         = (raw_header >> 4)  & 0x000000000000000F;
  pkt_header->fid         = raw_header         & 0x000000000000000F;
#endif

  // log rx'd mcnts to see if we are ever seeing packets come out of order
  rx_mcnt[rx_mcnt_idx++ % LOG_MAX_MCNT] = pkt_header->mcnt;

}

// Method to calculate the buffer address for packet payload
// Also verifies FID and XID of packets
static inline int calc_block_indices (block_info_t *binfo, packet_header_t *pkt_header)
{
  if(pkt_header->fid >= Nf) {
    hashpipe_error(__FUNCTION__,
        "current packet FID %u out of range (0-%d)",
        pkt_header->fid, Nf-1);
    return -1;
  } else if(pkt_header->xid != binfo->self_xid && binfo->self_xid != -1) {
    hashpipe_error(__FUNCTION__,
        "unexpected packet XID %d (expected %d)",
        pkt_header->xid, binfo->self_xid);
    return -1;
  }

  binfo->m = pkt_header->mcnt % Nm;
  binfo->f = pkt_header->fid;

  return 0;
}

// Method to mark the block as filled
int set_block_filled (flag_input_databuf_t *db,
                     block_info_t *binfo)
{
  uint32_t block_missed_pkt_cnt=N_PACKETS_PER_BLOCK, block_missed_mod_cnt, missed_pkt_cnt=0;

  uint32_t block_idx = get_block_idx(binfo->mcnt_start);

  // Validate that we're filling blocks in the proper sequence
  binfo->last_filled = (binfo->last_filled+1) % N_INPUT_BLOCKS;
  if(binfo->last_filled != block_idx) {
    printf("block %d being marked filled, but expected block %d!\n", block_idx, binfo->last_filled);
//#ifdef DIE_ON_OUT_OF_SEQ_FILL
//    die(paper_input_databuf_p, binfo);
//#endif
  }

  // Validate that block_idx matches binfo->cur_block_idx
  if(block_idx != binfo->cur_block_idx) {
    hashpipe_warn(__FUNCTION__,
        "block_idx for binfo's mcnt (%d) != binfo's cur_block_idx (%d)",
        block_idx, binfo->cur_block_idx);
  }

  // If all packets are accounted for, mark this block as good
  if(binfo->packet_count[block_idx] == N_REAL_PACKETS_PER_BLOCK) {
    db->block[block_idx].header.good_data = 1;
  } else {
      printf("NET: Bad Block! mcnt = %lld, %d/%d\n",
              (long long)db->block[block_idx].header.mcnt_start,
              binfo->packet_count[block_idx],
              N_REAL_PACKETS_PER_BLOCK);
  }

  // Set the block as filled
  if(flag_input_databuf_set_filled(db, block_idx) != HASHPIPE_OK) {
    hashpipe_error(__FUNCTION__, "error waiting for databuf filled call");
    pthread_exit(NULL);
  }

  // Calculate missing packets.
  block_missed_pkt_cnt = N_PACKETS_PER_BLOCK - binfo->packet_count[block_idx];
  block_missed_mod_cnt = block_missed_pkt_cnt % N_PACKETS_PER_BLOCK_PER_F;

  // Update status buffer
  hashpipe_status_lock_busywait_safe(st_p);
  hputu4(st_p->buf, "NETBKOUT", block_idx);
  if(block_missed_mod_cnt) {
    // Increment MISSEDPK by number of missed packets for this block
    hgetu4(st_p->buf, "MISSEDPK", &missed_pkt_cnt);
    missed_pkt_cnt += block_missed_mod_cnt;
    hputu4(st_p->buf, "MISSEDPK", missed_pkt_cnt);
  }
  hashpipe_status_unlock_safe(st_p);

  return binfo->mcnt_start;

}

// This allows for 2 out of sequence packets from each F engine (in a row)
#define MAX_OUT_OF_SEQ (2*Nf)

// This allows packets to be two full databufs late without being considered
// out of sequence.
#define LATE_PKT_MCNT_THRESHOLD (2*Nm*N_INPUT_BLOCKS)

// Initialize a block by clearing its "good data" flag and saving the first
// (i.e. earliest) mcnt of the block.  Note that mcnt does not have to be a
// multiple of Nm (number of mcnts per block).  In theory, the block's data
// could be cleared as well, but that takes time and is largely unnecessary in
// a properly functionong system.
static inline void initialize_block (flag_input_databuf_t *db,
                                     uint64_t mcnt)
{
  int block_idx = get_block_idx(mcnt); 

  db->block[block_idx].header.good_data = 0;
  //Rount mcnt down to nearest multiple of Nm
  db->block[block_idx].header.mcnt_start = mcnt - (mcnt%Nm);
}

// This function must be called once and only once per block_info structure!
// Subsequent calls are no-ops.
static inline void initialize_block_info(block_info_t * binfo)
{
  int i;

  // If this block_info structure has already been initialized
  if(binfo->initialized) {
    return;
  }

  for(i = 0; i < N_INPUT_BLOCKS; i++) {
    binfo->packet_count[i] = 0;
  }

  // Initialize our XID to -1 (unknown until read from status buffer)
  binfo->self_xid = -1;
  // Update our XID from status buffer
  hashpipe_status_lock_busywait_safe(st_p);
  hgeti4(st_p->buf, "XID", &binfo->self_xid);
  hashpipe_status_unlock_safe(st_p);

  // On startup mcnt_start will be zero and mcnt_log_late will be Nm.
  binfo->mcnt_start = 0;
  binfo->mcnt_log_late = Nm;
  binfo->cur_block_idx= 0;

  binfo->out_of_seq_cnt = 0;
  binfo->initialized = 1;
}

// Method to process a received packet
// Processing involves the following
// (1) header extraction
// (2) block population (output buffer data type is a block)
// (3) buffer population (if block is filled)
static inline int64_t process_packet (flag_input_databuf_t *db,
                                      block_info_t *binfo,
                                      struct hashpipe_udp_packet *p)
{
  packet_header_t pkt_header;
  //const uint64_t *payload_p;
  int pkt_block_idx;
  uint8_t *dest_p;
  const uint8_t * payload_p;
  int64_t pkt_mcnt_dist;
  uint64_t pkt_mcnt;
  uint64_t cur_mcnt;
  uint64_t netmcnt = -1; // Value to return (!=-1 is stored in status memory)

  // Lazy init binfo
  if(!binfo->initialized) {
    initialize_block_info(binfo);
  }

  // Parse packet header
  get_header(binfo, p, &pkt_header);
  pkt_mcnt = pkt_header.mcnt;
  pkt_block_idx = get_block_idx(pkt_mcnt);
  cur_mcnt = binfo->mcnt_start;

  // Packet mcnt distance (how far away is this packet's mcnt from the
  // current mcnt).  Positive distance for pcnt mcnts > current mcnt.
  pkt_mcnt_dist = pkt_mcnt - cur_mcnt;

  // We expect packets for the current block, the next block, and the block after.
  if(0 <= pkt_mcnt_dist && pkt_mcnt_dist < 3*Nm) {
    // If the packet is for the block after the next block (i.e. current
    // block + 2 blocks)
    if(pkt_mcnt_dist >= 2*Nm) {
      // Mark the current block as filled
      netmcnt = set_block_filled(db, binfo);

      // Advance mcnt_start to next block
      cur_mcnt += Nm;
      binfo->mcnt_start += Nm;
      binfo->cur_block_idx = (binfo->cur_block_idx + 1) % N_INPUT_BLOCKS;

      // Wait (hopefully not long!) to acquire the block after next (i.e.
      // the block that gets the current packet).
      if(flag_input_databuf_busywait_free(db, pkt_block_idx) != HASHPIPE_OK) {
        if (errno == EINTR) {
          // Interrupted by signal, return -1
          hashpipe_error(__FUNCTION__, "interrupted by signal waiting for free databuf");
          pthread_exit(NULL);
          return -1; // We're exiting so return value is kind of moot
        } else {
          hashpipe_error(__FUNCTION__, "error waiting for free databuf");
          pthread_exit(NULL);
          return -1; // We're exiting so return value is kind of moot
        }
      }
      // Initialize the newly acquired block
      initialize_block(db, pkt_mcnt);
      // Reset binfo's packet counter for this packet's block
      binfo->packet_count[pkt_block_idx] = 0;
    }

    // Reset out-of-seq counter
    binfo->out_of_seq_cnt = 0;

    // Increment packet count for block
    binfo->packet_count[pkt_block_idx]++;

    // Validate header FID and XID and calculate "m" and "f" indexes into
    // block (stored in binfo).
    if(calc_block_indices(binfo, &pkt_header)) {
      // Bad packet, error already reported
      return -1;
    }

    // Calculate starting points for unpacking this packet into block's data buffer.
    //dest_p = db->block[pkt_block_idx].data + flag_input_databuf_idx(binfo->m, binfo->f, 0, 0);
    //payload_p = (uint64_t *)(p->data+8);

    // Copy data into buffer
    //memcpy(dest_p, payload_p, N_BYTES_PER_PACKET);

    uint8_t * tmp_p;
    tmp_p  = db->block[pkt_block_idx].data_tmp + flag_input_e_databuf_idx(binfo->m, binfo->f, 0, 0, 0);
    //printf("Binfo.m: %d, binfo.f: %d\n",binfo.m,binfo.f);
    payload_p = (uint8_t *)(p->data+8); // Ignore header
    // Copy data into buffer
    memcpy(tmp_p, payload_p, N_BYTES_PER_PACKET-8); // Ignore header

    // Transpose done in net_thread now.
    int t; int c; int e;
    for (t = 0; t < Nt; t++) {
        for (c = 0; c < Nc; c++) {
            for (e = 0; e < Ne; e++) {
                // 8 bit copies ////////////////////////////////////////
                // Changed because the correlator requires 32 inputs so the macros are zero padded by 2 per FID. 4 FIDs with 8 inputs, 2 of which are zeros.
                dest_p  = db->block[pkt_block_idx].data + flag_gpu_input_e_databuf_idx(binfo->m, binfo->f, t, c, e);
                if(e < 6){
                    tmp_p  = db->block[pkt_block_idx].data_tmp + flag_input_e_databuf_idx(binfo->m, binfo->f, t, c, e);
		    memcpy(dest_p, tmp_p, 2);
                }		
		////////////////////////////////////////////////////////
            }
        }
    }

    return netmcnt;
  }
  // Else, if packet is late, but not too late (so we can handle F engine
  // restarts and MCNT rollover), then ignore it
  else if(pkt_mcnt_dist < 0 && pkt_mcnt_dist > -LATE_PKT_MCNT_THRESHOLD) {
    // If not just after an mcnt reset, issue warning.
    if(cur_mcnt >= binfo->mcnt_log_late) {
      hashpipe_warn("flag_net_thread",
          "Ignoring late packet (%d mcnts late)",
          cur_mcnt - pkt_mcnt);
    }

    return -1;
  }
  // Else, it is an "out-of-order" packet.
  else {
    // If not at start-up and this is the first out of order packet,
    // issue warning.
    if(cur_mcnt != 0 && binfo->out_of_seq_cnt == 0) {
      hashpipe_warn("flag_net_thread",
          "out of seq mcnt %lld (expected: %lld <= mcnt < %lld)",
          (long long)pkt_mcnt, (long long)cur_mcnt, (long long)cur_mcnt+3*Nm);
    }

    // Increment out-of-seq packet counter
    binfo->out_of_seq_cnt++;

    // If too may out-of-seq packets
    if(binfo->out_of_seq_cnt > MAX_OUT_OF_SEQ) {
      // Reset current mcnt. The value to reset to must be the first
      // value greater than or equal to pkt_mcnt that corresponds to the
      // same databuf block as the old current mcnt.
      if(binfo->cur_block_idx > pkt_block_idx) {
        // Advance pkt_mcnt to correspond to binfo->cur_block_idx
        pkt_mcnt += Nm*(binfo->cur_block_idx - pkt_block_idx);
      } else if(binfo->cur_block_idx < pkt_block_idx) {
        // Advance pkt_mcnt to binfo->cur_block_idx + N_INPUT_BLOCKS blocks
        pkt_mcnt += Nm*(binfo->cur_block_idx + N_INPUT_BLOCKS - pkt_block_idx);
      }
      // Round pkt_mcnt down to nearest multiple of Nm
      binfo->mcnt_start = pkt_mcnt - (pkt_mcnt%Nm);
      binfo->mcnt_log_late = binfo->mcnt_start + Nm;
      binfo->cur_block_idx = get_block_idx(binfo->mcnt_start);
      hashpipe_warn("flag_net_thread",
          "resetting to mcnt %lld block %d based on packet mcnt %lld",
          (long long)binfo->mcnt_start, get_block_idx(binfo->mcnt_start), (long long)pkt_mcnt);
      // Reinitialize/recycle our two already acquired blocks with new
      // mcnt values.
      initialize_block(db, binfo->mcnt_start);
      initialize_block(db, binfo->mcnt_start+Nm);
      // Reset binfo's packet counters for these blocks.
      binfo->packet_count[binfo->cur_block_idx] = 0;
      binfo->packet_count[(binfo->cur_block_idx+1)%N_INPUT_BLOCKS] = 0;
    }
    return -1;
  }

  return netmcnt;
}

// Enumerated types for flag_net_thread state machine
typedef enum {
  IDLE,
  ACQUIRE,
  CLEANUP
} state;

// Run method for the thread
// It is meant to do the following:
// (1) Initialize status buffer
// (2) Set up network parameters and socket
// (3) Start main loop
//     (3a) Receive packet on socket
//     (3b) Error check packet (packet size, etc)
//     (3c) Call process_packet on received packet
// (4) Terminate thread cleanly
static void *run (hashpipe_thread_args_t * args) {

  // Local aliases to shorten access to args fields
  // Our output buffer happens to be a paper_input_databuf
  flag_input_databuf_t *db = (flag_input_databuf_t *)args->obuf;
  hashpipe_status_t st = args->st;
  const char * status_key = args->thread_desc->skey;

  st_p = &st;	// allow global (this source file) access to the status buffer

  hashpipe_status_lock_safe(&st);
  hputl(st.buf, "NETREADY", 0);
  hashpipe_status_unlock_safe(&st);

  int tmp = -1;
  hashpipe_status_lock_safe(&st);
  hgeti4(st.buf, "XID", &tmp);
  hashpipe_status_unlock_safe(&st);

  /* Read network params */
  struct hashpipe_udp_params up = {
    .bindhost = "0.0.0.0",
    .bindport = 8511,
    .packet_size = N_BYTES_PER_PACKET
  };

  hashpipe_status_lock_safe(&st);
  // Get info from status buffer if present (no change if not present)
  hgets(st.buf, "BINDHOST", 80, up.bindhost);
  hgeti4(st.buf, "BINDPORT", &up.bindport);

  // Store bind host/port info etc in status buffer
  hputs(st.buf, "BINDHOST", up.bindhost);
  hputi4(st.buf, "BINDPORT", up.bindport);
  hputu4(st.buf, "MISSEDFE", 0);
  hputu4(st.buf, "MISSEDPK", 0);
  hashpipe_status_unlock_safe(&st);

  struct hashpipe_udp_packet p;
  struct hashpipe_udp_packet bh;
  /* Give all the threads a chance to start before opening network socket */
  /*
     int netready = 0;
     int traready = 0;
     int corready = 0;
  // int savready = 0;
  while (!netready) {
  // Check the correlator to see if it's ready yet
  hashpipe_status_lock_safe(&st);
  hgeti4(st.buf, "TRAREADY",  &traready);
  hgeti4(st.buf, "CORREADY",  &corready);
  // hgeti4(st.buf, "SAVEREADY", &savready);
  hashpipe_status_unlock_safe(&st);
  netready = traready & corready;
  }
  sleep(1);
  */

  // Create clean flags for other threads
  char modename[25];
  int useB = 0;
  int useC = 0;
  hashpipe_status_lock_safe(&st);
  hgets(st.buf, "MODENAME", 24, modename);
  hashpipe_status_unlock_safe(&st);

  //if (strcmp(modename, "FLAG_CALCORR_MODE") == 0) {
  //  useB = 1;
  //}
  if (strcmp(modename, "FLAG_PFBCORR_MODE") == 0) {
    useB = 1;
    useC = 1;
  }

  hashpipe_status_lock_safe(&st);
  hputl(st.buf, "CLEANA", 1);
  if (useB) {
    hputl(st.buf, "CLEANB", 1);
  }
  if (useC) {
    hputl(st.buf, "CLEANC", 1);
  }
  hashpipe_status_unlock_safe(&st);

  /* Set up UDP socket */
  fprintf(stderr, "NET: BINDHOST = %s\n", up.bindhost);
  fprintf(stderr, "NET: BINDPORT = %d\n", up.bindport);

#ifndef SIM_PACKETS
  int rv = hashpipe_udp_init(&up);
  if (rv!=HASHPIPE_OK) {
    hashpipe_error("paper_net_thread", "Error opening UDP socket.");
    pthread_exit(NULL);
  }
  pthread_cleanup_push((void *)hashpipe_udp_close, &up);
#endif

  uint64_t scan_start_mcnt = 0;
  // Set correlator's starting mcnt to 0
  hashpipe_status_lock_safe(&st);
  hputi4(st.buf, "NETMCNT", scan_start_mcnt);
  hashpipe_status_unlock_safe(&st);

  // declare and initialize the binfo struct. This structure monitors and
  // controls packet processing
  static block_info_t binfo = { .last_filled = -1 };
  static const block_info_t newScan = { .last_filled = -1 }; // to reset at each scan

  int ii;
  for (ii = 0; ii < N_INPUT_BLOCKS; ++ii) {
    if (flag_input_databuf_busywait_free(db, ii) != HASHPIPE_OK) {
      if (errno == EINTR) {
        // Interrupted by signal, return -1
        hashpipe_error(__FUNCTION__, "interrupted by signal waiting for free databuf");
        pthread_exit(NULL);
      } else {
        hashpipe_error(__FUNCTION__, "error waiting for free databuf");
        pthread_exit(NULL);
      }
    }

    initialize_block(db, scan_start_mcnt + ii*Nm);

  }
  print_block_info(&binfo);
  print_block_mcnts(db);

  // Set up FIFO controls
  int cmd = INVALID;
  int master_cmd = INVALID;
  int gpu_fifo_id = open_fifo("/tmp/command.fifo");
  state cur_state = IDLE;
  state next_state = IDLE;

  /* Main loop */
  uint64_t packet_count = 0;
  int64_t last_filled_mcnt = -1;
  int64_t scan_last_mcnt = -1;

  // Set correlator to "start" state and indicate the net thread has finished
  // initializing
  hashpipe_status_lock_safe(&st);
  hputs(st.buf, "INTSTAT", "start");
  hputl(st.buf, "NETREADY", 1);
  hashpipe_status_unlock_safe(&st);


  int n = 0;
  int n_loop = 10000;
  int cmd_n = 0;
  int cmd_n_Max = 10000;
  fprintf(stdout, "NET: Starting Thread!!!\n");
  while (run_threads()) {

    master_cmd = INVALID;
    cmd = INVALID;

    // Get command from Dealer/Player
    if (n++ >= n_loop) {
      master_cmd = check_cmd(gpu_fifo_id);
      if(master_cmd != INVALID){
        hashpipe_status_lock_safe(&st);
        if (master_cmd == START) hputs(st.buf, "MASTRCMD", "START");
        if (master_cmd == STOP)  hputs(st.buf, "MASTRCMD", "STOP");
        if (master_cmd == QUIT)  hputs(st.buf, "MASTRCMD", "QUIT");
        hashpipe_status_unlock_safe(&st);
      }
      n = 0;
    }

    // If command is QUIT, stop all processing
    if (master_cmd == QUIT) break;

    // If pipeline terminated somewhere else, stop processing
    if(!run_threads()) break;

    /************************************************************
     * IDLE state processing
     ************************************************************/
    // If in IDLE state, look for START command
    if (cur_state == IDLE) {
      master_cmd = check_cmd(gpu_fifo_id);
      // cmd = check_cmd(gpu_fifo_id);

      bh.packet_size = recv(up.sock, bh.data, HASHPIPE_MAX_PACKET_SIZE, 0);
      if(bh.packet_size != -1) {
        //packet_header_t bh_pkt_header;
        //get_header(&bh, &bh_pkt_header);
        //hashpipe_status_lock_safe(&st);
        //hputi4(st.buf, "BLKHOLE", bh_pkt_header.mcnt);
        //hashpipe_status_unlock_safe(&st);
      }

      // If command is START, proceed to ACQUIRE state
      if (master_cmd == START) {
        next_state = ACQUIRE;
        // Get scan length from shared memory (set by BeamformerBackend.py)
        int scanlen;
        hashpipe_status_lock_safe(&st);
        hgeti4(st.buf, "SCANLEN", &scanlen);
        hashpipe_status_unlock_safe(&st);
        scan_last_mcnt = scanlen*N_MCNT_PER_SECOND;
        printf("NET: Ending scan after mcnt = %lld\n", (long long int)scan_last_mcnt);
      }
    }

    /************************************************************
     * ACQUIRE state processing
     ************************************************************/
    // If in ACQUIRE state, get packets
    if (cur_state == ACQUIRE) {
      // Loop over (non-blocking) packet receive
#ifndef SIM_PACKETS
      do {
        if (master_cmd == STOP) break;

        p.packet_size = recv(up.sock, p.data, HASHPIPE_MAX_PACKET_SIZE, 0);

        if (cmd_n++ >= cmd_n_Max) {
          cmd = check_cmd(gpu_fifo_id);
          if (cmd != INVALID) {
            hashpipe_status_lock_safe(&st);
            if (cmd == START) hputs(st.buf, "FIFOCMD", "START");
            if (cmd == STOP)  hputs(st.buf, "FIFOCMD", "STOP");
            if (cmd == QUIT)  hputs(st.buf, "FIFOCMD", "QUIT");
            hashpipe_status_unlock_safe(&st);
          }
          cmd_n = 0;
        }
        if (cmd == STOP || cmd == QUIT) break;
      } while (p.packet_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)
                  && run_threads() && cmd==INVALID);

      if (!run_threads() || cmd == QUIT) break;
      // Check packet size and report errors
      if (up.packet_size != p.packet_size && cmd != STOP) {
        // If an error was returned instead of a valid packet size
        if (p.packet_size == -1) {
          // Log error and exit
          hashpipe_error("flag_net_thread", "hashpipe_udp_recv returned error");
          perror("hashpipe_udp_recv");
          pthread_exit(NULL);
        }
        else {
          // Log warning and ignore wrongly sized packet
          hashpipe_warn("flag_net_thread", "Incorrect pkt_size (%d)", p.packet_size);
          pthread_testcancel();
          continue;
        }
      }
#else
      cmd_n = cmd_n_Max; // satisfy compiler warning for now if sim works figure out what to do
#endif
      // Process packet
      packet_count++;
      last_filled_mcnt = process_packet(db, &binfo, &p);

      //if (last_filled_mcnt != -1) {
      //  printf("NET: filled mcnt=%lld\n", (long long)last_filled_mcnt);
      //}
      // Next state processing
      next_state = ACQUIRE;
      if ((last_filled_mcnt != -1 && last_filled_mcnt >= scan_last_mcnt)
            || cmd == STOP || master_cmd == STOP) {
 
        printf("NET: CLEANUP condition met!\n");
        int cleanA=1, cleanB=1, cleanC=1;
        sleep(1);

        printf("NET: Informing other threads of cleanup condition\n");
        while (cleanA != 0 && cleanB != 0 && cleanC != 0) {
          hashpipe_status_lock_safe(&st);
          hputl(st.buf, "CLEANA", 0);
          if (useB) {
            hputl(st.buf, "CLEANB", 0);
          }
          if (useC) {
            hputl(st.buf, "CLEANC", 0);
          }
          hashpipe_status_unlock_safe(&st);

          sleep(1);
          hashpipe_status_lock_safe(&st);
          hgetl(st.buf, "CLEANA", &cleanA);
          if (useB) {
            hgetl(st.buf, "CLEANB", &cleanB);
          }
          if (useC) {
            hgetl(st.buf, "CLEANC", &cleanC);
          }
          hashpipe_status_unlock_safe(&st);
        }
        next_state = CLEANUP;
        printf("NET: All other threads have been informed\n");
      }
    }

    /************************************************************
     * CLEANUP state processing
     ************************************************************/
    // If in CLEANUP state, cleanup and reinitialize. Proceed to IDLE state.
    if (cur_state == CLEANUP) {
      printf("NET: Finished scan... preparing for next...\n");
      print_block_info(&binfo);
      print_block_packet_count(&binfo);
      print_block_mcnts(db);
      //cleanup_blocks(db);

      // reinitalize block info for next scan
      binfo = newScan;
      // reinit db blocks
      for (ii = 0; ii < N_INPUT_BLOCKS; ++ii) {
        if (flag_input_databuf_busywait_free(db, ii) != HASHPIPE_OK) {
          if (errno == EINTR) {
            // Interrupted by signal, return -1
            hashpipe_error(__FUNCTION__, "interrupted by signal waiting for free databuf");
            pthread_exit(NULL);
          } else {
            hashpipe_error(__FUNCTION__, "error waiting for free databuf");
            pthread_exit(NULL);
          } 
        }

        initialize_block(db, scan_start_mcnt + ii*Nm);
      }
      print_block_info(&binfo);
      print_block_mcnts(db);
#ifdef SIM_PACKETS
      binfo.sim_pkt_counter = 0;
#endif
      // Set correlator's starting mcnt to 0
      hashpipe_status_lock_safe(&st);
      hputi4(st.buf, "NETMCNT", 0);
      hashpipe_status_unlock_safe(&st);
      // TODO: old cleanup_blocks set all blocks free? Do we need to do that?

      // Check other threads to make sure they've finished cleaning up
      int cleanA=0, cleanB=0, cleanC=0;
      int netready = 0;

      cleanB = useB ? 0 : 1;
      cleanC = useC ? 0 : 1;

      hashpipe_status_lock_safe(&st);
      hgetl(st.buf, "CLEANA", &cleanA);
      if (useB) {
        hgetl(st.buf, "CLEANB", &cleanB);
      }
      if (useC) {
        hgetl(st.buf, "CLEANC", &cleanC);
      }
      hashpipe_status_unlock_safe(&st);
      netready = cleanA & cleanB & cleanC;

      // Old cleanup logic...keeping around until vetted for awhile.
      // int traclean = 0;
      // int corclean = 0;
      // int netready = 0;
      // int lastclean;
      // if (useC) {
      //     lastclean = 0;   
      // }
      // else {
      //     lastclean = 1;
      // }
      // hashpipe_status_lock_safe(&st);
      // hgetl(st.buf, "CLEANA",  &traclean);
      // hgetl(st.buf, "CLEANB",  &corclean);
      // if (useC) {
      //     hgetl(st.buf, "CLEANC", &lastclean);
      // }
      // hashpipe_status_unlock_safe(&st);
      // netready = traclean & corclean & lastclean;

      if (netready) {
        next_state = IDLE;
        flag_databuf_clear((hashpipe_databuf_t *) db);
        printf("NET: CLEANUP complete; clearing output databuf and returning to IDLE\n");
      }
      else {
        next_state = CLEANUP;
        sleep(1);
      }
    }

    // Update state variable if needed
    if (next_state != cur_state) {
      hashpipe_status_lock_safe(&st);
      switch (next_state) {
        case IDLE: hputs(st.buf, status_key, "IDLE"); break;
        case ACQUIRE: hputs(st.buf, status_key, "ACQUIRE"); printf("\nNET: Moving back to ACQUIRE!\n"); break;
        case CLEANUP: hputs(st.buf, status_key, "CLEANUP"); break;
      }
      hashpipe_status_unlock_safe(&st);
      cur_state = next_state;
    }

    /* Will exit if thread has been cancelled */
    pthread_testcancel();
  }

  // log mcnts for this run
  print_rx_mcnts();

#ifndef SIM_PACKETS
  pthread_cleanup_pop(1); /* Closes push(hashpipe_udp_close) */
#endif
  hashpipe_status_lock_busywait_safe(&st);
  printf("NET: Exiting thread loop...\n");
  hputs(st.buf, status_key, "terminated");
  hashpipe_status_unlock_safe(&st);
  return NULL;
}


static hashpipe_thread_desc_t net_thread = {
name: "flag_net_transpose_thread",
      skey: "NETSTAT",
      init: NULL,
      run:  run,
      ibuf_desc: {NULL},
      obuf_desc: {flag_input_databuf_create}
};


static __attribute__((constructor)) void ctor() {
  register_hashpipe_thread(&net_thread);
}


