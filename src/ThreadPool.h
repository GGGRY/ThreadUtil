#ifndef __ThreadPool_H__
#define __ThreadPool_H__

#include "Queue.h"

#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>

namespace util {


/* A fix threads pool */
class ThreadPool {
public:
    ThreadPool(int nThreads) { this->init(nThreads); }

    // the destructor waits for all the functions in the queue to be finished
    ~ThreadPool() {
        this->stop(true);
    }

    // get the number of running threads in the pool
    int size() { return static_cast<int>(this->threads.size()); }

    // number of waiting threads
    int nWaitings() { return this->nWaiting; }

    // get the ith thread reference
    std::thread & getThread(int i) { return *this->threads[i]; }

    // wait for all computing threads to finish and stop all threads
    void waitAndStop() { this->stop(true) ; }

    // clear all task queue and stop all threads
    void clearAndStop() { this->stop(false); }

    // empty the queue
    void clear_queue() {
        std::function<void(int id)> * _f;
        while (this->q.pop(_f))
            delete _f; // empty the queue
    }

    template<typename F, typename... Rest>
    auto push(F && f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
            );
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        this->q.push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
        return pck->get_future();
    }

    // run the user's function that excepts argument int - id of the running thread. returned value is templatized
    // operator returns std::future, where the user can get the result and rethrow the catched exceptins
    template<typename F>
    auto push(F && f) ->std::future<decltype(f(0))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        this->q.push(_f);
        std::unique_lock<std::mutex> lock(this->mutex);
        this->cv.notify_one();
        return pck->get_future();
    }

private:
    std::vector<std::unique_ptr<std::thread>> threads;
    std::vector<std::shared_ptr<std::atomic<bool>>> flags;

    util::Queue<std::function<void(int id)> *> q;

    std::atomic<bool> isDone;
    std::atomic<bool> isStop;
    std::atomic<int>  nWaiting;

    std::mutex mutex;
    std::condition_variable cv;

private:

    ThreadPool(const ThreadPool &);// = delete;
    ThreadPool(ThreadPool &&);// = delete;
    ThreadPool & operator=(const ThreadPool &);// = delete;
    ThreadPool & operator=(ThreadPool &&);// = delete;

    void set_thread(int i) {
        std::shared_ptr<std::atomic<bool>> flag(this->flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]() {
            std::atomic<bool> & _flag = *flag;
            std::function<void(int id)> * _f;
            bool isPop = this->q.pop(_f);
            while (true) {
                while (isPop) {  // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                        return;  // the thread is wanted to stop, return even if the queue is not empty yet
                    else
                        isPop = this->q.pop(_f);
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(this->mutex);
                ++this->nWaiting;
                this->cv.wait(lock, [this, &_f, &isPop, &_flag](){ isPop = this->q.pop(_f); return isPop || this->isDone || _flag; });
                --this->nWaiting;
                if (!isPop)
                    return;  // if the queue is empty and this->isDone == true or *flag then return
            }
        };
        this->threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }

    void init(int nThreads) { 
        this->nWaiting = 0; 
        this->isStop = false; 
        this->isDone = false;

        this->threads.resize(nThreads);
        this->flags.resize(nThreads);

        for (int i = 0; i < nThreads; ++i) {
            this->flags[i] = std::make_shared<std::atomic<bool>>(false);
            this->set_thread(i);
        } 
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
    void stop(bool isWait = false) {
        if (!isWait) {
            if (this->isStop)
                return;
            this->isStop = true;
            for (int i = 0, n = this->size(); i < n; ++i) {
                *this->flags[i] = true;  // command the threads to stop
            }
            this->clear_queue();  // empty the queue
        }
        else {
            if (this->isDone || this->isStop)
                return;
            this->isDone = true;  // give the waiting threads a command to finish
        }
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_all();  // stop all waiting threads
        }
        for (int i = 0; i < static_cast<int>(this->threads.size()); ++i) {  // wait for the computing threads to finish
                if (this->threads[i]->joinable())
                    this->threads[i]->join();
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        this->clear_queue();
        this->threads.clear();
        this->flags.clear();
    }

};

}
#endif
