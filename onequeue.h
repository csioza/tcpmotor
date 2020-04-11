#ifndef ONE_QUEUE_H
#define ONE_QUEUE_H

#include <iostream> 
#include <atomic>
///////////////////////////////////////////////////////////////////////////////
//lock free queue, one producter, one consumer
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class OneQueue {
public:
    OneQueue(size_t qlen);
    ~OneQueue();
    bool Push(const T& val);
    bool Pop(T* ptr);
    inline size_t used_size() {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        return (tail >= head) ? (tail - head) : (tail + qlen_ - head);
    }
    inline size_t free_size() {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_relaxed);
        return (head > tail) ? (head - 1 - tail) : (head - 1 + qlen_ - tail);
    }
private:
    size_t qlen_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    T* array_;
};
template <typename T> OneQueue<T>::OneQueue(size_t qlen) : qlen_(qlen), head_(0), tail_(0) { array_ = new T[qlen]; }
template <typename T> OneQueue<T>::~OneQueue() { delete[] array_; }
template <typename T> bool OneQueue<T>::Push(const T& val) {
    if (free_size() <= 0)
        return false;
    size_t tail = tail_.load(std::memory_order_relaxed);
    array_[tail++] = val;
    if (tail >= qlen_)
        tail -= qlen_;
    tail_.store(tail, std::memory_order_relaxed);
    return true;
}
template <typename T> bool OneQueue<T>::Pop(T* ptr) {
    if (used_size() <= 0)
        return false;
    size_t head = head_.load(std::memory_order_relaxed);
    *ptr = std::move(array_[head++]);
    if (head >= qlen_)
        head -= qlen_;
    head_.store(head, std::memory_order_relaxed);
    return true;
}
#endif // ONE_QUEUE_H