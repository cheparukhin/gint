#ifndef TINYMT32_H
#define TINYMT32_H
/**
 * @file tinymt32.h
 *
 * @brief Tiny Mersenne Twister only 127 bit internal state
 *
 * @author Mutsuo Saito (Hiroshima University)
 * @author Makoto Matsumoto (University of Tokyo)
 *
 * Copyright (C) 2011 Mutsuo Saito, Makoto Matsumoto,
 * Hiroshima University and The University of Tokyo.
 * All rights reserved.
 *
 * The 3-clause BSD License is applied to this software, see
 * LICENSE.txt
 */

/* Note: this is a stripped-down version of TinyMT that only includes the
   32-bit integer generator. For the full version, please see
     <https://github.com/MersenneTwister-Lab/TinyMT>. */

#include <stdint.h>

/**
 * tinymt32 internal state vector and parameters
 */
struct TINYMT32_T {
    uint32_t status[4];
    uint32_t mat1;
    uint32_t mat2;
    uint32_t tmat;
};

typedef struct TINYMT32_T tinymt32_t;

uint32_t tinymt32_generate_uint32(tinymt32_t *random);
void tinymt32_init(tinymt32_t *random, uint32_t seed);

#endif
