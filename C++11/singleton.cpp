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
    /**
     * @return: The same instance of this class every time
     */
    static T& instance() {
        // C++ 11 thread-safe local-static-initialization.
        static T *val = new T();

        return *val;
    }

    // Noncopyable.
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

 private:
    Singleton() {}
    ~Singleton() {}
};

int main() {
    Singleton<string>::instance() = "Hello world!";
    cout << Singleton<string>::instance() << endl;
    return 0;
}
