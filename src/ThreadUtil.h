#ifndef THREAD_UTIL_H
#define THREAD_UTIL_H

#include "Job.h"
#include "ThreadPool.h"
#include <thread>

class ThreadUtil {
public:
    template <typename Fn, typename... Args>
    static void runInNewJoinThread(Fn&& fn, Args&&... args){
        std::thread th(std::forward<Fn>(fn), std::forward<Args>(args)...);
        th.join();
    }

    template <typename Fn, typename... Args>
    static void runInNewDaemonThread(Fn&& fn, Args&&... args){
        std::thread th(std::forward<Fn>(fn), std::forward<Args>(args)...);
        th.detach();
    }
};

#endif