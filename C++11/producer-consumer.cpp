#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using std::cout;
using std::endl;
using std::vector;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::rand;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

class Buffer {
public:
    Buffer() {}
    void add(int num) {
        while (true) {
            unique_lock<mutex> locker(mu);
            cond.wait(locker, [this]() { return buffer_.size() != size_; });
            buffer_.emplace_back(num);
            cond.notify_all();
            return;
        }
    }
    int remove() {
        while (true) {
            unique_lock<mutex> locker(mu);
            cond.wait(locker, [this]() { return buffer_.size() != 0; });
            int back = buffer_.back();
            buffer_.pop_back(); 
            cond.notify_all();
            return back;
        }
    }
private:
    mutex mu;
    condition_variable cond;
    vector<int> buffer_;
    const unsigned int size_ = 10;
};

class Producer {
public:
    Producer(Buffer& buffer, mutex& cout_mu) :
             buffer_(buffer), cout_mu_(cout_mu) {}
    
    void run() {
        while (true) {
            int num = rand() % 100;
            buffer_.add(num);
            unique_lock<mutex> locker(cout_mu_);
            cout << "Produced: " << num << endl;
            sleep_for(milliseconds(50));
        }
    }

private:
    Buffer& buffer_;
    mutex& cout_mu_;
};

class Consumer {
public:
    Consumer(Buffer& buffer, mutex& cout_mu) : buffer_(buffer), cout_mu_(cout_mu) {}
    void run() {
        while (true) {
            int num = buffer_.remove();
            unique_lock<mutex> locker(cout_mu_);
            cout << "Consumed: " << num << endl;
            sleep_for(milliseconds(50));
        }
    }

private:
    Buffer& buffer_;
    mutex& cout_mu_;
};

int main() {
    Buffer b;
    mutex cout_mu;
    Producer p(b, cout_mu);
    Consumer c(b, cout_mu);

    thread producer_thread(&Producer::run, &p);
    thread consumer_thread(&Consumer::run, &c);

    producer_thread.join();
    consumer_thread.join();
    
    return 0;
}
