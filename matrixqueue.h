#pragma once
#include "onequeue.h"
///////////////////////////////////////////////////////////////////////////////
//lock free queue, muti producters, muti consumers
///////////////////////////////////////////////////////////////////////////////
#define INVALID_NUM         -1
#define MAX_STEP            3
#define MAX_MATRIX_QUEUE    100
static thread_local std::vector<int> thread_local_producter_index_(MAX_MATRIX_QUEUE, INVALID_NUM);
static thread_local std::vector<int> thread_local_consumer_index_(MAX_MATRIX_QUEUE, INVALID_NUM);
template <typename T>
class MatrixQueue {
public:
    MatrixQueue(size_t max_len, size_t qlen) : max_len_(max_len), producter_num_(0), consumer_num_(0), id_(matrix_queue_index_++)
    {
        array_ = new OneQueue<T>**[max_len_];
        for (int i = 0; i < max_len_; ++i)
            array_[i] = (OneQueue<T>**)malloc(sizeof(OneQueue<T>**) * max_len_);
        for (int i = 0; i < max_len_; ++i)
            for (int j = 0; j < max_len_; ++j)
                array_[i][j] = new OneQueue<T>(qlen);
    }
    MatrixQueue(MatrixQueue const&) = delete;
    MatrixQueue& operator=(MatrixQueue const&) = delete;
    ~MatrixQueue() 
    {
        for (int i = 0; i < max_len_; ++i)
            for (int j = 0; j < max_len_; ++j)
                delete array_[i][j];
        for (int i = 0; i < max_len_; ++i)
            delete array_[i];
        delete[] array_;
    }
    bool Push(const T& val) {
        if (thread_local_producter_index_[id_] == INVALID_NUM)
        {
            thread_local_producter_index_[id_] = producter_num_++;
            std::cout << "id_=" << id_ << ", thread_local_producter_index_=" << thread_local_producter_index_[id_] << ", producter_num_=" << producter_num_ << std::endl;
            if (thread_local_producter_index_[id_] >= max_len_)
            {
                //TODO
                std::cout << "id_=" << id_ << ", error, thread_local_producter_index_=" << thread_local_producter_index_[id_] << ", max_len_=" << max_len_ << std::endl;
                return false;
            }
        }
        //std::cout << "thread_local_producter_index_=" << thread_local_producter_index_ << std::endl;
        size_t cur_consumer_num = consumer_num_.load(std::memory_order_relaxed);
        if (cur_consumer_num == 0)
        {
            if (array_[thread_local_producter_index_[id_]][0]->Push(val))
            {
                std::cout << "111 id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] ==> val[" << val << "]==> consumer_index[" << 0 << "]" << ", consumer_num_[" << consumer_num_.load(std::memory_order_relaxed)<< "]" << std::endl;
                return true;
            }
            std::cout << "111 id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] =/=> val[" << val << "]=/=> consumer_index[" << 0 << "]" << ", consumer_num[" << consumer_num_.load(std::memory_order_relaxed)<< "]" << std::endl;
            return false;
        }

        int aim_index   = 0;
        int min_size    = 100000000;
        int max_step    = 0;
        for (int i = 0; i < cur_consumer_num && max_step < MAX_STEP; ++i)
        {
            int cur     = array_[thread_local_producter_index_[id_]][i]->used_size();
            if (cur < min_size)
            {
                aim_index   = i;
                min_size    = cur;
                max_step++;
            }
        }
        if (array_[thread_local_producter_index_[id_]][aim_index]->Push(val))
        {
            std::cout << "id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] ==> val[" << val << "]==> consumer_index[" << aim_index << "]" << ", consumer_num_[" << consumer_num_.load(std::memory_order_relaxed)<< "]" << std::endl;
            return true;
        }
        std::cout << "err id_=" << id_ << ", producter_index[" << thread_local_producter_index_[id_] << "] =/=> val[" << val << "]=/=> consumer_index[" << aim_index << "]" << ", consumer_num_[" << consumer_num_.load(std::memory_order_relaxed)<< "]" << std::endl;
        return false;
    }
    bool Pop(T* ptr) {
        if (thread_local_consumer_index_[id_] == INVALID_NUM)
        {
            thread_local_consumer_index_[id_] = consumer_num_++;
            std::cout << "id_=" << id_ << ", thread_local_consumer_index_=" << thread_local_consumer_index_[id_] << ", consumer_num_=" << consumer_num_.load(std::memory_order_relaxed) << std::endl;
            if (thread_local_consumer_index_[id_] > max_len_)
            {
                //TODO
                return false;
            }
        }
        size_t cur_producter_num = producter_num_.load(std::memory_order_relaxed);;
        if (cur_producter_num == 0)
        {
            if (array_[0][thread_local_consumer_index_[id_]]->Pop(ptr))
            {
                std::cout << "111 id_=" << id_  << ", consumer_index[" << thread_local_consumer_index_[id_] << "] <== val[" << *ptr << "] <== producter_index[" << 0 << "]" << ", producter_num[" << producter_num_<< "]" << std::endl;
                return true;
            }
            std::cout << "111 id_=" << id_ << ", consumer_index[" << thread_local_consumer_index_[id_] << "] <=/= val[" << *ptr << "] <=/= producter_index[" << 0 << "]" << ", producter_num[" << producter_num_<< "]" << std::endl;
            return false;
        }
        int aim_index   = 0;
        int max_size    = 0;
        int max_step    = 0;
        for (int i = 0; i < cur_producter_num && max_step < MAX_STEP; ++i)
        {
            int cur     = array_[i][thread_local_consumer_index_[id_]]->used_size();
            if (cur > max_size)
            {
                aim_index   = i;
                max_size    = cur;
                max_step++;
            }
        }
        if (array_[aim_index][thread_local_consumer_index_[id_]]->Pop(ptr))
        {
            std::cout << "id_=" << id_ << "consumer_index[" << thread_local_consumer_index_[id_] << "] <== val[" << *ptr << "] <== producter_index[" << aim_index << "]" << ", producter_num[" << producter_num_<< "]" << std::endl;
            return true;
        }
        std::cout << "err id_=" << id_ << ", consumer_index[" << thread_local_consumer_index_[id_] << "] <=/= val[" << *ptr << "] <=/= producter_index[" << aim_index << "]" << ", producter_num[" << producter_num_<< "]" << std::endl;
        return false;
    }
    inline size_t size() {
        size_t res = 0;
        for (int i = 0; i < max_len_; ++i)
            for (int j = 0; j < max_len_; ++j)
                res += array_[i][j]->used_size();
        return res;
    }
private:
    size_t max_len_;//二位矩阵大小,最大生产者和消费者数量
    std::atomic<size_t> producter_num_;//当前生产者数量
    std::atomic<size_t> consumer_num_;//当前消费者数量
    std::atomic<size_t> id_;
    static std::atomic<size_t> matrix_queue_index_;
    OneQueue<T>*** array_;
};

template <typename T> std::atomic<size_t> MatrixQueue<T>::matrix_queue_index_(0);