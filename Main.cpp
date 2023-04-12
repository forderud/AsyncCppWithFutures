#include <iostream>

// enable boost::future with .then() continuations
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#include <boost/thread/future.hpp>

using namespace std;

// pull in boost versions of future into root namespace for convenience
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
    future<MyResult> GetVal() {
        MyResult result = { "Jane", 42 };
        return make_ready_future(result);
    }
};


int main() {
    MyAPI obj;

    // first future object
    auto f1 = obj.GetVal();

    // compose futures with non-blocking .then() continuations
    future<std::string> f2 = f1.then([](future<MyResult> f) {
        MyResult result = f.get(); //won't block since we're in a continuation
        std::cout << static_cast<string>(result) << std::endl;
        return result.name + " Doe";
    });

    // block to trigger evaluation of async chain
    string result = f2.get();
    std::cout << result << std::endl;

    return 0;
}
