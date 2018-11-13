#ifndef THREAD_POOL_EXECUTOR_H
#define THREAD_POOL_EXECUTOR_H

#include "ThreadPool.h"
#include "Job.h"

namespace util {

class ThreadPoolExecutor 
{
public:
    ThreadPoolExecutor(int nThreads): tp(nThreads) {}

    ~ThreadPoolExecutor() {}

    template<typename T>
    void executorJob(util::Job<T>& job, int numThread){
        int n = numThread > tp.size() ? tp.size() : numThread;
        for(int i = 0; i < n; i++)
        {
            tp.push([&](int j) { job(j); } );
        }
    }

    template <typename Fn, typename... Args>
    auto executor(Fn&& fn, Args&&... args) ->std::future<decltype(fn(0))> {
        return tp.push(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    void shutdown() {
        tp.waitAndStop();
    }

private:
    ThreadPool tp;
};

}

#endif