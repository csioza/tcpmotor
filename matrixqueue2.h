#pragma once
#include <atomic>
#include <memory>
///////////////////////////////////////////////////////////////////////////////
//lock free queue, muti producters, muti consumers
///////////////////////////////////////////////////////////////////////////////
#define INVALID_NUM                 -1
#define MAX_STEP                    3
#define MAX_MATRIX_QUEUE_INDEX      1024
#define MAX_MATRIX_QUEUE_SUM_NUM    1024
#define MAX_MATRIX_QUEUE_INIT_NUM   8

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

template <typename T>
class MatrixQueue {
public:
    MatrixQueue(int qlen) : producter_num_(0), consumer_num_(0), qlen_(qlen)
    {
        memset(array_, 0, sizeof(array_));
        for (int i = 0; i < MAX_MATRIX_QUEUE_INIT_NUM; ++i)
            for (int j = 0; j < MAX_MATRIX_QUEUE_INIT_NUM; ++j)
                array_[i][j].store(new OneQueue<T>(qlen_), std::memory_order_release);
        id_ = matrix_queue_index_.fetch_add(1, std::memory_order_release);
    }
    MatrixQueue(MatrixQueue const&) = delete;
    MatrixQueue& operator=(MatrixQueue const&) = delete;
    ~MatrixQueue() 
    {
        for (int i = 0; i < MAX_MATRIX_QUEUE_INIT_NUM; ++i)
            for (int j = 0; j < MAX_MATRIX_QUEUE_INIT_NUM; ++j)
            {
                auto queue = array_[i][j].load(std::memory_order_acquire);
                if (queue)
                {
                    array_[i][j].store(nullptr, std::memory_order_release);
                    delete queue;
                }
            }
    }
    bool Push(const T& val) {
        if (thread_local_producter_index_[id_] == INVALID_NUM)
        {
            thread_local_producter_index_[id_] = producter_num_.fetch_add(1, std::memory_order_release);
            std::cout << "id_=" << id_ << ", thread_local_producter_index_=" << thread_local_producter_index_[id_] << ", producter_num_=" << producter_num_ << std::endl;
            if (thread_local_producter_index_[id_] >= MAX_MATRIX_QUEUE_SUM_NUM)
            {
                std::cout << "id_=" << id_ << ", error, thread_local_producter_index_=" << thread_local_producter_index_[id_] << ", max_row_len=" << MAX_MATRIX_QUEUE_SUM_NUM << std::endl;
                return false;
            }
        }
        size_t cur_consumer_num = consumer_num_.load(std::memory_order_relaxed);
        OneQueue<T> *aim_queue = nullptr;
        if (cur_consumer_num >= 0)
        {
            int min_size    = 100000000;
            int max_step    = 0;
            for (int i = 0; i <= cur_consumer_num && max_step < MAX_STEP; ++i)
            {
                auto cur_queue = array_[thread_local_producter_index_[id_]][i].load(std::memory_order_acquire);
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
                    aim_queue = new OneQueue<T>(qlen_);
                    array_[thread_local_producter_index_[id_]][i].store(aim_queue, std::memory_order_release);
                    break;
                }
            }
        }
        if (aim_queue && aim_queue->Push(val))
        {
            // std::cout << "id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] ==> val[" << val << "]==> consumer_index[" << aim_index << "]" << ", cur_consumer_num[" << cur_consumer_num << "]" << std::endl;
            return true;
        }
        // std::cout << "id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] =/=> val[" << val << "]=/=> consumer_index[" << aim_index << "]" << ", cur_consumer_num[" << cur_consumer_num << "]" << std::endl;
        return false;
    }
    bool Pop(T& ptr) {
        if (thread_local_consumer_index_[id_] == INVALID_NUM)
        {
            thread_local_consumer_index_[id_] = consumer_num_.fetch_add(1, std::memory_order_release);
            std::cout << "id_=" << id_ << ", thread_local_consumer_index_=" << thread_local_consumer_index_[id_] << ", consumer_num_=" << consumer_num_.load(std::memory_order_relaxed) << std::endl;
            if (thread_local_consumer_index_[id_] > MAX_MATRIX_QUEUE_SUM_NUM)
            {
                std::cout << "id_=" << id_ << ", error, thread_local_consumer_index_=" << thread_local_consumer_index_[id_] 
                        << ", max_col_len=" << MAX_MATRIX_QUEUE_SUM_NUM << std::endl;
                return false;
            }
        }
        size_t cur_producter_num = producter_num_.load(std::memory_order_acquire);
        OneQueue<T> *aim_queue = nullptr;
        if (cur_producter_num > 0)
        {
            int max_size    = 0;
            int max_step    = 0;
            for (int i = 0; i < cur_producter_num && max_step < MAX_STEP; ++i)
            {
                auto cur_queue = array_[i][thread_local_consumer_index_[id_]].load(std::memory_order_relaxed);
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
        {
            // std::cout << "id_=" << id_ << ", consumer_index[" << thread_local_consumer_index_[id_] << "] <== val[" << ptr << "] <== producter_index[" << aim_index << "]" << ", cur_producter_num[" << cur_producter_num<< "]" << std::endl;
            return true;
        }
        // std::cout << "id_=" << id_ << ", consumer_index[" << thread_local_consumer_index_[id_] << "] <=/= val[" << ptr << "] <=/= producter_index[" << aim_index << "]" << ", cur_producter_num[" << cur_producter_num<< "]" << std::endl;
        return false;
    }
    inline size_t size() {
        size_t res = 0;
        size_t cur_producter_num = producter_num_.load(std::memory_order_relaxed);
        size_t cur_consumer_num  = consumer_num_.load(std::memory_order_relaxed);
        for (int i = 0; i < cur_consumer_num; ++i)
            for (int j = 0; j < cur_producter_num; ++j)
            {
                auto cur_queue = array_[i][j].load(std::memory_order_relaxed);
                if (cur_queue)
                {
                    res += cur_queue->used_size();
                }
            }
        return res;
    }
private:
    std::atomic<size_t> producter_num_;//当前生产者数量
    std::atomic<size_t> consumer_num_;//当前消费者数量
    int qlen_;
    std::atomic<size_t> id_;
    static std::atomic<size_t> matrix_queue_index_;
    std::atomic<OneQueue<T> *> array_[MAX_MATRIX_QUEUE_SUM_NUM][MAX_MATRIX_QUEUE_SUM_NUM];
    static thread_local std::vector<int> thread_local_producter_index_;
    static thread_local std::vector<int> thread_local_consumer_index_;
};
template <typename T> std::atomic<size_t> MatrixQueue<T>::matrix_queue_index_(0);
template <typename T> thread_local std::vector<int> MatrixQueue<T>::thread_local_producter_index_(MAX_MATRIX_QUEUE_INDEX, INVALID_NUM);
template <typename T> thread_local std::vector<int> MatrixQueue<T>::thread_local_consumer_index_(MAX_MATRIX_QUEUE_INDEX, INVALID_NUM);
