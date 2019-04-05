#ifndef XGPU_INFO_FRB_H
#define XGPU_INFO_FRB_H

#include "xgpu.h" // For MATRIX_ORDER_XXX macros

// Sizing parameters (fixed for now)
#ifndef NPOL
#define NPOL 2
#endif

#ifndef NSTATION
#define NSTATION 32
#endif

// 50 point fft x 5 coarse channels
#ifndef NFREQUENCY
#define NFREQUENCY 250
#endif

// 4250 samples/50 point fft = 85 (go up to the nearest power of 2 which is 128)
#ifndef NTIME
#define NTIME 128
#endif

// 4250 samples/50 point fft = 85 (go up to the nearest power of 2 which is 128)
#ifndef NTIME_PIPE
#define NTIME_PIPE 128
#endif

// Ensure that NTIME_PIPE is a multiple of 4
#if (NTIME_PIPE/4)*4 != NTIME_PIPE
#error NTIME_PIPE must be a multiple of 4
#endif

// Ensure that NTIME is a multiple of NTIME_PIPE
#if (NTIME/NTIME_PIPE)*NTIME_PIPE != NTIME
#error NTIME must be a multiple of NTIME_PIPE
#else
#define PIPE_LENGTH (NTIME/NTIME_PIPE)
#endif

// Derived from NSTATION (do not change)
#define NBASELINE ((NSTATION+1)*(NSTATION/2))

// how many pulsars are we binning for (Not implemented yet)
#define NPULSAR 0

// Define MATRIX_ORDER based on which MATRIX_ORDER_XXX is defined.
// There are three matrix packing options:
//
// TRIANGULAR_ORDER
// REAL_IMAG_TRIANGULAR_ORDER
// REGISTER_TILE_TRIANGULAR_ORDER (default) (not anymore, RB 2/17)
//
// To specify the matrix ordering scheme at library compile time, use one of
// these options to the compiler:
//
// -DMATRIX_ORDER_TRIANGULAR
// -DMATRIX_ORDER_REAL_IMAG
// -DMATRIX_ORDER_REGISTER_TILE
//
// Note that -DMATRIX_ORDER_REGISTER_TILE is assumed if neither of the other
// two are specified.
// (Not if I can help it-- RB 2/17)

#if defined MATRIX_ORDER_REGISTER_TILE
#define MATRIX_ORDER REGISTER_TILE
#elif defined MATRIX_ORDER_REAL_IMAG
#define MATRIX_ORDER REAL_IMAG_TRIANGULAR_ORDER
#else
#define MATRIX_ORDER TRIANGULAR_ORDER
#endif

// Set the number bytes transferred per shared memory transaction
#ifndef SHARED_ATOMIC_SIZE
#define SHARED_ATOMIC_SIZE 8
#endif

#endif // XGPU_INFO_FRB_H
