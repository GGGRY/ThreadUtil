#include "Queue.h"
#include "ThreadUtil.h"
#include "ThreadPool.h"
#include "uncopyable.h"

#include <thread>
#include <iostream>
#include <vector>

using namespace std;
using util::Queue;
using util::BlockingQueue;

class TestData : public UnCopyable {
public:
    TestData(int a) : num(a) {}

    int get() { return num; }


private:
    int num;  
};

template<typename T>
void dequeueAndPrint(Queue<T>& q)
{
    T temp;
    int i = 0;
    cout << "start pop :" << endl;
    while(q.pop(temp))
    {
        i++;
        cout << "pop " << i << " = " << temp->get() << endl;
    }
}

int main(int argc, char** argv) {
    
    vector<TestData*> vec;
    vec.push_back(new TestData(9));
    vec.push_back(new TestData(8));
    vec.push_back(new TestData(7));
    vec.push_back(new TestData(6));
    vec.push_back(new TestData(5));

    Queue<TestData*> q1;
    Queue<TestData*> q2(vec);

    q1.push(new TestData(1));
    q1.push(new TestData(3));
    q1.push(new TestData(5));
    q1.push(new TestData(7));
    q1.push(new TestData(9));

    // dequeueAndPrint(q1);
    dequeueAndPrint(q2);

    cout << "Is q1 empty ? " << (q1.empty() ? "true" : "false") << endl;
    cout << "Is q2 empty ? " << (q2.empty() ? "true" : "false") << endl;

    cout << vec.size() << endl;
    for (int i = 0; i < vec.size(); i++)
        cout << "vec " << i << " = " << vec[i]->get() << endl;

    BlockingQueue<TestData*> q3(vec);

    auto lamda = [&]() { dequeueAndPrint<TestData*>(q3);};

    ThreadUtil::runInNewDaemonThread(lamda);

    for(int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        q3.push(new TestData(i));
    }

    q3.shutdown();

    return 0;
}