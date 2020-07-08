#ifndef __RANDOM_H__
#define __RANDOM_H__

/* Returns the second of two non-matching results. Adds uncertain latency of at least 2us+? */
//#define RANDOM_FIPS

/* Uniform distribution for max == 2^32, or uniform-ish for max <<< 2*32 */
/* ~2us between random numbers generated (40 periods of RNG clock, usually ~24MHz) */
uint32_t random(uint32_t max);

#endif /* __RANDOM_H__ */