#include "ThreadUtil.h"
#include "ThreadPool.h"
#include "uncopyable.h"
#include "Queue.h"

#include <thread>
#include <iostream>
#include <vector>
#include <queue>

using namespace std;
using namespace util;

std::thread::id main_thread_id = std::this_thread::get_id();

static void is_main_thread() {
  if ( main_thread_id == std::this_thread::get_id() )
    std::cout << "This is the main thread.\n";
  else
    std::cout << "This is not the main thread.\n";
}

class Task : public UnCopyable {
public:
    Task(): c(9) {}
    Task(int n): c(n) {}
    ~Task() {}
    void increase_print(vector<int> & iv)
    {
        is_main_thread();
        for (int i = 0; i < iv.size(); i++)
        {
            cout << i << " : " << iv[i] << " => " << iv[i] + c << endl;
        }
    }

    void increate_print_single(int i)
    {
        cout << to_string(i) + " => " + to_string(i+c) + "\n";
    }

    void increase_print2(int id, vector<int> iv)
    {
        is_main_thread();
        for (int i = 0; i < iv.size(); i++)
        {
            cout << i << " : " << iv[i] << " => " << iv[i] + c << endl;
        }
    }
private:
    int c;
};

template<class T>
class TaskRunner {
typedef void (*PF)(int id, T t);
public:
    TaskRunner(): stopped(false) {}
    TaskRunner(std::queue<T>&& in): stopped(false) {
        qtask = std::move(in);
    }
    ~TaskRunner() {
        stopped = true;
    }

    template<typename Fn>
    void run(Fn&& f){
        auto rannable = [this, &f]() {
            is_main_thread();
            while(!this->qtask.empty() && !this->stopped){
                T tmp = this->qtask.front();
                this->qtask.pop();
                cout << "i am running: " << endl;
                f(1, tmp);
            }
        };
        ThreadUtil::runInNewJoinThread(rannable);
    }

    void run() {
            auto rannable = [this]() {
            is_main_thread();
            while(!this->qtask.empty() && !this->stopped){
                T tmp = this->qtask.front();
                this->qtask.pop();
                cout << "i am running: " << endl;
                this->functor(1, tmp);
            }
        };
        ThreadUtil::runInNewJoinThread(rannable);
    }

    void setFunctor(std::function<void(int, T)> f){
        functor = std::move(f); 
        functor(2, 5);
    }

    void stop() {stopped = true;}

private:
    bool stopped;
    std::queue<T> qtask;
    std::function<void(int, T)>  functor;

};

int main(int argc, char** argv) {
    cout << "Hello world!" << endl;

    vector<int> in;
    in.push_back(0);
    in.push_back(3);
    in.push_back(4);
    in.push_back(6);
    in.push_back(45);

    Task t(5);

    t.increase_print(in);
    
    util::ThreadPool p(1 /* two threads in the pool */);

    auto lamda = [&t](int id, int i) { t.increate_print_single(i);};

    lamda(1, 4);

    // auto f1 = p.push(lamda);
    queue<int> qtest;
    qtest.push(0);
    qtest.push(5);
    qtest.push(4);
    qtest.push(3);

    TaskRunner<int> tr(std::move(qtest));
    tr.setFunctor(lamda);
    tr.run();
    tr.run(lamda);
    // f1.get();

    // ThreadUtil::runInNewJoinThread(lamda, 1);
    // std::thread th(increase_print, std::ref(in), 9);
    // th.join();
    return 0;
}