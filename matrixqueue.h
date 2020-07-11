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

#define MATRIX_QUEUE_NUM_MAX_INDEX      1024    //一个进程创建MatrixQueue最大数量，注意要设置足够大，避免越界
#define MATRIX_QUEUE_ARRAY_MAX_NUM      1024    //MatrixQueue中生产者消费者矩阵大小，注意要设置足够大，避免越界
#define MATRIX_QUEUE_ARRAY_INIT_NUM     8       //保证初始化不久后的性能

#define INVALID_NUM                     -1
#define MATRIX_QUEUE_MAX_STEP           3

template <typename T>
class OneQueue 
{
public:
    OneQueue(size_t qlen) : qlen_(qlen), head_(0), tail_(0) { array_ = new T[qlen]; }
    OneQueue(OneQueue const&) = delete;
    OneQueue& operator=(OneQueue const&) = delete;
    OneQueue(OneQueue const&&) = delete;
    OneQueue& operator=(OneQueue const&&) = delete;
    ~OneQueue() { delete[] array_; }
    bool Push(const T& val) 
    {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        if (((head > tail) ? (head - 1 - tail) : (head - 1 + qlen_ - tail)) <= 0)
            return false;
        array_[tail++] = val;
        if (tail >= qlen_)
            tail -= qlen_;
        tail_.store(tail, std::memory_order_release);
        return true;
    }
    bool Pop(T& ptr) 
    {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        if (((tail >= head) ? (tail - head) : (tail + qlen_ - head)) <= 0)
            return false;
        ptr = std::move(array_[head++]);
        if (head >= qlen_)
            head -= qlen_;
        head_.store(head, std::memory_order_release);
        return true;
    }
    inline size_t used_size() 
    {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (tail >= head) ? (tail - head) : (tail + qlen_ - head);
    }
    inline size_t free_size() 
    {
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

template <typename T>
class MatrixQueue
{
public:
    MatrixQueue(int qlen) : producter_num_(0), consumer_num_(0), onequeue_len_(qlen)
    {
        memset(array_, 0, sizeof(array_));
        for (int i = 0; i < MATRIX_QUEUE_ARRAY_INIT_NUM; ++i)
            for (int j = 0; j < MATRIX_QUEUE_ARRAY_INIT_NUM; ++j)
                array_[i][j].store(new OneQueue<T>(onequeue_len_), std::memory_order_release);
        matrix_queue_id_ = matrix_queue_index_.fetch_add(1, std::memory_order_release);
    }
    MatrixQueue(MatrixQueue const&) = delete;
    MatrixQueue(MatrixQueue const&&) = delete;
    MatrixQueue& operator=(MatrixQueue const&) = delete;
    MatrixQueue& operator=(MatrixQueue const&&) = delete;
    ~MatrixQueue()
    {
        for (int i = 0; i < MATRIX_QUEUE_ARRAY_INIT_NUM; ++i)
            for (int j = 0; j < MATRIX_QUEUE_ARRAY_INIT_NUM; ++j)
            {
                auto queue = array_[i][j].load(std::memory_order_acquire);
                if (queue)
                {
                    array_[i][j].store(nullptr, std::memory_order_release);
                    delete queue;
                }
            }
    }
    bool Push(const T& val)
    {
        if (tl_producter_indexs_[matrix_queue_id_] == INVALID_NUM)
        {
            tl_producter_indexs_[matrix_queue_id_] = producter_num_.fetch_add(1, std::memory_order_release);
            if (tl_producter_indexs_[matrix_queue_id_] >= MATRIX_QUEUE_ARRAY_MAX_NUM)
            {
                std::cout << "MatrixQueue producter_id:" << tl_producter_indexs_[matrix_queue_id_] << " >= MATRIX_QUEUE_ARRAY_MAX_NUM" << std::endl;
                return false;
            }
        }
        size_t cur_consumer_num = consumer_num_.load(std::memory_order_relaxed);
        OneQueue<T> *aim_queue  = nullptr;
        if (cur_consumer_num >= 0)
        {
            int min_size    = 100000000;
            int max_step    = 0;
            for (int i = 0; i <= cur_consumer_num && max_step < MATRIX_QUEUE_MAX_STEP; ++i)
            {
                auto cur_queue = array_[tl_producter_indexs_[matrix_queue_id_]][i].load(std::memory_order_acquire);
                if (cur_queue)
                {
                    int cur = cur_queue->used_size();
                    if (cur < min_size)
                    {
                        aim_queue   = cur_queue;
                        min_size    = cur;
                        max_step++;
                    }
                }
                else
                {
                    aim_queue = new OneQueue<T>(onequeue_len_);
                    array_[tl_producter_indexs_[matrix_queue_id_]][i].store(aim_queue, std::memory_order_release);
                    break;
                }
            }
        }
        if (aim_queue && aim_queue->Push(val))
            return true;
        return false;
    }
    bool Pop(T& ptr) 
    {
        if (tl_consumer_indexs_[matrix_queue_id_] == INVALID_NUM)
        {
            tl_consumer_indexs_[matrix_queue_id_] = consumer_num_.fetch_add(1, std::memory_order_release);
            if (tl_consumer_indexs_[matrix_queue_id_] > MATRIX_QUEUE_ARRAY_MAX_NUM)
            {
                std::cout << "MatrixQueue consumer_id:" << tl_consumer_indexs_[matrix_queue_id_] << " >= MATRIX_QUEUE_ARRAY_MAX_NUM" << std::endl;
                return false;
            }
        }
        size_t cur_producter_num = producter_num_.load(std::memory_order_acquire);
        OneQueue<T> *aim_queue   = nullptr;
        if (cur_producter_num > 0)
        {
            int max_size    = 0;
            int max_step    = 0;
            for (int i = 0; i < cur_producter_num && max_step < MATRIX_QUEUE_MAX_STEP; ++i)
            {
                auto cur_queue = array_[i][tl_consumer_indexs_[matrix_queue_id_]].load(std::memory_order_relaxed);
                if (cur_queue)
                {
                    int cur = cur_queue->used_size();
                    if (cur > max_size)
                    {
                        aim_queue   = cur_queue;
                        max_size    = cur;
                        max_step++;
                    }
                }
            }
        }
        if (aim_queue && aim_queue->Pop(ptr))
            return true;
        return false;
    }
    inline size_t size() 
    {
        size_t res = 0;
        size_t cur_producter_num = producter_num_.load(std::memory_order_relaxed);
        size_t cur_consumer_num  = consumer_num_.load(std::memory_order_relaxed);
        for (int i = 0; i < cur_consumer_num; ++i)
            for (int j = 0; j < cur_producter_num; ++j)
            {
                auto cur_queue = array_[i][j].load(std::memory_order_relaxed);
                if (cur_queue)
                    res += cur_queue->used_size();
            }
        return res;
    }
private:
    std::atomic<size_t>         producter_num_;     //当前生产者数量
    std::atomic<size_t>         consumer_num_;      //当前消费者数量
    int                         onequeue_len_;      //OneQueue的长度
    int                         matrix_queue_id_;   //MatrixQueue实例的编号
    static std::atomic<size_t>  matrix_queue_index_;//MatrixQueue类个数的序号
    std::atomic<OneQueue<T> *>  array_[MATRIX_QUEUE_ARRAY_MAX_NUM][MATRIX_QUEUE_ARRAY_MAX_NUM];
    static thread_local std::vector<int> tl_producter_indexs_;
    static thread_local std::vector<int> tl_consumer_indexs_;
};
template <typename T> std::atomic<size_t> MatrixQueue<T>::matrix_queue_index_(0);
template <typename T> thread_local std::vector<int> MatrixQueue<T>::tl_producter_indexs_(MATRIX_QUEUE_NUM_MAX_INDEX, INVALID_NUM);
template <typename T> thread_local std::vector<int> MatrixQueue<T>::tl_consumer_indexs_(MATRIX_QUEUE_NUM_MAX_INDEX, INVALID_NUM);