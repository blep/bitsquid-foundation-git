#pragma once

#include <stdint.h>
#include <string.h>
#include "murmur_hash.h"

namespace foundation {

struct HString32
{
    uint32_t hash_;

    explicit HString32( uint32_t h )
        : hash_( h )
    {
    }

    explicit HString32( const char *id )
        : hash_( murmur3_hash_32(id, static_cast<uint32_t>( strlen(id)), 0 ) )
    {
    }

    bool operator <( const HString32 &other ) const
    {
        return hash_ < other.hash_;
    }

    bool operator ==( const HString32 &other ) const
    {
        return hash_ == other.hash_;
    }

    bool operator !=( const HString32 &other ) const
    {
        return hash_ != other.hash_;
    }

    uint32_t hash() const
    {
        return hash_;
    }

};




struct HString64
{
    uint64_t hash_;

    explicit HString64( uint64_t h )
        : hash_( h )
    {
    }

    explicit HString64( const char *id  )
        : hash_( murmur3_hash_64( id, static_cast<uint32_t>( strlen( id ) ), 0 ) )
    {
    }

    bool operator <( const HString64 &other ) const
    {
        return hash_ < other.hash_;
    }

    bool operator ==( const HString64 &other ) const
    {
        return hash_ == other.hash_;
    }

    bool operator !=( const HString64 &other ) const
    {
        return hash_ != other.hash_;
    }

    uint64_t hash() const
    {
        return hash_;
    }

};

} // namespace foundation {
