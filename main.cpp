#include "quickjspp.hpp"


#include <iostream>


#include <cstdlib>
#include <cstring>
#include <ctime>

int32_t f(int32_t x)
{
    return x * x;
};

#define TYPES bool, int32_t, double, std::shared_ptr<test>, const std::shared_ptr<test>&, std::string, const std::string&

class test
{
public:
    bool b;
    int32_t i;
    double d;
    std::shared_ptr<test> spt;
    std::string s;

    test(int32_t i, TYPES) : i(i)
    { printf("ctor!\n"); }

    test(int32_t i) : i(i)
    { printf("ctor %d!\n", i); }

    test(const test&) = delete;

    ~test()
    { printf("dtor!\n"); }

    int32_t fi(TYPES) { i++; return i; }
    bool fb(TYPES) { i++; return b; }
    double fd(TYPES) { i++; return d; }
    const std::shared_ptr<test>& fspt(TYPES) { i++; return spt; }
    const std::string& fs(TYPES) { i++; return s; }
    void f(TYPES) { i++; }

    static void fstatic(TYPES) {}
};


void f(TYPES) {}


int main(int argc, char ** argv)
{
    JSRuntime * rt;
    JSContext * ctx;
    using namespace qjs;

    Runtime runtime;
    rt = runtime.rt;

    Context context(runtime);
    ctx = context.ctx;

    {
        auto obj = context.newObject();
        obj["vi"] = 54;
        obj["vb"] = true;
        obj["vd"] = 56.0;
        obj["vs"] = std::string{"test string"};
        obj.add("lambda", [](int x) -> int { return x * x; });

        obj.add<&test::fi>("fi");
        obj.add<&test::fb>("fb");
        obj.add<&test::fd>("fd");
        obj.add<&test::fs>("fs");
        obj.add<&test::fspt>("fspt");
        obj.add<&test::f>("f");
        obj.add<&test::fstatic>("fstatic");

        obj.add<&test::i>("i");
        obj.add<&test::b>("b");
        obj.add<&test::d>("d");
        obj.add<&test::s>("s");
        obj.add<&test::spt>("spt");

        context.registerClass<test>("test_class", std::move(obj));
    }


    context.addModule("test")
            //.add("obj", obj)
            .add("Test", detail::ctor_wrapper<test, int32_t, TYPES>{"Test"})
    .add("TestSimple", detail::ctor_wrapper<test, int32_t>{"TestSimple"});




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

    {
        auto xxx = context.eval("var t = new test.TestSimple(12);"
                                "var q = new test.Test(13, t.vb, t.vi, t.vd, t, t, t.vs, t.vs);"
                                "q.b = true;"
                                "q.d = 456.789;"
                                "q.s = \"STRING\";"
                                "q.spt = t;"
                                "console.log(q.b === q.fb(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));"
                                "console.log(q.d === q.fd(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));"
                                "console.log(q.s === q.fs(t.vb, t.vi, t.vd, t, t, \"test\", t.vs));"
                                "console.log(q.spt === q.fspt(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));" // false
                                "q.fi(t.vb, t.vi, t.vd, t, t, t.vs, \"test\")");
        assert(xxx.cast<int>() == 18);
        auto yyy = context.eval("q.fi.bind(t)(t.vb, t.vi, t.vd, t, t, t.vs, \"test\")");
        assert(yyy.cast<int>() == 13);
    }
    try
    {
        auto f = context.eval("q.fi.bind(q)").cast<std::function<int32_t(TYPES)>>();
        int xxx = f(false, 1, 0., context.eval("q").cast<std::shared_ptr<test>>(),
                    context.eval("t").cast<std::shared_ptr<test>>(), "test string", std::string{"test"});
        assert(xxx == 19);
    }
    catch(detail::exception)
    {
        js_std_dump_error(ctx);
    }

    if(filename)
    context.evalFile(filename, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_SHEBANG);

    JSMemoryUsage mem;
    JS_ComputeMemoryUsage(rt, &mem);
    JS_DumpMemoryUsage(stderr, &mem, rt);

    js_std_loop(ctx);

    js_std_free_handlers(rt);

    return 0;

}
