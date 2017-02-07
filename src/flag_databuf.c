
#include "flag_databuf.h"

hashpipe_databuf_t * flag_input_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_input_block_t);
    int    n_block     = N_INPUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_input_databuf_wait_free(flag_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_input_databuf_wait_filled(flag_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_input_databuf_set_free(flag_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_input_databuf_set_filled(flag_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}



hashpipe_databuf_t * flag_gpu_input_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_gpu_input_block_t);
    int    n_block     = N_GPU_INPUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_gpu_input_databuf_wait_free(flag_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_input_databuf_wait_filled(flag_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_input_databuf_set_free(flag_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_input_databuf_set_filled(flag_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}



hashpipe_databuf_t * flag_frb_gpu_input_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_frb_gpu_input_block_t);
    int    n_block     = N_GRU_FRB_INPUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_frb_gpu_input_databuf_wait_free(flag_frb_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_frb_gpu_input_databuf_wait_filled(flag_frb_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_frb_gpu_input_databuf_set_free(flag_frb_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_frb_gpu_input_databuf_set_filled(flag_frb_gpu_input_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}



hashpipe_databuf_t * flag_gpu_correlator_output_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_gpu_correlator_output_block_t);
    int    n_block     = N_GPU_OUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_gpu_correlator_output_databuf_wait_free(flag_gpu_correlator_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_correlator_output_databuf_wait_filled(flag_gpu_correlator_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_correlator_output_databuf_set_free(flag_gpu_correlator_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_correlator_output_databuf_set_filled(flag_gpu_correlator_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}

hashpipe_databuf_t * flag_gpu_beamformer_output_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_gpu_beamformer_output_block_t);
    int    n_block     = N_GPU_OUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_gpu_beamformer_output_databuf_wait_free(flag_gpu_beamformer_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_beamformer_output_databuf_wait_filled(flag_gpu_beamformer_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_beamformer_output_databuf_set_free(flag_gpu_beamformer_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_beamformer_output_databuf_set_filled(flag_gpu_beamformer_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}


hashpipe_databuf_t * flag_gpu_power_output_databuf_create(int instance_id, int databuf_id) {
    size_t header_size = sizeof(hashpipe_databuf_t) + sizeof(hashpipe_databuf_cache_alignment);
    size_t block_size  = sizeof(flag_gpu_power_output_block_t);
    int    n_block     = N_GPU_OUT_BLOCKS;
    return hashpipe_databuf_create(
        instance_id, databuf_id, header_size, block_size, n_block);
}

int flag_gpu_power_output_databuf_wait_free(flag_gpu_power_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_power_output_databuf_wait_filled(flag_gpu_power_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_power_output_databuf_set_free(flag_gpu_power_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

int flag_gpu_power_output_databuf_set_filled(flag_gpu_power_output_databuf_t * d, int block_id) {
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}