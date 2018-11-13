#ifndef __JOB_H__
#define __JOB_H__

#include "Queue.h"
#include "Stoppable.h"

#include <functional>
#include <vector>

namespace util {

template<typename T>
class Job : public Stoppable
{
public:
    Job() {}
    ~Job() {}

    virtual void run(int id) {}

    void operator()(int id) {
        run(id);
    }

};

template<typename T>
class FixedJob : public Job<T> {
public:
    FixedJob(std::vector<T>& vec, std::function<void(int id, T&)> f): 
            completedFuture(completedSignal.get_future()),
            nCompleted(0),
            tasks(vec),
            taskSize(vec.size()),
            func(std::move(f))
    {

    }

    ~FixedJob() { }

    void run(int id) {
        T temp;
        while(this->stopRequested() == false && this->tasks.pop(temp)){
            func(id, temp);
            ++nCompleted;
            if (nCompleted >= taskSize) completedSignal.set_value();
        }
    }

    void wait() {
        completedFuture.get();
    }

private:
	std::promise<void> completedSignal;
	std::future<void> completedFuture;

    std::atomic<size_t>  nCompleted;
    
    util::Queue<T> tasks;
    size_t  taskSize;
    std::function<void(int id, T&)> func;
};

}
#endif