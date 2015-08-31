#ifndef _FNV_H_
#define _FNV_H_

#include "aos/Config.h"

#include "ace/OS.h"

//#include <stdlib.h>
//#include <stdint.h>
//#include <sys/uio.h>

#include <string>

namespace aos {
namespace hash {

// FNV-1a hash
template <int N> class FNV_1a; 

template <>
class AOS_API FNV_1a<32>
{
    static const ACE_UINT32 FNV_32_PRIME = 0x01000193UL;
    ACE_UINT32 _M_offset;

public:
    static const ACE_UINT32 INIT  = 0x811c9dc5UL;

    FNV_1a(const ACE_UINT32 init = INIT)
    : _M_offset(init) 
    {}

    void offset(ACE_UINT32 init)
    { _M_offset = init; }

    ACE_UINT32 operator()(const std::string &_buf)
    {
        return operator()(_buf.c_str(), _buf.length());
    }

    ACE_UINT32 operator()(const char *_buf, size_t _len)
    {
        const unsigned char *bp = reinterpret_cast<const unsigned char *>(_buf); /* start of buffer */
        const unsigned char *be = bp + _len;                                     /* beyond end of buffer */

        ACE_UINT32 hval = _M_offset;

        /*
         * FNV-1a hash each octet in the buffer
         */
        while (bp < be) {

            /* xor the bottom with the current octet */
            hval ^= (ACE_UINT32)*bp++;

            /* multiply by the 32 bit FNV magic prime mod 2^32 */

#if defined(NO_FNV_GCC_OPTIMIZATION)
            hval *= FNV_32_PRIME;
#else
            hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
        }

        /* return our new hash value */
        return _M_offset = hval;
    }

    // iovec implementation...
    //

    ACE_UINT32 
    operator()(const struct iovec * _vector, size_t _count)
    {
        unsigned char *bp;  /* start of block */
        unsigned char *be;  /* beyond end of buffer */

        ACE_UINT32 hval = _M_offset;

        for (unsigned int lcount = 0; lcount < _count; lcount++) {
            bp = (unsigned char *)_vector[lcount].iov_base;
            be = (unsigned char *)_vector[lcount].iov_base + _vector[lcount].iov_len;

            /*
             * FNV-1a hash each octet in the buffer
             */
            while (bp < be) {
                /* xor the bottom with the current octet */
                hval ^= (ACE_UINT32) * bp++;

                /* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
                hval *= FNV_32_PRIME;
#else
                hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
            }

        }

        /* return our new hash value */
        return _M_offset = hval;
    }

};

template <>
class AOS_API FNV_1a<64>
{
    static const ACE_UINT64 FNV_64_PRIME = 0x100000001b3ULL;
    ACE_UINT64 _M_offset;

public:
    static const ACE_UINT64 INIT  = 0xcbf29ce484222325ULL;

    FNV_1a(const ACE_UINT64 init = INIT)
    : _M_offset(init) 
    {}

    void offset(ACE_UINT64 init)
    { _M_offset = init; }

    ACE_UINT64 operator()(const std::string &_buf)
    {
        return operator()(_buf.c_str(), _buf.length());
    }

    ACE_UINT64 operator()(const char *_buf, size_t _len)
    {
        const unsigned char *bp = reinterpret_cast<const unsigned char *>(_buf); /* start of buffer */
        const unsigned char *be = bp + _len;                                     /* beyond end of buffer */

        ACE_UINT64 hval = _M_offset;

        /*
         * FNV-1a hash each octet of the buffer
         */
        while (bp < be) {

            /* xor the bottom with the current octet */
            hval ^= (ACE_UINT64)*bp++;

            /* multiply by the 64 bit FNV magic prime mod 2^64 */

#if defined(NO_FNV_GCC_OPTIMIZATION)
            hval *= FNV_64_PRIME;
#else
            hval += (hval << 1) + (hval << 4) + (hval << 5) +
            (hval << 7) + (hval << 8) + (hval << 40);
#endif
        }

        return _M_offset = hval;
    }

    ACE_UINT64 
    operator()(const struct iovec * _vector, size_t _count)
    {
        unsigned char *bp;  /* start of buffer */
        unsigned char *be;  /* beyond end of buffer */

        ACE_UINT64 hval = _M_offset;

        for (unsigned int lcount = 0; lcount < _count; lcount++) {

            bp = (unsigned char *)_vector[lcount].iov_base;
            be = (unsigned char *)_vector[lcount].iov_base + _vector[lcount].iov_len;

            /*
             * FNV-1a hash each octet of the buffer
             */
            while (bp < be) {

                /* xor the bottom with the current octet */
                hval ^= (ACE_UINT64) * bp++;

                /* multiply by the 64 bit FNV magic prime mod 2^64 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
                hval *= FNV_64_PRIME;
#else
                hval += (hval << 1) + (hval << 4) + (hval << 5) +
                (hval << 7) + (hval << 8) + (hval << 40);
#endif
            }
        }

        /* return our new hash value */
        return _M_offset = hval;
    }

};

} // namespace hash
} // namespace aos

#endif // _FNV_H_

