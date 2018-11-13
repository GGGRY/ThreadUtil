#ifndef _Queue_H__
#define _Queue_H__

// #include <queue>
#include <deque>
#include <mutex>
#include <vector>
#include <memory>
#include <condition_variable>

namespace util {

template<typename T>
class Queue {
public:
    Queue() {}
    Queue(std::vector<T>& vec): q(vec.begin(), vec.end()) {}

    virtual bool push(T const & value) {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->q.push_back(value);
        return true;
    }

    virtual bool pop(T & v) {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->q.empty())
            return false;
        v = this->q.front();
        this->q.pop_front();
        return true;
    }
    
    bool empty() {
        std::unique_lock<std::mutex> lock(this->mutex);
        return this->q.empty();
    }

protected:
    std::deque<T> q;    
    std::mutex mutex;
};

template <typename T>
class BlockingQueue : public Queue<T> {
public:

    BlockingQueue():_shutdown(false) {}
    BlockingQueue(std::vector<T>& vec): Queue<T>(vec), _shutdown(false) {}

    virtual bool push(T const & value)
    {
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->q.push_back(value);
        }
        condition.notify_one();
        return true;
    }

    virtual void shutdown() {
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            _shutdown = true;
        }
        condition.notify_all();
    }

    virtual bool pop(T & item) {
        std::unique_lock<std::mutex> lock(this->mutex);
        for (;;) {
            if (this->q.empty()) {
                if (_shutdown) {
                    return false;
                }
            }
            else {
                break;
            }
            condition.wait(lock);
        }
        item = std::move(this->q.front());
        this->q.pop_front();
        return true;
    }

protected:
    std::condition_variable condition;
    bool _shutdown;
};

}


#endif