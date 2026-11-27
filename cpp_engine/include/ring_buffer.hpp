#pragma once
#include <atomic>
#include <cstddef>
#include <new>
#include <array>

template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0 && (N & (N - 1)) == 0, "N must be power of two");
public:
    bool push(const T& v) noexcept {
        const std::size_t h = head_.load(std::memory_order_relaxed);
        const std::size_t next = (h + 1) & mask_;
        if (next == tail_.load(std::memory_order_acquire)) return false;
        buf_[h & mask_] = v;
        head_.store(next, std::memory_order_release);
        return true;
    }
    bool pop(T& out) noexcept {
        const std::size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) return false;
        out = buf_[t & mask_];
        tail_.store((t + 1) & mask_, std::memory_order_release);
        return true;
    }
    bool empty() const noexcept { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }
    std::size_t size() const noexcept {
        std::size_t h = head_.load(std::memory_order_acquire);
        std::size_t t = tail_.load(std::memory_order_acquire);
        return (h - t) & mask_;
    }
private:
    static constexpr std::size_t mask_ = N - 1;
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::array<T, N> buf_{};
};
