#pragma once

// this queue is multithread safe. but it's not a efficient implement.

#include <queue>
#include <thread>
#include <mutex>

namespace asio_kcp {

template<typename T>
class threadsafe_queue_mutex
{
private:
    std::mutex mtx;
    std::queue<T> data_queue;
public:
    threadsafe_queue_mutex(){
    }

    ~threadsafe_queue_mutex(){
    }

    size_t size()
    {
        std::unique_lock<std::mutex> lck(mtx);
        return data_queue.size();
    }
    void push(T new_value)
    {
        std::unique_lock<std::mutex> lck(mtx);
        data_queue.push(new_value);
    }
    bool try_pop(T& value)
    {
        std::unique_lock<std::mutex> lck(mtx);
        if(data_queue.empty())
            return false;
        value=data_queue.front();
        data_queue.pop();
        return true;
    }
    void grab_all(std::queue<T>& ret)
    {
        std::unique_lock<std::mutex> lck(mtx);
        std::swap(ret, data_queue);
    }

    // void grab_all(std::queue<T> &ret)
    // {
    //     std::unique_lock<std::mutex> lck(mtx);
    //     while(!data_queue.empty()){
    //         ret.push(data_queue.front());
    //         data_queue.pop();
    //     }
    // }

    bool empty() const
    {
        std::unique_lock<std::mutex> lck(mtx);
        return data_queue.empty();
    }
};

} // end of namespace asio_kcp
