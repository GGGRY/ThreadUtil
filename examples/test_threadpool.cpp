#include "ThreadPool.h"
#include "ThreadUtil.h"
#include "uncopyable.h"
#include "ThreadPoolExecutor.h"
#include "Job.h"

#include <thread>
#include <iostream>
#include <vector>

using namespace std;

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
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void increate_print_single(int i)
    {
        cout << to_string(i) + " => " + to_string(i+c);
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

void increase_print(int i, int& iv)
{
    // is_main_thread();
    
    cout << "Thread " << i << " : " << iv << " => " << iv + 5 << endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int get_value(int id)
{
    return 1000;
}

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

    // test threadpool
    cout << "Test ThreadPool..." << endl;
    util::ThreadPool tp1(1);

    auto lamda = [&t, &in](int id) -> void { t.increase_print(in);};

    auto f1 = tp1.push(lamda);
    tp1.waitAndStop();
    // tp1.clearAndStop();

    // test JOB
    cout << "Test Job..." << endl;

    util::FixedJob<int> job(in, increase_print);

    util::ThreadPoolExecutor  tpe(2);
    tpe.executorJob(job, 1);

    // std::this_thread::sleep_for(std::chrono::seconds(1));
    cout << "Stop test..." << endl;
    // job.wait();

    auto f2 = tpe.executor(get_value);

    cout << "Get value : "  << f2.get() << endl;

    return 0;
}