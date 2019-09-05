#ifndef _FLAG_DATABUF_H
#define _FLAG_DATABUF_H

#include <stdint.h>
#include "cublas_beamformer.h"
#include "total_power.h"
#include "hashpipe_databuf.h"
#include "pfb.h"
#include "config.h"

#define VERBOSE 0
#define SAVE 1
#define DEBUG 0

// Total number of antennas (nominally 40)
//#define N_INPUTS 64
//#define N_INPUTS 192L // L suffix turns this constant into a 'long' type instead of an int needed for later definition 'N_BYTES_PER_PFB_BLOCK'
#define N_INPUTS 32 //The correlator needs a multiple of 32. We "zero pad" the buffer to acheive 32 instead of 18

#if N_INPUTS!=(2*XGPU_NSTATION)
    #warning "N_INPUTS must match inputs needed by xGPU"
#endif
// xGPU needs a multiple of 32 inputs. The real number is...
#define N_REAL_INPUTS 18 // Should be 18, but needs to be divisible by 8 //40


// Number of antennas per F engine
// Determined by F engine DDL cards
#define N_INPUTS_PER_FENGINE 6 //8 //6 //8

// Number of F engines
#define N_FENGINES 4 //(N_INPUTS/N_INPUTS_PER_FENGINE)
#define N_REAL_FENGINES (N_REAL_INPUTS/N_INPUTS_PER_FENGINE)

// Number of X engines
#define N_XENGINES 12 //20

// Number of inputs per packet
#define N_INPUTS_PER_PACKET N_INPUTS_PER_FENGINE

// Number of time samples per packet
#define N_TIME_PER_PACKET 85 //20
#if N_TIME_PER_PACKET != NT
    #warning "N_TIME_PER_PACKET must match NT from total_power.h"
#endif
#define SAMP_RATE 400e6 //155.52e6
#define COARSE_SAMP_RATE (SAMP_RATE/256)
#define N_MCNT_PER_SECOND (COARSE_SAMP_RATE/N_TIME_PER_PACKET)

// Number of bits per I/Q sample
// Determined by F engine packetizer
#define N_BITS_IQ 8

// Total number of processed channels
#define N_CHAN 96 // Total number of FPGA input channels - channels thrown away after FFT = 256-160 = 96

// Number of channels per packet
#define N_CHAN_PER_PACKET (N_CHAN/N_XENGINES)

// Number of channels processed per XGPU instance?
#define N_CHAN_PER_X 8 //25
#if N_CHAN_PER_X!=XGPU_NFREQUENCY
    #warning "N_CHAN_PER_X must match frequency channels needed by xGPU"
#endif
#if N_CHAN_PER_X != BN_BIN
    #warning "N_CHAN_PER_X must match BN_BIN from cublas_beamformer.h"
#endif

// Number of time samples processed per XGPU instance
#define N_TIME_PER_BLOCK 4250 // 85 time samples per packet x 50 mcnts //4000
#if N_TIME_PER_BLOCK!=XGPU_NTIME
    #warning "N_TIME_PER_BLOCK must match the time samples needed by xGPU"
#endif
#if N_TIME_PER_BLOCK!=BN_TIME
    #warning "N_TIME_PER_BLOCK must match BN_TIME from cublas_beamformer.h"
#endif

// Number of bytes per packet (The + 8 offset is for the 8 byte header)
#define N_BYTES_PER_PACKET ((N_BITS_IQ * 2)*(N_INPUTS_PER_FENGINE)*N_CHAN_PER_PACKET/8*N_TIME_PER_PACKET + 8)

// Number of bytes in packet payload
#define N_BYTES_PER_PAYLOAD (N_BYTES_PER_PACKET - 8)

// Number of bytes per block
#define N_BYTES_PER_BLOCK (N_TIME_PER_BLOCK * N_CHAN_PER_X * N_INPUTS * N_BITS_IQ * 2 / 8)
#define N_REAL_BYTES_PER_BLOCK (N_TIME_PER_BLOCK * N_CHAN_PER_X * N_REAL_INPUTS * N_BITS_IQ * 2 / 8)
// #define N_BYTES_PER_BLOCK (N_TIME_PER_BLOCK * N_CHAN_PER_PACKET * N_INPUTS)

// Number of packets per block
#define N_PACKETS_PER_BLOCK (N_BYTES_PER_BLOCK / N_BYTES_PER_PAYLOAD)

// Paper parameters //////////////////////////////////////////////////////
#define N_SUB_BLOCKS_PER_INPUT_BLOCK (N_TIME_PER_BLOCK / N_TIME_PER_PACKET)

#define N_PACKETS_PER_BLOCK_PER_F    (N_PACKETS_PER_BLOCK / N_FENGINES)
////////////////////////////////////////////////////////////////////////

#define N_REAL_PACKETS_PER_BLOCK (N_REAL_BYTES_PER_BLOCK / N_BYTES_PER_PAYLOAD)

// Macro to compute data word offset for complex data word
#define Nm (N_TIME_PER_BLOCK/N_TIME_PER_PACKET) // Number of mcnts per block
#define Nf (N_FENGINES) // Number of fengines
#define Nt (N_TIME_PER_PACKET) // Number of time samples per packet
#define Nc (N_CHAN_PER_PACKET) // Number of channels per packet
#define Ne 8 // Number of antennas per SNAP board + 2 to accomodate xGPU

//#define flag_input_databuf_idx(m,f,t,c) ((2*N_INPUTS_PER_FENGINE/sizeof(uint64_t))*(c+Nc*(t+Nt*(f+Nf*m))))
#define flag_input_e_databuf_idx(m,f,t,c,e) (2*(e + (Ne-2)*(c+Nc*(t+Nt*(f+Nf*m)))))

// Macro to compute data word offset for transposed matrix
//#define flag_gpu_input_databuf_idx(m,f,t,c) ((2*N_INPUTS_PER_FENGINE/sizeof(uint64_t))*(f+Nf*(c+Nc*(t+Nt*m))))
#define flag_gpu_input_e_databuf_idx(m,f,t,c,e) (2*(e+(Ne)*(f+Nf*(c+Nc*(t+Nt*m)))))

// Number of entries in output correlation matrix
// #define N_COR_MATRIX (N_INPUTS*(N_INPUTS + 1)/2*N_CHAN_PER_X)
#define N_COR_MATRIX (N_INPUTS/2*(N_INPUTS/2 + 1)/2*N_CHAN_PER_X*4)
#define N_BEAM_SAMPS (2*BN_OUTPUTS)
#define N_POWER_SAMPS NA

// FRB and PFB unecessary for ONR, but kept for now //////////////////////////////////////////////////
// Macros specific to the rapid-dump correlator (FRB correlator)
#define N_TIME_PER_FRB_BLOCK XGPU_FRB_NTIME
#define N_CHAN_PER_FRB_BLOCK XGPU_FRB_NFREQUENCY
#define N_FRB_BLOCKS_PER_BLOCK (N_TIME_PER_BLOCK/N_TIME_PER_FRB_BLOCK)
#define N_BYTES_PER_FRB_BLOCK (N_TIME_PER_FRB_BLOCK * N_CHAN_PER_FRB_BLOCK * N_INPUTS * N_BITS_IQ * 2 / 8)
#define N_GPU_FRB_INPUT_BLOCKS (N_GPU_INPUT_BLOCKS*N_FRB_BLOCKS_PER_BLOCK)
#define N_MCNT_PER_FRB_BLOCK (Nm/N_FRB_BLOCKS_PER_BLOCK)
#define N_FRB_COR_MATRIX (N_INPUTS/2*(N_INPUTS/2 + 1)/2*N_CHAN_PER_FRB_BLOCK*4)

// Macro to comput data word offset for tranposed matrix in FRB mode
//#define flag_frb_gpu_input_databuf_idx(m,f,t,c) ((2*N_INPUTS_PER_FENGINE/sizeof(uint64_t))*(f+Nf*(c+N_CHAN_PER_FRB_BLOCK*(t+Nt*m))))
#define flag_frb_gpu_input_e_databuf_idx(m,f,t,c,e) ((2/sizeof(uint8_t))*(e+Ne*(f+Nf*(c+N_CHAN_PER_FRB_BLOCK*(t+Nt*m)))))

// Macros specific to the fine-channel correlator (PFB correlator)
#define N_TIME_PER_PFB_BLOCK XGPU_PFB_NTIME
#define N_CHAN_PER_PFB_BLOCK XGPU_PFB_NFREQUENCY
#define N_CHAN_PFB_SELECTED 5
#define N_PFB_COR_MATRIX (N_INPUTS/2*(N_INPUTS/2 + 1)/2*N_CHAN_PER_PFB_BLOCK*4)
#define N_BYTES_PER_PFB_BLOCK (N_TIME_PER_BLOCK * N_CHAN_PER_PFB_BLOCK * N_INPUTS * N_BITS_IQ * 2 / 8)
//#define N_BYTES_PER_PFB_BLOCK ((unsigned int)(N_TIME_PER_BLOCK * N_CHAN_PER_PFB_BLOCK * N_INPUTS * N_BITS_IQ * 2 / 8)) // The (unsigned int) cast converts the 'data' type initialized below, and lets the compiler know that you understand that the long value will fit into an 'unsigned int' and thus hides any warning it may throw at you ('integer overflow in expression' error)

//#define flag_pfb_gpu_input_databuf_idx(m,f,t,c) ((2*N_INPUTS_PER_FENGINE/sizeof(uint64_t))*(f+Nf*(c+N_CHAN_PFB_SELECTED*(t+Nt*m))))
#define flag_pfb_gpu_input_e_databuf_idx(m,f,t,c,e) ((2/sizeof(uint8_t))*(e+Ne*(f+Nf*(c+N_CHAN_PFB_SELECTED*(t+Nt*m)))))
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Macros to maintain cache alignment
#define CACHE_ALIGNMENT (128)
typedef uint8_t hashpipe_databuf_cache_alignment[
    CACHE_ALIGNMENT - (sizeof(hashpipe_databuf_t)%CACHE_ALIGNMENT)
];

/*
 * INPUT (NET) BUFFER STRUCTURES
 * This buffer is where captured data from the network is stored.
 * It is the output buffer of the flag_net_thread.
 * It is the input buffer of the flag_transpose_thread.
 */

#define N_INPUT_BLOCKS 4 //50
//RTBF number of input blocks
//#define N_INPUT_BLOCKS 100 //50

// A typedef for a block header
typedef struct flag_input_header {
    int64_t  good_data;
    uint64_t mcnt_start;
} flag_input_header_t;

typedef uint8_t flag_input_header_cache_alignment[
    CACHE_ALIGNMENT - (sizeof(flag_input_header_t)%CACHE_ALIGNMENT)
];

// A typedef for a block of data in the buffer
typedef struct flag_input_block {
    flag_input_header_t header;
    flag_input_header_cache_alignment padding;
    //uint64_t data[N_BYTES_PER_BLOCK/sizeof(uint64_t)];
    uint8_t data[N_BYTES_PER_BLOCK/sizeof(uint8_t)]; // Data type change to make it more dynamic
} flag_input_block_t;

// The data buffer structure
typedef struct flag_input_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding; // Only to maintain alignment
    flag_input_block_t block[N_INPUT_BLOCKS];
} flag_input_databuf_t;

/*
 * GPU INPUT BUFFER STRUCTURES
 * This buffer is where the reordered data for input to xGPU is stored.
 * It is the output buffer of the flag_transpose_thread.
 * It is the input buffer of the flag_correlator_thread.
 */
#define N_GPU_INPUT_BLOCKS 20

// A typedef for a GPU input block header
typedef struct flag_gpu_input_header {
    int64_t  good_data;
    uint64_t mcnt;
} flag_gpu_input_header_t;

typedef uint8_t flag_gpu_input_header_cache_alignment[
    CACHE_ALIGNMENT - (sizeof(flag_gpu_input_header_t)%CACHE_ALIGNMENT)
];

// A typedef for a block of data in the buffer
typedef struct flag_gpu_input_block {
    flag_gpu_input_header_t header;
    flag_gpu_input_header_cache_alignment padding;
    //uint64_t data[N_BYTES_PER_BLOCK/sizeof(uint64_t)];
    uint8_t data[N_BYTES_PER_BLOCK/sizeof(uint8_t)]; // Data type changed to make it more dynamic
} flag_gpu_input_block_t;

// The data buffer structure
typedef struct flag_gpu_input_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_gpu_input_block_t block[N_GPU_INPUT_BLOCKS];
} flag_gpu_input_databuf_t;


// FRB and PFB unecessary for ONR, but kept for now //////////////////////////////////////////////////
/*
 * FRB GPU INPUT BUFFER STRUCTURES
 */

// A typedef for a block of data in the buffer
typedef struct flag_frb_gpu_input_block {
    flag_gpu_input_header_t header;
    flag_gpu_input_header_cache_alignment padding;
    //uint64_t data[N_BYTES_PER_FRB_BLOCK/sizeof(uint64_t)];
    uint8_t data[N_BYTES_PER_FRB_BLOCK/sizeof(uint8_t)]; // Data type changed to make it more dynamic
} flag_frb_gpu_input_block_t;

// The data buffer structure
typedef struct flag_frb_gpu_input_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_frb_gpu_input_block_t block[N_GPU_FRB_INPUT_BLOCKS];
} flag_frb_gpu_input_databuf_t;


/*
 * PFB GPU INPUT BUFFER STRUCTURES
 */

// A typedef for a block of data in the buffer
typedef struct flag_pfb_gpu_input_block {
    flag_gpu_input_header_t header;
    flag_gpu_input_header_cache_alignment padding;
    //uint64_t data[N_BYTES_PER_PFB_BLOCK/sizeof(uint64_t)];
    uint8_t data[N_BYTES_PER_PFB_BLOCK/sizeof(uint8_t)]; // Data type changed to make it more dynamic
} flag_pfb_gpu_input_block_t;

// The data buffer structure
typedef struct flag_pfb_gpu_input_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_pfb_gpu_input_block_t block[N_GPU_INPUT_BLOCKS];
} flag_pfb_gpu_input_databuf_t;
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * GPU OUTPUT BUFFER STRUCTURES
 */
#define N_GPU_OUT_BLOCKS 20

// A typedef for a correlator output block header
typedef struct flag_gpu_output_header {
    int64_t  good_data;
    uint64_t mcnt;
    // uint64_t flags[(N_CHAN_PER_X+63)/64];
} flag_gpu_output_header_t;

typedef uint8_t flag_gpu_output_header_cache_alignment[
    CACHE_ALIGNMENT - (sizeof(flag_gpu_output_header_t)%CACHE_ALIGNMENT)
];


/**********************************************************************************
 * There are various different types of GPU output buffers that will all share
 * the same header information.
 * (1) flag_gpu_correlator_output_block
 * (1a) flag_frb_gpu_correlator_output_block
 * (2) flag_gpu_beamformer_output_block
 * (3) flag_gpu_power_output_block
 **********************************************************************************/

// flag_gpu_correlator_output_block
typedef struct flag_gpu_correlator_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[2*N_COR_MATRIX]; // x2 for real/imaginary samples
} flag_gpu_correlator_output_block_t;

// flag_gpu_correlator_output_databuf
typedef struct flag_gpu_correlator_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_gpu_correlator_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_gpu_correlator_output_databuf_t;

// FRB and PFB unecessary for ONR, but kept for now //////////////////////////////////////////////////
// flag_frb_gpu_correlator_output_block
typedef struct flag_frb_gpu_correlator_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[2*N_FRB_COR_MATRIX]; // x2 for real/imaginary samples
} flag_frb_gpu_correlator_output_block_t;

// flag_frb_gpu_correlator_output_databuf
typedef struct flag_frb_gpu_correlator_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_frb_gpu_correlator_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_frb_gpu_correlator_output_databuf_t;

// flag_pfb_gpu_correlator_output_block
typedef struct flag_pfb_gpu_correlator_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[2*N_PFB_COR_MATRIX]; // x2 for real/imaginary samples
} flag_pfb_gpu_correlator_output_block_t;

// flag_pfb_gpu_correlator_output_databuf
typedef struct flag_pfb_gpu_correlator_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_pfb_gpu_correlator_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_pfb_gpu_correlator_output_databuf_t;
///////////////////////////////////////////////////////////////////////////////////////////////////////

// flag_gpu_beamformer_output_block
typedef struct flag_gpu_beamformer_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[N_BEAM_SAMPS]; 
} flag_gpu_beamformer_output_block_t;

// flag_gpu_beamformer_output_databuf
typedef struct flag_gpu_beamformer_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_gpu_beamformer_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_gpu_beamformer_output_databuf_t;

// PFB and total power unecessary for ONR, but kept for now ///////////////////////////////////////////
//flag_gpu_pfb_output_block
typedef struct flag_gpu_pfb_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[PFB_OUTPUT_BLOCK_SIZE];
} flag_gpu_pfb_output_block_t;

// flag_gpu_pfb_output_databuf
typedef struct flag_gpu_pfb_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_gpu_pfb_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_gpu_pfb_output_databuf_t;

// flag_gpu_power_output_block
typedef struct flag_gpu_power_output_block {
    flag_gpu_output_header_t header;
    flag_gpu_output_header_cache_alignment padding;
    float data[N_POWER_SAMPS];
} flag_gpu_power_output_block_t;

// flag_gpu_power_output_databuf
typedef struct flag_gpu_power_output_databuf {
    hashpipe_databuf_t header;
    hashpipe_databuf_cache_alignment padding;
    flag_gpu_power_output_block_t block[N_GPU_OUT_BLOCKS];
} flag_gpu_power_output_databuf_t;
///////////////////////////////////////////////////////////////////////////////////////////////////////


/*********************
 * Input Buffer Functions
 *********************/
hashpipe_databuf_t * flag_input_databuf_create(int instance_id, int databuf_id);

int flag_input_databuf_wait_free   (flag_input_databuf_t * d, int block_id);
int flag_input_databuf_wait_filled (flag_input_databuf_t * d, int block_id);
int flag_input_databuf_busywait_free   (flag_input_databuf_t * d, int block_id);
int flag_input_databuf_busywait_filled (flag_input_databuf_t * d, int block_id);
int flag_input_databuf_set_free    (flag_input_databuf_t * d, int block_id);
int flag_input_databuf_set_filled  (flag_input_databuf_t * d, int block_id);

/*********************
 * Input Buffer Functions
 *********************/
hashpipe_databuf_t * flag_gpu_input_databuf_create(int instance_id, int databuf_id);
int flag_gpu_input_databuf_wait_free   (flag_gpu_input_databuf_t * d, int block_id);
int flag_gpu_input_databuf_wait_filled (flag_gpu_input_databuf_t * d, int block_id);
int flag_gpu_input_databuf_set_free    (flag_gpu_input_databuf_t * d, int block_id);
int flag_gpu_input_databuf_set_filled  (flag_gpu_input_databuf_t * d, int block_id);

// FRB and PFB unecessary for ONR, but kept for now ///////////////////////////////////////////////////
hashpipe_databuf_t * flag_frb_gpu_input_databuf_create(int instance_id, int databuf_id);
int flag_frb_gpu_input_databuf_wait_free   (flag_frb_gpu_input_databuf_t * d, int block_id);
int flag_frb_gpu_input_databuf_wait_filled (flag_frb_gpu_input_databuf_t * d, int block_id);
int flag_frb_gpu_input_databuf_set_free    (flag_frb_gpu_input_databuf_t * d, int block_id);
int flag_frb_gpu_input_databuf_set_filled  (flag_frb_gpu_input_databuf_t * d, int block_id);

hashpipe_databuf_t * flag_pfb_gpu_input_databuf_create(int instance_id, int databuf_id);
int flag_pfb_gpu_input_databuf_wait_free   (flag_pfb_gpu_input_databuf_t * d, int block_id);
int flag_pfb_gpu_input_databuf_wait_filled (flag_pfb_gpu_input_databuf_t * d, int block_id);
int flag_pfb_gpu_input_databuf_set_free    (flag_pfb_gpu_input_databuf_t * d, int block_id);
int flag_pfb_gpu_input_databuf_set_filled  (flag_pfb_gpu_input_databuf_t * d, int block_id);
void flag_pfb_gpu_input_databuf_clear(flag_pfb_gpu_input_databuf_t * d);
///////////////////////////////////////////////////////////////////////////////////////////////////////

void flag_databuf_clear(hashpipe_databuf_t * d);

/********************
 * GPU Output Buffer Functions
 ********************/
hashpipe_databuf_t * flag_gpu_correlator_output_databuf_create(int instance_id, int databuf_id);

int flag_gpu_correlator_output_databuf_wait_free   (flag_gpu_correlator_output_databuf_t * d, int block_id);
int flag_gpu_correlator_output_databuf_wait_filled (flag_gpu_correlator_output_databuf_t * d, int block_id);
int flag_gpu_correlator_output_databuf_set_free    (flag_gpu_correlator_output_databuf_t * d, int block_id);
int flag_gpu_correlator_output_databuf_set_filled  (flag_gpu_correlator_output_databuf_t * d, int block_id);

// FRB and PFB unecessary for ONR, but kept for now ///////////////////////////////////////////////////
hashpipe_databuf_t * flag_frb_gpu_correlator_output_databuf_create(int instance_id, int databuf_id);

int flag_frb_gpu_correlator_output_databuf_wait_free   (flag_frb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_frb_gpu_correlator_output_databuf_wait_filled (flag_frb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_frb_gpu_correlator_output_databuf_set_free    (flag_frb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_frb_gpu_correlator_output_databuf_set_filled  (flag_frb_gpu_correlator_output_databuf_t * d, int block_id);

hashpipe_databuf_t * flag_pfb_gpu_correlator_output_databuf_create(int instance_id, int databuf_id);

int flag_pfb_gpu_correlator_output_databuf_wait_free   (flag_pfb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_pfb_gpu_correlator_output_databuf_wait_filled (flag_pfb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_pfb_gpu_correlator_output_databuf_set_free    (flag_pfb_gpu_correlator_output_databuf_t * d, int block_id);
int flag_pfb_gpu_correlator_output_databuf_set_filled  (flag_pfb_gpu_correlator_output_databuf_t * d, int block_id);
///////////////////////////////////////////////////////////////////////////////////////////////////////

hashpipe_databuf_t * flag_gpu_beamformer_output_databuf_create(int instance_id, int databuf_id);

int flag_gpu_beamformer_output_databuf_wait_free   (flag_gpu_beamformer_output_databuf_t * d, int block_id);
int flag_gpu_beamformer_output_databuf_wait_filled (flag_gpu_beamformer_output_databuf_t * d, int block_id);
int flag_gpu_beamformer_output_databuf_set_free    (flag_gpu_beamformer_output_databuf_t * d, int block_id);
int flag_gpu_beamformer_output_databuf_set_filled  (flag_gpu_beamformer_output_databuf_t * d, int block_id);

// PFB and total power unecessary for ONR, but kept for now ///////////////////////////////////////////
hashpipe_databuf_t * flag_gpu_pfb_output_databuf_create(int instance_id, int databuf_id);

int flag_gpu_pfb_output_databuf_wait_free   (flag_gpu_pfb_output_databuf_t * d, int block_id);
int flag_gpu_pfb_output_databuf_wait_filled (flag_gpu_pfb_output_databuf_t * d, int block_id);
int flag_gpu_pfb_output_databuf_set_free    (flag_gpu_pfb_output_databuf_t * d, int block_id);
int flag_gpu_pfb_output_databuf_set_filled  (flag_gpu_pfb_output_databuf_t * d, int block_id);
void flag_gpu_pfb_output_databuf_clear(flag_gpu_pfb_output_databuf_t* d);

hashpipe_databuf_t * flag_gpu_power_output_databuf_create(int instance_id, int databuf_id);

int flag_gpu_power_output_databuf_wait_free   (flag_gpu_power_output_databuf_t * d, int block_id);
int flag_gpu_power_output_databuf_wait_filled (flag_gpu_power_output_databuf_t * d, int block_id);
int flag_gpu_power_output_databuf_set_free    (flag_gpu_power_output_databuf_t * d, int block_id);
int flag_gpu_power_output_databuf_set_filled  (flag_gpu_power_output_databuf_t * d, int block_id);
///////////////////////////////////////////////////////////////////////////////////////////////////////

// overloaded helper methods
int flag_pfb_gpu_correlator_output_databuf_total_status (flag_pfb_gpu_correlator_output_databuf_t * d);
int flag_gpu_pfb_output_databuf_total_status (flag_gpu_pfb_output_databuf_t * d);

int flag_pfb_gpu_input_databuf_total_status(flag_pfb_gpu_input_databuf_t * d);
int flag_gpu_correlator_output_databuf_total_status(flag_gpu_correlator_output_databuf_t * d);

int flag_input_databuf_total_status(flag_input_databuf_t * d);

#endif

























