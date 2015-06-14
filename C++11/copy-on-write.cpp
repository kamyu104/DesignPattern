// Copyright (c) 2015 kamyu. All rights reserved.

#include <iostream>
#include <unordered_set>
#include <thread>
#include <memory>
#include <mutex>
#include <chrono>
#include <cassert>

using std::cout;
using std::endl;
using std::unordered_set;
using std::thread;
using std::shared_ptr;
using std::enable_shared_from_this;
using std::mutex;
using std::lock_guard;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

class Request;
typedef shared_ptr<Request> RequestPtr;

class Inventory {
 public:
    Inventory() : requests_(new RequestList) {}

    // Copy on write.
    void add(const RequestPtr& req) {
        lock_guard<mutex> lock{mtx_};
        if (!requests_.unique()) {
            requests_.reset(new RequestList(*requests_));
            cout << "Inventory::add() copy on write" << endl;
        }
        assert(requests_.unique());
        requests_->emplace(req);
    }

    // Copy on write.
    void remove(const RequestPtr& req) {
        lock_guard<mutex> lock(mtx_);
        if (!requests_.unique()) {
            requests_.reset(new RequestList(*requests_));
            cout << "Inventory::remove() copy on write" << endl;
        }
        assert(requests_.unique());
        requests_->erase(req);
    }

    void printAll() const;

 private:
    typedef unordered_set<RequestPtr> RequestList;
    typedef shared_ptr<RequestList> RequestListPtr;

    RequestListPtr getData() const {  // 0.5s in timeline.
        lock_guard<mutex> lock{mtx_};
        return requests_;
    }

    mutable mutex mtx_;  // mutable is due to const method.
    RequestListPtr requests_;
};

class Request : public enable_shared_from_this<Request> {
 public:
    explicit Request(Inventory *inventory) : val_(0), inventory_(*inventory) {}

    ~Request() {
        val_ = -1;
    }

    void process() {
        lock_guard<mutex> lock(mtx_);
        cout << "Request::process()" << endl;  // 0s in timeline.
        inventory_.add(shared_from_this());
        // ...
    }

    void cancel() {
        lock_guard<mutex> lock{mtx_};
        val_ = 1;
        sleep_for(milliseconds(1000));
        cout << "Request::cancel()" << endl;  // 1s in timeline.
        inventory_.remove(shared_from_this());
        // ...
    }

    void print() const {  // 1.5s in timeline.
        lock_guard<mutex> lock{mtx_};
        cout << "Request::print() " << this << ", val_: " << val_ << endl;
        // ...
    }

 private:
    mutable mutex mtx_;  // mutable is due to const method.
    int val_;
    Inventory& inventory_;
};

void Inventory::printAll() const {
    RequestListPtr requests = getData();  // Copy a list.
    cout << "Inventory::printAll()" << endl;
    sleep_for(milliseconds(1000));
    for (auto it = requests->cbegin(); it != requests->cend(); ++it) {
        (*it)->print();  // Every request ptr is shared_ptr.
    }
}

void threadFunc(Inventory *inventory) {
    RequestPtr req{new Request{inventory}};
    req->process();
    req->cancel();
}

int main() {
    Inventory inventory;
    thread thrd(threadFunc, &inventory);

    sleep_for(milliseconds(500));
    inventory.printAll();

    thrd.join();

    return 0;
}
