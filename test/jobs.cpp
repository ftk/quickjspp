#include "quickjspp.hpp"
#include <iostream>
#include <stdexcept>

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);
    
    context.global().add("nextTick", [&context](std::function<void()> f) {
        context.enqueueJob(std::move(f));
    });

    bool called = false;
    context.global().add("testFcn", [&called]() {
        called = true;
    });

    qjs::Value caller = context.eval(R"xxx(
        nextTick(testFcn);
    )xxx");

    assert(!called && "`testFcn` should not be called yet");

    while (runtime.isJobPending()) {
        runtime.executePendingJob();
    }

    assert(called && "`testFcn` should have been called");

    return 0;
}
