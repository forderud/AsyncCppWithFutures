#include <iostream>
#include <thread>

// enable boost::future with .then() continuations
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>

using namespace std;

// pull in boost versions of future into root namespace for convenience
using boost::async;
using boost::future;
using boost::make_ready_future;


struct MyResult {
    string name;
    int age = 0;

    operator string () const {
        return "name=" + name + ", age=" + std::to_string(age);
    }
};

class MyAPI {
public:
    MyAPI() = default;

    /** Async method. */
    future<MyResult> ComputeResult() {
        return async([this] {
            // these two calls are slow
            string name = DetermineName();
            int age = DetermineAge();
            return MyResult{ DetermineName(), DetermineAge() };
        });
    }

private:
    string DetermineName() {
        // slow algorithm that takes time to complete
        this_thread::sleep_for(chrono::seconds(1));

        return "Jane";
    }
    int DetermineAge() {
        // slow algorithm that takes time to complete
        this_thread::sleep_for(chrono::seconds(1));

        return 42;
    }
};


int main() {
    MyAPI obj;

    // first future object
    auto f1 = obj.ComputeResult();

    // compose futures with non-blocking .then() continuations
    future<std::string> f2 = f1.then([](future<MyResult> f) {
        MyResult result = f.get(); //won't block since we're in a continuation
        cout << static_cast<string>(result) << endl;
        // return name with last-name suffix
        return result.name + " Doe";
    });

    cout << "Triggering evaluation of async chain..." << endl;
    string result = f2.get();
    std::cout << result << std::endl;

    return 0;
}
