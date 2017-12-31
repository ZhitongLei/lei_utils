// File Name: rate_limiter.h
// Author: lei
// Created Time: 2017-06-15 15:08:30
#ifndef __MMDATADISTRIBUTOR_TOOL_RATE_LIMITER_H__
#define __MMDATADISTRIBUTOR_TOOL_RATE_LIMITER_H__

#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

namespace mmdatadistributor {
namespace tool {

class RateLimiter {
public:
    RateLimiter() = default;
    virtual ~RateLimiter() {}
    RateLimiter(const RateLimiter &) = delete;
    RateLimiter& operator=(const RateLimiter &) = delete;

    virtual int64_t Aquire() = 0;

    // take ${permits} tokens from the bucket
    // return time spent sleeping to enforce rate, in millisecond; 0.0 if not rate-limited
    virtual int64_t Aquire(int32_t permits) = 0;

    virtual bool TryAquire(int32_t permits) = 0;
    virtual bool TryAquire(int32_t permits, int32_t timeout) = 0;

    virtual int32_t get_rate() const = 0;
    // rate must greater than 0
    virtual void set_rate(int32_t rate) = 0;

};

}
}

#endif
