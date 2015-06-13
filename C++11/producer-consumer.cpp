#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using std::cout;
using std::endl;
using std::queue;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::lock_guard;
using std::condition_variable;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

template<typename T>
class SyncQueue {
public:
    void put(const T& val) {
        unique_lock<mutex> lock{mtx};
        cond.wait(lock, [this]() { return q.size() != q_size; });
        q.emplace(val);
        cond.notify_one();
    }
    
    void get(T& val) {
        unique_lock<mutex> lock{mtx};
        cond.wait(lock, [this]() { return !q.empty(); });
        val = q.front();
        q.pop();
        cond.notify_one();
    }
    
private:
    mutex mtx;
    condition_variable cond;
    queue<T> q;
    const unsigned int q_size = 5;
};

class Producer {
public:
    Producer(SyncQueue<int>& q, mutex& cout_mtx) :
             q_(q), cout_mtx_(cout_mtx) {}
    
    void run() {
        for (int i = 0; i < 100; ++i) {
            const int num = i;
            q_.put(num);
            {
                lock_guard<mutex> lock(cout_mtx_);
                cout << "Produced: " << num << endl;
            }
            sleep_for(milliseconds(50));
        }
    }

private:
    SyncQueue<int>& q_;
    mutex& cout_mtx_;
};

class Consumer {
public:
    Consumer(SyncQueue<int>& q, mutex& cout_mtx) : q_(q), cout_mtx_(cout_mtx) {}
    void run() {
        for (int i = 0; i < 100; ++i) {
            int num;
            q_.get(num);
            {
                lock_guard<mutex> lock(cout_mtx_);
                cout << "Consumed: " << num << endl;
            }
            sleep_for(milliseconds(500));
        }
    }

private:
    SyncQueue<int>& q_;
    mutex& cout_mtx_;
};

int main() {
    SyncQueue<int> q;
    mutex cout_mtx;
    Producer p{q, cout_mtx};
    Consumer c{q, cout_mtx};

    thread producer_thread{&Producer::run, &p};
    thread consumer_thread{&Consumer::run, &c};

    producer_thread.join();
    consumer_thread.join();
    
    return 0;
}
