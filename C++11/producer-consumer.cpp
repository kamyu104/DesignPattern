// Copyright (c) 2015 kamyu. All rights reserved.

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using std::cout;
using std::endl;
using std::vector;
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
        unique_lock<mutex> lock{mtx_};
        // If the condition is not met,
        // wait until the thread is notified,
        // the mutex is acquirable, and the condition is met.
        cond_.wait(lock, [this]() { return q_.size() != q_size_; });
        q_.emplace(val);
        cond_.notify_all();  // Notify all of the waiting threads.
    }  // Unlock the mutex.

    void get(T *val) {
        unique_lock<mutex> lock{mtx_};
        // If the condition is not met,
        // wait until the thread is notified,
        // the mutex is acquirable, and the condition is met.
        cond_.wait(lock, [this]() { return !q_.empty(); });
        *val = q_.front();
        q_.pop();
        cond_.notify_all();  // Notify all of the waiting threads.
    }  // Unlock the mutex.

 private:
    mutex mtx_;
    condition_variable cond_;
    queue<T> q_;
    const unsigned int q_size_ = 5;
};

class Printer {
 public:
    static void print(const string& s, const int& i) {
        static mutex mtx;
        lock_guard<mutex> lock(mtx);
        cout << s << i << endl;
    }

    Printer() = delete;
    ~Printer() = delete;
    Printer(const Printer&) = delete;
    Printer& operator=(const Printer&) = delete;
};

class Producer {
 public:
    explicit Producer(SyncQueue<int> *q) : q_(*q) {}

    void run() {
        for (int i = 0; i < 10; ++i) {
            const int num = i;
            q_.put(num);
            Printer::print("Produced: ", num);
            sleep_for(milliseconds(150));
        }
    }

 private:
    SyncQueue<int>& q_;
};

class Consumer {
 public:
    explicit Consumer(SyncQueue<int> *q) : q_(*q) {}

    void run() {
        for (int i = 0; i < 10; ++i) {
            int num;
            q_.get(&num);
            Printer::print("Consumed: ", num);
            sleep_for(milliseconds(50));
        }
    }

 private:
    SyncQueue<int>& q_;
};

int main() {
    SyncQueue<int> q;

    // Create producers.
    vector<Producer> producers(10, Producer{&q});
    vector<thread> producer_threads;
    for (auto& producer : producers) {
        producer_threads.emplace_back(&Producer::run, &producer);
    }

    // Create consumers.
    vector<Consumer> consumers(10, Consumer{&q});
    vector<thread> consumer_threads;
    for (auto& consumer : consumers) {
        consumer_threads.emplace_back(&Consumer::run, &consumer);
    }

    // Wait until producers and consumers finish.
    for (auto& thrd : producer_threads) {
        thrd.join();
    }
    for (auto& thrd : consumer_threads) {
        thrd.join();
    }

    return 0;
}
