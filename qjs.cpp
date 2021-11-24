#include "quickjspp.hpp"
#include "quickjs/quickjs-libc.h"

#include <iostream>

int main(int argc, char ** argv)
{
    using namespace qjs;

    Runtime runtime;
    Context context(runtime);

    auto rt = runtime.rt;
    auto ctx = context.ctx;

    js_std_init_handlers(rt);
    js_std_add_helpers(ctx, argc - 1, argv + 1);

    /* system modules */
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    /* make 'std' and 'os' visible to non module code */
    context.eval(R"xxx(
        import * as std from 'std';
        import * as os from 'os';
        globalThis.std = std;
        globalThis.os = os;
    )xxx", "<input>", JS_EVAL_TYPE_MODULE);

    try
    {
        if(argv[1])
            context.evalFile(argv[1], JS_EVAL_TYPE_MODULE);
    }
    catch(exception & e)
    {
        auto exc = e.get();
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
