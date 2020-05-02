#pragma once
#include "onequeue.h"
///////////////////////////////////////////////////////////////////////////////
//lock free queue, muti producters, muti consumers
///////////////////////////////////////////////////////////////////////////////
#define INVALID_NUM     -1
static thread_local size_t thread_local_producter_index_ = INVALID_NUM;
static thread_local size_t thread_local_consumer_index_  = INVALID_NUM;
template <typename T>
class MutiQueue {
public:
    MutiQueue(size_t max_len, size_t qlen) : max_len_(max_len), producter_num_(0), consumer_num_(0) 
    {
        array_ = new OneQueue<T>**[max_len_];
        for (int i = 0; i < max_len_; ++i)
            array_[i] = (OneQueue<T>**)malloc(sizeof(OneQueue<T>**) * max_len_);
        for (int i = 0; i < max_len_; ++i)
            for (int j = 0; j < max_len_; ++j)
                array_[i][j] = new OneQueue<T>(qlen);
    }
    MutiQueue(MutiQueue const&) = delete;
    MutiQueue& operator=(MutiQueue const&) = delete;
    ~MutiQueue() 
    {
        for (int i = 0; i < max_len_; ++i)
            for (int j = 0; j < max_len_; ++j)
                delete array_[i][j];
        for (int i = 0; i < max_len_; ++i)
            delete array_[i];
        delete[] array_;
    }
    bool Push(const T& val) {
        if (thread_local_producter_index_ == INVALID_NUM)
        {
            thread_local_producter_index_ = producter_num_++;
            if (thread_local_producter_index_ >= max_len_)
            {
                //TODO
                std::cout << "error, thread_local_producter_index_=" << thread_local_producter_index_ << ", max_len_=" << max_len_ << std::endl;
                return false;
            }
        }
        //std::cout << "thread_local_producter_index_=" << thread_local_producter_index_ << std::endl;
        size_t cur_consumer_num = consumer_num_;
        if (cur_consumer_num == 0)
        {
            if (array_[thread_local_producter_index_][0]->Push(val))
            {
                std::cout << "producter_index[" << thread_local_producter_index_ << "] ==> val[" << val << "]==> consumer_index[" << 0 << "]" << std::endl;
                return true;
            }
            return false;
        }
        for (int i = 0; i < cur_consumer_num; ++i)
        {
            if (array_[thread_local_producter_index_][i]->Push(val))
            {
                std::cout << "producter_index[" << thread_local_producter_index_ << "] ==> val[" << val << "]==> consumer_index[" << i << "]" << std::endl;
                return true;
            }
        }
        
        return false;
        int aim_index   = 0;
        int min_size    = 100000000;
        int max_step    = 0;
        for (int i = 0; i < max_len_ && max_step < 3; ++i)
        {
            int cur     = array_[thread_local_producter_index_][i]->used_size();
            if (cur < min_size)
            {
                aim_index   = i;
                min_size    = cur;
                max_step++;
            }
        }
        std::cout << "thread_local_producter_index_= " << thread_local_producter_index_ << ", val=" << val << ", aim_index=" << aim_index << ", min_size=" << min_size << ", max_step=" << max_step << std::endl;
        return array_[thread_local_producter_index_][aim_index]->Push(val);//TOTO:算法优化
    }
    bool Pop(T* ptr) {
        if (thread_local_consumer_index_ == INVALID_NUM)
        {
            thread_local_consumer_index_ = consumer_num_++;
            if (thread_local_consumer_index_ > max_len_)
            {
                //TODO
                return false;
            }
        }
        size_t cur_producter_num = producter_num_;
        if (cur_producter_num == 0)
        {
            if (array_[0][thread_local_consumer_index_]->Pop(ptr))
            {
                std::cout << "consumer_index[" << thread_local_consumer_index_ << "] <== val[" << *ptr << "] <== producter_index[" << 0 << "]" << std::endl;
                return true;
            }
        }
        for (int i = 0; i < cur_producter_num; ++i)
        {
            if (array_[i][thread_local_consumer_index_]->Pop(ptr))
            {
                std::cout << "consumer_index[" << thread_local_consumer_index_ << "] <== val[" << *ptr << "] <== producter_index[" << i << "]" << std::endl;
                return true;
            }
        }
        return false;
        // int aim_index   = 0;
        // int max_size    = 0;
        // int max_step    = 0;
        // for (int i = 0; i < max_len_ && max_step < 3; ++i)
        // {
        //     int cur     = array_[i][thread_local_consumer_index_]->used_size();
        //     if (cur > max_size)
        //     {
        //         aim_index   = i;
        //         max_size    = cur;
        //         max_step++;
        //     }
        // }
        // std::cout << " aim_index=" << aim_index << ", max_size=" << max_size << ", max_step=" << max_step << std::endl;
        // return array_[aim_index][thread_local_consumer_index_]->Pop(ptr);//TOTO:算法优化
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
    OneQueue<T>*** array_;
};