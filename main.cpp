#include "quickjspp.hpp"


#include <iostream>


#include <cstdlib>
#include <cstring>
#include <ctime>


int32_t f(int32_t x)
{
    return x * x;
};

class test
{
    int x;
public:
    inline static JSClassID QJSClassId;


    test(int32_t x) : x{x}
    { printf("ctor %d!\n", x); }

    test(const test&) = delete;

    ~test()
    { printf("dtor!\n"); }

    void f()
    { printf("f(%p): x=%d\n", this, x); }

    static void g()
    { printf("g()\n"); }
};


int32_t f2(int32_t x, std::shared_ptr<test> ptr)
{
    std::cout << ptr.get() << std::endl;
    return x * x;
};


int main(int argc, char ** argv)
{
    JSRuntime * rt;
    JSContext * ctx;
    using namespace qjs;

    Runtime runtime;
    rt = runtime.rt;

    Context context(runtime);
    ctx = context.ctx;

    auto obj = context.newObject();
    obj["test"] = 54;
    obj["test2"] = 56;
    obj["g"] = detail::fwrapper<&test::g>{"g"};
    obj["fun2"] = detail::fwrapper<&f2>{"f2"};
    obj["f"] = detail::fwrapper<&test::f>{"f"};


    context.registerClass<test>("test_class", std::move(obj));


    context.addModule("test")
            .add("sqr", detail::fwrapper<&f>{"sqr"})
            .add("sqr2", detail::fwrapper<&f>{})
            .add("p", detail::fwrapper<&f2>{"p"})
            .add("obj", obj)
            .add("Test", detail::ctor_wrapper<test, int32_t>{"Test"});



    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(rt, NULL, js_module_loader, NULL);
    js_std_add_helpers(ctx, argc, argv);

    /* system modules */
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    /* make 'std' and 'os' visible to non module code */
    const char * str = "import * as std from 'std';\n"
                       "import * as os from 'os';\n"
                       "import * as test from 'test';\n"
                       "std.global.std = std;\n"
                       "std.global.test = test;\n"
                       "std.global.os = os;\n";
    context.eval(str, "<input>", JS_EVAL_TYPE_MODULE);
    const char * filename = argv[1];

    context.evalFile(filename, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_SHEBANG);

    js_std_loop(ctx);

    js_std_free_handlers(rt);

    return 0;

}
