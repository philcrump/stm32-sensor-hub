#ifndef __HWRANDOM_H__
#define __HWRANDOM_H__

/* Returns the second of two non-matching results. Adds uncertain latency of at least 2us+? */
//#define HWRANDOM_FIPS

/* Uniform distribution for max == 2^32, or uniform-ish for max <<< 2*32 */
/* ~2us between random numbers generated (40 periods of RNG clock, usually ~24MHz) */
uint32_t hwrandom(uint32_t max);

#endif /* __HWRANDOM_H__ */