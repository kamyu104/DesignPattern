// Copyright (c) 2015 kamyu. All rights reserved.

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using std::cout;
using std::endl;
using std::queue;
using std::string;
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
        // If the condition is not met,
        // wait until the thread is notified,
        // the mutex is acquirable, and the condition is met.
        cond.wait(lock, [this]() { return q.size() != q_size; });
        q.emplace(val);
        cond.notify_one();  // Notify one of the waiting thread.
    }  // Unlock the mutex.

    void get(T *val) {
        unique_lock<mutex> lock{mtx};
        // If the condition is not met,
        // wait until the thread is notified,
        // the mutex is acquirable, and the condition is met.
        cond.wait(lock, [this]() { return !q.empty(); });
        *val = q.front();
        q.pop();
        cond.notify_one();  // Notify one of the waiting thread.
    }  // Unlock the mutex.

 private:
    mutex mtx;
    condition_variable cond;
    queue<T> q;
    const unsigned int q_size = 5;
};

class Printer {
 public:
    static void print(const string& s, const int& i) {
        static mutex mtx;
        unique_lock<mutex> lock(mtx);
        cout << s << i << endl;
    }
};

class Producer {
 public:
    explicit Producer(SyncQueue<int> *q) : q_(*q) {}

    void run() {
        for (int i = 0; i < 100; ++i) {
            const int num = i;
            q_.put(num);
            Printer::print("Produced: ", num);
            sleep_for(milliseconds(50));
        }
    }

 private:
    SyncQueue<int>& q_;
};

class Consumer {
 public:
    explicit Consumer(SyncQueue<int> *q) : q_(*q) {}
    
    void run() {
        for (int i = 0; i < 100; ++i) {
            int num;
            q_.get(&num);
            Printer::print("Consumed: ", num);
            sleep_for(milliseconds(500));
        }
    }

 private:
    SyncQueue<int>& q_;
};

int main() {
    SyncQueue<int> q;
    Producer p{&q};
    Consumer c{&q};

    thread producer_thread{&Producer::run, &p};
    thread consumer_thread{&Consumer::run, &c};

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
