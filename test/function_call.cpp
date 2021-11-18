#include "quickjs/quickjs.h"
#include "quickjspp.hpp"
#include <cstdio>
#include <string_view>

void qjs_assert(bool cond) {
    if (!cond) {
        printf("FAIL\n");
        throw qjs::exception{};
    }
}

void test_not_enough_arguments(qjs::Context & ctx) {
    std::string msg;

    ctx.global().add("test_fcn", [](int a, int b, int c) {
        printf("%d %d %d\n", a, b, c);
    });

    ctx.eval(
        "try {"
        "  test_fcn(1);"
        "  assert(false);"
        "} catch (err) {"
        "  assert("
        "    (err instanceof TypeError) && "
        "    err.message === 'Expected type 3 arguments but only 1 were provided'"
        "  );"
        "}"
    );
}

void test_call_with_rest_arguments(qjs::Context & ctx) {
    ctx.global().add("test_fcn", [](int a, qjs::rest<int> args) {
        qjs_assert(a == 1);
        qjs_assert(args.size() == 3);
        qjs_assert(args[0] == 2);
        qjs_assert(args[1] == 3);
        qjs_assert(args[2] == 4);
    });

    ctx.eval("test_fcn(1,2,3,4);");
}

int main()
{
    qjs::Runtime runtime;
    qjs::Context context(runtime);

    context.global().add("assert", &qjs_assert);

    test_not_enough_arguments(context);
    test_call_with_rest_arguments(context);

    return 0;
}