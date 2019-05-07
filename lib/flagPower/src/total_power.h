#ifndef TOTAL_POWER_H
#define TOTAL_POWER_H

#include <stdlib.h>
#include <stdio.h>

#define NF 4 //8 //3 //32 //8 //3 		// Number of Fengines
#define NI 6 //8 //6  //8 //6 		// Number of inputs per Fengine
#define NA 32 //(NF*NI) //64 //32 //(NF*NI) 	// Number of total antennas
#define NC 8 // 25 // 20 		// Number of frequency channels
#define NT 85 // 20 		// Number of time samples per packet/mcnt
#define NM 50 // 200 // 400 	// Number of packets/mcnts per block
#define pow1 32768 	// Next power of 2 >= Nm*Nt
#define nblocks2 (pow1/1024) // Block size for second kernel
#define nblocks1 (pow1/nblocks2) // Block size for first kernel
#define pow2 32 	// Next power of 2 >= Nc

#ifdef __cplusplus
extern "C" {
#endif
void initTotalPower();
void getTotalPower(unsigned char * input, float * output);
#ifdef __cplusplus
}
#endif

#endif
