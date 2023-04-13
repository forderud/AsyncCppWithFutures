#include <iostream>
#include <thread>

// enable boost::future with .then() continuations
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
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
    boost::future<MyResult> ComputeResult() {
        return boost::async([this] {
            // these two calls are slow
            std::string name = DetermineName();
            int age = DetermineAge();
            return MyResult{ name, age };
        });
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
    auto f1 = obj.ComputeResult();

    // compose futures with non-blocking .then() continuations
    boost::future<std::string> f2 = f1.then([](boost::future<MyResult> f) {
        MyResult result = f.get(); //won't block since we're in a continuation
        std::cout << static_cast<std::string>(result) << std::endl;
        // return name with last-name suffix
        return result.name + " Doe";
    });

    std::cout << "Triggering evaluation of async chain..." << std::endl;
    std::string result = f2.get();
    std::cout << "Result=" << result << std::endl;

    return 0;
}
