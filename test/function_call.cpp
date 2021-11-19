#include "quickjspp.hpp"
#include <cstdio>

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);

    std::string msg;

    context.global().add("f_with_3_args", [](int a, int b, int c) {
        printf("%d %d %d\n", a, b, c);
    });

    try
    {
        context.eval("f_with_3_args(1);");
    }
    catch(qjs::exception)
    {
        msg = (std::string)context.getException();
    }

    assert(msg == "TypeError: Expected type 3 arguments but only 1 were provided");

    return 0;
}