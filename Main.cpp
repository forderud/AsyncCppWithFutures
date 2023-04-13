#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
inline void SetThreadName(const wchar_t name[]) {
    SetThreadDescription(GetCurrentThread(), name);
}
#else
inline void SetThreadName(wchar_t name[]) {
}
#endif

// enable boost::future with .then() continuations
#define BOOST_THREAD_PROVIDES_VARIADIC_THREAD     // required for launch::deferred
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION // requried for .then()
#include <boost/thread/future.hpp>


struct MyResult {
    std::string name;
    int age = 0;

    operator std::string () const {
        return "name=" + name + ", age=" + std::to_string(age);
    }
};

class MyAPI {
public:
    MyAPI() = default;

    /** Async method. */
    boost::future<MyResult> ComputeResult(bool threaded) {
        // "deferred" will run everything in main thread with lazy evaluation
        boost::launch policy = threaded ? boost::launch::async : boost::launch::deferred;

        return boost::async(policy, [this] {
            //SetThreadName(L"boost::async thread");

            // these two calls are slow
            std::string name = DetermineName();
            int age = DetermineAge();
            return MyResult{ name, age };
        });
    }

    MyResult NextCalculation(boost::future<MyResult> f) {
        MyResult result = f.get();
        std::cout << static_cast<std::string>(result) << std::endl;
        result.name += " " + DetermineName(); // add last-name suffix
        return result;
    }

private:
    std::string DetermineName() {
        // slow algorithm that takes time to complete
        std::cout << "Calling DetermineName..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        return "Jane";
    }
    int DetermineAge() {
        // slow algorithm that takes time to complete
        std::cout << "Calling DetermineAge..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        return 42;
    }
};


int main() {
    MyAPI obj;

    // first future object
    auto f1 = obj.ComputeResult(false);

    // compose futures with non-blocking .then() continuations
    boost::future<MyResult> f2 = f1.then([&obj](boost::future<MyResult> f) {
        //SetThreadName(L"continuation #1 thread");
        return obj.NextCalculation(std::move(f));
    });

    // compose futures with non-blocking .then() continuations
    boost::future<MyResult> f3 = f2.then([&obj](boost::future<MyResult> f) {
        //SetThreadName(L"continuation #2 thread");
        return obj.NextCalculation(std::move(f));
    });

    std::cout << "Triggering evaluation of async chain..." << std::endl;
    MyResult result = f3.get();
    std::cout << "Result=" << result.name << std::endl;

    return 0;
}
