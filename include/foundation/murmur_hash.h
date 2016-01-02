#pragma once

#include "types.h"

namespace foundation
{
	/// Implementation of the 64 bit MurmurHash2 function
	/// http://murmurhash.googlepages.com/
	uint64_t murmur_hash_64(const void *key, uint32_t len, uint64_t seed);

    /// Implementation of the 32 bit MurmurHash3 function
    uint32_t murmur3_hash_32( const void *key, uint32_t len, uint32_t seed );

    /// Implementation of the 64 bit MurmurHash3 function
    /// (get low 64bits of 128 bits hash)
    uint64_t murmur3_hash_64( const void *key, uint32_t len, uint64_t seed );

    /// Special cased implementation of the 64 bit MurmurHash3 function taking two 64 bits integer as input.
    uint64_t murmur3_hash_64_mix( uint64_t key1, uint64_t key2, uint64_t seed );



}