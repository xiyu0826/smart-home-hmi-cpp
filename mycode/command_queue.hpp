#pragma once
// command_queue.hpp —— 线程安全的固定容量环形队列
//
// 替代原 C 版的 http_bridge.c 环形队列（pthread_mutex + 裸数组）。
// 用 std::mutex / std::condition_variable 保护；pop 时按值拷贝返回，
// 避免原版"解锁后继续解引用队列内部指针"的 TOCTOU 竞态。

#include <array>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace smarthome {

template <typename T, std::size_t Capacity>
class RingBuffer {
public:
    // 入队；队满时丢弃最旧元素（不阻塞，不扩容）。
    void push(const T &item) {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            std::size_t next = (tail_ + 1) % Capacity;
            if (next == head_) {
                head_ = (head_ + 1) % Capacity; // 满 → 前进 head 丢弃最旧
            }
            buf_[tail_] = item;
            tail_ = next;
        }
        cv_.notify_one();
    }

    // 出队；空则返回 nullopt。按值返回，调用方拿到独立副本。
    std::optional<T> pop() {
        std::lock_guard<std::mutex> lk(mutex_);
        if (head_ == tail_) return std::nullopt;
        T item = buf_[head_];
        head_ = (head_ + 1) % Capacity;
        return item;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mutex_);
        return head_ == tail_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::array<T, Capacity> buf_{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
};

} // namespace smarthome
