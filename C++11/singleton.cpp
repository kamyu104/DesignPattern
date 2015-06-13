// Copyright (c) 2015 kamyu. All rights reserved.

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

// Thread-Safe, Lazy Initilization
template<typename T>
class Singleton {
 public:
    static T& instance() {
        // C++ 11 thread-safe local-static-initialization.
        static T *val = new T();

        return *val;
    }

    // Non instanceable.
    Singleton() = delete;
    ~Singleton() = delete;

    // Noncopyable.
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

int main() {
    Singleton<string>::instance() = "Hello world!";
    cout << Singleton<string>::instance() << endl;
    return 0;
}
