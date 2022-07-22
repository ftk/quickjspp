#include "quickjspp.hpp"
#include "quickjs/quickjs-libc.h"
#include <iostream>


#define TYPES bool, int32_t, double, qjs::shared_ptr<test>, const qjs::shared_ptr<test>&, std::string, const std::string&

class base_test : public qjs::enable_shared_from_this<base_test>
{
public:
    std::vector<std::vector<int>> base_field;

    int base_method(int x)
    {
        std::swap(x, base_field[0][0]);
        return x;
    }

    // TODO: make it work in inherited classes
    base_test * self() { return this; }
    bool check_self(base_test * self) { return(this == self); }
};

class test : public base_test
{
public:
    bool b;
    mutable int32_t i;
    double d = 7.;
    qjs::shared_ptr<test> spt;
    std::string s;

    test(int32_t i, TYPES) : i(i)
    { printf("ctor!\n"); }

    test(int32_t i) : i(i)
    { printf("ctor %d!\n", i); }

    test(const test&) = delete;

    ~test()
    { printf("dtor!\n"); }

    int32_t fi(TYPES) const { i++; return i; }
    bool fb(TYPES) { i++; return b; }
    double fd(TYPES) const { i++; return d; }
    const qjs::shared_ptr<test>& fspt(TYPES) { i++; return spt; }
    const std::string& fs(TYPES) { i++; return s; }
    void f(TYPES) { i++; }

    double get_d()  { i++; return d; }
    double set_d(double new_d) { i++; d = new_d; return d; }


    static void fstatic(TYPES) {}
    static bool bstatic;
};
bool test::bstatic = true;

void f(TYPES) {}

void qjs_glue(qjs::Context::Module& m) {
    m.function<&::f>("f"); // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
    m.class_<::base_test>("base_test")
            // implicit: .constructor<::base_test const &>()
            .constructor<>()
            .fun<&::base_test::base_method>("base_method") // (double)
            .fun<&::base_test::base_field>("base_field") // double
            ;

    m.class_<::test>("test")
            .base<::base_test>()
            .constructor<::int32_t, bool, ::int32_t, double, ::qjs::shared_ptr<test>, ::qjs::shared_ptr<test> const &, ::std::string, ::std::string const &>("Test")
            .static_fun<&::test::fstatic>("fstatic") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test>const &, ::std::string, ::std::string const &)
            .static_fun<&::test::bstatic>("bstatic") // bool
            .constructor<::int32_t>("TestSimple")
            .fun<&::test::fi>("fi") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
            .fun<&::test::fb>("fb") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
            .fun<&::test::fd>("fd") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
            .fun<&::test::fspt>("fspt") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const&, ::std::string, ::std::string const &)
            .fun<&::test::fs>("fs") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
            .fun<&::test::f>("f") // (bool, ::int32_t, double, ::std::shared_ptr<test>, ::std::shared_ptr<test> const &, ::std::string, ::std::string const &)
            .fun<&::test::b>("b") // bool
            .fun<&::test::i>("i") // ::int32_t
            .fun<&::test::d>("d") // double
            .fun<&::test::spt>("spt") // ::std::shared_ptr<test>
            .fun<&::test::s>("s") // ::std::string
            .fun<&::test::self>("self")
            .fun<&::test::check_self>("check_self")
            .property<&test::get_d, &test::set_d>("property_rw")
            .property<&test::get_d>("property_ro")
            .mark<&::test::spt>()
            ;
} // qjs_glue

int main()
{
    JSRuntime * rt;
    JSContext * ctx;
    using namespace qjs;

    Runtime runtime;
    rt = runtime.rt;

    Context context(runtime);
    ctx = context.ctx;

    try
    {
        js_std_init_handlers(rt);
        js_std_add_helpers(ctx, 0, nullptr);

        qjs_glue(context.addModule("test"));

        /* system modules */
        js_init_module_std(ctx, "std");
        js_init_module_os(ctx, "os");

        /* make 'std' and 'os' visible to non module code */
        const char * str = "import * as std from 'std';\n"
                           "import * as os from 'os';\n"
                           "import * as test from 'test';\n"
                           "globalThis.std = std;\n"
                           "globalThis.test = test;\n"
                           "globalThis.os = os;\n"
                           "globalThis.assert = function(b, str)\n"
                           "{\n"
                           "    if (b) {\n"
                           "        std.printf('OK' + str + '\\n'); return;\n"
                           "    } else {\n"
                           "        throw Error(\"assertion failed: \" + str);\n"
                           "    }\n"
                           "}";
        context.eval(str, "<input>", JS_EVAL_TYPE_MODULE);



        auto xxx = context.eval("\"use strict\";"
                                "var b = new test.base_test();"
                                "b.base_field = [[5],[1,2,3,4],[6]];"
                                "assert(b.base_field[1][3] === 4);"
                                "assert(b.base_method(123) === 5);"
                                "assert(b.base_field[0][0] === 123);"

                                "var t = new test.TestSimple(12);"
                                "var q = new test.Test(13, t.vb, t.vi, t.vd, t, t, t.vs, t.vs);"
                                "q.b = true;"
                                "q.d = 456.789;"
                                "q.s = \"STRING\";"
                                "q.spt = t;"
                                "t.spt = q;"
                                "q.base_field = [[5],[1,2,3,4],[6]];"
                                "assert(q.b === q.fb(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));"
                                "assert(q.d === q.fd(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));"
                                "assert(q.s === q.fs(t.vb, t.vi, t.vd, t, t, \"test\", t.vs));"
                                "assert(5 === q.base_method(7));"
                                "assert(7 === q.base_field[0][0]);"
                                "assert(q.spt === q.fspt(t.vb, t.vi, t.vd, t, t, t.vs, \"test\"));" // same objects
                                "assert(q.check_self(q));"
                                "assert(!t.check_self(q));"
                                "q.fi(t.vb, t.vi, t.vd, t, t, t.vs, \"test\")");
        assert((int)xxx == 18);
        auto yyy = context.eval("q.fi.bind(t)(t.vb, t.vi, t.vd, t, t, t.vs, \"test\")");
        assert((int)yyy == 13);

        auto f = context.eval("q.fi.bind(q)").as<std::function<int32_t(TYPES)>>();
        int zzz = f(false, 1, 0., context.eval("q").as<qjs::shared_ptr<test>>(),
                    context.eval("t").as<qjs::shared_ptr<test>>(), "test string", std::string{"test"});
        assert(zzz == 19);

        zzz = (int)context.eval("q.property_rw = q.property_ro - q.property_rw + 1;"
                               "assert(q.property_ro === 1);"
                               "q.i"
                               );
        assert(zzz == 23);

        auto qbase = context.eval("q").as<qjs::shared_ptr<base_test>>();
        assert(qbase->base_field[0][0] == 7);

        assert((bool)context.eval("test.Test.bstatic === true"));
    }
    catch(exception)
    {
        auto exc = context.getException();
        std::cerr << (exc.isError() ? "Error: " : "Throw: ") << (std::string)exc << std::endl;
        if((bool)exc["stack"])
            std::cerr << (std::string)exc["stack"] << std::endl;

        js_std_free_handlers(rt);
        return 1;
    }

    js_std_loop(ctx);

    js_std_free_handlers(rt);

    return 0;

}
