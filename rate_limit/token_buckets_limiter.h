#ifndef __MMDATADISTRIBUTOR_TOOL_TOKEN_BUCKETS_LIMITER_H__
#define __MMDATADISTRIBUTOR_TOOL_TOKEN_BUCKETS_LIMITER_H__

#include "rate_limiter.h"
#include <chrono>
#include <thread>

namespace mmdatadistributor {
namespace tool {

class TokenBucketsLimiter : public RateLimiter {
public:
    TokenBucketsLimiter(int32_t capacity, int32_t init_tokens)
        : m_capacity(capacity),
          m_tokens(init_tokens),
          m_rate(1)
        {
            m_start_time = std::chrono::system_clock::now();
        }

    int64_t Aquire() { return Aquire(1); }

    int64_t Aquire(int32_t permits) {
        if (permits <= 0) {
            std::runtime_error("RateLimiter: Must request positive amount of permits");
        }

        auto now = std::chrono::system_clock::now();
        int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
        int32_t new_tokens = (duration * m_rate) / 1000;
        m_start_time = now;

        m_tokens = std::min(m_capacity, m_tokens + new_tokens);
        // printf("new_tokens: %u, duration: %lu, m_tokens: %u\n", new_tokens, duration, m_tokens);
        int64_t wait_time = 0;
        if (permits > m_tokens) {
            wait_time = (permits - m_tokens) * 1000 / m_rate;
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
            m_tokens = 0;
            m_start_time = std::chrono::system_clock::now();
        } else { m_tokens -= permits; }
        
        return wait_time;
    }

    bool TryAquire(int32_t permits) { return TryAquire(permits, 0); }

    bool TryAquire(int32_t permits, int32_t timeout) {
        auto now = std::chrono::system_clock::now();
        int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
        int32_t new_tokens = (duration * m_rate) / 1000 + timeout * m_rate;
        int32_t expected_tokens = std::min(m_capacity, m_tokens + new_tokens);
        if (expected_tokens < permits) { return false; }
        else { Aquire(permits); }
        return true;            
    }

    int32_t get_rate() const { return m_rate; } 
    void set_rate(int32_t rate) { m_rate = rate; }

private:
    int32_t m_capacity;
    int32_t m_tokens;
    int32_t m_rate;
    std::chrono::system_clock::time_point m_start_time;
};

}
}

#endif
