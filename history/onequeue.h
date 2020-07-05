/* Copyright (c) 2020 科英 <csioza@163.com>. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include <atomic>
///////////////////////////////////////////////////////////////////////////////
//lock free queue, one producter, one consumer
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class OneQueue {
public:
    OneQueue(size_t qlen) : qlen_(qlen), head_(0), tail_(0) { array_ = new T[qlen]; }
    OneQueue(OneQueue const&) = delete;
    OneQueue& operator=(OneQueue const&) = delete;
    ~OneQueue() { delete[] array_; }
    bool Push(const T& val) { 
        if (free_size() <= 0)
            return false;
        size_t tail = tail_.load(std::memory_order_acquire);// memory_order_relaxed
        array_[tail++] = val;
        if (tail >= qlen_)
            tail -= qlen_;
        tail_.store(tail, std::memory_order_release);
        return true;
    }
    bool Pop(T& ptr) {
        if (used_size() <= 0)
            return false;
        size_t head = head_.load(std::memory_order_acquire);
        ptr = std::move(array_[head++]);
        if (head >= qlen_)
            head -= qlen_;
        head_.store(head, std::memory_order_release);
        return true;
    }
    inline size_t used_size() {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (tail >= head) ? (tail - head) : (tail + qlen_ - head);
    }
    inline size_t free_size() {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (head > tail) ? (head - 1 - tail) : (head - 1 + qlen_ - tail);
    }
private:
    size_t qlen_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    T* array_;
};