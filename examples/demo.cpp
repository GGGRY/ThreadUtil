#include "ThreadUtil.h"
#include "ThreadPool.h"

#include <thread>
#include <iostream>
#include <vector>

using namespace std;

std::thread::id main_thread_id = std::this_thread::get_id();

void is_main_thread() {
  if ( main_thread_id == std::this_thread::get_id() )
    std::cout << "This is the main thread.\n";
  else
    std::cout << "This is not the main thread.\n";
}

void increase_print(vector<int> iv, int c)
{
    is_main_thread();
    for (int i = 0; i < iv.size(); i++)
    {
        cout << i << " : " << iv[i] << " => " << iv[i] + c << endl;
    }
}

void increase_print2(int id, vector<int> iv, int c)
{
    is_main_thread();
    for (int i = 0; i < iv.size(); i++)
    {
        cout << i << " : " << iv[i] << " => " << iv[i] + c << endl;
    }
}

int main(int argc, char** argv) {
    cout << "Hello world!" << endl;

    vector<int> in;
    in.push_back(0);
    in.push_back(3);
    in.push_back(4);
    in.push_back(6);
    in.push_back(45);

    increase_print(in, 9);
    // ThreadUtil::runInNewJoinThread(increase_print, std::move(in), 9);
    // ThreadUtil::runInNewJoinThread(increase_print, std::ref(in), 9);
    // ThreadUtil::runInNewDamonThread(increase_print, std::move(in), 9);
    // ThreadUtil::runInNewDamonThread(increase_print, std::ref(in), 9);

    util::ThreadPool p(1 /* two threads in the pool */);

    auto f1 = p.push(increase_print2, (in), 9);

    f1.get();

    ThreadUtil::runInNewJoinThread(increase_print, std::move(in), 9);
    // std::thread th(increase_print, std::ref(in), 9);
    // th.join();
    return 0;
}